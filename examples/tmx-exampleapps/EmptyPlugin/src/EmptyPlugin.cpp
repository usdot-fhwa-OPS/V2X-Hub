//============================================================================
// Name        : EpcwPlugin.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <time.h>
#include <sys/time.h>
#include <atomic>
#include <tmx/tmx.h>
#include <tmx/IvpPlugin.h>
#include <tmx/messages/IvpBattelleDsrc.h>
#include <tmx/messages/IvpSignalControllerStatus.h>
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <tmx/messages/IvpJ2735.h>
#include <GeoVector.h>

//#include "XmlMapParser.h"
//#include "ConvertToJ2735r41.h"
#include "PluginClient.h"

/*You cannot go to eclipse project properties and Path&Symbols and add Common to the Include directories and then hope
 * to clear up include errors.
 * This WOULD work if Eclipse was managing the makefile - it would add a -I compile flag with 'Common' added to the include
 * directories.
 * But we have disabled eclipse from doing this - instead cmake manages the makefile - and the only -I in there is /usr/local/include
 * that has the ivpapi libraries.
 * 4/11/16: makefile now accounts for Common. Above still true if trying to link to some other new directory.
 * */

#include <ApplicationMessage.h>
#include <ApplicationDataMessage.h>
#include <VehicleBasicMessage.h>
#include <LocationMessage.h>
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <tmx/j2735_messages/SpatMessage.hpp>
#include <tmx/messages/auto_message.hpp>

#include <Intersection.h>
#include <IntersectionList.h>
#include <Roadway.h>
#include <AppAlert.h>
#include <algorithm>
#include <tmx/messages/message_document.hpp>
#include <pugixml.hpp>
#include <ParsedMap.h>
#include <MapSupport.h>
#include <sstream>
#include <vector>
using boost::property_tree::ptree;
using namespace pugi;

using namespace std;
//using namespace etrp;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;

namespace EmptyPlugin {
#define INPUTSREADY "Have Map/Spat/Veh"
#define INGRESSREGION "Ingress Region"
#define EGRESSREGION "Egress Region"


class EmptyPlugin: public PluginClient {
public:
	EmptyPlugin(std::string);
	virtual ~EmptyPlugin();
	int Main();
protected:

	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	//void OnMessageReceived(IvpMessage *msg);
	void OnStateChange(IvpPluginState state);

	// Message handler functions
	/*void handleVehicleBasicMessage(VehicleBasicMessage &msg,
			routeable_message &routeableMsg);
	void handleLocationMessage(LocationMessage &msg,
			routeable_message &routeableMsg);
	void HandleMapDataMessage(MapDataMessage &msg,
			routeable_message &routeableMsg);*/
	void HandleSpatMessage(SpatMessage &msg, routeable_message &routeableMsg);

private:
	
};

/**
 * Construct a new EpcwPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes
 */
EmptyPlugin::EmptyPlugin(string name) :
		PluginClient(name) {
	//We want to listen for Vehicle Basic messages.
	/*AddMessageFilter < VehicleBasicMessage
			> (this, &EmptyPlugin::handleVehicleBasicMessage);

	//We want to listen for location messages
	AddMessageFilter < LocationMessage
			> (this, &EmptyPlugin::handleLocationMessage);

	//We want to listen for Map/Spat Messages
	AddMessageFilter < MapDataMessage
			> (this, &EmptyPlugin::HandleMapDataMessage);*/
	AddMessageFilter < SpatMessage > (this, &EmptyPlugin::HandleSpatMessage);
	//AddMessageFilter(SpatMessage::MessageType, SpatMessage::MessageSubType);

	SubscribeToMessages();

}

EmptyPlugin::~EmptyPlugin() {
}

void EmptyPlugin::UpdateConfigSettings() {
	// The below code shows an example of how to retrieve a configuration
	// value using the IVP API.
	// In this case, there is a configuration value named Frequency.
	// All configuration values are retrieved as strings,
	// then must be converted as appropriate.
	/*GetConfigValue < uint64_t > ("Frequency", _frequencySetting);
	GetConfigValue < uint64_t > ("LocationMsgExpiration", _locationMsgExpireMs);
	GetConfigValue < uint64_t > ("VehicleMsgExpiration", _vehicleMsgExpireMs);
	GetConfigValue < uint64_t > ("SpatMsgExpiration", _spatMsgExpireMs);
	GetConfigValue < uint64_t > ("Sleep", _sleepUs);
	GetConfigValue<int>("SlowDistanceToAlertM", _SlowDistanceToAlertM);
	GetConfigValue<int>("MediumDistanceToAlertM", _MediumDistanceToAlertM);
	GetConfigValue<int>("FastDistanceToAlertM", _FastDistanceToAlertM);
*/
}

void EmptyPlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

/*void EmptyPlugin::HandleMapDataMessage(MapDataMessage &msg,
		routeable_message &routeableMsg) {

}
*/
void EmptyPlugin::HandleSpatMessage(SpatMessage &msg,
		routeable_message &routeableMsg) {
	FILE_LOG(logINFO) << "In HandleSpatMessage";
	cout << Clock::ToUtcPreciseTimeString(routeableMsg.get_timestamp()) << endl;
	routeableMsg.refresh_timestamp();
	cout << Clock::ToUtcPreciseTimeString(routeableMsg.get_timestamp()) << endl;
}
/*
void EmptyPlugin::handleVehicleBasicMessage(VehicleBasicMessage &msg,
		routeable_message &routeableMsg) {
}

void EmptyPlugin::handleLocationMessage(LocationMessage &msg,
		routeable_message &routeableMsg) {

}
*/
void EmptyPlugin::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered) {
		UpdateConfigSettings();
	}
}

/*
void EmptyPlugin::OnMessageReceived(IvpMessage *msg) {
	FILE_LOG(logINFO) << msg->subtype << " received.";

	if (IsMessageOfType<SpatMessage>(msg)) {
		FILE_LOG(logINFO) << "Found SPAT";

		routeable_message routeableMsg(msg);
		routeableMsg.refresh_timestamp();
		cout << Clock::ToUtcPreciseTimeString(routeableMsg.get_timestamp()) << endl; //routeableMsg.get_payload_str();

		//char *ser = ivpMsg_createJsonString(msg, IvpMsg_FormatOptions_none);
		//routeableMsg.refresh_timestamp();
		//cout << Clock::ToUtcPreciseTimeString(routeableMsg.get_timestamp()) << endl;
	}
}
*/
int EmptyPlugin::Main() {
	FILE_LOG(logINFO) << "Starting plugin.";

	uint64_t lastSendTime = 0;

	while (_plugin->state != IvpPluginState_error) {
		


		usleep(100000); //sleep for microseconds set from config.
	}

	return (EXIT_SUCCESS);
}
} /* namespace EpcwPlugin */

int main(int argc, char *argv[]) {
	return run_plugin < EmptyPlugin::EmptyPlugin > ("EmptyPlugin", argc, argv);
}
