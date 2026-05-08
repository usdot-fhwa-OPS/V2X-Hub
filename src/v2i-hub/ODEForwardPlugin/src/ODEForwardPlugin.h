
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

#ifndef TMX_PLUGINS_ODEForwardPlugin_H_
#define TMX_PLUGINS_ODEForwardPlugin_H_

#include <PluginClient.h>
#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <atomic>
#include <thread>
#include <boost/algorithm/string.hpp>
#include <tmx/messages/IvpJ2735.h>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <tmx/j2735_messages/SpatMessage.hpp>
#include <tmx/j2735_messages/TravelerInformationMessage.hpp>
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <BasicSafetyMessage.h>
#include <tmx/messages/auto_message.hpp>
#include <tmx/json/cJSON.h>
#include <environment/EnvUtils.h>
#include "UDPMessageForwarder.h"

 


namespace ODEForwardPlugin
{

	/**
	 * This plugin logs the BSM messages received in the following CSV format.
	 */
	class ODEForwardPlugin: public tmx::utils::PluginClient
	{
		public:
			explicit ODEForwardPlugin(const std::string &name);
			~ODEForwardPlugin() override = default;
		protected:
			void UpdateConfigSettings();

			// Virtual method overrides.
			void OnConfigChanged(const char *key, const char *value) override;
			void OnStateChange(IvpPluginState state) override;

			void HandleRealTimePublish(tmx::messages::BsmMessage &msg, tmx::routeable_message &routeableMsg);
			void HandleSPaTPublish(tmx::messages::SpatMessage &msg, tmx::routeable_message &routeableMsg);
			void HandleTimPublish(tmx::messages::TimMessage &msg, tmx::routeable_message &routeableMsg);
			void HandleMapPublish(tmx::messages::MapDataMessage &msg, tmx::routeable_message &routeableMsg);

		private:

			void sendUDPMessage(tmx::routeable_message &routeableMsg, UDPMessageType udpMessageType) const;

			uint16_t _scheduleFrequency;
			uint16_t _freqCounter;
			uint16_t _forwardMSG;
			int _MAPUDPPort;
			int _TIMUDPPort;
			int _BSMUDPPort;
			int _SPATUDPPort;
			std::string _udpServerIpAddress;
			std::shared_ptr<UDPMessageForwarder> _udpMessageForwarder;
			std::mutex _cfgLock;

	};


} /* namespace ODEForwardPlugin */

#endif /* ODEForwardPlugin.h */
