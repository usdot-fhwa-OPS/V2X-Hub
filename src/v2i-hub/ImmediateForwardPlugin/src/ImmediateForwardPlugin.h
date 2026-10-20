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

inline bool IsSPDU(const IvpMessage *msg)
{
	return msg &&
		msg->type && strcmp(msg->type, tmx::messages::RawSpdu::MessageType) == 0;
}

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
		 * @param signMessage Value of the Signature field, asking the RSU to sign the payload before
		 * broadcast. Taken from imfConfig.signMessage for J2735 messages, and always false when
		 * forwarding a raw SPDU, which already carries its own signature.
		 * @return string The formatted message, ready to send to the RSU over UDP.
		 */
		inline string ConstructMessageRSU_4_1(const ImfConfiguration& imfConfig, const MessageConfig& messageConfig, IvpMessage* msg, const string& payloadbyte, bool signMessage);

		/**
		 * @brief Check whether the incoming message is a SPDU regardless
		 * of subtype.
		 * @param ivp 
		 * @return true (the type is SPDU, regardless of subtype)
		 * @return false 
		 */

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
		std::atomic<bool> _configRead = false;

		uint _skippedNoDsrcMetadata = 0;
		uint _skippedNoMessageRoute = 0;
		uint _skippedInvalidUdpClient = 0;
		uint _skippedSignErrorResponse = 0;
		uint _skippedInvalidSpdu = 0;

		static constexpr const char* Key_SkippedNoDsrcMetadata = "Messages Skipped (No DSRC metadata)";
		static constexpr const char* Key_SkippedNoMessageRoute = "Messages Skipped (No route)";
		static constexpr const char* Key_SkippedSignError = "Message Skipped (Signature Error Response)";
		static constexpr const char* Key_SkippedInvalidUdpClient = "Messages Skipped (Invalid UDP Client)";
		static constexpr const char* Key_SkippedInvalidSpdu = "Messages Skipped (Invalid SPDU)";

	};

} /* namespace ImmediateForward */

