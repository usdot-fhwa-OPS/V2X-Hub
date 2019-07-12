//==========================================================================
// Name        : PreemptionPlugin.cpp
// Author      : Leidos Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 Leidos Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Preemption Plugin
//==========================================================================

#include "PreemptionPlugin.hpp"

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace PreemptionPlugin
{

/**
 * Construct a new PreemptionPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes.
 */
PreemptionPlugin::PreemptionPlugin(string name): PluginClient(name)
{
	// The log level can be changed from the default here.
	FILELog::ReportingLevel() = FILELog::FromString("DEBUG");

	// Add a message filter and handler for each message this plugin wants to receive.
	AddMessageFilter<DecodedBsmMessage>(this, &PreemptionPlugin::HandleDecodedBsmMessage);

	// This is an internal message type that is used to track some plugin data that changes
	AddMessageFilter<DataChangeMessage>(this, &PreemptionPlugin::HandleDataChangeMessage);

	AddMessageFilter<MapDataMessage>(this, &PreemptionPlugin::HandleMapDataMessage);

	AddMessageFilter <BsmMessage> (this, &PreemptionPlugin::HandleBasicSafetyMessage);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();
}

PreemptionPlugin::~PreemptionPlugin()
{
	if (_signSimClient != NULL)
		delete _signSimClient;
}

void PreemptionPlugin::UpdateConfigSettings()
{
	// Configuration settings are retrieved from the API using the GetConfigValue template class.
	// This method does NOT execute in the main thread, so variables must be protected
	// (e.g. using std::atomic, std::mutex, etc.).

	int instance;
	GetConfigValue("Instance", instance);
	GetConfigValue("Frequency", __frequency_mon.get());
	__frequency_mon.check();

	GetConfigValue("BasePreemptionOid", BasePreemptionOid);
	GetConfigValue("ipwithport", ipwithport);
	GetConfigValue("snmp_community", snmp_community);
	GetConfigValue("map_path", map_path);

}


void PreemptionPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void PreemptionPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
		SetStatus("ReceivedMaps", 0);
	}
}

void PreemptionPlugin::HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg)
{
	static std::atomic<int> count {0};

	PLOG(logINFO) << "New MAP: " << msg;

	int mapCount = count;
	SetStatus("ReceivedMaps", mapCount);
}

void PreemptionPlugin::HandleDecodedBsmMessage(DecodedBsmMessage &msg, routeable_message &routeableMsg)
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

void PreemptionPlugin::HandleDataChangeMessage(DataChangeMessage &msg, routeable_message &routeableMsg)
{
	PLOG(logINFO) << "Received a data change message: " << msg;

	PLOG(logINFO) << "Data field " << msg.get_untyped(msg.Name, "?") <<
			" has changed from " << msg.get_untyped(msg.OldValue, "?") <<
			" to " << msg.get_untyped(msg.NewValue, to_string(_frequency));
}

void PreemptionPlugin::HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg) {

	PLOG(logDEBUG)<<"HandleBasicSafetyMessage";

	mp->VehicleLocatorWorker(&msg);
}

void PreemptionPlugin::GetInt32(unsigned char *buf, int32_t *value) {
	*value = (int32_t)((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
}

int PreemptionPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	uint msCount = 0;
	while (_plugin->state != IvpPluginState_error)
	{
		if(mp->map == nullptr){
			PLOG(logINFO) << "loading map ... " << std::endl;
			mp->ProcessMapMessageFile(map_path);
		}

		mp->ip_with_port = ipwithport;
		mp->snmp_version = SNMP_VERSION_1;
		mp->snmp_community = snmp_community;
		mp->base_preemption_oid = BasePreemptionOid;

		// BsmMessage msg_1;
		// BsmMessage &msg = msg_1;
		// mp->VehicleLocatorWorker(&msg);

		// PLOG(logDEBUG4) << "Sleeping 1 ms" << endl;

		// this_thread::sleep_for(chrono::milliseconds(10));

		msCount += 10;

		if (_plugin->state == IvpPluginState_registered && _frequency <= msCount)
		{
			// PLOG(logINFO) << _frequency << " ms wait is complete.";

			this_thread::sleep_for(chrono::milliseconds(100));

			msCount = 0;
		}
	}

	PLOG(logINFO) << "Plugin terminating gracefully.";
	return EXIT_SUCCESS;
}

} /* namespace PreemptionPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<PreemptionPlugin::PreemptionPlugin>("PreemptionPlugin", argc, argv);
}
