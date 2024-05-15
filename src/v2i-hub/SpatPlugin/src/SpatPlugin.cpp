#include "SpatPlugin.h"

#include <PluginLog.h>

using namespace std;
using namespace tmx::messages;
using namespace tmx::utils;

namespace SpatPlugin {

SpatPlugin::SpatPlugin(string name) :
		PluginClientClockAware(name), sc(getClock()), intersectionId(0) {
	SubscribeToMessages();
}

SpatPlugin::~SpatPlugin() {

}

void SpatPlugin::UpdateConfigSettings() {

	GetConfigValue<string>("SignalGroupMapping", signalGroupMappingJson, &data_lock);
	GetConfigValue<string>("Local_IP", localIp, &data_lock);
	GetConfigValue<string>("Local_UDP_Port", localUdpPort, &data_lock);
	GetConfigValue<string>("TSC_IP", tscIp, &data_lock);
	GetConfigValue<string>("TSC_Remote_SNMP_Port", tscRemoteSnmpPort,
			&data_lock);
	GetConfigValue<string>("Intersection_Name", intersectionName,
			&data_lock);
	GetConfigValue<int>("Intersection_Id", intersectionId, &data_lock);

	isConfigurationLoaded = true;
}

void SpatPlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void SpatPlugin::OnStateChange(IvpPluginState state) {
	PluginClientClockAware::OnStateChange(state);

	if (state == IvpPluginState_registered) {
		UpdateConfigSettings();
	}
}


int SpatPlugin::Main() {

	int iCounter = 0;

	PLOG(logINFO) << "Waiting for clock initialization";
	// wait for the clock to be initialized and record the time when it is ready
	getClock()->wait_for_initialization();
	auto nextSpatTime = getClock()->nowInMilliseconds();
	PLOG(logINFO) << "Initial nextSpatTime=" << nextSpatTime;

	try {
		while (_plugin->state != IvpPluginState_error) {
			// wait to send next message
			if (isConfigurationLoaded) {
				if (!isConfigured) {

					{
						std::lock_guard<std::mutex> lock(data_lock);
						string ptlm = "";
						sc.setConfigs(localIp, localUdpPort, tscIp,
								tscRemoteSnmpPort, intersectionName,
								intersectionId);
					}
					// Start the signal controller thread.
					sc.Start(signalGroupMappingJson);
					// Give the spatdata pointer to the message class
					isConfigured = true;
				}

				// SPaT must be sent exactly every 100 ms.  So adjust for how long it took to do the last send.
				nextSpatTime += 100;
				getClock()->sleep_until(nextSpatTime);

				iCounter++;

				bool messageSent = false;

				if (sc.getIsConnected()) {
					SetStatus<string>("TSC Connection", "Connected");

					// Add pedestrian detection
					string pedZones;
					{
						lock_guard<mutex> lock(data_lock);
						pedZones = _pedMessage.get_DetectionZones();
					}
					if (!pedZones.empty()) {
						PLOG(logDEBUG) << "Pedestrians detected in lanes " << pedZones;
					}

					SpatEncodedMessage spatEncodedMsg;
					

					spatEncodedMsg.set_flags(IvpMsgFlags_RouteDSRC);
					spatEncodedMsg.addDsrcMetadata(0x8002);


					BroadcastMessage(static_cast<routeable_message &>(spatEncodedMsg));

					
				} else {
					SetStatus<string>("TSC Connection", "Disconnected");
				}
			}
		}
	} catch (const exception &ex) {
		stringstream ss;
		PLOG(logERROR) << "SpatPlugin terminating from unhandled exception: " << ex.what();

		ivp_addEventLog(_plugin, IvpLogLevel_error, ex.what().c_str());
		std::terminate();
	}

	return EXIT_SUCCESS;
}

}
/* End namespace SpatPlugin */

int main(int argc, char *argv[]) {
	return run_plugin<SpatPlugin::SpatPlugin>("SpatPlugin", argc, argv);
}
