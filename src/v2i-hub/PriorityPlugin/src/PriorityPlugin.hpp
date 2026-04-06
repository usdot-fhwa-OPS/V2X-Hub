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
#include <unordered_map>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "SNMPClient.h"
#include <TmxMessageManager.h>
#include <tmx/messages/TmxJ2735.hpp>
#include <tmx/messages/TmxJ2735Codec.hpp>
#include <tmx/j2735_messages/SignalRequestMessage.hpp>
#include <tmx/j2735_messages/SignalStatusMessage.hpp>

namespace PriorityPlugin {

    // NTCIP 1211 OID for prgPriorityRequestAbsolute (1211 v02A-SE03f PRS-MIB1 5.1.2.8)
    static const std::string NTCIP1211_PRIORITY_REQUEST_ABSOLUTE_OID = "1.3.6.1.4.1.1206.4.2.11.2.8.0";

    // NTCIP 1211 OID for prsServiceRequest (1211 v0224j CO-MIB1 5.2.2.1)
    static const std::string NTCIP1211_PRS_SERVICE_REQUEST_OID = "1.3.6.1.4.1.1206.4.2.11.4.1.0";

    // Size of the OER-encoded priority request OCTET STRING
    static constexpr size_t PRIORITY_REQUEST_SIZE = 29;

    // Size of the OER-encoded service request OCTET STRING
    static constexpr size_t SERVICE_REQUEST_SIZE = 110;

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
            // Per-package signal request state decoded from an SRM
            struct SignalRequest {
                uint8_t requestID;
                long intersectionID;
                long requestType;
                uint16_t timeOfService;
                uint16_t timeOfDepart;
            };

            // Per-requestor state decoded from an SRM, keyed by vehicle ID
            struct RequestorState {
                std::vector<uint8_t> vehicleID;
                uint8_t classType;
                uint8_t sequenceNumber;
                uint32_t timeOfRequest;
                std::vector<SignalRequest> requests;
            };

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
             * @brief Encodes a service request per NTCIP 1211 prsServiceRequest into a 110-byte OER-encoded OCTET STRING.
             * @param strategyNum      Index in the priorityStrategyTable. INTEGER (1..255)
             * @param timeofService    The estimated time of service desired expressed as global time. INTEGER (0..4294967295)
             * @param timeOfDeparture  The estimated time of departure expressed as global time. INTEGER (0..4294967295)
             * @param requestStatus    Type of strategy request sent by the PRS to the CO. INTEGER (1..255)
             * @return std::vector<uint8_t> 110-byte OER-encoded buffer.
             */
            std::vector<uint8_t> EncodeServiceRequest(uint8_t strategyNum, uint32_t timeOfService, uint32_t timeOfDeparture, uint8_t requestStatus);

            /**
             * @brief Sends the encoded priority or service request OCTET STRING to a TSC via SNMP SET.
             * @param client The SNMP client for the target controller.
             * @param oid The OID to set.
             * @param data The raw byte buffer to send as an OCTET STRING.
             * @return bool true on success, false on failure.
             */
            bool SendRequest(const std::shared_ptr<tmx::utils::snmp_client> &client, const std::string &oid, const std::vector<uint8_t> &data);

            /**
             * @brief Builds and broadcasts a SignalStatusMessage with applicable status
             *        for all signal requests in the given requestor state.
             * @param state The RequestorState after a new SRM is stored.
             */
            void BuildSSM(const RequestorState &state);

            // Per-controller configuration and SNMP client
            struct ControllerInfo {
                long intersectionID;
                std::string ip;
                uint16_t port;
                std::shared_ptr<tmx::utils::snmp_client> snmpClient;
            };

            // Map of intersection ID to controller info
            std::unordered_map<long, ControllerInfo> _controllers;

            // Map of vehicle ID (raw bytes as string key) to latest requestor state
            std::unordered_map<std::string, RequestorState> _requestorStates;

            // Configuration values
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
