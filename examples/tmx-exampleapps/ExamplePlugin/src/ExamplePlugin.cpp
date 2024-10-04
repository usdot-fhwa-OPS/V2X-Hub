//==========================================================================
// Name        : ExamplePlugin.cpp
// Author      : Battelle Memorial Institute
// Version     :
// Copyright   : Copyright (c) 2014 Battelle Memorial Institute. All rights reserved.
// Description : Example Plugin
//==========================================================================

#include "PluginClientClockAware.h"
#include "PluginDataMonitor.h"

#include <atomic>
#include <thread>
#include <DecodedBsmMessage.h>
#include <tmx/j2735_messages/MapDataMessage.hpp>

using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;

namespace ExamplePlugin
{

/**
 * This plugin is an example to demonstrate the capabilities of a TMX plugin.
 */
class ExamplePlugin: public PluginClientClockAware
{
public:
	ExamplePlugin(std::string);
	virtual ~ExamplePlugin();
	int Main();
protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);

	void HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg);
	void HandleDecodedBsmMessage(DecodedBsmMessage &msg, routeable_message &routeableMsg);
	void HandleDataChangeMessage(DataChangeMessage &msg, routeable_message &routeableMsg);
private:
	std::atomic<uint64_t> _frequency{0};
	DATA_MONITOR(_frequency);   // Declares the
};

/**
 * Construct a new ExamplePlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes.
 */
ExamplePlugin::ExamplePlugin(string name): PluginClientClockAware(name)
{
	// The log level can be changed from the default here.
	FILELog::ReportingLevel() = FILELog::FromString("DEBUG");

	// Add a message filter and handler for each message this plugin wants to receive.
	AddMessageFilter<DecodedBsmMessage>(this, &ExamplePlugin::HandleDecodedBsmMessage);

	// This is an internal message type that is used to track some plugin data that changes
	AddMessageFilter<DataChangeMessage>(this, &ExamplePlugin::HandleDataChangeMessage);

	AddMessageFilter<MapDataMessage>(this, &ExamplePlugin::HandleMapDataMessage);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();
}

ExamplePlugin::~ExamplePlugin()
{
}

void ExamplePlugin::UpdateConfigSettings()
{
	// Configuration settings are retrieved from the API using the GetConfigValue template class.
	// This method does NOT execute in the main thread, so variables must be protected
	// (e.g. using std::atomic, std::mutex, etc.).

	int instance;
	GetConfigValue("Instance", instance);

	GetConfigValue("Frequency", __frequency_mon.get());
	__frequency_mon.check();
}


void ExamplePlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void ExamplePlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
		SetStatus("ReceivedMaps", 0);
	}
}

void ExamplePlugin::HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg)
{
	static std::atomic<int> count {0};

	PLOG(logINFO) << "New MAP: " << msg;

	int mapCount = count;
	SetStatus("ReceivedMaps", mapCount);
}

void ExamplePlugin::HandleDecodedBsmMessage(DecodedBsmMessage &msg, routeable_message &routeableMsg)
{
	PLOG(logDEBUG) << "Received Decoded BSM: " << msg;

	// Determine if location, speed, and heading are valid.
	bool isValid = msg.get_IsLocationValid() && msg.get_IsSpeedValid() && msg.get_IsHeadingValid();

	// Print some of the BSM values.
	PLOG(logDEBUG) << "ID: " << msg.get_TemporaryId()
		<< ", Location: (" <<  msg.get_Latitude() << ", " <<  msg.get_Longitude() << ")"
		<< ", Speed: " << msg.get_Speed_mph() << " mph"
		<< ", Heading: " << msg.get_Heading() << "°"
		<< ", All Valid: " << isValid
		<< ", IsOutgoing: " << msg.get_IsOutgoing();
}

// Example of handling
void ExamplePlugin::HandleDataChangeMessage(DataChangeMessage &msg, routeable_message &routeableMsg)
{
	PLOG(logINFO) << "Received a data change message: " << msg;

	PLOG(logINFO) << "Data field " << msg.get_untyped(msg.Name, "?") <<
			" has changed from " << msg.get_untyped(msg.OldValue, "?") <<
			" to " << msg.get_untyped(msg.NewValue, to_string(_frequency));
}

// Override of main method of the plugin that should not return until the plugin exits.
// This method does not need to be overridden if the plugin does not want to use the main thread.
int ExamplePlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	uint msCount = 0;
	while (_plugin->state != IvpPluginState_error)
	{
		PLOG(logDEBUG4) << "Sleeping 10 ms" << endl;

		PluginClientClockAware::getClock()->sleep_for(10);

		msCount += 10;

		// Example showing usage of _frequency configuraton parameter from main thread.
		// Access is thread safe since _frequency is declared using std::atomic.
		if (_plugin->state == IvpPluginState_registered && _frequency <= msCount)
		{
			PLOG(logINFO) << _frequency << " ms wait is complete.";
			msCount = 0;
		}
	}

	PLOG(logINFO) << "Plugin terminating gracefully.";
	return EXIT_SUCCESS;
}

} /* namespace ExamplePlugin */

int main(int argc, char *argv[])
{
	return run_plugin<ExamplePlugin::ExamplePlugin>("ExamplePlugin", argc, argv);
}
