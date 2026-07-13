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

#include "PriorityPlugin.hpp"

using namespace tmx::messages;
using namespace tmx::utils;

namespace PriorityPlugin {
    PriorityPlugin::PriorityPlugin(const std::string &name) : PluginClientClockAware(name)
    {
        AddMessageFilter <SrmMessage> (this, &PriorityPlugin::HandleSRM);
        SubscribeToMessages();
    }

    PriorityPlugin::~PriorityPlugin()
    {
        _running = false;
        if (_exchangeThread.joinable()) {
            _exchangeThread.join();
        }
    }

    void PriorityPlugin::OnStateChange(IvpPluginState state)
    {
        PluginClientClockAware::OnStateChange(state);
		if (state == IvpPluginState_registered) {
			UpdateConfigSettings();

            // Start the PRS-CO exchange loop thread if in PRS mode
            if (_pluginRole == PluginRole::PRS && !_running) {
                _running = true;
                _exchangeThread = std::thread(&PriorityPlugin::ServiceExchangeLoop, this);
                PLOG(logINFO) << "Started PRS-CO service exchange loop thread.";
            }
		}
    }

    void PriorityPlugin::OnConfigChanged(const char *key, const char *value)
    {
        PluginClientClockAware::OnConfigChanged(key, value);
		UpdateConfigSettings();
    }

    void PriorityPlugin::UpdateConfigSettings()
    {
        std::string tscControllersJson;
        GetConfigValue<std::string>("TSC_Configuration_List", tscControllersJson);
        std::string pluginRoleStr;
        GetConfigValue<std::string>("PluginRole", pluginRoleStr);
        try {
            _pluginRole = parsePluginRole(pluginRoleStr);
        } catch (const PriorityConfigurationException &e) {
            PLOG(logERROR) << "Keeping role " << pluginRoleToString(_pluginRole) << ": " << e.what();
        }
        GetConfigValue<uint16_t>("EstimatedTimeofArrival", _estimatedArrivalTime);
        GetConfigValue<uint16_t>("EstimatedDepartureTime", _estimatedDepartureTime);
        if (_estimatedArrivalTime < 1) {
            PLOG(logWARNING) << "EstimatedTimeofArrival must be 1..65535, clamping from 0 to 1";
            _estimatedArrivalTime = 1;
        }
        if (_estimatedDepartureTime < 1) {
            PLOG(logWARNING) << "EstimatedDepartureTime must be 1..65535, clamping from 0 to 1";
            _estimatedDepartureTime = 1;
        }
        GetConfigValue<uint32_t>("PollIntervalMs", _pollIntervalMs);
        GetConfigValue<uint32_t>("TimeToLiveSec", _timeToLiveSec);
        uint32_t maxSsmBroadcasts = _maxSsmBroadcastsPerStatus;
        GetConfigValue<uint32_t>("MaxSsmBroadcastsPerStatus", maxSsmBroadcasts);
        maxSsmBroadcasts = std::clamp(maxSsmBroadcasts, static_cast<uint32_t>(1), static_cast<uint32_t>(255));
        _maxSsmBroadcastsPerStatus = static_cast<uint8_t>(maxSsmBroadcasts);

        // Parse per-class reservice times (comma-separated, up to 10 values)
        std::string reserviceStr;
        GetConfigValue<std::string>("ReserviceClassTimes", reserviceStr);
        try {
            _reserviceClassTime = parseReserviceClassTimes(reserviceStr);
        } catch (const PriorityConfigurationException &e) {
            PLOG(logERROR) << "Invalid ReserviceClassTimes, keeping zeroed reservice times: " << e.what();
            _reserviceClassTime = {};
        }

        // Repopulate the processor's lane-strategy map from the LaneStrategyMapping JSON array
        std::string laneStrategyJson;
        GetConfigValue<std::string>("LaneStrategyMapping", laneStrategyJson);
        {
            std::lock_guard lock(_tableMutex);
            _processor.ClearLaneStrategyMap();
        }
        try {
            auto laneStrategyEntries = parseLaneStrategyMapping(laneStrategyJson);
            for (const auto &entry : laneStrategyEntries) {
                {
                    std::lock_guard lock(_tableMutex);
                    _processor.SetLaneStrategy(entry.intersectionID, entry.lane, entry.strategy);
                }
                PLOG(logINFO) << "Lane strategy: IntersectionID=" << entry.intersectionID
                              << " Lane=" << entry.lane
                              << " Strategy=" << static_cast<int>(entry.strategy);
            }
        } catch (const boost::property_tree::json_parser_error &e) {
            PLOG(logERROR) << "Failed to parse LaneStrategyMapping JSON: " << e.what();
        } catch (const boost::property_tree::ptree_error &e) {
            PLOG(logERROR) << "Invalid LaneStrategyMapping entry: " << e.what();
        } catch (const PriorityConfigurationException &e) {
            PLOG(logERROR) << "Invalid LaneStrategyMapping: " << e.what();
        }

        // Parse the TSC_Controllers JSON array and create an SNMP client per entry
        _controllers.clear();
        try {
            auto controllerConfigs = parseTscConfigurationList(tscControllersJson);
            for (const auto &cfg : controllerConfigs) {
                ControllerInfo info;
                info.intersectionID = cfg.intersectionID;
                info.ip             = cfg.ip;
                info.port           = cfg.port;

                try {
                    info.snmpClient = std::make_shared<snmp_client>(
                        info.ip, info.port, _snmpCommunity,
                        "", "", "", "", "", "",
                        SNMP_VERSION_1, SNMP_DEFAULT_TIMEOUT);
                } catch (const snmp_client_exception &e) {
                    std::string detail = "Failed to create SNMP client for IntersectionID="
                                         + std::to_string(info.intersectionID) + " ("
                                         + info.ip + ":" + std::to_string(info.port) + "): " + e.what();
                    PLOG(logERROR) << detail;

                    // Display the failure in the UI and the event log
                    SetStatus<std::string>(("Controller " + std::to_string(info.intersectionID)).c_str(), "Disconnected");
                    TmxEventLogMessage eventLogMsg(e, detail, false);
                    BroadcastMessage(eventLogMsg, GetName());
                    continue;
                }

                PLOG(logINFO) << "Configured controller: IntersectionID=" << info.intersectionID
                               << " IP=" << info.ip << " Port=" << info.port;
                _controllers[info.intersectionID] = std::move(info);
            }
        } catch (const boost::property_tree::json_parser_error &e) {
            PLOG(logERROR) << "Failed to parse TSC_Controllers JSON: " << e.what();
        } catch (const boost::property_tree::ptree_error &e) {
            PLOG(logERROR) << "Invalid TSC_Controllers entry: " << e.what();
        } catch (const PriorityConfigurationException &e) {
            PLOG(logERROR) << "Invalid TSC_Controllers configuration: " << e.what();
        }

        PLOG(logINFO) << "PriorityPlugin configured: " << _controllers.size() << " controller(s)"
                                    << " Role=" << pluginRoleToString(_pluginRole)
                                    << " PollInterval=" << _pollIntervalMs << "ms"
                                    << " TimeToLive=" << _timeToLiveSec << "s";
    }

