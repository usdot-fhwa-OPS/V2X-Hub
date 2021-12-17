/*
 * Plugin.cpp
 *
 *  Created on: Jul 23, 2014
 *      Author: ivp
 */

#ifndef __CYGWIN__
#include <sys/prctl.h>
#endif
#include "Plugin.h"
#include <assert.h>
#include <string.h>
#include "tmx/tmx.h"

using namespace std;

set<string> Plugin::clientNames;
pthread_mutex_t Plugin::clientLock = PTHREAD_MUTEX_INITIALIZER;

Plugin::Plugin(MessageRouter *messageRouter) : MessageRouterClient(messageRouter)
{
	this->mRegistered = false;

	pthread_mutexattr_t lockAttr;
	pthread_mutexattr_init(&lockAttr);
	pthread_mutexattr_settype(&lockAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&this->mConfigValueLock, &lockAttr);

	this->mConfigMonitorThread = boost::thread(&Plugin::configMonitorThreadEntry, this);
}

Plugin::~Plugin()
{
	pthread_mutex_lock(&clientLock);
	clientNames.erase(this->mInfo.pluginInfo.name);
	pthread_mutex_unlock(&clientLock);

	this->mConfigMonitorThread.interrupt();
	this->mConfigMonitorThread.join();

	if (this->mRegistered)
		this->addEventLogEntry(LogLevel_Warning, "Plugin deconstructing");
	else
		LOG_INFO("<Unknown> Plugin deconstructing");

	try {
		this->subscribeForMessages(vector<MessageFilterEntry>());
		this->setPluginStatus(IVP_STATUS_STOPPED_DISCONENCTED);
	} catch (const PluginException &e) { }

	try {
		PluginContext pcontext;
		pcontext.removeAllPluginStatusItems(this->mInfo.pluginInfo.id);
	} catch (const DbException &e) { }

}

void Plugin::registerPlugin(RegistrationInformation info)
{
	if (info.pluginInfo.name.empty())
	{
		LOG_ERROR("EMPTY NAME: A plugin attempted to register with an empty name field");
		throw PluginException("Plugin's name field is empty");
	}

	bool isUniqueName = false;
	pthread_mutex_lock(&clientLock);
	if (clientNames.find(info.pluginInfo.name) == clientNames.end())
	{
		isUniqueName = true;
		clientNames.insert(info.pluginInfo.name);
	}
	pthread_mutex_unlock(&clientLock);
	if (!isUniqueName)
	{
		LOG_ERROR("<" << info.pluginInfo.name << "> DUPLICATE NAME: A plugin attempted to register with a name identical to an active plugin.");
		throw PluginException("Another plugin is registered with the same name.");
	}

	std::map<string, PluginConfigurationParameterEntry> configMap;
	try {
		PluginContext pcontext;
		ConfigContext ccontext;
		MessageContext mcontext;

		pcontext.insertOrUpdatePlugin(info.pluginInfo);

		ccontext.initializePluginConfigParameters(info.pluginInfo.id, info.configDefaultEntries);
		configMap = ccontext.getPluginConfigParameters(info.pluginInfo.id);

		for(vector<MessageTypeEntry>::iterator itr = info.messageTypeEntries.begin(); itr != info.messageTypeEntries.end(); itr++)
		{
			mcontext.insertMessageType(*itr);
			mcontext.mapPluginToMessageType(info.pluginInfo.id, itr->id);
		}

	} catch (DbException &e) {
		LOG_FATAL("<" << info.pluginInfo.name << "> MySQL: Unable to register plugin with database [" << e.what() << "]");
		throw PluginException("Unable to register plugin with database [" + string(e.what()) + "]");
	}

	//TODO:...

	this->mInfo = info;
	this->mConfigValues = configMap;
	this->mRegistered = true;
	this->pluginName = mInfo.pluginInfo.name;

	this->addEventLogEntry(LogLevel_Info, "Plugin registered");
	this->setPluginStatus(IVP_STATUS_RUNNING);

	IvpMessage *msg = ivpMsg_create(nullptr, nullptr, nullptr, IvpMsgFlags_None, nullptr);
	if (msg != nullptr)
		this->sendMessageToRouter(msg);
}

void Plugin::subscribeForMessages(const std::vector<MessageFilterEntry> &filter)
{

	if (!this->mRegistered)
	{
		string what = "Unable to subscribe for messages, plugin is not registered";
		LOG_WARN("<" << string(this->mRegistered ? this->mInfo.pluginInfo.name : "Unknown") << "> " << what);
		throw PluginNotRegisteredException(what);
	}

	MessageRouterClient::subscribeForMessages(filter);
}


