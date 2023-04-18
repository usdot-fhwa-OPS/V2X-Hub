/*
 * @file PluginClient.h
 *
 *  Created on: Feb 25, 2016
 *      Author: ivp
 */

#ifndef SRC_PLUGINCLIENT_H_
#define SRC_PLUGINCLIENT_H_

#include <atomic>
#include <map>
#include <mutex>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <string>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include <boost/algorithm/string.hpp>
#pragma GCC diagnostic pop
#include <tmx/apimessages/TmxEventLog.hpp>
#include <tmx/messages/routeable_message.hpp>
#include <tmx/messages/TmxJ2735.hpp>
#include <tmx/IvpPlugin.h>

#include "Clock.h"
#include "PluginExec.h"
#include "PluginLog.h"
#include "PluginException.h"
#include "PluginKeepAlive.h"
#include "database/DbConnectionPool.h"
#include "database/SystemContext.h"

// Redefine PLOG for plugins
#ifdef PLOG
#undef PLOG
#endif

#define PLOG(level) PLUGIN_LOG(level, _name)

#define LOG_LEVEL_CFG "TMXLogLevel"

#define SYSTEM_PARAMETER_ADD \
	"INSERT INTO `pluginConfigurationParameter` (`pluginId`, `key`, `value`, `defaultValue`, `description`) \
	 VALUES ( ?, ?, ?, ?, ? ) \
	 ON DUPLICATE KEY UPDATE value = VALUES(value), defaultValue = VALUES(defaultValue), description = VALUES(description)"

namespace tmx {
namespace utils {


// C++ wrapper for an ivpapi plugin.
// If the ivpapi is rewritten in C++, this class will be moved to the API, and it will
// no longer be a wrapper, but a first class citizen.
class PluginClient: public Runnable {
	friend class PluginExtender;

public:
	PluginClient(std::string name);
	virtual ~PluginClient();

	virtual bool ProcessOptions(const boost::program_options::variables_map &);

	/// Static map used to track which PluginClient instance goes with which IvpPlugin* created.
	/// This allows the static callback functions below to call the instance virtual callback functions.
	static std::map<IvpPlugin*, PluginClient*> _instanceMap;

	static void StaticOnConfigChanged(IvpPlugin *plugin, const char *key, const char *value);
	static void StaticOnError(IvpPlugin *plugin, IvpError err);
	static void StaticOnMessageReceived(IvpPlugin *plugin, IvpMessage *msg);
	static void StaticOnStateChange(IvpPlugin *plugin, IvpPluginState state);

	static PluginClient *FindPlugin(std::string name);
	static void StaticOnConfigChanged(PluginClient *plugin, const char *key, const char *value);
	static void StaticOnError(PluginClient *plugin, IvpError err);
	static void StaticOnMessageReceived(PluginClient *plugin, IvpMessage *msg);
	static void StaticOnStateChange(PluginClient *plugin, IvpPluginState state);

	void AddMessageFilter(const char *type, const char *subtype, IvpMsgFlags flags = IvpMsgFlags_None);
	void SubscribeToMessages();

	/// @return The name given to the plugin
	inline std::string GetName() { return _name; }

	template <typename MsgType, class HandlerType>
	void AddMessageFilter(HandlerType *plugin, void (HandlerType::*handler)(MsgType &, tmx::routeable_message &) = 0)
	{
		typedef MsgType msg_type;

		AddMessageFilter(MsgType::MessageType, MsgType::MessageSubType);

		std::string typeName(MsgType::MessageType);
		std::string subTypeName(MsgType::MessageSubType);

		static msg_type msgType;

		PLOG(logINFO) << "Registering message type " <<
				battelle::attributes::type_id_name<msg_type>() <<
				" for receiving message Type=" << typeName <<
				", SubType=" << subTypeName;

		if (handler)
			this->register_handler(plugin, typeName, subTypeName, handler);
		else
			this->register_handler(plugin, typeName, subTypeName, &HandlerType::handleMessage);
	}

