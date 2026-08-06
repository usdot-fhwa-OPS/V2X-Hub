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
#include "IMFNTCIP1218Worker.h"
#include "SpduForwarder.h"

namespace ImmediateForward
{

class ImmediateForwardPlugin : public tmx::utils::PluginClient
{
	public:
		explicit ImmediateForwardPlugin(const std::string &name);
	private:
		void UpdateConfigSettings();
		bool UpdateUdpClientFromConfigSettings(uint clientIndex);
		void SendMessageToRadio(IvpMessage *msg);	


		// Virtual method overrides.
		void OnConfigChanged(const char *key, const char *value);
		void OnMessageReceived(IvpMessage *msg);
		void OnStateChange(IvpPluginState state);

		/**
		 * @brief Sign a message payload by posting it to the configured SCMS/HSM container.
		 *
		 * @param imfConfig RSU configuration providing the HSM URL.
		 * @param messageConfig Message configuration providing the send type.
		 * @param msg TMX message whose hex payload is signed.
		 * @param payloadbyte Out parameter set to the hex encoded signed payload on success.
		 * @return true Signing succeeded and payloadbyte was populated.
		 */
		inline void SignWithHsm(const ImfConfiguration& imfConfig, const MessageConfig& messageConfig, IvpMessage* msg, string& payloadbyte);

		/**
		 * @brief Format a forwarding message per the USDOT RSU Specifications v4.1 Appendix C protocol.
		 *
		 * @param imfConfig RSU configuration providing the TX mode and signature flag.
		 * @param messageConfig Message configuration providing the send type, PSID and optional channel.
		 * @param msg TMX message supplying the DSRC channel when messageConfig has none.
		 * @param payloadbyte Hex encoded payload to forward.
		 * @param psidOverride Optional PSID to broadcast instead of the configured one. Used when
		 * forwarding a raw SPDU, which carries its own PSID.
		 * @return string The formatted message, ready to send to the RSU over UDP.
		 */
		inline string ConstructMessageRSU_4_1(const ImfConfiguration& imfConfig, const MessageConfig& messageConfig, IvpMessage* msg, const string& payloadbyte, const std::optional<std::string>& psidOverride = std::nullopt);

		// Mutex along with the data it protects.
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
		uint _skippedInvalidSpdu;

	};

} /* namespace ImmediateForward */