void Plugin::setPluginStatus(std::string status)
{
	if (!this->mRegistered)
	{
		string what = "Unable to set plugin status, plugin is not registered";
		LOG_WARN("<" << string(this->mRegistered ? this->mInfo.pluginInfo.name : "Unknown") << "> " << what);
		throw PluginNotRegisteredException(what);
	}

	LOG_INFO("<" << this->mInfo.pluginInfo.name << "> Setting plugin status: " << status);

	try {
		PluginContext context;
		context.setPluginStatus(this->mInfo.pluginInfo.id, status);
	} catch (DbException &e) {
		LOG_ERROR("<" << this->mInfo.pluginInfo.name << "> MySQL: Unable to set plugin status [" << e.what() << "]");
		//throw PluginException("Error setting plugin status [" + string(e.what()) + "]");
	}
}

void Plugin::setStatusItems(std::map<std::string, std::string> statusItems)
{
	if (!this->mRegistered)
	{
		string what = "Unable to set plugin status items, plugin is not registered";
		LOG_WARN("<" << string(this->mRegistered ? this->mInfo.pluginInfo.name : "Unknown") << "> " << what);
		throw PluginNotRegisteredException(what);
	}

	vector<PluginStatusItem> dbEntries;

	for(map<std::string, std::string>::iterator itr = statusItems.begin(); itr != statusItems.end(); itr++)
	{
		PluginStatusItem entry;
		entry.key = itr->first;
		entry.value = itr->second;
		dbEntries.push_back(entry);
	}

	try {
		PluginContext context;
		context.setPluginStatusItems(this->mInfo.pluginInfo.id, dbEntries);
	} catch (DbException &e) {
		LOG_WARN("<" << this->mInfo.pluginInfo.name << "> MySQL: Unable to status entries [" << e.what() << "]");
		//throw PluginException("Error setting status entries [" + string(e.what()) + "]");
	}
}

void Plugin::removeStatusItems(std::vector<std::string> itemKeys)
{
	if (!this->mRegistered)
	{
		string what = "Unable to remove plugin status items, plugin is not registered";
		LOG_WARN("<" << string(this->mRegistered ? this->mInfo.pluginInfo.name : "Unknown") << "> " << what);
		throw PluginNotRegisteredException(what);
	}

	try {
		PluginContext context;
		context.removePluginStatusItems(this->mInfo.pluginInfo.id, itemKeys);
	} catch (DbException &e) {
		LOG_WARN("<" << this->mInfo.pluginInfo.name << "> MySQL: Unable to remove status items [" << e.what() << "]");
		//throw PluginException("Error setting status entries [" + string(e.what()) + "]");
	}
}

void Plugin::addEventLogEntry(LogLevel level, std::string description)
{
	if (!this->mRegistered)
	{
		string what = "Unable to add Event Log entry, plugin is not registered";
		LOG_WARN("<" << string(this->mRegistered ? this->mInfo.pluginInfo.name : "Unknown") << "> " << what);
		throw PluginNotRegisteredException(what);
	}

	dhlogging::Logger::addEventLogEntry(this->mInfo.pluginInfo.name, description, level);
}

uint64_t Plugin::GetMsTimeSinceEpoch2()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return (uint64_t)((double)(tv.tv_sec) * 1000 + (double)(tv.tv_usec) / 1000);
}

void Plugin::sendMessageToRouter(IvpMessage *msg)
{
	assert(msg != NULL);

	if (!this->mRegistered)
	{
		string what = "Unable to send message, plugin is not registered";
		LOG_WARN("<" << string(this->mRegistered ? this->mInfo.pluginInfo.name : "Unknown") << "> " << what);
		throw PluginNotRegisteredException(what);
	}

	if(msg->source)
		free(msg->source);
	msg->source = strdup(mInfo.pluginInfo.name.c_str());
	msg->sourceId = mInfo.pluginInfo.id;


	MessageRouterClient::sendMessageToRouter(msg);
}

void Plugin::receiveMessage(IvpMessage *msg)
{
	this->onMessageReceived(msg);
}