	/// Broadcast a message to TMX core and optionally route it out the DSRC radio.
	/// @param message The message to send.  The message must be of type tmx::message
	/// and must have static char* members called MessageType and MessageSubType.
	/// @param source The source of the message, defaults to empty string
	/// @param sourceId The source ID of the message, defaults to 0
	/// @param flags Bit-or'ed flags.  Specify IvpMsgFlags_RouteDSRC to send the message out the DSRC radio.
	template <typename MsgType>
	void BroadcastMessage(MsgType& message, std::string source = "", unsigned int sourceId = 0, IvpMsgFlags flags = IvpMsgFlags_None)
	{
		tmx::routeable_message routeableMsg;
		routeableMsg.initialize(message, source, sourceId, flags);
		BroadcastMessage(routeableMsg);
	}

	/// Broadcast a given message to TMX core.  The message is routed verbatim
	void BroadcastMessage(const IvpMessage *ivpMsg)
	{
		if (ivpMsg)
			ivp_broadcastMessage(_plugin, const_cast<IvpMessage *>(ivpMsg));

		// Make sure to update the keep alive that no message is needed since one was just sent
		// But, neither the keep alive message itself nor any API messages count
		if (!IsMessageOfType<tmx::messages::KeepAliveMessage>(ivpMsg) &&
			!IsMessageOfType<tmx::messages::TmxEventLogMessage>(ivpMsg))
			this->_keepAlive->touch();
	}

	/// Broadcast a given message to TMX core.  The message is routed verbatim.
	/// @param routeableMsg The TMX routeable message to send
	void BroadcastMessage(const tmx::routeable_message &routeableMsg)
	{
		PLOG(logDEBUG2) << "Sending: " << routeableMsg;
		BroadcastMessage(routeableMsg.get_message());
	}

	/// Broadcast a given message to TMX core.  The message is routed verbatim.
	/// Note that this function is needed to avoid the template being used.
	/// @param routeableMsg The TMX routeable message to send
	void BroadcastMessage(tmx::routeable_message &routeableMsg)
	{
		PLOG(logDEBUG2) << "Sending: " << routeableMsg;
		const IvpMessage *copy = routeableMsg.get_message();
		BroadcastMessage(copy);
		ivpMsg_destroy(const_cast<IvpMessage *>(copy));
	}

	/// Function for determining if a IvpMessage received from the core is a specific type.
	/// Validates the type category and subtype category.
	/// @param message The message to compare against the message type specified.
	/// @return true if the message is the message type specified.
	template <typename MsgType>
	bool IsMessageOfType(const IvpMessage *message)
	{
		return (message &&
				message->type && strcmp(message->type, MsgType::MessageType) == 0 &&
				message->subtype && strcmp(message->subtype, MsgType::MessageSubType) == 0);
	}

	/// Function for determining if a routeable_message received from the core is a specific type.
	/// Validates the type category and subtype category.
	/// @param message The routeable message to compare against the message type specified.
	/// @return true if the message is the message type specified.
	template <typename MsgType>
	bool IsMessageOfType(tmx::routeable_message &message)
	{
		return (message.get_type().compare(MsgType::MessageType) == 0 &&
				message.get_subtype().compare(MsgType::MessageSubType) == 0);
	}

	template <typename MsgType>
	bool IsJ2735Message(MsgType &message)
	{
		return message.get_type().compare(tmx::messages::api::MSGSUBTYPE_J2735_STRING) == 0;
	}

	/// Main method of the plugin that should not return until the plugin exits.
	virtual int Main();

	/// Handle an exception thrown in the plugin.  The requirement is to log the message
	/// in the event log.  By default, the program also terminates
	/// @param ex The exception to handle
	/// @param abort Terminate the process.  Default is true.
	void HandleException(std::exception &ex, bool abort = true);

	// Get a configuration value for this plugin.
	// @param key The name of the configuration value.
	// @param value The returned value.
	// @param lock If non-NULL, this mutex is locked while value is set.
	// @return true on success; false if the value could not be retrieved.
	template <typename T>
	bool GetConfigValue(const std::string &key, T &value, std::mutex *lock = NULL)
	{
		bool success = false;
		char *text = ivp_getCopyOfConfigurationValue(_plugin, key.c_str());

		if (lock != NULL)
			lock->lock();

		// Maybe this is a system-wide parameter?
		if (text == NULL && _sysConfig != NULL)
		{
			pthread_mutex_lock(&_plugin->lock);
			text = ivpConfig_getCopyOfValueFromCollection(_sysConfig, key.c_str());
			pthread_mutex_unlock(&_plugin->lock);
		}

		if (text != NULL)
		{
			try
			{
				value = boost::lexical_cast<T>(text);
				success = true;
			}
			catch (boost::bad_lexical_cast const &ex)
			{
				PLOG(logERROR) << "Unable to convert config value from \"" << text << "\": " << ex.what();
				success = false;
			}
			free(text);
		}

		if (lock != NULL)
			lock->unlock();

		return success;
	}

