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
    PriorityPlugin::PriorityPlugin(const std::string &name) : TmxMessageManager(name)
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
        TmxMessageManager::OnStateChange(state);
		if (state == IvpPluginState_registered) {
			UpdateConfigSettings();

            // Start the PRS-CO exchange loop thread if in PRS mode
            if (_pluginRole == "PRS" && !_running) {
                _running = true;
                _exchangeThread = std::thread(&PriorityPlugin::ServiceExchangeLoop, this);
                PLOG(logINFO) << "Started PRS-CO service exchange loop thread.";
            }
		}
    }

    void PriorityPlugin::OnConfigChanged(const char *key, const char *value)
    {
        TmxMessageManager::OnConfigChanged(key, value);
		UpdateConfigSettings();
    }

    void PriorityPlugin::OnMessageReceived(tmx::routeable_message &msg)
    {
        PLOG(logDEBUG1) << "Routable Message: " << msg.to_string();
    }

    void PriorityPlugin::UpdateConfigSettings()
    {
        std::string tscControllersJson;
        GetConfigValue<std::string>("TSC_Configuration_List", tscControllersJson);
        GetConfigValue<std::string>("PluginRole", _pluginRole);
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
        _maxSsmBroadcastsPerStatus = static_cast<uint8_t>(std::min(maxSsmBroadcasts, static_cast<uint32_t>(255)));

        // Parse per-class reservice times (comma-separated, up to 10 values)
        std::string reserviceStr;
        GetConfigValue<std::string>("ReserviceClassTimes", reserviceStr);
        _reserviceClassTime = {};
        if (!reserviceStr.empty()) {
            std::istringstream rss(reserviceStr);
            std::string tok;
            size_t idx = 0;
            while (std::getline(rss, tok, ',') && idx < 10) {
                try {
                    _reserviceClassTime[idx] = static_cast<uint32_t>(std::stoul(tok));
                } catch (const std::invalid_argument &e) {
                    PLOG(logWARNING) << "Invalid ReserviceClassTimes value at index " << idx
                                     << ": '" << tok << "' (" << e.what() << ")";
                } catch (const std::out_of_range &e) {
                    PLOG(logWARNING) << "ReserviceClassTimes value out of range at index " << idx
                                     << ": '" << tok << "' (" << e.what() << ")";
                }
                idx++;
            }
        }

        // Clear strategy mapping JSON array
        {
            std::lock_guard lock(_tableMutex);
            _processor.ClearLaneStrategyMap();
        }
        // Parse the LaneStrategyMapping JSON array and populate the processor's lane-strategy map
        std::string laneStrategyJson;
        GetConfigValue<std::string>("LaneStrategyMapping", laneStrategyJson);
        if (!laneStrategyJson.empty()) {
            try {
                boost::property_tree::ptree lsPt;
                std::istringstream lsIss(laneStrategyJson);
                boost::property_tree::read_json(lsIss, lsPt);
                for (const auto &[_, node] : lsPt) {
                    auto intID = node.get<long>("IntersectionID");
                    auto lane = node.get<long>("Lane");
                    auto strategyVal = node.get<int>("Strategy");
                    if (strategyVal < 1 || strategyVal > 255) {
                        PLOG(logWARNING) << "LaneStrategyMapping Strategy must be 1..255, skipping entry:"
                                         << " IntersectionID=" << intID << " Lane=" << lane
                                         << " Strategy=" << strategyVal;
                        continue;
                    }
                    auto strategy = static_cast<uint8_t>(strategyVal);
                    // Set the processor's lane-strategy map for a given intersection and lane
                    {
                        std::lock_guard lock(_tableMutex);
                        _processor.SetLaneStrategy(intID, lane, strategy);
                    }
                    PLOG(logINFO) << "Lane strategy: IntersectionID=" << intID
                                  << " Lane=" << lane << " Strategy=" << static_cast<int>(strategy);
                }
            } catch (const boost::property_tree::json_parser_error &e) {
                PLOG(logERROR) << "Failed to parse LaneStrategyMapping JSON: " << e.what();
            } catch (const boost::property_tree::ptree_error &e) {
                PLOG(logERROR) << "Invalid LaneStrategyMapping entry: " << e.what();
            }
        }

        // Parse the TSC_Controllers JSON array and create an SNMP client per entry
        _controllers.clear();
        try {
            boost::property_tree::ptree pt;
            std::istringstream iss(tscControllersJson);
            boost::property_tree::read_json(iss, pt);

            for (const auto &[_, node] : pt) {
                ControllerInfo info;
                info.intersectionID = node.get<long>("IntersectionID");
                info.ip = node.get<std::string>("IP");
                info.port = node.get<uint16_t>("Port");

                try {
                    info.snmpClient = std::make_shared<snmp_client>(
                        info.ip, info.port, _snmpCommunity,
                        "", "", "", "", "", "",
                        SNMP_VERSION_1, SNMP_DEFAULT_TIMEOUT);
                } catch (const snmp_client_exception &e) {
                    PLOG(logERROR) << "Failed to create SNMP client for IntersectionID="
                                   << info.intersectionID << " (" << info.ip << ":" << info.port
                                   << "): " << e.what();
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
        }

        PLOG(logINFO) << "PriorityPlugin configured: " << _controllers.size() << " controller(s)"
                                    << " Role=" << _pluginRole
                                    << " PollInterval=" << _pollIntervalMs << "ms"
                                    << " TimeToLive=" << _timeToLiveSec << "s";
    }

    void PriorityPlugin::HandleSRM(SrmMessage &msg, tmx::routeable_message &routeableMsg)
    {
        PLOG(logINFO) << "Received Signal Request Message (SRM)";
        tmx::messages::SrmEncodedMessage srmEnc(routeableMsg);
        auto srmDecoded = srmEnc.decode_j2735_message();
        auto srm = srmDecoded.get_j2735_data();
        if (!srm) {
            PLOG(logWARNING) << "SRM decode returned null, skipping.";
            _skippedMessages++;
            SetStatus(_keySkippedMessages, _skippedMessages);
            return;
        }

        if (!srm->requests || srm->requests->list.count <= 0) {
            PLOG(logWARNING) << "SRM contains no signal request packages, skipping.";
            _skippedMessages++;
            SetStatus(_keySkippedMessages, _skippedMessages);
            return;
        }

        // Extract the vehicle ID from the requestor
        std::vector<uint8_t> vehicleID;
        if (srm->requestor.id.present == VehicleID_PR_entityID) {
            auto *buf = srm->requestor.id.choice.entityID.buf;
            auto len = srm->requestor.id.choice.entityID.size;
            vehicleID.assign(buf, buf + len);
        }
        else if (srm->requestor.id.present == VehicleID_PR_stationID) {
            vehicleID.resize(sizeof(srm->requestor.id.choice.stationID));
            std::memcpy(vehicleID.data(), &srm->requestor.id.choice.stationID, vehicleID.size());
        }

        if (vehicleID.empty()) {
            PLOG(logWARNING) << "SRM has no identifiable vehicle ID, skipping.";
            _skippedMessages++;
            SetStatus(_keySkippedMessages, _skippedMessages);
            return;
        }

        std::string vehicleKey(vehicleID.begin(), vehicleID.end());

        time_t nowEpoch = std::time(nullptr);
        struct tm utcNow;
        gmtime_r(&nowEpoch, &utcNow);
        auto currentDayOfYear = utcNow.tm_yday;
        auto currentMinuteOfYear = static_cast<long>(currentDayOfYear) * 1440L
                                 + static_cast<long>(utcNow.tm_hour) * 60L
                                 + static_cast<long>(utcNow.tm_min);
        auto currentMsInMinute = static_cast<long>(utcNow.tm_sec) * 1000L;

        // Determine vehicle class from the SRM requestor type
        long role = 0;
        if (srm->requestor.type != nullptr) {
            role = srm->requestor.type->role;
        }
        auto [classType, classLevel] = PriorityRequestProcessor::MapVehicleClass(role);

        uint8_t newSeq = srm->sequenceNumber ? static_cast<uint8_t>(*srm->sequenceNumber) : 0;

        // PRS mode: handle SRM per NTCIP 1211 4.2.3.1 (how PRS receives a priority request from a PRG).
        if (_pluginRole == "PRS") {
            std::lock_guard lock(_tableMutex);
            for (int i = 0; i < srm->requests->list.count; i++) {
                const auto *pkg = srm->requests->list.array[i];
                if (pkg) {
                    ProcessPrsPackage(*pkg, vehicleID, classType, classLevel, newSeq, role,
                                      currentMinuteOfYear, currentMsInMinute, nowEpoch);
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

        // Sweep stale tracked requests - send clear for canceled entries, evict expired
        auto now = std::chrono::steady_clock::now();
        for (auto it = _prgTrackedRequests.begin(); it != _prgTrackedRequests.end(); ) {
            auto &tracked = it->second;
            auto ageSec = std::chrono::duration_cast<std::chrono::seconds>(now - tracked.sentTime).count();

            if (tracked.state == PrgRequestState::canceled && ageSec >= 2) {
                // Send prgPriorityClear to the PRS
                auto clearEncoded = PriorityRequestProcessor::EncodePriorityClear(
                    tracked.requestID, tracked.vehicleID.data(), tracked.vehicleID.size(),
                    tracked.classType, tracked.classLevel, tracked.strategyNumber);
                if (auto ctrlIt = _controllers.find(tracked.intersectionID);
                    ctrlIt != _controllers.end()) {
                    PLOG(logDEBUG) << "Sending prgPriorityClear for requestID=" << static_cast<int>(tracked.requestID)
                                   << " intersectionID=" << tracked.intersectionID;
                    SnmpSet(ctrlIt->second.snmpClient, NTCIP1211_PRIORITY_CLEAR_OID, clearEncoded);
                }
                it = _prgTrackedRequests.erase(it);
            }
            else if (ageSec >= static_cast<long>(_timeToLiveSec)) {
                PLOG(logDEBUG) << "Evicting stale tracked request for requestID=" << static_cast<int>(tracked.requestID);
                it = _prgTrackedRequests.erase(it);
            }
            else {
                ++it;
            }
        }

        RequestorState &state = _requestorStates[vehicleKey];
        state.vehicleID = vehicleID;
        state.classType = classType;
        state.sequenceNumber = newSeq;
        state.timeOfRequest = timeOfRequest;
        state.role = role;
        state.requests.clear();

        for (int i = 0; i < srm->requests->list.count; i++) {
            const auto *pkg = srm->requests->list.array[i];
            if (pkg) {
                ProcessPrgPackage(*pkg, vehicleID, vehicleKey, classType, classLevel,
                                  currentMinuteOfYear, currentMsInMinute, nowEpoch,
                                  timeOfRequest, state);
            }
        }

        uint8_t broadcastCount = _maxSsmBroadcastsPerStatus > 0 ? _maxSsmBroadcastsPerStatus : 1;
        for (uint8_t i = 0; i < broadcastCount; i++) {
            BuildSSM(state);
        }
    }

    void PriorityPlugin::ProcessPrsPackage(const SignalRequestPackage &pkg, const std::vector<uint8_t> &vehicleID, uint8_t classType, uint8_t classLevel, uint8_t newSeq, long role, long currentMinuteOfYear, long currentMsInMinute, time_t nowEpoch)
    {
        auto &table = _processor.Table();

        auto requestID = static_cast<uint8_t>(pkg.request.requestID);
        long intersectionID = pkg.request.id.id;

        // Compute global TSD and TED
        long etaOffsetMs = 0;
        if (pkg.minute) {
            auto etaMinuteOfYear = static_cast<long>(*pkg.minute);
            long etaMs = pkg.second ? static_cast<long>(*pkg.second) : 0;
            auto etaTotalMs = etaMinuteOfYear * 60L * 1000L + etaMs;
            auto nowTotalMs = currentMinuteOfYear * 60L * 1000L + currentMsInMinute;
            etaOffsetMs = etaTotalMs - nowTotalMs;
            etaOffsetMs = (etaOffsetMs < 0) ? etaOffsetMs + 525960L * 60L * 1000L : etaOffsetMs;
        }

        uint32_t globalTSD;
        uint32_t globalTED;
        if (pkg.minute) {
            globalTSD = static_cast<uint32_t>(nowEpoch) + static_cast<uint32_t>(etaOffsetMs / 1000L);
            auto departOffsetMs = etaOffsetMs;
            if (pkg.duration) {
                departOffsetMs += static_cast<long>(*pkg.duration);
            }
            globalTED = static_cast<uint32_t>(nowEpoch) + static_cast<uint32_t>(departOffsetMs / 1000L);
        }
        else {
            globalTSD = static_cast<uint32_t>(nowEpoch) + static_cast<uint32_t>(_estimatedArrivalTime);
            globalTED = static_cast<uint32_t>(nowEpoch) + static_cast<uint32_t>(_estimatedDepartureTime);
        }

        // Get strategy number for this intersection and inbound lane
        long inBoundLane = (pkg.request.inBoundLane.present == IntersectionAccessPoint_PR_lane)
                                ? static_cast<long>(pkg.request.inBoundLane.choice.lane) : -1;
        auto strategy = _processor.LookupStrategy(intersectionID, inBoundLane);

        // Check for an existing entry for this request (per 4.2.3.2 (b))
        if (auto existEntryIt = std::find_if(table.begin(), table.end(), [&](const auto &entry) {
                return entry.statusInPRS != RequestStatus::idleNotValid &&
                       entry.requestID == requestID &&
                       entry.vehicleID == vehicleID &&
                       entry.vehicleClassType == classType &&
                       entry.vehicleClassLevel == classLevel &&
                       strategy && entry.serviceStrategyNumber == *strategy;
            }); existEntryIt != table.end()) {
            // Update existing entry (priority request update per 4.2.3.2)
            existEntryIt->timeOfServiceDesiredInPRS = globalTSD;
            existEntryIt->timeOfEstimatedDepartureInPRS = globalTED;
            existEntryIt->sequenceNumber = newSeq;
            PLOG(logINFO) << "Updated priority request table entry " << (existEntryIt - table.begin())
                          << " for requestID=" << static_cast<int>(requestID);
            return;
        }

        // Find an idle slot per 4.2.3.1 (b)
        auto freeIt = std::find_if(table.begin(), table.end(), [](const auto &entry) {
            return entry.statusInPRS == RequestStatus::idleNotValid;
        });

        if (freeIt == table.end()) {
            PLOG(logWARNING) << "Priority request table full, cannot accept requestID="
                              << static_cast<int>(requestID) << " (buffer full).";
            _skippedMessages++;
            SetStatus(_keySkippedMessages, _skippedMessages);
            return;
        }

        // Store contents into the free slot per 4.2.3.1 (c-g)
        auto &entry = *freeIt;
        entry.requestID = requestID;
        entry.vehicleID = vehicleID;
        entry.vehicleClassType = classType;
        entry.vehicleClassLevel = classLevel;
        entry.role = role;
        entry.inboundPresent = pkg.request.inBoundLane.present;
        if (pkg.request.inBoundLane.present == IntersectionAccessPoint_PR_lane) {
            entry.inboundValue = pkg.request.inBoundLane.choice.lane;
        }
        else if (pkg.request.inBoundLane.present == IntersectionAccessPoint_PR_approach) {
            entry.inboundValue = pkg.request.inBoundLane.choice.approach;
        }

        if (!strategy.has_value()) {
            PLOG(logWARNING) << "No lane strategy mapping for IntersectionID=" << intersectionID
                                                                  << ", Lane=" << inBoundLane
                                                                  << ". Rejecting requestID=" << static_cast<int>(requestID);
            entry.timeOfServiceDesiredInPRS = globalTSD;
            entry.timeOfEstimatedDepartureInPRS = globalTED;
            entry.timeOfMessage = static_cast<uint32_t>(nowEpoch);
            entry.timeToLive = static_cast<uint32_t>(nowEpoch) + _timeToLiveSec;
            entry.intersectionID = intersectionID;
            entry.sequenceNumber = newSeq;
            entry.statusInPRS = RequestStatus::closedStrategyError;
            return;
        }
        entry.serviceStrategyNumber = *strategy;
        entry.timeOfServiceDesiredInPRS = globalTSD;
        entry.timeOfEstimatedDepartureInPRS = globalTED;
        entry.timeOfMessage = static_cast<uint32_t>(nowEpoch);
        entry.timeToLive = static_cast<uint32_t>(nowEpoch) + _timeToLiveSec;
        entry.intersectionID = intersectionID;
        entry.sequenceNumber = newSeq;

        // Check reservice timer per 4.2.3.1 (h)
        uint8_t classIdx = (classType >= 1 && classType <= 10) ? (classType - 1) : 9;
        auto reservicePeriod = _reserviceClassTime[classIdx];
        auto lastCompleted = _processor.ReserviceLastCompleted(classType);
        if (reservicePeriod > 0 && lastCompleted > 0 &&
            (static_cast<uint32_t>(nowEpoch) - lastCompleted) < reservicePeriod) {
            entry.statusInPRS = RequestStatus::reserviceError;
            PLOG(logINFO) << "Reservice period not met for class " << static_cast<int>(classType)
                          << ", setting reserviceError for slot " << (freeIt - table.begin());
            return;
        }

        entry.statusInPRS = RequestStatus::readyQueued;
        PLOG(logINFO) << "Accepted priority request into slot " << (freeIt - table.begin())
                      << " as readyQueued for requestID=" << static_cast<int>(requestID)
                      << " intersection=" << intersectionID;

        // Set request status per 4.2.3.1 (i) - check for override of active entries in the CO.
        if (!_prsBusy) {
            return;
        }
        for (auto &other : table) {
            if (&other == &entry) {
                continue;
            }
            bool isActive = other.statusInCO == RequestStatus::activeProcessing ||
                            other.statusInCO == RequestStatus::activeAdjustNotNeeded;
            bool isLowerClass = classType < other.vehicleClassType ||
                                (classType == other.vehicleClassType && classLevel < other.vehicleClassLevel);
            if (isActive && isLowerClass) {
                other.statusInPRS = RequestStatus::activeOverride;
                PLOG(logINFO) << "New request overrode active entry in slot: " << (&other - &table[0])
                              << " with lower priority class. Marking overridden entry as activeOverride.";
            }
        }
    }

    void PriorityPlugin::ProcessPrgPackage(const SignalRequestPackage &pkg,
                                            const std::vector<uint8_t> &vehicleID, const std::string &vehicleKey,
                                            uint8_t classType, uint8_t classLevel,
                                            long currentMinuteOfYear, long currentMsInMinute, time_t nowEpoch,
                                            uint32_t timeOfRequest, RequestorState &state)
    {
        auto requestID = static_cast<uint8_t>(pkg.request.requestID);
        auto requestType = pkg.request.requestType;
        long intersectionID = pkg.request.id.id;

        long etaOffsetMs = 0;
        if (pkg.minute) {
            auto etaMinuteOfYear = static_cast<long>(*pkg.minute);
            long etaMs = pkg.second ? static_cast<long>(*pkg.second) : 0;
            auto etaTotalMs = etaMinuteOfYear * 60L * 1000L + etaMs;
            auto nowTotalMs = currentMinuteOfYear * 60L * 1000L + currentMsInMinute;
            etaOffsetMs = etaTotalMs - nowTotalMs;
        }
        // Wrap negative offset to next year
        etaOffsetMs = (etaOffsetMs < 0) ? etaOffsetMs + 525960L * 60L * 1000L : etaOffsetMs;

        auto timeOfServiceOffsetMs = etaOffsetMs;
        auto timeOfDepartOffsetMs = timeOfServiceOffsetMs;
        if (pkg.duration) {
            timeOfDepartOffsetMs += static_cast<long>(*pkg.duration);
        }

        uint16_t timeOfService;
        uint16_t timeOfDepart;
        if (pkg.minute) {
            timeOfService = static_cast<uint16_t>(std::min(65535L, std::max(1L, timeOfServiceOffsetMs / 1000L)));
            timeOfDepart = static_cast<uint16_t>(std::min(65535L, std::max(1L, timeOfDepartOffsetMs / 1000L)));
        }
        else {
            timeOfService = _estimatedArrivalTime;
            timeOfDepart = _estimatedDepartureTime;
        }

        long inBoundLane = (pkg.request.inBoundLane.present == IntersectionAccessPoint_PR_lane)
            ? static_cast<long>(pkg.request.inBoundLane.choice.lane) : -1;
        std::optional<uint8_t> strategy;
        {
            std::lock_guard lock(_tableMutex);
            strategy = _processor.LookupStrategy(intersectionID, inBoundLane);
        }

        uint8_t inbPresent = pkg.request.inBoundLane.present;
        long inbValue = 0;
        if (inbPresent == IntersectionAccessPoint_PR_lane) {
            inbValue = pkg.request.inBoundLane.choice.lane;
        }
        else if (inbPresent == IntersectionAccessPoint_PR_approach) {
            inbValue = pkg.request.inBoundLane.choice.approach;
        }

        long etaMin = pkg.minute ? static_cast<long>(*pkg.minute) : 0;
        long etaSec = pkg.second ? static_cast<long>(*pkg.second) : 0;
        long dur = pkg.duration ? static_cast<long>(*pkg.duration) : 0;

        if (!strategy.has_value()) {
            PLOG(logWARNING) << "No lane strategy mapping for IntersectionID="
                             << intersectionID << " Lane=" << inBoundLane
                             << ", rejecting requestID=" << static_cast<int>(requestID);
            _skippedMessages++;
            SetStatus(_keySkippedMessages, _skippedMessages);
            state.requests.push_back({requestID, intersectionID, requestType,
                                      timeOfService, timeOfDepart, true,
                                      inbPresent, inbValue, etaMin, etaSec, dur});
            return;
        }

        auto ctrlIt = _controllers.find(intersectionID);
        if (ctrlIt == _controllers.end()) {
            PLOG(logWARNING) << "No controller configured for IntersectionID=" << intersectionID;
            _skippedMessages++;
            SetStatus(_keySkippedMessages, _skippedMessages);
            state.requests.push_back({requestID, intersectionID, requestType,
                                      timeOfService, timeOfDepart, true,
                                      inbPresent, inbValue, etaMin, etaSec, dur});
            return;
        }

        // Composite tracker key for this request
        std::string trackerKey = vehicleKey + "|" + std::to_string(requestID) + "|" + std::to_string(intersectionID);

        // Branch on J2735 requestType per NTCIP 1211 4.2.3.1-4.2.3.4
        std::vector<uint8_t> encoded;
        std::string targetOID;
        bool isCancelRequest = false;

        if (requestType == PriorityRequestType_priorityCancellation) {
            // requestType 3: Cancel - 21-byte payload to prgPriorityCancel OID
            encoded = PriorityRequestProcessor::EncodePriorityCancel(
                requestID, vehicleID.data(), vehicleID.size(),
                classType, classLevel, *strategy);
            targetOID = NTCIP1211_PRIORITY_CANCEL_OID;
            isCancelRequest = true;
            PLOG(logDEBUG) << "PRG cancel for requestID=" << static_cast<int>(requestID);
        }
        else if (requestType == PriorityRequestType_priorityRequestUpdate) {
            // requestType 2: Explicit update
            auto trackerIt = _prgTrackedRequests.find(trackerKey);
            if (trackerIt == _prgTrackedRequests.end()) {
                // No existing tracked request - send as new request with a warning
                PLOG(logWARNING) << "Update requested but no tracked request found for requestID="
                                 << static_cast<int>(requestID) << ", sending as new request.";
                encoded = PriorityRequestProcessor::EncodePriorityRequest(
                    requestID, vehicleID.data(), vehicleID.size(),
                    classType, classLevel, *strategy,
                    timeOfService, timeOfDepart, timeOfRequest);
                targetOID = NTCIP1211_PRIORITY_REQUEST_ABSOLUTE_OID;
            }
            else {
                encoded = PriorityRequestProcessor::EncodePriorityUpdate(
                    requestID, vehicleID.data(), vehicleID.size(),
                    classType, classLevel, *strategy,
                    timeOfService, timeOfDepart, timeOfRequest);
                targetOID = NTCIP1211_PRIORITY_UPDATE_ABSOLUTE_OID;
            }
        }
        else {
            // requestType 0 (reserved) or 1 (new request):
            // Check tracker to decide new vs update
            auto trackerIt = _prgTrackedRequests.find(trackerKey);
            if (trackerIt != _prgTrackedRequests.end() && trackerIt->second.state == PrgRequestState::sent) {
                // Existing tracked request in sent state - send as update
                encoded = PriorityRequestProcessor::EncodePriorityUpdate(
                    requestID, vehicleID.data(), vehicleID.size(),
                    classType, classLevel, *strategy,
                    timeOfService, timeOfDepart, timeOfRequest);
                targetOID = NTCIP1211_PRIORITY_UPDATE_ABSOLUTE_OID;
            }
            else {
                // No existing entry or canceled - send as new request
                encoded = PriorityRequestProcessor::EncodePriorityRequest(
                    requestID, vehicleID.data(), vehicleID.size(),
                    classType, classLevel, *strategy,
                    timeOfService, timeOfDepart, timeOfRequest);
                targetOID = NTCIP1211_PRIORITY_REQUEST_ABSOLUTE_OID;
            }
        }

        PLOG(logDEBUG2) << "Sending priority " << (isCancelRequest ? "cancel" : "request/update")
                       << " to: " << ctrlIt->second.ip << ":" << ctrlIt->second.port
                       << "\nrequestID=" << static_cast<int>(requestID)
                       << "\nIntersectionID=" << intersectionID
                       << "\nstrategy=" << static_cast<int>(*strategy)
                       << "\nOID=" << targetOID;
        PLOG(logDEBUG2) << "Sending command:\n" << "snmpset -v1 -c public "
                        << ctrlIt->second.ip << ":" << ctrlIt->second.port << " "
                        << targetOID << " x " << tmx::byte_stream_encode(encoded);

        if (SnmpSet(ctrlIt->second.snmpClient, targetOID, encoded)) {
            _priorityRequestsSent++;
            SetStatus(_keyPriorityRequestsSent, _priorityRequestsSent);
            PLOG(logINFO) << "Priority " << (isCancelRequest ? "cancel" : "request") << " sent for requestID=" << static_cast<int>(requestID);

            // Update tracker
            if (isCancelRequest) {
                auto trackerIt = _prgTrackedRequests.find(trackerKey);
                if (trackerIt != _prgTrackedRequests.end()) {
                    trackerIt->second.state = PrgRequestState::canceled;
                    trackerIt->second.sentTime = std::chrono::steady_clock::now();
                }
            }
            else {
                PrgTrackedRequest &tracked = _prgTrackedRequests[trackerKey];
                tracked.requestID = requestID;
                tracked.intersectionID = intersectionID;
                tracked.vehicleID = vehicleID;
                tracked.classType = classType;
                tracked.classLevel = classLevel;
                tracked.strategyNumber = *strategy;
                tracked.sentTime = std::chrono::steady_clock::now();
                tracked.state = PrgRequestState::sent;
            }

            state.requests.push_back({requestID, intersectionID, requestType,
                                      timeOfService, timeOfDepart, false,
                                      inbPresent, inbValue, etaMin, etaSec, dur});
        }
        else {
            PLOG(logERROR) << "Failed to send priority " << (isCancelRequest ? "cancel" : "request") << " for requestID=" << static_cast<int>(requestID);
            state.requests.push_back({requestID, intersectionID, requestType,
                                      timeOfService, timeOfDepart, true,
                                      inbPresent, inbValue, etaMin, etaSec, dur});
        }
    }

} /* namespace PriorityPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<PriorityPlugin::PriorityPlugin>("Priority Plugin", argc, argv);
}