    void PriorityPlugin::HandleSRM(SrmMessage &msg, tmx::routeable_message &routeableMsg)
    {
        PLOG(logINFO) << "Received Signal Request Message (SRM)";
        SrmEncodedMessage srmEnc(routeableMsg);
        auto srmDecoded = srmEnc.decode_j2735_message();
        auto srm = srmDecoded.get_j2735_data();

        if (srm) {
            if (!srm->requests || srm->requests->list.count <= 0) {
                PLOG(logWARNING) << "SRM contains no signal request packages, skipping.";
                _skippedMessages++;
                SetStatus(_keySkippedMessages, _skippedMessages);
                return;
            }

            // Extract the vehicle ID from the requestor
            auto vehicleID = ExtractVehicleID(srm->requestor.id);
            std::string vehicleKey(vehicleID.begin(), vehicleID.end());

            auto nowEpoch = static_cast<time_t>(getClock()->nowInSeconds());
            auto [currentMinuteOfYear, currentMsInMinute] = ComputeMinuteAndMsOfYear(nowEpoch);

            // Determine vehicle class from the SRM requestor type
            long role = 0;
            if (srm->requestor.type != nullptr) {
                role = srm->requestor.type->role;
            }
            auto [classType, classLevel] = MapVehicleClass(role);

            uint8_t newSeq = srm->sequenceNumber ? static_cast<uint8_t>(*srm->sequenceNumber) : 0;

            // PRS mode: handle SRM per NTCIP 1211 4.2.3.1 (how PRS receives a priority request from a PRG).
            if (_pluginRole == PluginRole::PRS) {
                // Dedup: the same SRM may sometimes be routed back from multiple nearby intersections.
                if (auto existing = _prsLastSeqByVehicle.find(vehicleKey);
                    existing != _prsLastSeqByVehicle.end() && existing->second == newSeq) {
                    PLOG(logDEBUG1) << "SRM sequence number unchanged (" << static_cast<int>(newSeq)
                                    << ") for this vehicle, discarding.";
                    return;
                }
                _prsLastSeqByVehicle[vehicleKey] = newSeq;

                PrsPackageInput input{
                    vehicleID, classType, classLevel, newSeq, role,
                    currentMinuteOfYear, currentMsInMinute, nowEpoch,
                    _reserviceClassTime, _timeToLiveSec,
                    _estimatedArrivalTime, _estimatedDepartureTime, _prsBusy};

                std::lock_guard lock(_tableMutex);
                for (int i = 0; i < srm->requests->list.count; i++) {
                    const auto *pkg = srm->requests->list.array[i];
                    if (pkg) {
                        ProcessPrsPackage(*pkg, input);
                    }
                }
                return;
            }

            // PRG mode: direct conversion of SRM to priority request
            auto timeOfRequest = static_cast<uint32_t>(nowEpoch);

            if (auto existing = _requestorStates.find(vehicleKey);
                existing != _requestorStates.end() && existing->second.sequenceNumber == newSeq) {
                PLOG(logDEBUG1) << "SRM sequence number unchanged (" << static_cast<int>(newSeq)
                                << ") for this vehicle, discarding.";
                return;
            }

            SweepStaleTrackedRequests(getClock()->nowInMilliseconds());

            RequestorState &state = _requestorStates[vehicleKey];
            state.vehicleID = vehicleID;
            state.classType = classType;
            state.sequenceNumber = newSeq;
            state.timeOfRequest = timeOfRequest;
            state.role = role;
            state.requests.clear();

            PrgPackageInput input{
                vehicleID, vehicleKey, classType, classLevel,
                currentMinuteOfYear, currentMsInMinute, timeOfRequest,
                _estimatedArrivalTime, _estimatedDepartureTime,
                getClock()->nowInMilliseconds()};

            for (int i = 0; i < srm->requests->list.count; i++) {
                const auto *pkg = srm->requests->list.array[i];
                if (pkg) {
                    ProcessPrgPackage(*pkg, input, state);
                }
            }

            for (uint8_t i = 0; i < _maxSsmBroadcastsPerStatus; i++) {
                BuildSSM(state);
            }

        }

        else {
            PLOG(logWARNING) << "SRM decode returned null, skipping.";
            _skippedMessages++;
            SetStatus(_keySkippedMessages, _skippedMessages);
        }
    }