	// Get a configuration value for this plugin and store the result in an atomic container.
	// @param key The name of the configuration value.
	// @param value The returned value stored in an atomic type.
	// @return true on success; false if the value could not be retrieved.
	template <typename T>
	bool GetConfigValue(const std::string &key, std::atomic<T> &value)
	{
		T tempValue;
		bool success = GetConfigValue<T>(key, tempValue);
		if (success)
			value = tempValue;
		return success;
	}

	// TODO: Determine if defaultValue parameter should be removed.
	// If so then behavior would be that if key is not in the database, then value is used for DB default value.
	template <typename T>
	bool SetSystemConfigValue(const std::string &key, const T value, const T defaultValue, bool notify = true)
	{
		bool success = true;

		std::string valString;
		std::string defString;

		try
		{
			valString = boost::lexical_cast<std::string>(value);
			defString = boost::lexical_cast<std::string>(defaultValue);
		}
		catch (boost::bad_lexical_cast const &ex)
		{
			PLOG(logERROR) << "Unable to convert type " << battelle::attributes::type_name(value) <<
					" to string for parameter " << key;
			success = false;
		}

		_sysConfig = ivpConfig_addItemToCollection(_sysConfig, key.c_str(), valString.c_str(), defString.c_str());

		if (notify)
		{
			IvpConfigCollection *collection = NULL;
			collection = ivpConfig_addItemToCollection(collection, key.c_str(), valString.c_str(), defString.c_str());

			tmx::routeable_message msg(ivpConfig_createMsg(collection));
			ivpConfig_destroyCollection(collection);

			msg.set_source("_global");
			this->BroadcastMessage(msg);
		}
		return success;
	}

	/**
	 * Set a status item.
	 * The status is only set if the string representation of the value is not the same as the last
	 * time this method was called.
	 * @param key The key of the status item.
	 * @param value The value of the status item.
	 * @param prependTime When true, the current time is prepended to the value of the status item.
	 * @param precision The precision used when converting floating point numbers to a string.
	 * @return true if the status string is new and it was set; false otherwise.
	 */
	template<typename T>
	bool SetStatus(const char *key, T value, bool prependTime = false, std::streamsize precision = 2)
	{
		bool mute = false;
		GetConfigValue("MuteStatus", mute);

		if (mute)
			return false;

		std::ostringstream ss;

		if (prependTime)
			ss << "[" << Clock::ToLocalPreciseTimeString(std::chrono::system_clock().now()) << "] ";

		ss.setf(std::ios::boolalpha);
		ss.precision(precision);
		ss << std::fixed << value;

		bool isNewValue = false;

		if (_statusMap.count(key) == 0)
		{
			_statusMap[key] = ss.str();
			isNewValue = true;
		}
		else
		{
			if (ss.str().compare(_statusMap[key]) != 0)
			{
				_statusMap[key] = ss.str();
				isNewValue = true;
			}
		}

		if (isNewValue)
		{
			PLOG(logDEBUG1) << "New Status. " << key << ": " << ss.str();
			ivp_setStatusItem(_plugin, key, ss.str().c_str());
		}

		return isNewValue;
	}

	void RemoveStatus(const char *key);

	static std::string NewGuid();

	tmx::utils::DbConnection get_DbConnection() { return _dbConnPool.Connection(); }

	bool IsPluginState(IvpPluginState state);

protected:
	virtual void OnConfigChanged(const char *key, const char *value);
	virtual void OnError(IvpError err);
	virtual void OnMessageReceived(IvpMessage *msg);
	virtual void OnStateChange(IvpPluginState state);

	// The default message handler. This logs an error for an unhandled message, but
	// also serves as the prototype function declaration for message handlers.
	// @param The specific message payload
	// @param The source TMX routeable message from which the payload was taken
	void handleMessage(tmx::message &msg, tmx::routeable_message &src);

