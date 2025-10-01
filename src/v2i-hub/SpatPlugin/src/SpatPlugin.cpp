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
			GetConfigValue<std::string>("SPAT_Mode", spatMode, &data_lock);
			
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
					
					spatReceiverThread->Start();
				}
				catch (const TmxException &e) {
					PLOG(tmx::utils::logERROR) << "Encountered error " << e.what() << " during SPAT Processing." << std::endl
											   << e.GetBacktrace();
					SetStatus(keyConnectionStatus, "DISCONNECTED"); 
					this->isConnected = false;

				}
			}
			else {
				PLOG(tmx::utils::logERROR) << "Traffic Signal Controller at " << signal_controller_ip << ":" << signal_controller_snmp_port << " failed!";
				SetStatus(keyConnectionStatus, "DISCONNECTED");
				this->isConnected = false;

			}
		}
	}

	void SpatPlugin::processSpat() {
		if (this->scConnection ) {
			PLOG(tmx::utils::logDEBUG)  << "Processing SPAT ... " << std::endl;
			try {
				
				if (spatMode == "J2735_HEX") {
					PLOG(logDEBUG) << "Starting HEX SPaT Receiver ...";
					auto spatEncoded_ptr = std::make_shared<tmx::messages::SpatEncodedMessage>();
					scConnection->receiveUPERSPAT(spatEncoded_ptr);
					spatEncoded_ptr->set_flags(IvpMsgFlags_RouteDSRC);
					spatEncoded_ptr->addDsrcMetadata(tmx::messages::api::msgPSID::signalPhaseAndTimingMessage_PSID);
					auto rMsg = dynamic_cast<routeable_message *>(spatEncoded_ptr.get());
					BroadcastMessage(*rMsg);	
				}
				else {
					if ( spatMode != "BINARY"){
						PLOG(tmx::utils::logWARNING) << spatMode << " is an unsupport SPAT MODE. Defaulting to BINARY. Supported options are BINARY and J2735_HEX";
					}
					auto spat_ptr = (SPAT*)calloc(1, sizeof(SPAT));
					PLOG(logDEBUG) << "Starting BINARY SPaT Receiver ...";
					auto spatEncoded_ptr = std::make_shared<tmx::messages::SpatEncodedMessage>();
					scConnection->receiveBinarySPAT(spat_ptr, PluginClientClockAware::getClock()->nowInMilliseconds());
					
					tmx::messages::SpatMessage _spatMessage(*spat_ptr);
					MessageFrameMessage frame(_spatMessage.get_j2735_data());
					spatEncoded_ptr->set_data(TmxJ2735EncodedMessage<SPAT>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame));
					spatEncoded_ptr->addDsrcMetadata(tmx::messages::api::msgPSID::signalPhaseAndTimingMessage_PSID);
					auto rMsg = dynamic_cast<routeable_message*>(spatEncoded_ptr.get());
					BroadcastMessage(_spatMessage);
					// Recursively free SPAT struct 
					ASN_STRUCT_FREE(asn_DEF_SPAT, spat_ptr);
					// TODO fix MessageFrameMessage destructor to properly free internal SPAT pointer
					free(frame.get_j2735_data().get());

				
				}
			}
			catch (const UdpServerRuntimeError &e) {
				PLOG(tmx::utils::logWARNING) << "Encountered UDP Server Runtime Error" << e.what() << " attempting to process SPAT." << std::endl
										   << e.GetBacktrace();
			}
			catch (const tmx::TmxException &e) {
				PLOG(tmx::utils::logERROR) << "Encountered Tmx Exception " << e.what() << " attempting to process SPAT." << std::endl
										   << e.GetBacktrace();
				skippedMessages++;
				SetStatus<uint>(keySkippedMessages, skippedMessages);
			}
			
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