    void PriorityPlugin::ProcessPrsPackage(const SignalRequestPackage &pkg, const PrsPackageInput &input)
    {
        // All table decision logic lives in the testable free function; the plugin
        // owns only the lock (held by the caller) and the resulting status side effects.
        auto result = ApplyPrsPackage(_processor.Table(), _processor, pkg, input);

        if (result.action == PrsPackageResult::Action::TableFull) {
            _skippedMessages++;
            SetStatus(_keySkippedMessages, _skippedMessages);
        }
    }

    void PriorityPlugin::ProcessPrgPackage(const SignalRequestPackage &pkg, const PrgPackageInput &input, RequestorState &state)
    {
        // Snapshot the configured intersection IDs so the pure builder can decide
        // routing without touching the controller/SNMP map directly.
        std::unordered_set<long> configuredIntersectionIDs;
        {
            std::lock_guard lock(_tableMutex);
            for (const auto &[intID, info] : _controllers) {
                configuredIntersectionIDs.insert(intID);
            }
        }

        PrgPackageResult result;
        {
            std::lock_guard lock(_tableMutex);
            result = BuildPrgPackage(
                _prgTrackedRequests, configuredIntersectionIDs, _processor, pkg, input);
        }

        if (result.outcome != PrgPackageResult::Outcome::Send) {
            // Builder already recorded the rejected SignalRequest; surface it to the UI.
            _skippedMessages++;
            SetStatus(_keySkippedMessages, _skippedMessages);
            state.requests.push_back(result.signalRequest);
            return;
        }

        auto ctrlIt = _controllers.find(pkg.request.id.id);
        if (ctrlIt == _controllers.end()) {
            // Controller vanished between snapshot and send; treat as failure.
            PLOG(logWARNING) << "No controller configured for IntersectionID=" << pkg.request.id.id;
            _skippedMessages++;
            SetStatus(_keySkippedMessages, _skippedMessages);
            result.signalRequest.rejected = true;
            state.requests.push_back(result.signalRequest);
            return;
        }

        PLOG(logDEBUG2) << "Sending priority " << (result.isCancel ? "cancel" : "request/update")
                       << " to: " << ctrlIt->second.ip << ":" << ctrlIt->second.port
                       << "\nrequestID=" << static_cast<int>(result.trackerEntry.requestID)
                       << "\nIntersectionID=" << pkg.request.id.id
                       << "\nOID=" << result.targetOID;
        PLOG(logDEBUG2) << "Sending command:\n" << "snmpset -v1 -c public "
                        << ctrlIt->second.ip << ":" << ctrlIt->second.port << " "
                        << result.targetOID << " x " << tmx::byte_stream_encode(result.encodedPayload);

        if (SnmpSet(ctrlIt->second.snmpClient, result.targetOID, result.encodedPayload)) {
            _priorityRequestsSent++;
            SetStatus(_keyPriorityRequestsSent, _priorityRequestsSent);
            PLOG(logINFO) << "Priority " << (result.isCancel ? "cancel" : "request")
                          << " sent for requestID=" << static_cast<int>(result.trackerEntry.requestID);

            // Apply the tracker mutation only after a successful SET.
            std::lock_guard lock(_tableMutex);
            if (result.isCancel) {
                if (auto trackerIt = _prgTrackedRequests.find(result.trackerKey);
                    trackerIt != _prgTrackedRequests.end()) {
                    trackerIt->second.state = PrgRequestState::canceled;
                    trackerIt->second.sentTimeMs = input.nowMs;
                }
            }
            else {
                _prgTrackedRequests[result.trackerKey] = result.trackerEntry;
            }

            state.requests.push_back(result.signalRequest);
        }
        else {
            PLOG(logERROR) << "Failed to send priority " << (result.isCancel ? "cancel" : "request")
                           << " for requestID=" << static_cast<int>(result.trackerEntry.requestID);
            result.signalRequest.rejected = true;
            state.requests.push_back(result.signalRequest);
        }
    }

