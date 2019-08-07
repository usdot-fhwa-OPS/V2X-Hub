//==========================================================================
// Name        : PedestrianPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
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


void PedestrianPlugin::HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg) {
	PLOG(logDEBUG)<<"HandleBasicSafetyMessage";
}

void PedestrianPlugin::BroadcastPsm(PersonalSafetyMessage &psm) {
	PLOG(logDEBUG)<<"Broadcasting PSM";

	PsmMessage psmmessage;
	tmx::message_container_type container;
	container.load<XML>("/PSM.xml");
	psmmessage.set_contents(container.get_storage().get_tree());
	PLOG(logDEBUG) << "Encoding " << psmmessage;
	PsmEncodedMessage psmENC;
	psmENC.encode_j2735_message(psmmessage);
	// mapFile.set_Bytes(mapEnc.get_payload_str());

	PLOG(logINFO) << " XML file encoded as " << psmENC.get_payload_str();

}

int PedestrianPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	uint msCount = 0;
	while (_plugin->state != IvpPluginState_error)
	{

		msCount += 10;

		if (_plugin->state == IvpPluginState_registered)
		{
			PersonalSafetyMessage psm_1;
			PersonalSafetyMessage &psm = psm_1;
			BroadcastPsm(psm);

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
