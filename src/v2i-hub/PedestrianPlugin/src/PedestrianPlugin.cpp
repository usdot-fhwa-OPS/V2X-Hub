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
	PsmEncodedMessage psmENC;
	tmx::message_container_type container;
	std::unique_ptr<PsmEncodedMessage> msg;

	container.load<XML>("/PSM.xml");
	PLOG(logINFO) << "loaded data";
	psmmessage.set_contents(container.get_storage().get_tree());
	PLOG(logDEBUG) << "Encoding " << psmmessage;
	psmENC.encode_j2735_message(psmmessage);

	msg.reset();
	msg.reset(dynamic_cast<PsmEncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_PERSONALSAFETYMESSAGE_STRING)));

	string enc = psmENC.get_encoding();
	msg->set_payload(psmENC.get_payload_str());
	msg->set_encoding(enc);
	msg->set_flags(IvpMsgFlags_RouteDSRC);
	msg->addDsrcMetadata(172, 0x8002);
	msg->refresh_timestamp();

	routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
	BroadcastMessage(*rMsg);

	PLOG(logINFO) << " sending PSM " << psmENC.get_payload_str();

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