    void PriorityPlugin::SweepStaleTrackedRequests(uint64_t nowMs)
    {
        for (auto it = _prgTrackedRequests.begin(); it != _prgTrackedRequests.end(); ) {
            auto &tracked = it->second;
            auto ageSec = static_cast<long>((nowMs - tracked.sentTimeMs) / 1000);
            bool isCanceled = (tracked.state == PrgRequestState::canceled);

            switch (ClassifyStaleTrackedRequest(isCanceled, ageSec, static_cast<long>(_timeToLiveSec))) {
                case StaleTrackedAction::SendClearAndErase: {
                    auto clearEncoded = PriorityRequestProcessor::EncodePriorityClear(
                        tracked.requestID, tracked.vehicleID.data(), tracked.vehicleID.size(),
                        tracked.classType, tracked.classLevel, tracked.strategyNumber);
                    if (auto ctrlIt = _controllers.find(tracked.intersectionID);
                        ctrlIt != _controllers.end()) {
                        PLOG(logDEBUG) << "Sending prgPriorityClear for requestID=" << static_cast<int>(tracked.requestID)
                                       << " intersectionID=" << tracked.intersectionID;
                        SnmpSet(ctrlIt->second.snmpClient, tsc::mib::ntcip1211::PRIORITY_CLEAR_OID, clearEncoded);
                    }
                    it = _prgTrackedRequests.erase(it);
                    break;
                }
                case StaleTrackedAction::Evict:
                    PLOG(logDEBUG) << "Evicting stale tracked request for requestID=" << static_cast<int>(tracked.requestID);
                    it = _prgTrackedRequests.erase(it);
                    break;
                case StaleTrackedAction::Keep:
                    ++it;
                    break;
            }
        }
    }

