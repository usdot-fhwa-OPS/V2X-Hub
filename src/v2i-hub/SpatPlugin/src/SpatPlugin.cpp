#include "SpatPlugin.h"


using namespace std;
using namespace tmx::messages;
using namespace tmx::utils;

namespace SpatPlugin {

	SpatPlugin::SpatPlugin(string name) :PluginClientClockAware(name) {
		spatReceiverThread = std::make_unique<tmx::utils::ThreadTimer>(std::chrono::milliseconds(5));

		if ( PluginClientClockAware::isSimulationMode() ) {
			SubscribeToMessages();
		}
	}

	SpatPlugin::~SpatPlugin() {

	}

	void SpatPlugin::UpdateConfigSettings() {

		if (this->IsPluginState(IvpPluginState_registered)) {
			std::string signalGroupMappingJson;
			std::string ip_address;
			unsigned int port;
			std::string scIp;
			unsigned int scSNMPPort;
			std::string intersectionName;
			unsigned int intersectionId;
			GetConfigValue<string>("SignalGroupMapping", signalGroupMappingJson, &data_lock);
			GetConfigValue<std::string>("Local_IP", ip_address, &data_lock);
			GetConfigValue<unsigned int>("Local_UDP_Port", port, &data_lock);
			GetConfigValue<string>("TSC_IP", scIp, &data_lock);
			GetConfigValue<unsigned int>("TSC_Remote_SNMP_Port", scSNMPPort,&data_lock);
			GetConfigValue<string>("Intersection_Name", intersectionName,&data_lock);
			GetConfigValue<unsigned int>("Intersection_Id", intersectionId, &data_lock);
			if (scConnection) {
				scConnection.reset(new SignalControllerConnection(ip_address, port, signalGroupMappingJson, scIp, scSNMPPort, intersectionName, intersectionId));
			}
			else {
				scConnection = std::make_unique<SignalControllerConnection>(ip_address, port, signalGroupMappingJson, scIp, scSNMPPort, intersectionName, intersectionId);
			}
			auto connected = scConnection->initializeSignalControllerConnection();
			if  ( connected ) {
				SetStatus(keyConnectionStatus, "IDLE");

				spatReceiverThread->AddPeriodicTick([this]()
					{
						this->processSpat();
					}, // end of lambda expression
					std::chrono::milliseconds(5));
				spatReceiverThread->Start();
			}
			else {

			}
		}
	}

	void SpatPlugin::processSpat() {
		if (this->scConnection ) {
			PLOG(tmx::utils::logDEBUG)  << "Processing SPAT ... " << std::endl;
			auto spatMessage = scConnection->receiveSPAT(PluginClientClockAware::getClock()->nowInMilliseconds());
			spatMessage.set_flags(IvpMsgFlags_RouteDSRC);
			spatMessage.addDsrcMetadata(0x8002);
			BroadcastMessage(static_cast<routeable_message &>(spatMessage));
			SetStatus(keyConnectionStatus, "CONNECTED");

		}
	}
	void SpatPlugin::OnConfigChanged(const char *key, const char *value) {
		PluginClientClockAware::OnConfigChanged(key, value);
		UpdateConfigSettings();
	}

	void SpatPlugin::OnStateChange(IvpPluginState state) {
		PluginClientClockAware::OnStateChange(state);

		if (state == IvpPluginState_registered) {
			UpdateConfigSettings();
		}
	}


}
/* End namespace SpatPlugin */

int main(int argc, char *argv[]) {
	return run_plugin<SpatPlugin::SpatPlugin>("SpatPlugin", argc, argv);
}
