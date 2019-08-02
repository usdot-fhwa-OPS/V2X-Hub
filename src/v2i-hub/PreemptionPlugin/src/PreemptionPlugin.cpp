//==========================================================================
// Name        : PreemptionPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     : 1.0
// Copyright   : Copyright (c) 2019 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
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

	AddMessageFilter <BsmMessage> (this, &PreemptionPlugin::HandleBasicSafetyMessage);

	SubscribeToMessages();
}

PreemptionPlugin::~PreemptionPlugin()
{
	if (_signSimClient != NULL)
		delete _signSimClient;
}

void PreemptionPlugin::UpdateConfigSettings()
{

	GetConfigValue("BasePreemptionOid", BasePreemptionOid);
	GetConfigValue("ipwithport", ipwithport);
	GetConfigValue("snmp_community", snmp_community);
	GetConfigValue("map_path", map_path);
	mp->ProcessMapMessageFile(map_path);

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

void PreemptionPlugin::HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg) {

	PLOG(logDEBUG)<<"HandleBasicSafetyMessage";
	if(mp->GeofenceSet.size() < 0) {
		mp->ProcessMapMessageFile(map_path);
	}

	mp->VehicleLocatorWorker(&msg);
}

int PreemptionPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	uint msCount = 0;
	while (_plugin->state != IvpPluginState_error)
	{

		if(mp->GeofenceSet.size() < 1 && map_path != "") {
			mp->ProcessMapMessageFile(map_path);
		}

		mp->ip_with_port = ipwithport;
		mp->snmp_version = SNMP_VERSION_1;
		mp->snmp_community = snmp_community;
		mp->base_preemption_oid = BasePreemptionOid;

		msCount += 10;

		if (_plugin->state == IvpPluginState_registered)
		{

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
