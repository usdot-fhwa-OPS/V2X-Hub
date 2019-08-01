//==========================================================================
// Name        : PedestrianPlugin.cpp
// Author      : Leidos Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 Leidos Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Pedestrian Plugin
//==========================================================================

#include "PedestrianPlugin.hpp"

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace PedestrianPlugin
{

/**
 * Construct a new PedestrianPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes.
 */
PedestrianPlugin::PedestrianPlugin(string name): PluginClient(name)
{
	// The log level can be changed from the default here.
	FILELog::ReportingLevel() = FILELog::FromString("DEBUG");

	// Add a message filter and handler for each message this plugin wants to receive.
	AddMessageFilter<DecodedBsmMessage>(this, &PedestrianPlugin::HandleDecodedBsmMessage);

	// This is an internal message type that is used to track some plugin data that changes
	AddMessageFilter<DataChangeMessage>(this, &PedestrianPlugin::HandleDataChangeMessage);

	AddMessageFilter<MapDataMessage>(this, &PedestrianPlugin::HandleMapDataMessage);

	AddMessageFilter <BsmMessage> (this, &PedestrianPlugin::HandleBasicSafetyMessage);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();
}

PedestrianPlugin::~PedestrianPlugin()
{
	if (_signSimClient != NULL)
		delete _signSimClient;
}

void PedestrianPlugin::UpdateConfigSettings()
{
	// Configuration settings are retrieved from the API using the GetConfigValue template class.
	// This method does NOT execute in the main thread, so variables must be protected
	// (e.g. using std::atomic, std::mutex, etc.).

	int instance;
	GetConfigValue("Instance", instance);
	GetConfigValue("Frequency", __frequency_mon.get());
	__frequency_mon.check();

}


void PedestrianPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void PedestrianPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
		SetStatus("ReceivedMaps", 0);
	}
}

void PedestrianPlugin::HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg)
{
	static std::atomic<int> count {0};

	PLOG(logINFO) << "New MAP: " << msg;

	int mapCount = count;
	SetStatus("ReceivedMaps", mapCount);
}

void PedestrianPlugin::HandleDecodedBsmMessage(DecodedBsmMessage &msg, routeable_message &routeableMsg)
{
	//PLOG(logDEBUG) << "Received Decoded BSM: " << msg;

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

void PedestrianPlugin::HandleDataChangeMessage(DataChangeMessage &msg, routeable_message &routeableMsg)
{
	PLOG(logINFO) << "Received a data change message: " << msg;

	PLOG(logINFO) << "Data field " << msg.get_untyped(msg.Name, "?") <<
			" has changed from " << msg.get_untyped(msg.OldValue, "?") <<
			" to " << msg.get_untyped(msg.NewValue, to_string(_frequency));
}

void PedestrianPlugin::HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg) {
	PLOG(logDEBUG)<<"HandleBasicSafetyMessage";
}

int PedestrianPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	uint msCount = 0;
	while (_plugin->state != IvpPluginState_error)
	{

		msCount += 10;

		if (_plugin->state == IvpPluginState_registered && _frequency <= msCount)
		{
			PLOG(logINFO) << _frequency << " ms wait is complete.";

			this_thread::sleep_for(chrono::milliseconds(100));

			msCount = 0;
		}
	}

	PLOG(logINFO) << "Plugin terminating gracefully.";
	return EXIT_SUCCESS;
}

} /* namespace PedestrianPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<PedestrianPlugin::PedestrianPlugin>("PedestrianPlugin", argc, argv);
}
