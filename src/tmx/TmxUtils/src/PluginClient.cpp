/*
 * @file PluginClient.cpp
 *
 *  Created on: Feb 25, 2016
 *      Author: ivp
 */

// Include header file for self first to test that it has all includes it needs for inclusion anywhere.
#include "PluginClient.h"

#include <chrono>
#include <cstdio>
#include <execinfo.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

#include "PluginUtil.h"
#include "PluginUpgrader.h"
#include "Uuid.h"
#include "database/DbConnectionPool.h"
#include "version.h"

#define BT_BUFFER_SIZE 100

using namespace std;
using namespace tmx;

namespace tmx {
namespace utils {

// Define static instance members.
std::map<IvpPlugin*, PluginClient*> PluginClient::_instanceMap;
SystemContext PluginClient::_sysContext;


PluginClient::PluginClient(std::string name) :
	_name(name),
	_logPrefix(name + " - "),
	_msgFilter(NULL),
	_sysConfig(NULL)
{
	PLOG(logDEBUG2) << "Constructing the plugin";

	// Populate the info for the plugin by specifying the callback functions.
	// Since info.manifestLocation is not specified below, the default value of
	// "manifest.json" is assumed.
	IvpPluginInformation info = IVP_PLUGIN_INFORMATION_INITIALIZER;
	info.onConfigChanged = StaticOnConfigChanged;
	info.onError = StaticOnError;
	info.onMsgReceived = StaticOnMessageReceived;
	info.onStateChange = StaticOnStateChange;

	PLOG(logDEBUG2) << "Creating the plugin in IVP";

	// Create the plugin.
	_plugin = ivp_create(info);

	if (!_plugin)
	{
		std::runtime_error ex("Error creating plugin: " + name);
		HandleException(ex, true);
	}

	// Pull the name from the manifest, if possible
	if (_plugin->jsonManifest)
	{
		IvpManifest *manifest = ivpRegister_getManifestFromJson(_plugin->jsonManifest);
		if (manifest->name && strlen(manifest->name) > 0)
			_name = string(manifest->name);
		ivpRegister_destroyManifest(manifest);
		manifest = NULL;

		PLOG(logDEBUG) << "Plugin " << _name << " has been registered with IVP";
	}

	// Make sure to run any upgrades
	try {
		PLOG(logDEBUG) << "Calling DB upgrader";
		DbConnectionPool pool;
		std::string pwd = pool.GetPwd();
		// "tcp://127.0.0.1:3306","IVP", pwd, "IVP"
		DbConnection conn = pool.Connection("tcp://127.0.0.1:3306","IVP", pwd, "IVP");

		PluginUpgrader::UpgradeDatabase(&conn, IVPUTILS_VERSION);
	} catch (runtime_error &ex) {
		PLOG(logERROR) << "Unable to upgrade database: " << ex.what();
	}

	PLOG(logDEBUG2) << "Registering the IVP plugin instance";
	PluginClient::_instanceMap[_plugin] = this;

	_keepAlive = new PluginKeepAlive(this);
	_startTime = std::chrono::system_clock::now();
}

PluginClient::~PluginClient()
{
	if (this->_msgFilter)
	{
		ivpSubscribe_destroyFilter(this->_msgFilter);
		this->_msgFilter = NULL;
	}

	if (this->_sysConfig)
	{
		ivpConfig_destroyCollection(this->_sysConfig);
		this->_sysConfig = NULL;
	}

	if (this->_keepAlive)
	{
		delete this->_keepAlive;
		this->_keepAlive = NULL;
	}
}

PluginClient *PluginClient::FindPlugin(string name)
{
	for (map<IvpPlugin *, PluginClient *>::iterator i = PluginClient::_instanceMap.begin(); i != PluginClient::_instanceMap.end(); i++)
	{
		if (i->second && i->second->GetName() == name)
		{
			return i->second;
		}
	}

	return NULL;
}

// static wrapper for OnConfigChanged.
void PluginClient::StaticOnConfigChanged(IvpPlugin *plugin, const char *key, const char *value)
{
	PluginClient *p = PluginClient::_instanceMap[plugin];
	if (p)
	{
		try
		{
			p->OnConfigChanged(key, value);
		}
		catch (exception &ex)
		{
			p->HandleException(ex, false);
		}
	}
	if (strcmp(SYS_CONTEXT_CONFIGKEY_DB_RFRSH_INTERVAL, key) == 0)
	{
		PluginClient::_sysContext.setDbUpdateFrequency(value);
	}
}

void PluginClient::StaticOnConfigChanged(PluginClient *plugin, const char *key, const char *value)
{
	if (plugin)
		StaticOnConfigChanged(plugin->_plugin, key, value);
}

// TMX API calls this method whenever a configuration value changes.
void PluginClient::OnConfigChanged(const char *key, const char *value)
{
	PLOG(logINFO) << "Config Changed. " << key << ": " << value;

	// If this is a system parameter, update the value
	pthread_mutex_lock(&_plugin->lock);

	char *results = NULL;
	if (_sysConfig != NULL)
		results = ivpConfig_getCopyOfValueFromCollection(_sysConfig, key);

	if (results != NULL)
	{
		ivpConfig_updateValueInCollection(_sysConfig, key, value);
	}
	else
	{
		// If it is not in the manifest, assume it belongs as a system parameter
		if (_plugin->config != NULL)
			results = ivpConfig_getCopyOfValueFromCollection(_plugin->config, key);

		if (!results)
		{
			SetSystemConfigValue(key, value, value, false);
		}
	}

	if (results)
		free(results);

	pthread_mutex_unlock(&_plugin->lock);

	// Handle the logging parameter
	if (strcmp("LogLevel", key) == 0)
	{
		std::string lvl(value);
		std::transform(lvl.begin(), lvl.end(), lvl.begin(), ::toupper);
		LogLevel newLvl = FILELog::FromString(lvl);
		FILELog::ReportingLevel() = newLvl;
	}
	// Handle the keep alive frequency
	else if (strcmp("KeepAliveFrequency", key) == 0)
	{
		try
		{
			double minVal = battelle::attributes::attribute_lexical_cast<double>(value);
			this->_keepAlive->set_Frequency(chrono::milliseconds((int)(minVal * 60 * 1000)));
		}
		catch (exception &ex)
		{
			this->HandleException(ex, false);
		}
	}
}

// static wrapper for OnError.
void PluginClient::StaticOnError(IvpPlugin *plugin, IvpError err)
{
	PluginClient *p = PluginClient::_instanceMap[plugin];
	if (p)
	{
		try
		{
			p->OnError(err);
		}
		catch (exception &ex)
		{
			p->HandleException(ex, false);
		}
	}
}

void PluginClient::StaticOnError(PluginClient *plugin, IvpError err)
{
	if (plugin)
		StaticOnError(plugin->_plugin, err);
}

// TMX API calls this method to pass errors back to the plugin application.
void PluginClient::OnError(IvpError err)
{
	PLOG(logERROR) << "Error: " << err.error << ", Level: " << err.level <<
			", Sys Error: " << err.sysErrNo;
}

// static wrapper for OnMessageReceived.
void PluginClient::StaticOnMessageReceived(IvpPlugin *plugin, IvpMessage *msg)
{
	PluginClient *p = PluginClient::_instanceMap[plugin];
	if (p)
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		uint64_t microsecondsSinceEpoch = (uint64_t)(tv.tv_sec) * 1000000 + (uint64_t)(tv.tv_usec);

		try
		{
			p->OnMessageReceived(msg);
			PluginClient::_sysContext.trackMessageHandled(p->GetName(), msg, microsecondsSinceEpoch);
		}
		catch (exception &ex)
		{
			p->HandleException(ex, false);
		}
	}
}

void PluginClient::StaticOnMessageReceived(PluginClient *plugin, IvpMessage *msg)
{
	if (plugin)
		StaticOnMessageReceived(plugin->_plugin, msg);
}

// TMX API calls this method to pass received message to the plugin application.
// Only message that have been requested (using IvpMsgFilter) are received.
// Note that the API destroys the IvpMessage after this callback returns.
void PluginClient::OnMessageReceived(IvpMessage *msg)
{
	// Count the number of messages received and output the message details.
	static unsigned int count = 1;
	count++;

	routeable_message routeableMsg(msg);

	PLOG(logDEBUG1) << "Received Message. Type: " << routeableMsg.get_type() <<
			", Subtype: " << routeableMsg.get_subtype() <<
			", Source: " << routeableMsg.get_source() <<
			", Count: " << count;

	invoke_handler(routeableMsg.get_type(), routeableMsg.get_subtype(), routeableMsg);
}

// static wrapper for OnStateChange.
void PluginClient::StaticOnStateChange(IvpPlugin *plugin, IvpPluginState state)
{
	PluginClient *p = PluginClient::_instanceMap[plugin];
	if (p)
	{
		try
		{
			p->SetStartTimeStatus();
			p->OnStateChange(state);
		}
		catch (exception &ex)
		{
			p->HandleException(ex, false);
		}
	}
}

void PluginClient::StaticOnStateChange(PluginClient *plugin, IvpPluginState state)
{
	if (plugin)
		StaticOnStateChange(plugin->_plugin, state);
}

// TMX API calls this method whenever the plugin state changes.
void PluginClient::OnStateChange(IvpPluginState state)
{
	PLOG(logDEBUG1) << "State Changed: " << PluginUtil::IvpPluginStateToString(state);
}

void PluginClient::RemoveStatus(const char *key)
{
	if (_plugin)
		ivp_removeStatusItem(_plugin, key);
}

void PluginClient::SetStartTimeStatus()
{
	if (_isStartTimeStatusSet || _plugin->state != IvpPluginState_registered)
		return;

	SetStatus("Start Time", Clock::ToLocalPreciseTimeString(_startTime));

	_isStartTimeStatusSet = true;
}

void PluginClient::AddMessageFilter(const char *type, const char *subtype, IvpMsgFlags flags)
{
	_msgFilter = ivpSubscribe_addFilterEntryWithFlagMask(_msgFilter, type, subtype, flags);
}

void PluginClient::SubscribeToMessages()
{
	if (_msgFilter == NULL)
		throw PluginException("Error subscribing to messages.  No message filters were added.");

	ivp_subscribe(_plugin, _msgFilter);
	ivpSubscribe_destroyFilter(_msgFilter);
	_msgFilter = NULL;
}

int PluginClient::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	while (_plugin->state != IvpPluginState_error)
	{
		PLOG(logDEBUG4) << "Sleeping 1 ms" << endl;

		this_thread::sleep_for(chrono::milliseconds(1000));
	}

