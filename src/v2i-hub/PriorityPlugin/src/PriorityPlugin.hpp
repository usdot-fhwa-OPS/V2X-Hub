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

#include "PluginClientClockAware.h"

#include "PriorityConfiguration.hpp"
#include "PriorityPluginWorker.hpp"
#include "PriorityRequestBuilder.hpp"
#include "PriorityRequestProcessor.hpp"
#include "PrsServiceExchange.hpp"
#include "SsmBuilder.hpp"
#include "PriorityTypes.hpp"

#include <atomic>
#include <chrono>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "SNMPClient.h"
#include <tsc/NTCIP_1211_MIB.h>
#include <tmx/j2735_messages/SignalRequestMessage.hpp>
#include <tmx/j2735_messages/SignalStatusMessage.hpp>


namespace PriorityPlugin {

    class PriorityPlugin : public tmx::utils::PluginClientClockAware {
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
             * @brief Handles incoming Signal Request Messages (SRMs).
             * @param msg the SRM received.
             * @param routeableMsg the original routeable message that was received.
             */
            void HandleSRM(tmx::messages::SrmMessage &msg, tmx::routeable_message &routeableMsg);

        private:
            // Per-controller configuration and SNMP client
            struct ControllerInfo {
                long intersectionID;
                std::string ip;
                uint16_t port;
                std::shared_ptr<tmx::utils::snmp_client> snmpClient;
            };

            /**
             * @brief Background thread entry point for the PRS-CO exchange loop. Contains the
             *        while/sleep loop and controller selection. The per-iteration protocol
             *        logic is in DoOneServiceExchange (PrsServiceExchange.hpp).
             *        Implements NTCIP 1211 4.2.4.1.2 (PRS is Manager).
             */
            void ServiceExchangeLoop();

            /**
             * @brief Builds and broadcasts SSMs reflecting the current priorityRequestTable statuses.
             */
            void BroadcastSSMFromTable();

            /**
             * @brief Encodes the given SSM tree and broadcasts it.
             * @param ssmPtr the SSM tree to encode.
             */
            void EncodeAndBroadcastSSM(const std::shared_ptr<SignalStatusMessage_t> &ssmPtr);

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
             * @param input the per-SRM request context, built once in HandleSRM and reused for every package in the SRM.
             */
            void ProcessPrsPackage(const SignalRequestPackage &pkg, const PrsPackageInput &input);

            /**
             * @brief Processes an SRM signal request package in PRG mode.
             *        Called from HandleSRM for each package in the SRM when PluginRole is PRG.
             * @param pkg the signal request package to process.
             * @param input the per-SRM request context, built once in HandleSRM and reused for every package in the SRM.
             * @param state the RequestorState to record the resulting SignalRequest into.
             */
            void ProcessPrgPackage(const SignalRequestPackage &pkg, const PrgPackageInput &input, RequestorState &state);

            /**
             * @brief Sweeps stale entries from _prgTrackedRequests (PRG mode).
             *        Sends prgPriorityClear for canceled entries that have aged out and evicts entries past _timeToLiveSec.
             * @param nowMs Current time in milliseconds (from CarmaClock).
             */
            void SweepStaleTrackedRequests(uint64_t nowMs);

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
