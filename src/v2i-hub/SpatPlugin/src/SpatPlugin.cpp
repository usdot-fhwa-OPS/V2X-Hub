/**
 * Copyright (C) 2024 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
#include "SpatPlugin.h"


using namespace std;
using namespace tmx::messages;
using namespace tmx::utils;

namespace SpatPlugin {

	SpatPlugin::SpatPlugin(const std::string &name) :PluginClientClockAware(name) {
		if (isSimulationMode()) {
			// Subscribe to the TimeSyncMessage
			SubscribeToMessages();
		}
		spatReceiverThread = std::make_unique<tmx::utils::ThreadTimer>(std::chrono::milliseconds(5));
	}

	SpatPlugin::~SpatPlugin() {

	}

	void SpatPlugin::UpdateConfigSettings() {

		if (this->IsPluginState(IvpPluginState_registered)) {
			std::string signal_group_mapping_json;
			std::string ip_address;
			unsigned int port;
			std::string signal_controller_ip;
			unsigned int signal_controller_snmp_port;
			std::string signal_controller_snmp_community;
			std::string intersection_name;
			unsigned int intersection_id;
			GetConfigValue<std::string>("SignalGroupMapping", signal_group_mapping_json, &data_lock);
			GetConfigValue<std::string>("Local_IP", ip_address, &data_lock);
			GetConfigValue<unsigned int>("Local_UDP_Port", port, &data_lock);
			GetConfigValue<std::string>("TSC_IP", signal_controller_ip, &data_lock);
			GetConfigValue<unsigned int>("TSC_SNMP_Port", signal_controller_snmp_port,&data_lock);
			GetConfigValue<std::string>("TSC_SNMP_Community", signal_controller_snmp_community,&data_lock);

			GetConfigValue<std::string>("Intersection_Name", intersection_name,&data_lock);
			GetConfigValue<unsigned int>("Intersection_Id", intersection_id, &data_lock);
			std::string spat_string;
			GetConfigValue<std::string>("SPAT_Mode", spat_string, &data_lock);
			spatMode = spat_mode_from_string(spat_string);
			if (spatMode == SPAT_MODE::UNKNOWN) {
				tmx::messages::TmxEventLogMessage eventLogMsg;
				std::string description = "Unknown configured SPAT format" + spat_string + ". Accepted values are SPAT and TSCBM";
				eventLogMsg.set_level(IvpLogLevel::IvpLogLevel_warn);
				eventLogMsg.set_description(description);
				BroadcastMessage(eventLogMsg);
			}
			if (scConnection) {
				scConnection.reset(new SignalControllerConnection(ip_address, port, signal_group_mapping_json, signal_controller_ip, signal_controller_snmp_port, signal_controller_snmp_community ,intersection_name, intersection_id));
			}
			else {
				scConnection = std::make_unique<SignalControllerConnection>(ip_address, port, signal_group_mapping_json, signal_controller_ip, signal_controller_snmp_port,signal_controller_snmp_community, intersection_name, intersection_id);
			}
			// TODO: SNMP OID set call is now only required for old physical controllers and varies by controller. Implement more permanent fix that allows user
			// to define an OID and value to set or none at all
			auto connected = scConnection->initializeSignalControllerConnection(false);
			if  ( connected ) {
				SetStatus<std::string>(keyConnectionStatus, CONNECTION_STATUS_IDLE);
				try {
					spatReceiverThread->AddPeriodicTick([this]()
							{
								this->processTSCPacket();
								if (!this->isConnected) {
									SetStatus<std::string>(keyConnectionStatus, CONNECTION_STATUS_HEALTHY);
									this->isConnected = true;
								}
							}, // end of lambda expression
							std::chrono::milliseconds(5)
					);
					
					spatReceiverThread->Start();
				}
				catch (const TmxException &e) {
					PLOG(tmx::utils::logERROR) << "Encountered error " << e.what() << " during SPAT Processing." << std::endl
											   << e.GetBacktrace();
					SetStatus<std::string>(keyConnectionStatus, CONNECTION_STATUS_DISONNECTED); 
					this->isConnected = false;

				}
			}
			else {
				PLOG(tmx::utils::logERROR) << "Traffic Signal Controller at " << signal_controller_ip << ":" << signal_controller_snmp_port << " failed!";
				SetStatus<std::string>(keyConnectionStatus, CONNECTION_STATUS_DISONNECTED);
				this->isConnected = false;

			}
		}
	}

	void SpatPlugin::processTSCPacket() {
		if (this->scConnection ) {
			PLOG(tmx::utils::logDEBUG)  << "Processing TSC Packet ... " << std::endl;
			try {
				switch (spatMode)
				{
				case SPAT_MODE::SPAT:
					processSpat();
					break;
				case SPAT_MODE::TSCBM:
					processTSCBM();
					break;		
				default:
					PLOG(tmx::utils::logWARNING) << "Configured SPAT_MODE is unknown. Attempting to process SPAT as TSCBM. Update configuration value to valid values SPAT or TSCBM!";
					processTSCBM();					
					
				}
				measureSpatInterval();
				auto status = this->scConnection->getIntersectionStatus();
				for (const auto & [key, value]: status) {
					SetStatus<bool>(key.c_str(), value);
				}
				
			}
			catch (const UdpServerRuntimeError &e) {
				PLOG(tmx::utils::logWARNING) << "Encountered UDP Server Runtime Error" << e.what() << " attempting to process SPAT." << std::endl
										   << e.GetBacktrace();
				TmxEventLogMessage msg(e, "SPAT Plugin Traffic Signal Controller UNHEALTHY", true);
				BroadcastMessage(msg);
				SetStatus<std::string>(keyConnectionStatus, CONNECTION_STATUS_UNHEALTHY);
			}
			catch (const tmx::TmxException &e) {
				PLOG(tmx::utils::logERROR) << "Encountered Tmx Exception " << e.what() << " attempting to process SPAT." << std::endl
										   << e.GetBacktrace();
				skippedMessages++;
				SetStatus<uint>(keySkippedMessages, skippedMessages);
			}
			
		}
	}

	void SpatPlugin::processSpat() {
		PLOG(logDEBUG) << "Attempting to process packet as SPAT ...";
		auto spatEncoded_ptr = std::make_shared<tmx::messages::SpatEncodedMessage>();
		scConnection->receiveUPERSPAT(spatEncoded_ptr);
		spatEncoded_ptr->set_flags(IvpMsgFlags_RouteDSRC);
		spatEncoded_ptr->addDsrcMetadata(tmx::messages::api::msgPSID::signalPhaseAndTimingMessage_PSID);
		auto rMsg = dynamic_cast<routeable_message *>(spatEncoded_ptr.get());
		BroadcastMessage(*rMsg);
	}

	void SpatPlugin::processTSCBM() {
		PLOG(logDEBUG) << "Attempting to process package as TSCBM...";
		auto spat_ptr = (SPAT*)calloc(1, sizeof(SPAT));
		tmx::messages::SpatEncodedMessage spatEncoded_ptr;
		scConnection->receiveBinarySPAT(spat_ptr, PluginClientClockAware::getClock()->nowInMilliseconds());
		// SpatMessage and SpatEncodeMsg assume responsibilty for SPAT pointers (see constructor documentation for TmxJ2735Message and TmxJ2735EncodedMessage)
		tmx::messages::SpatMessage _spatMessage(spat_ptr);
		spatEncoded_ptr.initialize(_spatMessage);
		spatEncoded_ptr.addDsrcMetadata(tmx::messages::api::msgPSID::signalPhaseAndTimingMessage_PSID);
		spatEncoded_ptr.set_flags(IvpMsgFlags_RouteDSRC);
		auto rMsg = dynamic_cast<routeable_message*>(&spatEncoded_ptr);
		BroadcastMessage(*rMsg);

	}
	

	void SpatPlugin::measureSpatInterval() {
		// Measure interval between SPAT messages
		if ( lastSpatTimeMs != 0 ) {
			uint64_t currentTimeMs = PluginClientClockAware::getClock()->nowInMilliseconds();
			try {
				uint intervalMs = SignalControllerConnection::calculateSPaTInterval(lastSpatTimeMs, currentTimeMs);
				if ( intervalMs > maxSpatIntervalMs ) {
					maxSpatIntervalMs = intervalMs;
					SetStatus<uint>(keySpatMaxInterval, intervalMs);
					
				}
			}
			catch(const TmxException &e) {
				tmx::messages::TmxEventLogMessage msg(e);
				BroadcastMessage(msg);
				SetStatus<std::string>(keyConnectionStatus, CONNECTION_STATUS_UNHEALTHY);
				maxSpatIntervalMs= 301;
				SetStatus<uint>(keySpatMaxInterval, maxSpatIntervalMs);

			}
			lastSpatTimeMs = currentTimeMs;
		}
		else {
			lastSpatTimeMs = PluginClientClockAware::getClock()->nowInMilliseconds();
		}
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


}
/* End namespace SpatPlugin */

int main(int argc, char *argv[]) {
	return run_plugin<SpatPlugin::SpatPlugin>("SpatPlugin", argc, argv);
}
