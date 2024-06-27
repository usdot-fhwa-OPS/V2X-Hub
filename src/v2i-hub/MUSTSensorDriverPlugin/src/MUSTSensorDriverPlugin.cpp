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
#include "MUSTSensorDriverPlugin.h"

using namespace tmx::utils;
using namespace std;

namespace MUSTSensorDriverPlugin {
	
	MUSTSensorDriverPlugin::MUSTSensorDriverPlugin(const string &name): PluginClientClockAware(name)
	{
		mustSensorPacketReceiverThread = std::make_unique<tmx::utils::ThreadTimer>(std::chrono::milliseconds(5));
		if (PluginClientClockAware::isSimulationMode()) {
			PLOG(tmx::utils::logINFO) << "Simulation mode on " << std::endl;
			SubscribeToMessages();
		}
	}

	void  MUSTSensorDriverPlugin::OnStateChange(IvpPluginState state) {
		PluginClientClockAware::OnStateChange(state);
		if (state == IvpPluginState_registered) {
			UpdateConfigSettings();
		}else {
			connected = false;
			SetStatus(keyMUSTSensorConnectionStatus, "DISCONNECTED");
		}
	}

	void MUSTSensorDriverPlugin::UpdateConfigSettings()
	{
		// Configuration settings are retrieved from the API using the GetConfigValue template class.
		// This method does NOT execute in the main thread, so variables must be protected
		// (e.g. using std::atomic, std::mutex, etc.).
		if (this->IsPluginState(IvpPluginState_registered)) {
			std::scoped_lock<std::mutex> lock(_configMutex);
			GetConfigValue<std::string>("ProjectionString", projString);
			GetConfigValue<std::string>("SensorId", sensorId);
			// Setup  UDP Server 
			std::string ip_address;
			unsigned int port;
			GetConfigValue<std::string>("DetectionReceiverIP", ip_address);
			GetConfigValue<uint>("DetectionReceiverPort", port);
			createUdpServer(ip_address, port);
			SetStatus(keyMUSTSensorConnectionStatus, "IDLE");

			mustSensorPacketReceiverThreadId = mustSensorPacketReceiverThread->AddPeriodicTick([this]() {
            	this->processMUSTSensorDetection();

       		 	} // end of lambda expression
        		, std::chrono::milliseconds(5) );
        	mustSensorPacketReceiverThread->Start();
		}
	}
	void MUSTSensorDriverPlugin::processMUSTSensorDetection(){
		if (mustSensorPacketReceiver) {
			try {
				PLOG(logDEBUG1) << "Processing MUST Sensor Detection ... " << std::endl;
				MUSTSensorDetection detection = csvToDetection(mustSensorPacketReceiver->stringTimedReceive());
				if ( !connected ) {
					connected = true;
					SetStatus(keyMUSTSensorConnectionStatus, "CONNECTED");
				}
				tmx::messages::SensorDetectedObject msg = mustDetectionToSensorDetectedObject(detection, sensorId, projString);
				PLOG(logDEBUG1) << "Sending Simulated SensorDetectedObject Message " << msg << std::endl;
				this->BroadcastMessage<tmx::messages::SensorDetectedObject>(msg, _name, 0 , IvpMsgFlags_None);
			}
			catch( const tmx::utils::UdpServerRuntimeError &e) {
				PLOG(logERROR) << "Error occurred processing MUSTSensorDetection" << e << std::endl;
				SetStatus(keyMUSTSensorConnectionStatus, "DISCONNECTED");
				connected = false;
			}
			catch ( const tmx::TmxException &e){
				PLOG(logERROR) << "Error occurred processing MUSTSensorDetection" << e.what() << std::endl;
				SetStatus(keyMUSTSensorConnectionStatus, "DISCONNECTED");
				connected = false;
			} 
		}else {
			SetStatus(keyMUSTSensorConnectionStatus, "DISCONNECTED");
			connected = false;
		}
	}

	void MUSTSensorDriverPlugin::createUdpServer(const std::string &address, unsigned int port) {
		if ( mustSensorPacketReceiver ) {
			mustSensorPacketReceiver.reset(new UdpServer(address, port));
		}
		else {
			mustSensorPacketReceiver = std::make_unique<UdpServer>(address, port);
		}
	}

	void MUSTSensorDriverPlugin::OnConfigChanged(const char *key, const char *value)
	{
		PluginClientClockAware::OnConfigChanged(key, value);
		UpdateConfigSettings();
	}


} /* namespace MUSTSensorDriver */

int main(int argc, char *argv[])
{
	return run_plugin<MUSTSensorDriverPlugin::MUSTSensorDriverPlugin>("MUSTSensorDriverPlugin", argc, argv);
}
