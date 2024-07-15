#include "SpatPlugin.h"


using namespace std;
using namespace tmx::messages;
using namespace tmx::utils;

namespace SpatPlugin {

	SpatPlugin::SpatPlugin(const std::string &name) :PluginClientClockAware(name) {
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
			GetConfigValue<std::string>("SignalGroupMapping", signalGroupMappingJson, &data_lock);
			GetConfigValue<std::string>("Local_IP", ip_address, &data_lock);
			GetConfigValue<unsigned int>("Local_UDP_Port", port, &data_lock);
			GetConfigValue<std::string>("TSC_IP", scIp, &data_lock);
			GetConfigValue<unsigned int>("TSC_Remote_SNMP_Port", scSNMPPort,&data_lock);
			GetConfigValue<std::string>("Intersection_Name", intersectionName,&data_lock);
			GetConfigValue<unsigned int>("Intersection_Id", intersectionId, &data_lock);
			GetConfigValue<std::string>("SPAT_Mode", spatMode, &data_lock);
			
			if (scConnection) {
				scConnection.reset(new SignalControllerConnection(ip_address, port, signalGroupMappingJson, scIp, scSNMPPort, intersectionName, intersectionId));
			}
			else {
				scConnection = std::make_unique<SignalControllerConnection>(ip_address, port, signalGroupMappingJson, scIp, scSNMPPort, intersectionName, intersectionId);
			}
			auto connected = scConnection->initializeSignalControllerConnection();
			if  ( connected ) {
				SetStatus(keyConnectionStatus, "IDLE");
				try {
					spatReceiverThread->AddPeriodicTick([this]()
							{
								this->processSpat();
								if (!this->isConnected) {
									SetStatus(keyConnectionStatus, "CONNECTED");
									this->isConnected = true;
								}
							}, // end of lambda expression
							std::chrono::milliseconds(5)
					);
					PluginClientClockAware::getClock()->wait_for_initialization();
					spatReceiverThread->Start();
				}
				catch (const TmxException &e) {
					PLOG(tmx::utils::logERROR) << "Encountered error " << e.what() << " during SPAT Processing." << std::endl
											   << e.GetBacktrace();
					SetStatus(keyConnectionStatus, "ERROR"); 
					this->isConnected = false;

				}
			}
			else {
				PLOG(tmx::utils::logERROR) << "Traffic Signal Controller at " << scIp << ":" << scSNMPPort << " failed!";
				SetStatus(keyConnectionStatus, "DISCONNECTED");
				this->isConnected = false;

			}
		}
	}

	void SpatPlugin::processSpat() {
		if (this->scConnection ) {
			PLOG(tmx::utils::logDEBUG)  << "Processing SPAT ... " << std::endl;
			try {
				if (spatMode == "BINARY")
				{
					auto spat_ptr = std::make_shared<SPAT>();
					scConnection->receiveBinarySPAT(spat_ptr, PluginClientClockAware::getClock()->nowInMilliseconds());
					tmx::messages::SpatMessage _spatMessage(spat_ptr);
					auto spatEncoded_ptr = std::make_shared<tmx::messages::SpatEncodedMessage>();
					spatEncoded_ptr->initialize(_spatMessage,"", 0U, IvpMsgFlags_RouteDSRC);
					spatEncoded_ptr->addDsrcMetadata(tmx::messages::api::msgPSID::signalPhaseAndTimingMessage_PSID);
					auto rMsg = dynamic_cast<routeable_message*>(spatEncoded_ptr.get());
					BroadcastMessage(*rMsg);
				}
				else if (spatMode == "J2735_HEX") {
					auto spatEncoded_ptr = std::make_shared<tmx::messages::SpatEncodedMessage>();
					scConnection->receiveUPERSPAT(spatEncoded_ptr);
					spatEncoded_ptr->set_flags(IvpMsgFlags_RouteDSRC);
					spatEncoded_ptr->addDsrcMetadata(tmx::messages::api::msgPSID::signalPhaseAndTimingMessage_PSID);
					auto rMsg = dynamic_cast<routeable_message *>(spatEncoded_ptr.get());
					BroadcastMessage(*rMsg);	
				}
				else {
					throw TmxException("SPAT Mode " + spatMode + " is not supported. Support SPAT Modes are J2735_HEX and BINARY.");
				}
			}
			catch (const tmx::J2735Exception &e) {
				PLOG(tmx::utils::logERROR) << "Encountered J2735 Exception " << e.what() << " attempting to process SPAT." << std::endl
										   << e.GetBacktrace();
				skippedMessages++;
				SetStatus<uint>(keySkippedMessages, skippedMessages);
			}
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