std::string Plugin::getConfigValue(std::string key)
{
	if (!this->mRegistered)
	{
		string what = "Unable to get configuration value, plugin is not registered";
		LOG_WARN("<" << string(this->mRegistered ? this->mInfo.pluginInfo.name : "Unknown") << "> " << what);
		throw PluginNotRegisteredException(what);
	}

	pthread_mutex_lock(&this->mConfigValueLock);
	map<string, PluginConfigurationParameterEntry>::iterator item = this->mConfigValues.find(key);
	bool itemIsValid = item != this->mConfigValues.end();
	pthread_mutex_unlock(&this->mConfigValueLock);

	if(!itemIsValid)
		throw UnknownConfigurationKeyException("Unknown configuration key '" + key + "'");

	return item->second.value;
}


std::map<std::string, PluginConfigurationParameterEntry> Plugin::getAllConfigValues()
{
	if (!this->mRegistered)
	{
		string what = "Unable to get configuration value, plugin is not registered";
		LOG_WARN("<" << string(this->mRegistered ? this->mInfo.pluginInfo.name : "Unknown") << "> " << what);
		throw PluginNotRegisteredException(what);
	}

	pthread_mutex_lock(&this->mConfigValueLock);
	map<string, PluginConfigurationParameterEntry> temp = this->mConfigValues;
	pthread_mutex_unlock(&this->mConfigValueLock);

	return temp;
}

void Plugin::setConfigValue(std::string key, std::string value)
{
	if (!this->mRegistered)
	{
		string what = "Unable to set configuration value, plugin is not registered";
		LOG_WARN("<" << string(this->mRegistered ? this->mInfo.pluginInfo.name : "Unknown") << "> " << what);
		throw PluginNotRegisteredException(what);
	}

	pthread_mutex_lock(&this->mConfigValueLock);
	map<string, PluginConfigurationParameterEntry>::iterator item = this->mConfigValues.find(key);
	bool itemIsValid = item != this->mConfigValues.end();
	pthread_mutex_unlock(&this->mConfigValueLock);

	if(!itemIsValid)
		throw UnknownConfigurationKeyException("Unknown configuration key '" + key + "'");

	PluginConfigurationParameterEntry newEntry = item->second;
	newEntry.value = value;

	try {
		ConfigContext ccontext;
		ccontext.updatePluginConfigParameterValue(newEntry);
	} catch (DbException &e) {
		LOG_ERROR("<" << string(this->mRegistered ? this->mInfo.pluginInfo.name : "Unknown") << "> MySQL: Unable to update configuration value for key '" << key << "' [" << e.what() << "]");
	}
}

void Plugin::configMonitorThreadEntry()
{
#ifndef __CYGWIN__
	prctl(PR_SET_NAME, "PluginCnfgMntr", 0, 0, 0);
#endif

	while(1)
	{
		if (boost::this_thread::interruption_requested())
			throw boost::thread_interrupted();

		boost::this_thread::sleep(boost::posix_time::seconds(2));
		if (!this->mRegistered)
			continue;

		boost::this_thread::disable_interruption di;
		{
			std::map<string, PluginConfigurationParameterEntry> configMap;
			try {
				ConfigContext ccontext;

				configMap = ccontext.getPluginConfigParameters(this->mInfo.pluginInfo.id);


			} catch (DbException &e) {
				LOG_ERROR("<" << this->mInfo.pluginInfo.name << "> MySQL: Unable to get configuration information [" << e.what() << "]");
				continue;
			}

			pthread_mutex_lock(&this->mConfigValueLock);

			for(map<string, PluginConfigurationParameterEntry>::iterator itr = this->mConfigValues.begin(); itr != this->mConfigValues.end(); itr++)
			{
				map<string, PluginConfigurationParameterEntry>::iterator newEntry = configMap.find(itr->first);
				if (newEntry == configMap.end())
				{
					//bad fatal not good...
					continue;
				}

				if (newEntry->second.value != itr->second.value)
				{
					itr->second = newEntry->second;
					this->onConfigChanged(itr->second.key, itr->second.value);
				}

				configMap.erase(newEntry);
			}

			for (map<string, PluginConfigurationParameterEntry>::iterator itr = configMap.begin(); itr != configMap.end(); itr++)
			{
				// Some new config discovered that was not registered.  Add it as an acceptable value.
				try
				{
					this->mConfigValues[itr->first] = itr->second;
					this->onConfigChanged(itr->second.key, itr->second.value);
				}
				catch (UnknownConfigurationKeyException &ex)
				{
					// Some plugins may not like this
					LOG_ERROR("<" << this->mInfo.pluginInfo.name << "> Exception from handling new config parameter " << itr->second.key << " [" << ex.what() << "]");
				}
			}

			pthread_mutex_unlock(&this->mConfigValueLock);

		}
	}
}

