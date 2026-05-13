
/**
 * Copyright (C) 2019 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this plogFile except in compliance with the License. You may obtain a copy of
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


#include "ODEForwardPlugin.h"
using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
namespace ODEForwardPlugin
{
			
 /**
  * Construct a new ODEForwardPlugin with the given name.
  *
  * @param name The name to give the plugin for identification purposes.
  */
	ODEForwardPlugin::ODEForwardPlugin(const string &name): PluginClient(name)
	{
		AddMessageFilter < BsmMessage > (this, &ODEForwardPlugin::HandleRealTimePublish);
		AddMessageFilter < SpatMessage > (this, &ODEForwardPlugin::HandleSPaTPublish);
		AddMessageFilter < MapDataMessage > (this, &ODEForwardPlugin::HandleMapPublish);
		AddMessageFilter < TimMessage > (this, &ODEForwardPlugin::HandleTimPublish);
		// Subscribe to all messages specified by the filters above.
		SubscribeToMessages();
		_udpMessageForwarder = std::make_shared<UDPMessageForwarder>();
	}


	/**
	 * Updates configuration settings
	 */
	void ODEForwardPlugin::UpdateConfigSettings()
	{
		// Configuration settings are retrieved from the API using the GetConfigValue template class.
		// This method does NOT execute in the main thread, so variables must be protected
		// (e.g. using std::atomic, std::mutex, etc.).

		int instance;
		GetConfigValue("Instance", instance);
		std::scoped_lock lock(_cfgLock);
		GetConfigValue<uint16_t>("schedule_frequency", _scheduleFrequency);
		GetConfigValue<uint16_t>("ForwardMSG", _forwardMSG);
		GetConfigValue<int>("MAPUDPPort", _MAPUDPPort);
		GetConfigValue<int>("TIMUDPPort", _TIMUDPPort);
		GetConfigValue<int>("BSMUDPPort", _BSMUDPPort);
		GetConfigValue<int>("SPATUDPPort", _SPATUDPPort);
		_udpServerIpAddress = tmx::utils::environment::get_local_ip();
		//Update communication mode

		

		//Create UDP clients for different messages
		_udpMessageForwarder->attachUdpClient(UDPMessageType::BSM, std::make_shared<UdpClient>(_udpServerIpAddress, _BSMUDPPort));
		_udpMessageForwarder->attachUdpClient(UDPMessageType::MAP, std::make_shared<UdpClient>(_udpServerIpAddress, _MAPUDPPort));
		_udpMessageForwarder->attachUdpClient(UDPMessageType::TIM, std::make_shared<UdpClient>(_udpServerIpAddress, _TIMUDPPort));
		_udpMessageForwarder->attachUdpClient(UDPMessageType::SPAT, std::make_shared<UdpClient>(_udpServerIpAddress, _SPATUDPPort));
		
	}

	/**
	 * Called when configuration is changed
	 *
	 * @param key Key of the configuration value changed
	 * @param value Changed value
	 */
	void ODEForwardPlugin::OnConfigChanged(const char *key, const char *value)
	{
		PluginClient::OnConfigChanged(key, value);
		UpdateConfigSettings();
	}

	/**
	 * Called on plugin state change
	 *
	 * @para state New plugin state
	 */
	void ODEForwardPlugin::OnStateChange(IvpPluginState state)
	{
		PluginClient::OnStateChange(state);

		if (state == IvpPluginState_registered)
		{
			UpdateConfigSettings();
		}
	}

	/**
	 * Method that's called to process a message that this plugin has
	 * subscribed for.  This particular method decodes the BSM message and
	 * logs selective fields to a log file.
	 *
	 * @param msg BSMMessage that is received
	 * @routeable_message not used
	 */
	void ODEForwardPlugin::HandleRealTimePublish([[maybe_unused]] BsmMessage &msg, routeable_message &routeableMsg) {
		try {
			sendUDPMessage(routeableMsg, UDPMessageType::BSM);
			++_bsmFwdCount;
			SetStatus<uint>("BSM Forwarded", _bsmFwdCount);
		} catch (const tmx::TmxException &e) {
			PLOG(logERROR) << "Failed to forward BSM message: " << e.what();
			++_bsmSkipCount;
			SetStatus<uint>("BSM Skipped", _bsmSkipCount);
		}

	}

	void ODEForwardPlugin::HandleSPaTPublish([[maybe_unused]] SpatMessage &msg, routeable_message &routeableMsg) {
		try {
			sendUDPMessage(routeableMsg, UDPMessageType::SPAT);
			++_spatFwdCount;
			SetStatus<uint>("SPAT Forwarded", _spatFwdCount);
		} catch (const tmx::TmxException &e) {
			PLOG(logERROR) << "Failed to forward SPAT message: " << e.what();
			++_spatSkipCount;
			SetStatus<uint>("SPAT Skipped", _spatSkipCount);
		}
	}

	void ODEForwardPlugin::HandleTimPublish([[maybe_unused]] TimMessage &msg, routeable_message &routeableMsg) {
		try {
			sendUDPMessage(routeableMsg, UDPMessageType::TIM);
			++_timFwdCount;
			SetStatus<uint>("TIM Forwarded", _timFwdCount);
		} catch (const tmx::TmxException &e) {
			PLOG(logERROR) << "Failed to forward TIM message: " << e.what();
			++_timSkipCount;
			SetStatus<uint>("TIM Skipped", _timSkipCount);
		}
	}


	void ODEForwardPlugin::HandleMapPublish([[maybe_unused]] MapDataMessage &msg, routeable_message &routeableMsg) {
		try {
			sendUDPMessage(routeableMsg, UDPMessageType::MAP);
			++_mapFwdCount;
			SetStatus<uint>("MAP Forwarded", _mapFwdCount);
		} catch (const tmx::TmxException &e) {
			PLOG(logERROR) << "Failed to forward MAP message: " << e.what();
			++_mapSkipCount;
			SetStatus<uint>("MAP Skipped", _mapSkipCount);
		}
	}

	void ODEForwardPlugin::sendUDPMessage(routeable_message &routeableMsg, UDPMessageType udpMessageType) const{
		std::string message = routeableMsg.get_payload_str().c_str();
		PLOG(logDEBUG) << "Sending UDP Message: " << message;
		_udpMessageForwarder->sendMessage(udpMessageType, message);
	}


} /* namespace ODEForwardPlugin */


/**
 * Main method for running the plugin
 * @param argc number of arguments
 * @param argv array of arguments
 */
int main(int argc, char *argv[])
{
	return tmx::utils::run_plugin<ODEForwardPlugin::ODEForwardPlugin>("ODEForwardPlugin", argc, argv);
}