	tmx::utils::DbConnectionPool _dbConnPool;
	bool _isStartTimeStatusSet = false;

	std::string _name;
	std::string _logPrefix;
	IvpPlugin* _plugin;

	// Static system context - to log msg latency and system load
	static tmx::utils::SystemContext _sysContext;

	inline const std::chrono::system_clock::time_point & getStartTime() const {
		return _startTime;
	}

private:
	void SetStartTimeStatus();

	IvpMsgFilter* _msgFilter;
	IvpConfigCollection *_sysConfig;
	PluginKeepAlive *_keepAlive;
	std::chrono::system_clock::time_point _startTime;

	// Map a plugin status key to the last value set for that key.
	std::map<std::string, std::string> _statusMap;

	// Code for message handler registration and invoking
	struct handler_allocator {
		virtual ~handler_allocator() {}

		virtual std::string get_messageType() = 0;
		virtual void invokeHandler(tmx::routeable_message &routeableMsg) = 0;
	};

	template <typename MsgType, class PluginType, class HandlerType>
	struct handler_allocator_impl: public handler_allocator {
		typedef MsgType type;

		handler_allocator_impl(PluginType *plugin,
				void (HandlerType::*handler)(MsgType &, tmx::routeable_message &)):
					instance(plugin), fn(handler) {}

		std::string get_messageType()
		{
			return battelle::attributes::type_id_name<MsgType>();
		}

		void invokeHandler(tmx::routeable_message &routeableMsg)
		{
			MsgType msg = routeableMsg.template get_payload<MsgType>();
			if (fn)
				(instance->*fn)(msg, routeableMsg);
			else
				throw PluginException("Missing handler for " + get_messageType());
		}
	private:
		PluginType *instance;
		void (HandlerType::*fn)(MsgType &, tmx::routeable_message &);
	};

	// Map for message handlers
	std::map<std::pair<std::string, std::string>, handler_allocator *> _msgHandlers;

	template <typename MsgType, class PluginType, class HandlerType>
	void register_handler(PluginType *plugin, std::string messageType, std::string messageSubType,
			void (HandlerType::*handler)(MsgType &, tmx::routeable_message &))
	{
		static handler_allocator_impl<MsgType, PluginType, HandlerType> *allocator =
				new handler_allocator_impl<MsgType, PluginType, HandlerType>(plugin, handler);

		_msgHandlers[std::pair<std::string, std::string>(messageType, messageSubType)] = allocator;
	}

	bool invoke_handler(std::string, std::string, tmx::routeable_message &);
};

template<>
inline bool PluginClient::GetConfigValue(const std::string &key, boost::property_tree::ptree &value, std::mutex *lock)
{
	bool success = false;
	std::string string_val;
	success = GetConfigValue<std::string>(key, string_val, lock);
	if (!success) return success;

	std::stringstream ss;
	ss << string_val;

	try
	{
		boost::property_tree::read_json(ss, value);
	}
	catch (boost::property_tree::json_parser_error const &ex)
	{
		PLOG(logERROR) << "Unable to read JSON config value from \"" << string_val <<
				"\" into property tree: " << ex.what();
		success = false;
	}

	return success;
}

template<>
inline bool PluginClient::GetConfigValue<bool>(const std::string &key, bool &value, std::mutex *lock)
{
	std::string strValue;
	bool success = GetConfigValue<std::string>(key, strValue, lock);
	if (!success)
		return false;

	if (boost::iequals(strValue, "1")
		|| boost::iequals(strValue, "true")
		|| boost::iequals(strValue, "t")
		|| boost::iequals(strValue, "on"))
	{
		value = true;
	}
	else
		value = false;

	return true;
}

template<>
inline void PluginClient::BroadcastMessage<tmx::messages::TmxEventLogMessage>(tmx::messages::TmxEventLogMessage &message,
		std::string source, unsigned int sourceId, IvpMsgFlags flags)
{
	PLOG(logDEBUG2) << "Sending Event Log Message: " << message;

	ivp_addEventLog(_plugin, message.get_level(), message.get_description().c_str());
}

}} // namespace tmx::utils

#endif /* SRC_PLUGINCLIENT_H_ */