    void PriorityPlugin::ServiceExchangeLoop()
    {
        PLOG(logINFO) << "PRS service exchange loop started, poll interval = " << _pollIntervalMs << "ms";

        while (_running) {
            // Determine which controller to communicate with.
            // Use the first controller that has any active/ready request in the table.
            std::shared_ptr<snmp_client> targetClient;
            long targetIntersectionID = 0;
            {
                std::lock_guard lock(_tableMutex);
                for (const auto &entry : _processor.Table()) {
                    if (entry.statusInPRS == RequestStatus::idleNotValid) {
                        continue;
                    }
                    auto it = _controllers.find(entry.intersectionID);
                    if (it != _controllers.end() && it->second.snmpClient) {
                        targetClient = it->second.snmpClient;
                        targetIntersectionID = it->second.intersectionID;
                        break;
                    }
                }
                // If no active requests, use first available controller (idle polling)
                if (!targetClient && !_controllers.empty()) {
                    targetClient = _controllers.begin()->second.snmpClient;
                    targetIntersectionID = _controllers.begin()->second.intersectionID;
                }
            }

            if (!targetClient) {
                PluginClientClockAware::getClock()->sleep_for(_pollIntervalMs);
                continue;
            }

            // Run one PRS-CO exchange iteration against the selected controller.
            ServiceExchangeContext ctx{_processor, _tableMutex, _lastSentServiceRequest, _prsBusy};
            DoOneServiceExchange(ctx, targetClient, targetIntersectionID,
                                 PluginClientClockAware::getClock(), _running);

            _serviceExchanges++;
            if (_serviceExchanges % 10 == 0) {
                SetStatus(_keyServiceExchanges, _serviceExchanges);
            }

            // Broadcast SSM reflecting current table state
            BroadcastSSMFromTable();

            PluginClientClockAware::getClock()->sleep_for(_pollIntervalMs);
        }

        PLOG(logINFO) << "PRS service exchange loop stopped.";
    }

    void PriorityPlugin::EncodeAndBroadcastSSM(const std::shared_ptr<SignalStatusMessage_t> &ssmPtr)
    {
        try {
            SsmEncodedMessage encodedSSM;
            MessageFrameMessage frame(ssmPtr);
            encodedSSM.set_data(TmxJ2735EncodedMessage<SignalStatusMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame));
            free(frame.get_j2735_data().get()); // NOSONAR: ASN.1 C struct allocated via calloc

            encodedSSM.set_flags(IvpMsgFlags_RouteDSRC);
            encodedSSM.addDsrcMetadata(api::signalStatusMessage_PSID);
            BroadcastMessage(static_cast<tmx::routeable_message &>(encodedSSM));
            PLOG(logDEBUG) << "SSM broadcast.";
        } catch (const std::invalid_argument &ex) {
            PLOG(logERROR) << "Failed to encode/broadcast SSM: " << ex.what();
        }
    }

    void PriorityPlugin::BroadcastSSMFromTable()
    {
        std::shared_ptr<SignalStatusMessage_t> ssmPtr;
        {
            std::lock_guard lock(_tableMutex);
            SsmBroadcastState ssmState{_ssmSequenceCounter, _signalStatusSeqByIntersection, _lastSignalStatusKey};
            ssmPtr = BuildSsmFromTable(_processor.Table(), _maxSsmBroadcastsPerStatus,
                                       static_cast<time_t>(getClock()->nowInSeconds()), ssmState);
        }

        if (ssmPtr) {
            EncodeAndBroadcastSSM(ssmPtr);
        }
    }

    void PriorityPlugin::BuildSSM(const RequestorState &state)
    {
        SsmBroadcastState ssmState{_ssmSequenceCounter, _signalStatusSeqByIntersection, _lastSignalStatusKey};
        auto ssmPtr = BuildSsmFromRequestor(state, getClock()->nowInMilliseconds(),
                                            _estimatedArrivalTime, _estimatedDepartureTime, ssmState);

        if (ssmPtr) {
            EncodeAndBroadcastSSM(ssmPtr);
            PLOG(logINFO) << "SSM (processing) broadcast for " << state.requests.size() << " request(s).";
        }
    }

} /* namespace PriorityPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<PriorityPlugin::PriorityPlugin>("Priority Plugin", argc, argv);
}
