/**
 * Copyright (C) 2026 LEIDOS.
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

#include <cstring>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "SNMPClient.h"
#include <TmxMessageManager.h>
#include <tmx/messages/TmxJ2735.hpp>
#include <tmx/messages/TmxJ2735Codec.hpp>
#include <tmx/j2735_messages/SignalRequestMessage.hpp>
#include <tmx/j2735_messages/SignalStatusMessage.hpp>

namespace PriorityPlugin {

    // NTCIP 1211 OID for prgPriorityRequestAbsolute (PRS-MIB1 5.1.2.8)
    static const std::string NTCIP1211_PRIORITY_REQUEST_ABSOLUTE_OID = "1.3.6.1.4.1.1206.4.2.11.2.8.0";

    // Size of the OER-encoded priority request OCTET STRING
    static constexpr size_t PRIORITY_REQUEST_SIZE = 29;

    // Vehicle ID field size within the NTCIP 1211 priority request
    static constexpr size_t VEHICLE_ID_FIELD_SIZE = 17;

    class PriorityPlugin : public tmx::utils::TmxMessageManager {
        public:
            explicit PriorityPlugin(const std::string &name);
            ~PriorityPlugin() override;

        protected:
            /**
			 * @brief Called everytime a configuration value is changed for the plugin.
			 */
			void UpdateConfigSettings();
			/**
			 * @brief Overrides PluginClient OnStateChange(IvpPluginState state) method.
			 * @param state new state of the plugin.
			 */
			void OnStateChange(IvpPluginState state) override;
            /**
             * @brief Overrides PluginClient OnConfigChanged(const char *key, const char *value) method.
             * @param key configuration key that was changed.
             * @param value new value of the configuration key.
             */
            void OnConfigChanged(const char *key, const char *value) override;
            /**
            * @brief Overrides PluginClient OnMessageReceived(tmx::routeable_message &msg) method.
            * @param msg incoming message received by the plugin.
            */
            void OnMessageReceived(tmx::routeable_message &msg) override;
            /**
             * @brief Handles incoming Signal Request Messages (SRMs).
             * @param msg the SRM received.
             * @param routeableMsg the original routeable message that was received.
             */
            void HandleSRM(tmx::messages::SrmMessage &msg, tmx::routeable_message &routeableMsg);

        private:
            /**
             * @brief Maps J2735 BasicVehicleRole to NTCIP 1211 priorityRequestVehicleClassType (1..10).
             * NTCIP 1211 class type is a precedence value:
             *   1 = highest priority
             *   10 = lowest priority
             * A request with a higher class type (lower number) overrides a lower class type.
             * @param role BasicVehicleRole enumeration value from the SRM requestor type.
             * @return uint8_t NTCIP 1211 vehicle class type (1..10).
             */
            uint8_t MapVehicleClassType(long role) const;

            /**
             * @brief Encodes a priority request per NTCIP 1211 prgPriorityRequestAbsolute into a
             *        29-byte OER-encoded OCTET STRING.
             * @param requestID      priorityRequestID (1..255)
             * @param vehicleID      Raw bytes of the vehicle identifier from the SRM requestor.
             * @param vehicleIDLen   Length of the vehicleID buffer.
             * @param classType      priorityRequestVehicleClassType (1..10)
             * @param classLevel     priorityRequestVehicleClassLevel (1..10)
             * @param strategyNum    priorityRequestServiceStrategyNumber (1..255)
             * @param timeOfService  priorityRequestTimeOfServiceDesired (1..65535) relative seconds
             *                       to arrive at the intersection stopping point from message receipt.
             * @param timeOfDepart   priorityRequestTimeOfEstimatedDeparture (1..65535) relative seconds
             *                       of estimated departure from the stopping point from message receipt.
             * @param timeOfRequest  priorityRequestTimeOfRequest (0..4294967295) epoch seconds.
             * @return std::vector<uint8_t> 29-byte OER-encoded buffer.
             */
            std::vector<uint8_t> EncodePriorityRequest(uint8_t requestID, const uint8_t *vehicleID, size_t vehicleIDLen, uint8_t classType, uint8_t classLevel, uint8_t strategyNum, uint16_t timeOfService, uint16_t timeOfDepart, uint32_t timeOfRequest) const;

            /**
             * @brief Sends the encoded priority request OCTET STRING to the TSC via SNMP SET.
             * @param oid The OID to set.
             * @param data The raw byte buffer to send as an OCTET STRING.
             * @return bool true on success, false on failure.
             */
            bool SendPriorityRequest(const std::string &oid, const std::vector<uint8_t> &data);

            // SNMP client for TSC communication
            std::shared_ptr<tmx::utils::snmp_client> _snmpClient;

            // Configuration values
            std::string _tscIP;
            uint16_t _tscPort;
            std::string _snmpCommunity;
            uint8_t _classLevelStr;
            uint8_t _strategyStr;
            uint16_t _tsd;
            uint16_t _ted;

            // Status tracking
            unsigned long _priorityRequestsSent = 0;
            unsigned long _skippedMessages = 0;
            const char* _keyPriorityRequestsSent = "Priority Requests Sent";
            const char* _keySkippedMessages = "Skipped Messages";
    };
} /* namespace PriorityPlugin */
