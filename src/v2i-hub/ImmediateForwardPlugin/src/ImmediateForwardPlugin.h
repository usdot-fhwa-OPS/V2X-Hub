/**
 * Copyright (C) 2025 LEIDOS.
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

#pragma once

#include <atomic>
#include <array>
#include <map>
#include <mutex>
#include <vector>
#include <PluginClient.h>
#include <UdpClient.h>
#include <SNMPClient.h>
#include <sstream>
#include <chrono>
#include <boost/algorithm/hex.hpp>
#include <FrequencyThrottle.h>
#include <curl/curl.h>
#include <tmx/Security/include/base64.h>

#include "ImmediateForwardConfiguration.h"
#include "SNMPWorker.h"

namespace ImmediateForward
{

class ImmediateForwardPlugin : public tmx::utils::PluginClient
{
	public:
		ImmediateForwardPlugin(const std::string &name);
	private:
		void UpdateConfigSettings();
		bool UpdateUdpClientFromConfigSettings(uint clientIndex);
		void SendMessageToRadio(IvpMessage *msg);	


		// Virtual method overrides.
		void OnConfigChanged(const char *key, const char *value);
		void OnMessageReceived(IvpMessage *msg);
		void OnStateChange(IvpPluginState state);


		// Mutex along with the data it protects.
		std::mutex _configMutex;
		// A map of UDP clients for sending V2X communication to different RSUs for broadcast (RSU Spec 4.1)
		std::unordered_map<std::string, std::unique_ptr<tmx::utils::UdpClient>> _udpClientMap;
		// A map of SNMP Clients for sending V2X communication to different RSUs for broadvast (RSU Spec NTCIP 1218)
		std::unordered_map<std::string, std::unique_ptr<tmx::utils::snmp_client>> _snmpClientMap;
		// A map of maps message types and Immediate Forward Table indexes for NTCIP 1218 IMF functionality
		std::unordered_map< std::string, std::unordered_map<std::string, unsigned int>> _imfNtcipMessageTypeIndex;
		std::vector<ImfConfiguration> _imfConfigs;
		std::map<std::string, int> _messageCountMap;

		// Thread safe bool set to true the first time the configuration has been read.
		std::atomic<bool> _configRead;

		uint _skippedNoDsrcMetadata;
		uint _skippedNoMessageRoute;
		uint _skippedInvalidUdpClient;
		uint _skippedSignErrorResponse;

	};

} /* namespace ImmediateForward */

