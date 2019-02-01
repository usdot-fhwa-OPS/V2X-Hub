/*
 * PluginDataMonitor.h
 *
 *  Created on: Nov 2, 2016
 *      Author: gmb
 */

#ifndef SRC_PLUGINDATAMONITOR_H_
#define SRC_PLUGINDATAMONITOR_H_

#include <atomic>
#include <memory>
#include <tmx/messages/auto_message.hpp>
#include <tmx/messages/routeable_message.hpp>

#include "ApplicationDataMessage.h"
#include "PluginExtender.h"
#include "PluginLog.h"

#define DATA_MONITOR_FIELD_NAME(X) _ ## X ## _mon
#define DATA_MONITOR(X) tmx::utils::PluginDataMonitor<decltype(X)> DATA_MONITOR_FIELD_NAME(X) {*this, attr_str(X), X}

namespace tmx
{

namespace messages
{

class DataChangeMessage: public tmx::auto_message
{
public:
	static constexpr const char *MessageType = MSGSUBTYPE_DATA_STRING;
	static constexpr const char *MessageSubType = MSGSUBTYPE_STATECHANGE_STRING;

	static constexpr const char *Name = "Name";
	static constexpr const char *OldValue = "OldValue";
	static constexpr const char *NewValue = "NewValue";
};

} /* namespace messages */

namespace utils
{

template <typename DataType>
class PluginDataMonitor: public PluginExtender
{
public:
	typedef DataType data_type;
	typedef tmx::messages::DataChangeMessage message_type;

	PluginDataMonitor(PluginClient &pluginClient, std::string name, data_type &data):
		PluginExtender(pluginClient),
		_name(name), _data(data), _cache(data)
	{
	}

	std::string get_name() { return _name; }

	data_type &get() { return _data; }

	bool set(const data_type &newData)
	{
		_data = newData;
		return check();
	}

	bool check()
	{
		if (hasChanged())
		{
			// Fire off a data change message
			message_type msg;
			msg.auto_attribute<message_type>(_name, msg.Name);
			msg.auto_attribute<message_type>(_data, msg.NewValue);
			msg.auto_attribute<message_type>(_cache, msg.OldValue);
			tmx::routeable_message routeableMsg;
			routeableMsg.initialize(msg, this->_pluginClient.GetName());

			PluginClient::StaticOnMessageReceived(&(this->_pluginClient), routeableMsg.get_message());

			_cache = _data;
			return true;
		}

		return false;
	}
private:
	std::string _name;
	data_type &_data;
	data_type _cache;

	bool hasChanged()
	{
		return _data != _cache;
	}
};

template <typename DataType>
class PluginDataMonitor<std::atomic<DataType> >: public PluginExtender
{
public:
	typedef DataType data_type;
	typedef tmx::messages::DataChangeMessage message_type;

	PluginDataMonitor(PluginClient &pluginClient, std::string name, std::atomic<data_type> &data):
		PluginExtender(pluginClient),
		_name(name), _data(data), _cache(data)
	{
	}

	std::string get_name() { return _name; }

	std::atomic<data_type> &get() { return _data; }

	bool set(const std::atomic<data_type> &newData)
	{
		_data = newData;
		return check();
	}

	bool check()
	{
		if (hasChanged())
		{
			// Fire off a data change message
			data_type newData = _data;
			message_type msg;
			msg.auto_attribute<message_type>(_name, msg.Name);
			msg.auto_attribute<message_type>(newData, msg.NewValue);
			msg.auto_attribute<message_type>(_cache, msg.OldValue);
			tmx::routeable_message routeableMsg;
			routeableMsg.initialize(msg, this->_pluginClient.GetName());

			IvpMessage *ivpMsg = routeableMsg.get_message();
			PluginClient::StaticOnMessageReceived(&(this->_pluginClient), ivpMsg);
			ivpMsg_destroy(ivpMsg);

			_cache = newData;
			return true;
		}

		return false;
	}
private:
	std::string _name;
	std::atomic<data_type> &_data;
	data_type _cache;

	bool hasChanged()
	{
		return _data != _cache;
	}
};


} /* namespace utils */
} /* namespace tmx */



#endif /* SRC_PLUGINDATAMONITOR_H_ */