	PLOG(logINFO) << "Plugin terminating gracefully.";
	return EXIT_SUCCESS;
}

bool PluginClient::invoke_handler(string messageType, string messageSubType,
		routeable_message &routeableMsg)
{
	try
	{
		handler_allocator *allocator =
				_msgHandlers[pair<string, string>(messageType, messageSubType)];
		if (allocator)
			allocator->invokeHandler(routeableMsg);
		else
			return false;
	}
	catch (exception &ex)
	{
		HandleException(ex, false);
		return false;
	}

	return true;
}

void PluginClient::handleMessage(message &msg, routeable_message &src)
{
	throw PluginException(this->_name + " received unhandled message of Type=" +
			src.get_type() + ", SubType=" + src.get_subtype());
}

std::string PluginClient::NewGuid()
{
	return Uuid::NewGuid();
}

void PluginClient::HandleException(exception &ex, bool abort)
{
	tmx::messages::TmxEventLogMessage elm;
	elm.set_level(Output2Eventlog::ToEventLogLevel(abort ? logERROR : logWARNING));
	elm.set_description(ExceptionToString(ex, this->GetName(), abort));
	PLOG(Output2Eventlog::FromEventLogLevel(elm.get_level())) << elm.get_description();
	BroadcastMessage(elm);

	if (abort)
	{
		// Wait for the message
		sleep(1);
		std::terminate();
	}
}

bool PluginClient::IsPluginState(IvpPluginState state)
{
	bool ret = false;

	if (_plugin)
	{
		pthread_mutex_lock(&_plugin->lock);
		ret = _plugin->state == state;
		pthread_mutex_unlock(&_plugin->lock);
	}

	return ret;
}

bool PluginClient::ProcessOptions(const boost::program_options::variables_map &opts)
{
	return Runnable::ProcessOptions(opts);
}

}} // namespace tmx::utils
