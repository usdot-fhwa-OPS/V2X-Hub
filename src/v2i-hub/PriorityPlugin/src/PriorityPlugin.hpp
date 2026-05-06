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

#include "PriorityRequestProcessor.hpp"
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

    // NTCIP 1211 OID for prgPriorityUpdateAbsolute (1211 v02A-SE03f PRS-MIB1 5.1.2.9)
    static const std::string NTCIP1211_PRIORITY_UPDATE_ABSOLUTE_OID = "1.3.6.1.4.1.1206.4.2.11.2.9.0";

    // NTCIP 1211 OID for prgPriorityCancel (1211 v02A-SE03f PRS-MIB1 5.1.2.5)
    static const std::string NTCIP1211_PRIORITY_CANCEL_OID = "1.3.6.1.4.1.1206.4.2.11.2.5.0";

    // NTCIP 1211 OID for prgPriorityClear (1211 v02A-SE03f PRS-MIB1 5.1.2.6)
    static const std::string NTCIP1211_PRIORITY_CLEAR_OID = "1.3.6.1.4.1.1206.4.2.11.2.6.0";

    // NTCIP 1211 OID for prsServiceRequest (1211 v0224j CO-MIB1 5.2.2.1)
    static const std::string NTCIP1211_PRS_SERVICE_REQUEST_OID = "1.3.6.1.4.1.1206.4.2.11.4.1.0";

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
            // PRG: request tracker 
            // Tracks requests sent to the PRS so we can track new requests, updates, and when to send clear after cancel
            enum class PrgRequestState : uint8_t {
                sent,
                canceled
            };

            struct PrgTrackedRequest {
                uint8_t requestID;
                long intersectionID;
                std::vector<uint8_t> vehicleID;
                uint8_t classType;
                uint8_t classLevel;
                uint8_t strategyNumber;
                std::chrono::steady_clock::time_point sentTime;
                PrgRequestState state = PrgRequestState::sent;
            };

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

            // Per-controller configuration and SNMP client
            struct ControllerInfo {
                long intersectionID;
                std::string ip;
                uint16_t port;
                std::shared_ptr<tmx::utils::snmp_client> snmpClient;
            };

            /**
             * @brief Sends the encoded OCTET STRING to a TSC via SNMP SET.
             * @return true on success, false on failure.
             */
            bool SnmpSet(const std::shared_ptr<tmx::utils::snmp_client> &client, const std::string &oid, const std::vector<uint8_t> &data) const;

            /**
             * @brief Performs an SNMP GET on a given OID and returns the raw bytes.
             * @return true on success, false on failure.
             */
            bool SnmpGet(const std::shared_ptr<tmx::utils::snmp_client> &client, const std::string &oid, std::vector<uint8_t> &data) const;

            /**
             * @brief Background thread entry point for the PRS-CO exchange loop. Implements NTCIP 1211 4.2.4.1.2 (PRS is Manager).
             */
            void ServiceExchangeLoop();

            /**
             * @brief Builds and broadcasts SSMs reflecting the current priorityRequestTable statuses.
             */
            void BroadcastSSMFromTable();

            /**
             * @brief Maps the NTCIP 1211 priority request/strategy status to the J2735 PrioritizationResponseStatus for SSM.
             */
            long MapNTCIPstatusToSSM(RequestStatus status) const;

            /**
             * @brief Builds and broadcasts a SignalStatusMessage with applicable status
             *        for all signal requests in the given requestor state (PRG mode only).
             * @param state The RequestorState after a new SRM is stored.
             */
            void BuildSSM(const RequestorState &state);

            /**
             * @brief Processes an SRM signal request package in PRS mode.
             *        Called from HandleSRM for each package in the SRM when PluginRole is PRS.
             * @param pkg the signal request package to process.
             * @param vehicleID the decoded vehicle ID from the SRM requestor.
             * @param classType the mapped NTCIP 1211 vehicle class type for this
             * @param classLevel the mapped NTCIP 1211 vehicle class level for this request.
             * @param newSeq the sequence number from the SRM, used for update checks.
             * @param role the role from the SRM requestor, used for class mapping and override checks.
             * @param currentMinuteOfYear the current minute of the year, used for ETA calculations.
             * @param currentMsInMinute the current millisecond in the minute, used for ETA calculations.
             * @param nowEpoch the current time in epoch seconds, used for time of request and ETA calculations.
             */
            void ProcessPrsPackage(const SignalRequestPackage &pkg, const std::vector<uint8_t> &vehicleID, uint8_t classType, uint8_t classLevel, uint8_t newSeq, long role, long currentMinuteOfYear, long currentMsInMinute, time_t nowEpoch);

            /**
             * @brief Processes an SRM signal request package in PRG mode.
             *        Called from HandleSRM for each package in the SRM when PluginRole is PRG.
             * @param pkg the signal request package to process.
             * @param vehicleID the decoded vehicle ID from the SRM requestor.
             * @param vehicleKey the string key for this vehicle ID, used for request tracking in the PRG mode maps.
             * @param classType the mapped NTCIP 1211 vehicle class type for this request.
             * @param classLevel the mapped NTCIP 1211 vehicle class level for this request.
             * @param currentMinuteOfYear the current minute of the year, used for ETA calculations.
             * @param currentMsInMinute the current millisecond in the minute, used for ETA calculations.
             * @param nowEpoch the current time in epoch seconds, used for time of request and ETA calculations.
             * @param timeOfRequest the time of request to include in the priority request sent to the PRS, in epoch seconds.
             */
            void ProcessPrgPackage(const SignalRequestPackage &pkg, const std::vector<uint8_t> &vehicleID, const std::string &vehicleKey, uint8_t classType, uint8_t classLevel, long currentMinuteOfYear, long currentMsInMinute, time_t nowEpoch, uint32_t timeOfRequest, RequestorState &state);

            // Map of intersection ID to controller info
            std::unordered_map<long, ControllerInfo> _controllers;

            // Map of vehicle ID to latest requestor state (PRG mode)
            std::unordered_map<std::string, RequestorState> _requestorStates;

            // Map of request ID to tracked PRG request for active requests
            std::unordered_map<std::string, PrgTrackedRequest> _prgTrackedRequests;

            // PRS mode: last seen SRM sequence number per vehicle, used to drop
            // duplicates delivered by multiple intersections near the vehicle.
            std::unordered_map<std::string, uint8_t> _prsLastSeqByVehicle;

            // For PRS mode: NTCIP 1211 priority request processor.
            PriorityRequestProcessor _processor;

            // Mutex to protect access to the priority request table and related state during the service exchange loop
            std::mutex _tableMutex;

            // PRS busy flag per 4.3.2: true while prioritization processing is in progress
            bool _prsBusy = false;

            // PRS mode: last prsServiceRequest payload sent to the CO.
            // Used to avoid sending redundant SETs when the table state has not changed.
            std::vector<uint8_t> _lastSentServiceRequest;

            // Background exchange loop thread
            std::thread _exchangeThread;
            std::atomic<bool> _running{false};

            // Configuration values
            std::string _snmpCommunity = "public";
            std::string _pluginRole;
            uint16_t _estimatedArrivalTime;
            uint16_t _estimatedDepartureTime;
            uint32_t _pollIntervalMs;        // PRS-CO poll interval (100-1000ms)
            uint32_t _timeToLiveSec;         // Max time PRS considers a priority request
            uint8_t  _maxSsmBroadcastsPerStatus = 2;
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
            std::unordered_map<long, std::string> _lastSignalStatusKey;
    };
} /* namespace PriorityPlugin */
