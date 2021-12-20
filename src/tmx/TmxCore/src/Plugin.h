/*
 * Plugin.h
 *
 *  Created on: Jul 23, 2014
 *      Author: ivp
 */

#ifndef PLUGIN_H_
#define PLUGIN_H_

#include "MessageRouterClient.h"
#include <string>
#include <set>
#include <map>
#include <pthread.h>
#include "tmx/tmx.h"
#include "database/PluginContext.h"
#include "database/ConfigContext.h"
#include "database/MessageContext.h"
#include "logger.h"
#include "boost/thread.hpp"

/*!
 * \ingroup IVPCore
 * Base class for all exceptions thrown by a plugin.
 */
class PluginException : public std::runtime_error
{
public:
	PluginException(const PluginException& e) : std::runtime_error(e.what()) { };
	explicit PluginException(const std::string& reason) : std::runtime_error(reason) { };
};

/*!
 * \ingroup IVPCore
 * Exception class for operations done when the plugin has not been registered
 */
struct PluginNotRegisteredException : public PluginException
{
	PluginNotRegisteredException(const PluginNotRegisteredException& e) : PluginException(e.what()) { }
	explicit PluginNotRegisteredException(const std::string& reason) : PluginException(reason) {}
};

/*!
 * \ingroup IVPCore
 * Exception class for operations done with unknown configuration keys.
 */
struct UnknownConfigurationKeyException : public PluginException
{
	UnknownConfigurationKeyException(const UnknownConfigurationKeyException& e) : PluginException(e.what()) { }
	explicit UnknownConfigurationKeyException(const std::string& reason) : PluginException(reason) {}
};

/*!
 * Provides the required information to register the plugin with the core system. For internal "Core" plugins
 * this has to be provided through code, otherwise this is the information from the manifest file of the application.
 */
struct RegistrationInformation {
	PluginEntry pluginInfo;

	std::vector<PluginConfigurationParameterEntry> configDefaultEntries;
	std::vector<MessageTypeEntry> messageTypeEntries;
};

/*!
 * \ingroup IVPCore
 * Provides a common base set of functionality for all plugins, either internal "Core" plugins that are coded by directly deriving from this class,
 * or for cross process plugin's that use sockets or pipes to send special 'Control' messages to provide the same functionality (plugin's that use the ivpapi).
 *
 *
 * The functionality provided here is:
 * 		-> Registration
 * 			-> Inserts or update's plugin's information in the DB.
 * 			-> Only one plugin can be registered with the same name at a given time.
 * 		-> Status Updates
 * 			-> Status of the plugin
 * 			-> Custom status entries based off of key-value
 * 		-> Configuration
 * 			-> Initialization of default values
 * 			-> Get/Set operations
 * 			-> Event driven notification of configuration changes.
 */
class Plugin : MessageRouterClient {
public:
	virtual ~Plugin();
	RegistrationInformation mInfo;

protected:
	explicit Plugin(MessageRouter *messageRouter);

	/*!
	 * Attempts to reserve the plugin name and register the plugin in the database.  This function must be successfully called before
	 * a plugin can use most of the other functionality.
	 *
	 * throws: PluginException
	 */
	void registerPlugin(RegistrationInformation info);

	/*!
	 * throws: PluginNotRegisteredException
	 */
	void subscribeForMessages(const std::vector<MessageFilterEntry> &filter);

	/*!
	 * throws: PluginNotRegisteredException
	 */
	void setPluginStatus(std::string status);

	/*!
	 *
	 * throws: PluginNotRegisteredException
	 *
	 * @params statusItems
	 * 		'key' / 'status' pairs of custom status items.
	 */
	void setStatusItems(std::map<std::string, std::string> statusItems);

	/*!
	 *
	 * throws: PluginNotRegisteredException
	 *
	 * @params itemKeys
	 * 		keys of the items to remove
	 */
	void removeStatusItems(std::vector<std::string> itemKeys);

	/*!
	 *
	 * throws: PluginNotRegisteredException
	 */
	void addEventLogEntry(LogLevel level, std::string description);

	/*!
	 * throws: PluginNotRegisteredException
	 */
	void sendMessageToRouter(IvpMessage *msg);

	/*!
	 * This is the implementation of MessageReceiver and this is where the Message Router sends messages too.
	 */
	void receiveMessage(IvpMessage *msg);

	/*!
	 * Interface for derived classes to receive messages.  Implement this and the function will be called when messages are received.
	 */
	virtual void onMessageReceived(IvpMessage *msg) { };

	/*!
	 * Interface for derived classes to receive configuration changes.  Implement this and the function will be called when messages are received.
	 */
	virtual void onConfigChanged(std::string key, std::string value) { };

	/*!
	 *
	 * throws: PluginNotRegisteredException, UnknownConfigurationKeyException
	 */
	std::string getConfigValue(std::string key);

	/*!
	 *
	 * throws: PluginNotRegisteredException
	 */
	std::map<std::string, PluginConfigurationParameterEntry> getAllConfigValues();

	/*!
	 *
	 * throws: PluginNotRegisteredException, UnknownConfigurationKeyException
	 */
	void setConfigValue(std::string key, std::string value);



private:

	bool mRegistered;

	uint64_t GetMsTimeSinceEpoch2();

	pthread_mutex_t mConfigValueLock;
	std::map<std::string, PluginConfigurationParameterEntry> mConfigValues;
	boost::thread mConfigMonitorThread;

	static std::set<std::string> clientNames;
	static pthread_mutex_t clientLock;

	void configMonitorThreadEntry();
};

#endif /* PLUGINCLIENT_H_ */
