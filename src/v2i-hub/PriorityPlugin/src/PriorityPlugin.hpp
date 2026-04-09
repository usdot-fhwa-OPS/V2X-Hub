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

#include "PriorityTypes.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <map>
#include <mutex>
#include <optional>
#include <thread>
#include <tuple>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

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
            // Per-package signal request state decoded from an SRM (used for PRG mode and SSM building)
            struct SignalRequest {
                uint8_t requestID;
                long intersectionID;
                long requestType;
                uint16_t timeOfService;
                uint16_t timeOfDepart;
                bool rejected = false;
                uint8_t inboundPresent = 0;
                long inboundValue = 0;
                long etaMinute = 0;
                long etaSecond = 0;
                long duration = 0;
            };

            // Per-requestor state decoded from an SRM, keyed by vehicle ID (used for PRG mode)
            struct RequestorState {
                std::vector<uint8_t> vehicleID;
                uint8_t classType;
                uint8_t sequenceNumber;
                uint32_t timeOfRequest;
                std::vector<SignalRequest> requests;
                long role = 0;
            };

            /**
             * @brief Maps J2735 BasicVehicleRole to NTCIP 1211 priorityRequestVehicleClassType (1..10) and priorityRequestVehicleClassLevel (1..10).
             * @param role BasicVehicleRole enumeration value from the SRM requestor type.
             * @return std::pair<uint8_t, uint8_t> NTCIP 1211 vehicle class type (1..10) and class level (1..10).
             */
            std::pair<uint8_t, uint8_t> MapVehicleClass(long role) const;

            /**
             * @brief Looks up the serviceStrategyNumber for a given intersection and inbound lane.
             * @param intersectionID The intersection ID from the SRM.
             * @param lane The inbound lane ID from the SRM.
             * @return The service strategy number or std::nullopt, if no mapping exists.
             */
            std::optional<uint8_t> LookupStrategy(long intersectionID, long lane) const;

            /**
             * @brief Encodes a priority request per NTCIP 1211 prgPriorityRequestAbsolute into a 29-byte OER-encoded OCTET STRING.
             * @param requestID      priorityRequestID (1..255)
             * @param vehicleID      Raw bytes of the vehicle identifier from the SRM requestor.
             * @param vehicleIDLen   Length of the vehicleID buffer.
             * @param classType      priorityRequestVehicleClassType (1..10)
             * @param classLevel     priorityRequestVehicleClassLevel (1..10)
             * @param strategyNum    priorityRequestServiceStrategyNumber (1..255)
             * @param timeOfService  priorityRequestTimeOfServiceDesired (1..65535) relative seconds to arrive at the intersection stopping point from message receipt.
             * @param timeOfDepart   priorityRequestTimeOfEstimatedDeparture (1..65535) relative seconds of estimated departure from the stopping point from message receipt.
             * @param timeOfRequest  priorityRequestTimeOfRequest (0..4294967295) epoch seconds.
             * @return std::vector<uint8_t> 29-byte OER-encoded buffer.
             */
            std::vector<uint8_t> EncodePriorityRequest(uint8_t requestID, const uint8_t *vehicleID, size_t vehicleIDLen, uint8_t classType, uint8_t classLevel, uint8_t strategyNum, uint16_t timeOfService, uint16_t timeOfDepart, uint32_t timeOfRequest) const;

            /**
             * @brief Encodes the prsServiceRequest OCTET STRING per NTCIP 1211 CO-MIB 5.2.2.1.
             * @return std::vector<uint8_t> 110-byte OER-encoded buffer.
             */
            std::vector<uint8_t> EncodeServiceRequest() const;

            /**
             * @brief Decodes a prsServiceRequest OCTET STRING received from a CO (GET response) into per-row CO status and the coBusy flag.
             * @param data The raw 110-byte buffer.
             * @param rows Array of 10 CoServiceResponseRows.
             * @param coBusy Flag indicating if the CO reports busy.
             * @return true on successful decode, false if data size is invalid.
             */
            bool DecodeCoServiceResponse(const std::vector<uint8_t> &data, std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> &rows, bool &coBusy) const;

            /**
             * @brief Sends the encoded OCTET STRING to a TSC via SNMP SET.
             * @return true on success, false on failure.
             */
            bool SnmpSet(const std::shared_ptr<tmx::utils::snmp_client> &client, const std::string &oid, const std::vector<uint8_t> &data);

            /**
             * @brief Performs an SNMP GET on a given OID and returns the raw bytes.
             * @return true on success, false on failure.
             */
            bool SnmpGet(const std::shared_ptr<tmx::utils::snmp_client> &client, const std::string &oid, std::vector<uint8_t> &data);

            /**
             * @brief Background thread entry point for the PRS-CO exchange loop. Implements NTCIP 1211 4.2.4.1.2 (PRS is Manager).
             */
            void ServiceExchangeLoop();

            /**
             * @brief Performs prioritization processing per NTCIP 1211 4.2.4.1.4.
             *        Sorts the priority request table entries by class type (highest first),
             *        class level (highest first), then soonest TSD. Expires stale entries.
             *        Must be called while holding _tableMutex.
             */
            void RunPrioritizationProcessing();

            /**
             * @brief Applies the CO response statuses back into the priority request table
             *        per NTCIP 1211 4.2.4.1.2 step (i) and 4.3.1 state transitions.
             *        Must be called while holding _tableMutex.
             */
            void ApplyCoStatusUpdates(const std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> &coRows);

            /**
             * @brief Builds and broadcasts SSMs reflecting the current priorityRequestTable statuses.
             */
            void BroadcastSSMFromTable();

            /**
             * @brief Maps a RequestStatus to the J2735 PrioritizationResponseStatus for SSM.
             */
            long MapStatusToSSM(RequestStatus status) const;

            /**
             * @brief Builds and broadcasts a SignalStatusMessage with applicable status
             *        for all signal requests in the given requestor state (PRG mode only).
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

            // Map of vehicle ID (raw bytes as string key) to latest requestor state (PRG mode)
            std::unordered_map<std::string, RequestorState> _requestorStates;

            // PRS mode: NTCIP 1211 priority request table (5.1.1.1)
            // Protected by _tableMutex; accessed by SRM handler thread and exchange loop thread.
            std::mutex _tableMutex;
            std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> _priorityRequestTable;

            // PRS busy flag per 4.3.2: true while prioritization processing is in progress
            bool _prsBusy = false;

            // Per-class reservice timers: epoch seconds when the last request of each class completed.
            // Index 0 = class type 1, index 9 = class type 10.
            std::array<uint32_t, 10> _reserviceLastCompletedTime = {};

            // Background exchange loop thread
            std::thread _exchangeThread;
            std::atomic<bool> _running{false};

            // Lane-to-strategy mapping: key = (intersectionID, lane) → strategyNumber
            std::map<std::pair<long, long>, uint8_t> _laneStrategyMap;

            // Configuration values
            std::string _snmpCommunity = "public";
            std::string _pluginRole;
            uint16_t _estimatedArrivalTime;
            uint16_t _estimatedDepartureTime;
            uint32_t _pollIntervalMs;        // PRS-CO poll interval (100-1000ms)
            uint32_t _timeToLiveSec;        // Max time PRS considers a priority request
            std::array<uint32_t, 10> _reserviceClassTime = {}; // Per-class reservice period (seconds)

            // Status tracking
            unsigned long _priorityRequestsSent = 0;
            unsigned long _skippedMessages = 0;
            unsigned long _serviceExchanges = 0;
            const char* _keyPriorityRequestsSent = "Priority Requests Sent";
            const char* _keySkippedMessages = "Skipped Messages";
            const char* _keyServiceExchanges = "Service Exchanges";

            // SSM sequence number tracking (per intersectionID)
            uint8_t _ssmSequenceCounter = 0;
            std::unordered_map<long, uint8_t> _signalStatusSeqByIntersection;
            std::unordered_map<long, std::string> _lastSignalStatusFingerprint;
    };
} /* namespace PriorityPlugin */
