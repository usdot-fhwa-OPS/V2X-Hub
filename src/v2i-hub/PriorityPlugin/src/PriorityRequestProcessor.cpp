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
#include <map>

using namespace tmx::messages;
using namespace tmx::utils;

namespace PriorityPlugin {

    void PriorityPlugin::ServiceExchangeLoop()
    {
        PLOG(logINFO) << "PRS service exchange loop started, poll interval = " << _pollIntervalMs << "ms";

        while (_running) {
            // Determine which controller to communicate with.
            // Use the first controller that has any active/ready request in the table.
            std::shared_ptr<snmp_client> targetClient;
            {
                std::lock_guard<std::mutex> lock(_tableMutex);
                for (const auto &entry : _priorityRequestTable) {
                    if (entry.statusInPRS != RequestStatus::idleNotValid) {
                        auto it = _controllers.find(entry.intersectionID);
                        if (it != _controllers.end() && it->second.snmpClient) {
                            targetClient = it->second.snmpClient;
                            break;
                        }
                    }
                }
                // If no active requests, use first available controller (idle polling)
                if (!targetClient && !_controllers.empty()) {
                    targetClient = _controllers.begin()->second.snmpClient;
                }
            }

            if (!targetClient) {
                std::this_thread::sleep_for(std::chrono::milliseconds(_pollIntervalMs));
                continue;
            }

            // b) PRS shall SET prsServiceRequest to the CO. 
            // Note: steps c-e are actions on the CO side.
            std::vector<uint8_t> setData;
            {
                std::lock_guard<std::mutex> lock(_tableMutex);
                setData = EncodeServiceRequest();
            }

            PLOG(logDEBUG) << "PRS SET prsServiceRequest to CO (" << setData.size() << " bytes)";
            bool setOk = SnmpSet(targetClient, NTCIP1211_PRS_SERVICE_REQUEST_OID, setData);
            if (!setOk) {
                PLOG(logERROR) << "PRS failed to SET prsServiceRequest to CO";
                std::this_thread::sleep_for(std::chrono::milliseconds(_pollIntervalMs));
                continue;
            }

            // f) PRS shall then send a GET prsServiceRequest to the CO.
            // Note: step g is on the CO side.
            // h) If coBusy is True, keep polling GET until False
            bool coBusy = true;
            std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;
            int maxRetries = 50; // Safety limit to avoid infinite loop
            while (coBusy && _running && maxRetries-- > 0) {
                std::vector<uint8_t> getData;
                bool getOk = SnmpGet(targetClient, NTCIP1211_PRS_SERVICE_REQUEST_OID, getData);
                if (!getOk) {
                    PLOG(logERROR) << "PRS failed to GET prsServiceRequest from CO";
                    break;
                }

                if (!DecodeCoServiceResponse(getData, coRows, coBusy)) {
                    PLOG(logERROR) << "Failed to decode CO service response";
                    break;
                }

                if (coBusy) {
                    PLOG(logDEBUG) << "coBusy is True, re-polling CO...";
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }

            // i) coBusy is False: set prsBusy to True and perform prioritization
            if (!coBusy) {
                std::vector<uint8_t> updatedSetData;
                {
                    std::lock_guard<std::mutex> lock(_tableMutex);

                    // Apply CO status updates
                    ApplyCoStatusUpdates(coRows);

                    // Set prsBusy to True and perform prioritization processing
                    _prsBusy = true;
                    RunPrioritizationProcessing();

                    // j) Upon completing prioritization, set prsBusy to False
                    _prsBusy = false;
                    updatedSetData = EncodeServiceRequest();
                }

                // j) SET prsServiceRequest to the CO with updated table and prsBusy=False
                if (!SnmpSet(targetClient, NTCIP1211_PRS_SERVICE_REQUEST_OID, updatedSetData)) {
                    PLOG(logERROR) << "PRS failed to SET prsServiceRequest to CO after prioritization";
                }
            }

            _serviceExchanges++;
            SetStatus(_keyServiceExchanges, _serviceExchanges);

            // Broadcast SSM reflecting current table state
            BroadcastSSMFromTable();

            std::this_thread::sleep_for(std::chrono::milliseconds(_pollIntervalMs));
        }

        PLOG(logINFO) << "PRS service exchange loop stopped.";
    }

    void PriorityPlugin::RunPrioritizationProcessing()
    {
        uint32_t now = static_cast<uint32_t>(std::time(nullptr));

        // a) If priorityRequestStatusInPRS is 'readyX' or 'closedX',
        //    AND priorityRequestTimeToLive >= GLO.globalTime, then reset the entire
        //    priorityRequestTableEntry to its default value state (statusInPRS = idleNotValid).
        //    Status is checked by name prefix (not by numeric enum range) so that all readyX
        //    and closedX values are captured regardless of their position in the enum.
        for (auto &entry : _priorityRequestTable) {
            if (entry.statusInPRS == RequestStatus::idleNotValid) continue;

            bool readyOrClosed = IsReadyX(entry.statusInPRS) || IsClosedX(entry.statusInPRS);
            if (readyOrClosed && entry.timeToLive > 0 && entry.timeToLive >= now) {
                PLOG(logDEBUG) << "Expiring entry requestID=" << static_cast<int>(entry.requestID)
                               << " (timeToLive >= globalTime)";
                entry = PriorityRequestEntry{}; // Reset to default (sets statusInPRS to idleNotValid)
                continue;
            }

            // b) If priorityRequestTimeOfServiceDesiredInPRS >
            //    priorityRequestTimeToLive, set statusInPRS to 'closedTimeToLiveError'.
            //    The standard places no precondition on current status, so this applies to
            //    any non-idle entry whose desired service time exceeds the time-to-live.
            if (entry.timeToLive > 0 &&
                entry.timeOfServiceDesiredInPRS > entry.timeToLive) {
                entry.statusInPRS = RequestStatus::closedTimeToLiveError;
                PLOG(logDEBUG) << "requestID=" << static_cast<int>(entry.requestID)
                               << " TSD exceeds timeToLive, closedTimeToLiveError";
            }
        }

        // c) If none of the entries in the priorityRequestTable
        //    has a priorityRequestStatusInPRS of 'activeX', then reorder the table by priority.
        //    All activeX statuses are checked by name (including activeAdjustNotNeeded)
        //    rather than by numeric enum range.
        bool hasActive = false;
        for (const auto &entry : _priorityRequestTable) {
            if (IsActiveX(entry.statusInPRS)) {
                hasActive = true;
                break;
            }
        }

        if (hasActive) {
            // Check if a higher-priority readyQueued request should override the active one (4.2.3.1 i)
            PriorityRequestEntry *activeEntry = nullptr;
            for (auto &entry : _priorityRequestTable) {
                if (IsActiveX(entry.statusInPRS)) {
                    activeEntry = &entry;
                    break;
                }
            }
            if (activeEntry) {
                for (auto &entry : _priorityRequestTable) {
                    if (entry.statusInPRS == RequestStatus::readyQueued) {
                        // Higher class type = lower numeric value = higher priority
                        if (entry.vehicleClassType < activeEntry->vehicleClassType ||
                            (entry.vehicleClassType == activeEntry->vehicleClassType &&
                             entry.vehicleClassLevel < activeEntry->vehicleClassLevel)) {
                            activeEntry->statusInPRS = RequestStatus::activeOverride;
                            PLOG(logINFO) << "Higher priority request overriding active entry";
                            break;
                        }
                    }
                }
            }
        }

        if (!hasActive) {
            // Build a sortable index of readyQueued entries
            struct SortEntry {
                size_t idx;
                uint8_t classType;
                uint8_t classLevel;
                uint32_t tsd;
            };
            std::vector<SortEntry> queued;
            for (size_t i = 0; i < MAX_SERVICE_REQUESTS; i++) {
                if (_priorityRequestTable[i].statusInPRS == RequestStatus::readyQueued) {
                    queued.push_back({i,
                        _priorityRequestTable[i].vehicleClassType,
                        _priorityRequestTable[i].vehicleClassLevel,
                        _priorityRequestTable[i].timeOfServiceDesiredInPRS});
                }
            }

            // Sort ascending by classType, classLevel, and TSD
            std::sort(queued.begin(), queued.end(), [](const SortEntry &a, const SortEntry &b) {
                if (a.classType != b.classType) return a.classType < b.classType;
                if (a.classLevel != b.classLevel) return a.classLevel < b.classLevel;
                return a.tsd < b.tsd;
            });

            // Reorder the table so that the readyQueued entries are in sorted priority order
            //   c.i)   readyQueued entries sorted by priority (assigned entryNumber 1..N)
            //   c.ii)  followed by readyOverridden entries
            //   c.iii) followed by closedX entries (and reserviceError)
            //   c.iv)  followed by idleNotValid entries
            // Entry #1 (index 0) is what the CO acts on first.
            std::vector<size_t> readyOverridden, closedEntries, idleEntries;
            for (size_t i = 0; i < MAX_SERVICE_REQUESTS; i++) {
                auto s = _priorityRequestTable[i].statusInPRS;
                if (s == RequestStatus::readyQueued) continue; // already in sorted queued list
                if (s == RequestStatus::readyOverridden) readyOverridden.push_back(i);
                else if (s == RequestStatus::idleNotValid) idleEntries.push_back(i);
                else closedEntries.push_back(i); // closedX and reserviceError
            }

            std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> reordered;
            size_t pos = 0;
            for (const auto &se : queued) {
                if (pos < MAX_SERVICE_REQUESTS) reordered[pos++] = _priorityRequestTable[se.idx];
            }
            for (auto idx : readyOverridden) {
                if (pos < MAX_SERVICE_REQUESTS) reordered[pos++] = _priorityRequestTable[idx];
            }
            for (auto idx : closedEntries) {
                if (pos < MAX_SERVICE_REQUESTS) reordered[pos++] = _priorityRequestTable[idx];
            }
            for (auto idx : idleEntries) {
                if (pos < MAX_SERVICE_REQUESTS) reordered[pos++] = _priorityRequestTable[idx];
            }
            while (pos < MAX_SERVICE_REQUESTS) {
                reordered[pos++] = PriorityRequestEntry{};
            }
            _priorityRequestTable = reordered;
        }
    }

    void PriorityPlugin::ApplyCoStatusUpdates(const std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> &coRows)
    {
        // Per Figure 24 Status Transition Diagram and 5.1.1.1.9 / 5.2.1.2.5 state values. 
        // The PRS updates priorityRequestStatusInPRS based on the CO's returned
        // priorityStrategyRequestStatusInCO. Each transition is guarded by the
        // current PRS status matching the source state in the diagram.
        for (size_t i = 0; i < MAX_SERVICE_REQUESTS; i++) {
            auto &entry = _priorityRequestTable[i];
            const auto &coRow = coRows[i];

            if (entry.statusInPRS == RequestStatus::idleNotValid) continue;

            RequestStatus coStatus = coRow.requestStatusInCO;

            switch (coStatus) {
                // readyQueued/readyOverridden > activeProcessing ("CO says okay")
                case RequestStatus::activeProcessing:
                    if (IsReadyX(entry.statusInPRS)) {
                        entry.statusInPRS = RequestStatus::activeProcessing;
                        PLOG(logDEBUG) << "Row " << i << " > activeProcessing";
                    }
                    break;

                // readyQueued/readyOverridden > activeAdjustNotNeeded ("CO says okay")
                case RequestStatus::activeAdjustNotNeeded:
                    if (IsReadyX(entry.statusInPRS)) {
                        entry.statusInPRS = RequestStatus::activeAdjustNotNeeded;
                        PLOG(logDEBUG) << "Row " << i << " > activeAdjustNotNeeded";
                    }
                    break;

                // readyQueued/readyOverridden > closedTimerError ("CO says TSD & TED <> criteria")
                case RequestStatus::closedTimerError:
                    if (IsReadyX(entry.statusInPRS)) {
                        entry.statusInPRS = RequestStatus::closedTimerError;
                        PLOG(logDEBUG) << "Row " << i << " > closedTimerError";
                    }
                    break;

                // readyQueued/readyOverridden > closedStrategyError ("CO says bad strategy")
                case RequestStatus::closedStrategyError:
                    if (IsReadyX(entry.statusInPRS)) {
                        entry.statusInPRS = RequestStatus::closedStrategyError;
                        PLOG(logDEBUG) << "Row " << i << " > closedStrategyError";
                    }
                    break;

                // readyQueued/readyOverridden > closedFlash ("CO says controller in flash")
                case RequestStatus::closedFlash:
                    if (IsReadyX(entry.statusInPRS)) {
                        entry.statusInPRS = RequestStatus::closedFlash;
                        PLOG(logDEBUG) << "Row " << i << " > closedFlash";
                    }
                    break;

                // activeCancel > closedCanceled 
                // readyQueued/readyOverridden > closedCanceled ("Cancel Received")
                case RequestStatus::closedCanceled:
                    bool validSource = IsReadyX(entry.statusInPRS) ||
                                       entry.statusInPRS == RequestStatus::activeCancel;
                    if (validSource) {
                        entry.statusInPRS = RequestStatus::closedCanceled;
                        PLOG(logDEBUG) << "Row " << i << " > closedCanceled";
                    }
                    break;

                // closedCompleted can be reached from any activeX state ("CO says it finished")
                case RequestStatus::closedCompleted: {
                    if (IsActiveX(entry.statusInPRS)) {
                        entry.statusInPRS = RequestStatus::closedCompleted;
                        // Reset reservice timer (4.2.4.1.3 f))
                        uint8_t classIdx = (entry.vehicleClassType >= 1 && entry.vehicleClassType <= 10)
                                            ? (entry.vehicleClassType - 1) : 9;
                        _reserviceLastCompletedTime[classIdx] = static_cast<uint32_t>(std::time(nullptr));
                        PLOG(logDEBUG) << "Row " << i << " > closedCompleted";
                    }
                    break;
                }

                // activeOverride > activeNotOverridden ("CO can do both")
                case RequestStatus::activeNotOverridden:
                    if (entry.statusInPRS == RequestStatus::activeOverride) {
                        entry.statusInPRS = RequestStatus::activeNotOverridden;
                        PLOG(logDEBUG) << "Row " << i << " > activeNotOverridden";
                    }
                    break;

                // activeOverride > readyOverridden ("CO can terminate early")
                case RequestStatus::readyOverridden:
                    if (entry.statusInPRS == RequestStatus::activeOverride) {
                        entry.statusInPRS = RequestStatus::readyOverridden;
                        PLOG(logDEBUG) << "Row " << i << " > readyOverridden";
                    }
                    break;

                // activeOverride > readyQueued ("CO can terminate early")
                case RequestStatus::readyQueued:
                    if (entry.statusInPRS == RequestStatus::activeOverride) {
                        entry.statusInPRS = RequestStatus::readyQueued;
                        PLOG(logDEBUG) << "Row " << i << " > readyQueued";
                    }
                    break;

                default:
                    break;
            }
        }
    }

    std::vector<uint8_t> PriorityPlugin::EncodeServiceRequest() const
    {
        std::vector<uint8_t> buf(SERVICE_REQUEST_SIZE, 0);

        for (size_t i = 0; i < MAX_SERVICE_REQUESTS; i++) {
            const auto &entry = _priorityRequestTable[i];
            size_t offset = i * SERVICE_REQUEST_ROW_SIZE;

            // Byte 0: priorityRequestServiceStrategyNumber
            buf[offset + 0] = entry.serviceStrategyNumber;

            // Bytes 1-4: priorityRequestTimeOfServiceDesiredInPRS
            uint32_t tsd = entry.timeOfServiceDesiredInPRS;
            buf[offset + 1] = static_cast<uint8_t>((tsd >> 24) & 0xFF);
            buf[offset + 2] = static_cast<uint8_t>((tsd >> 16) & 0xFF);
            buf[offset + 3] = static_cast<uint8_t>((tsd >> 8) & 0xFF);
            buf[offset + 4] = static_cast<uint8_t>(tsd & 0xFF);

            // Bytes 5-8: priorityRequestTimeOfEstimatedDepartureInPRS
            uint32_t ted = entry.timeOfEstimatedDepartureInPRS;
            buf[offset + 5] = static_cast<uint8_t>((ted >> 24) & 0xFF);
            buf[offset + 6] = static_cast<uint8_t>((ted >> 16) & 0xFF);
            buf[offset + 7] = static_cast<uint8_t>((ted >> 8) & 0xFF);
            buf[offset + 8] = static_cast<uint8_t>(ted & 0xFF);

            // Byte 9: priorityRequestStatusInPRS
            buf[offset + 9] = static_cast<uint8_t>(entry.statusInPRS);
        }

        // Byte 100: prsBusy (0 = False, 1 = True)
        buf[SERVICE_REQUEST_BUSY_OFFSET] = _prsBusy ? 1 : 0;

        // Bytes 101-109: reserved (already zero)
        return buf;
    }

    bool PriorityPlugin::DecodeCoServiceResponse(const std::vector<uint8_t> &data, std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> &rows, bool &coBusy) const
    {
        if (data.size() < SERVICE_REQUEST_SIZE) {
            PLOG(logERROR) << "CO response too short: " << data.size() << " bytes (expected " << SERVICE_REQUEST_SIZE << ")";
            return false;
        }

        for (size_t i = 0; i < MAX_SERVICE_REQUESTS; i++) {
            size_t offset = i * SERVICE_REQUEST_ROW_SIZE;

            rows[i].strategyRequested = data[offset + 0];

            rows[i].requestedTimeOfServiceDesired =
                (static_cast<uint32_t>(data[offset + 1]) << 24) |
                (static_cast<uint32_t>(data[offset + 2]) << 16) |
                (static_cast<uint32_t>(data[offset + 3]) << 8)  |
                 static_cast<uint32_t>(data[offset + 4]);

            rows[i].requestedTimeOfEstimatedDeparture =
                (static_cast<uint32_t>(data[offset + 5]) << 24) |
                (static_cast<uint32_t>(data[offset + 6]) << 16) |
                (static_cast<uint32_t>(data[offset + 7]) << 8)  |
                 static_cast<uint32_t>(data[offset + 8]);

            uint8_t statusByte = data[offset + 9];
            if (statusByte >= 1 && statusByte <= 15) {
                rows[i].requestStatusInCO = static_cast<RequestStatus>(statusByte);
            } else {
                rows[i].requestStatusInCO = RequestStatus::idleNotValid;
            }
        }

        // Byte 100: coBusy (the CO uses the same position for its busy flag in response)
        coBusy = (data[SERVICE_REQUEST_BUSY_OFFSET] != 0);

        return true;
    }

    std::vector<uint8_t> PriorityPlugin::EncodePriorityRequest(uint8_t requestID, const uint8_t *vehicleID, size_t vehicleIDLen, uint8_t classType, uint8_t classLevel, uint8_t strategyNum, uint16_t timeOfService, uint16_t timeOfDepart, uint32_t timeOfRequest) const
    {
        std::vector<uint8_t> buf(PRIORITY_REQUEST_SIZE, 0);
        buf[0] = requestID;
        if (vehicleID && vehicleIDLen > 0) {
            std::memcpy(&buf[1], vehicleID, std::min(vehicleIDLen, VEHICLE_ID_FIELD_SIZE));
        }
        buf[18] = classType;
        buf[19] = classLevel;
        buf[20] = strategyNum;
        buf[21] = static_cast<uint8_t>((timeOfService >> 8) & 0xFF);
        buf[22] = static_cast<uint8_t>(timeOfService & 0xFF);
        buf[23] = static_cast<uint8_t>((timeOfDepart >> 8) & 0xFF);
        buf[24] = static_cast<uint8_t>(timeOfDepart & 0xFF);
        buf[25] = static_cast<uint8_t>((timeOfRequest >> 24) & 0xFF);
        buf[26] = static_cast<uint8_t>((timeOfRequest >> 16) & 0xFF);
        buf[27] = static_cast<uint8_t>((timeOfRequest >> 8) & 0xFF);
        buf[28] = static_cast<uint8_t>(timeOfRequest & 0xFF);
        return buf;
    }

    bool PriorityPlugin::SnmpSet(const std::shared_ptr<snmp_client> &client, const std::string &oidStr, const std::vector<uint8_t> &data)
    {
        if (!client) {
            PLOG(logERROR) << "SNMP client not initialized for SET.";
            return false;
        }

        snmp_response_obj val;
        val.type = snmp_response_obj::response_type::STRING;
        val.val_string.assign(data.begin(), data.end());

        bool success = client->process_snmp_request(oidStr, request_type::SET, val);
        if (!success) {
            PLOG(logERROR) << "SNMP SET failed for OID: " << oidStr;
        }
        return success;
    }

    bool PriorityPlugin::SnmpGet(const std::shared_ptr<snmp_client> &client, const std::string &oidStr, std::vector<uint8_t> &data)
    {
        if (!client) {
            PLOG(logERROR) << "SNMP client not initialized for GET.";
            return false;
        }

        snmp_response_obj val;
        val.type = snmp_response_obj::response_type::STRING;

        bool success = client->process_snmp_request(oidStr, request_type::GET, val);
        if (!success) {
            PLOG(logERROR) << "SNMP GET failed for OID: " << oidStr;
            return false;
        }

        data.assign(val.val_string.begin(), val.val_string.end());
        return true;
    }

    std::pair<uint8_t, uint8_t> PriorityPlugin::MapVehicleClass(long role) const
    {
        switch (role) {
            case 6:  return {1, 1}; // emergency
            case 12: return {1, 2}; // police
            case 13: return {1, 3}; // fire
            case 14: return {1, 4}; // ambulance
            case 5:  return {1, 5}; // roadRescue
            case 7:  return {1, 6}; // safetyCar
            case 11: return {1, 7}; // roadSideSource
            case 1:  return {3, 1}; // publicTransport
            case 16: return {3, 2}; // transit
            case 15: return {5, 1}; // dot
            case 4:  return {5, 2}; // roadWork
            case 3:  return {7, 1}; // dangerousGoods
            case 9:  return {7, 2}; // truck
            case 17: return {7, 3}; // slowMoving
            case 18: return {7, 4}; // stopNgo
            default: return {10, 1};
        }
    }

    std::optional<uint8_t> PriorityPlugin::LookupStrategy(long intersectionID, long lane) const
    {
        if (lane >= 0) {
            auto it = _laneStrategyMap.find({intersectionID, lane});
            if (it != _laneStrategyMap.end()) {
                return it->second;
            }
        }
        return std::nullopt;
    }

    long PriorityPlugin::MapStatusToSSM(RequestStatus status) const
    {
        switch (status) {
            case RequestStatus::readyQueued:
            case RequestStatus::readyOverridden:
                return PrioritizationResponseStatus_requested;
            case RequestStatus::activeProcessing:
            case RequestStatus::activeAdjustNotNeeded:
                return PrioritizationResponseStatus_processing;
            case RequestStatus::closedCompleted:
                return PrioritizationResponseStatus_granted;
            case RequestStatus::closedCanceled:
            case RequestStatus::closedTimerError:
            case RequestStatus::closedStrategyError:
            case RequestStatus::closedFlash:
            case RequestStatus::closedTimeToLiveError:
            case RequestStatus::reserviceError:
                return PrioritizationResponseStatus_rejected;
            case RequestStatus::activeOverride:
            case RequestStatus::activeNotOverridden:
                return PrioritizationResponseStatus_processing;
            default:
                return PrioritizationResponseStatus_unknown;
        }
    }

    void PriorityPlugin::BroadcastSSMFromTable()
    {
        // Collect non-idle entries grouped by intersection
        std::map<long, std::vector<const PriorityRequestEntry *>> byIntersection;
        {
            std::lock_guard<std::mutex> lock(_tableMutex);
            for (const auto &entry : _priorityRequestTable) {
                if (entry.statusInPRS != RequestStatus::idleNotValid) {
                    byIntersection[entry.intersectionID].push_back(&entry);
                }
            }
        }

        if (byIntersection.empty()) return;

        auto ssmPtr = std::make_shared<SignalStatusMessage_t>();
        memset(ssmPtr.get(), 0, sizeof(SignalStatusMessage_t));

        time_t nowEpoch = std::time(nullptr);
        struct tm utcNow;
        gmtime_r(&nowEpoch, &utcNow);
        ssmPtr->second = static_cast<DSecond_t>(utcNow.tm_sec * 1000);

        for (const auto &pair : byIntersection) {
            long intID = pair.first;
            const auto &entries = pair.second;

            SignalStatus *signalStatus = (SignalStatus *)calloc(1, sizeof(SignalStatus));
            signalStatus->id.id = intID;
            signalStatus->sequenceNumber = entries.front()->sequenceNumber;

            for (const auto *entry : entries) {
                SignalStatusPackage *pkg = (SignalStatusPackage *)calloc(1, sizeof(SignalStatusPackage));
                pkg->requester = (SignalRequesterInfo *)calloc(1, sizeof(SignalRequesterInfo));
                pkg->requester->request = entry->requestID;
                pkg->requester->sequenceNumber = entry->sequenceNumber;

                if (entry->vehicleID.size() == sizeof(StationID_t)) {
                    StationID_t sid = 0;
                    std::memcpy(&sid, entry->vehicleID.data(), sizeof(StationID_t));
                    pkg->requester->id.choice.stationID = sid;
                    pkg->requester->id.present = VehicleID_PR_stationID;
                } else if (!entry->vehicleID.empty()) {
                    pkg->requester->id.choice.entityID.buf =
                        (uint8_t *)calloc(entry->vehicleID.size(), 1);
                    std::memcpy(pkg->requester->id.choice.entityID.buf,
                                entry->vehicleID.data(), entry->vehicleID.size());
                    pkg->requester->id.choice.entityID.size = entry->vehicleID.size();
                    pkg->requester->id.present = VehicleID_PR_entityID;
                }

                pkg->status = MapStatusToSSM(entry->statusInPRS);
                asn_sequence_add(&signalStatus->sigStatus.list, pkg);
            }

            asn_sequence_add(&ssmPtr->status.list, signalStatus);
        }

        try {
            SsmEncodedMessage encodedSSM;
            MessageFrameMessage frame(ssmPtr);
            encodedSSM.set_data(
                TmxJ2735EncodedMessage<SignalStatusMessage>::encode_j2735_message<
                    codec::uper<MessageFrameMessage>>(frame));
            free(frame.get_j2735_data().get());
            ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());

            encodedSSM.set_flags(IvpMsgFlags_RouteDSRC);
            encodedSSM.addDsrcMetadata(0xE0000015);
            BroadcastMessage(static_cast<tmx::routeable_message &>(encodedSSM));
            PLOG(logDEBUG) << "SSM broadcast from PRS table.";
        } catch (const std::exception &ex) {
            ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());
            PLOG(logERROR) << "Failed to encode/broadcast SSM: " << ex.what();
        }
    }

    void PriorityPlugin::BuildSSM(const RequestorState &state)
    {
        if (state.requests.empty()) {
            PLOG(logWARNING) << "No signal requests in state, skipping SSM build.";
            return;
        }

        auto ssmPtr = std::make_shared<SignalStatusMessage_t>();
        memset(ssmPtr.get(), 0, sizeof(SignalStatusMessage_t));

        time_t nowEpoch = std::time(nullptr);
        struct tm utcNow;
        gmtime_r(&nowEpoch, &utcNow);
        ssmPtr->second = static_cast<DSecond_t>(utcNow.tm_sec * 1000);

        std::map<long, std::vector<const SignalRequest *>> byIntersection;
        for (const auto &req : state.requests) {
            byIntersection[req.intersectionID].push_back(&req);
        }

        for (const auto &mapEntry : byIntersection) {
            long intID = mapEntry.first;
            const auto &reqs = mapEntry.second;

            SignalStatus *signalStatus = (SignalStatus *)calloc(1, sizeof(SignalStatus));
            signalStatus->id.id = intID;
            signalStatus->sequenceNumber = state.sequenceNumber;

            for (const auto *req : reqs) {
                SignalStatusPackage *pkg = (SignalStatusPackage *)calloc(1, sizeof(SignalStatusPackage));
                pkg->requester = (SignalRequesterInfo *)calloc(1, sizeof(SignalRequesterInfo));
                pkg->requester->request = req->requestID;
                pkg->requester->sequenceNumber = state.sequenceNumber;

                if (state.vehicleID.size() == sizeof(StationID_t)) {
                    StationID_t sid = 0;
                    std::memcpy(&sid, state.vehicleID.data(), sizeof(StationID_t));
                    pkg->requester->id.choice.stationID = sid;
                    pkg->requester->id.present = VehicleID_PR_stationID;
                } else {
                    pkg->requester->id.choice.entityID.buf =
                        (uint8_t *)calloc(state.vehicleID.size(), 1);
                    std::memcpy(pkg->requester->id.choice.entityID.buf,
                                state.vehicleID.data(), state.vehicleID.size());
                    pkg->requester->id.choice.entityID.size = state.vehicleID.size();
                    pkg->requester->id.present = VehicleID_PR_entityID;
                }

                pkg->status = req->rejected ? PrioritizationResponseStatus_rejected
                                             : PrioritizationResponseStatus_processing;
                asn_sequence_add(&signalStatus->sigStatus.list, pkg);
            }
            asn_sequence_add(&ssmPtr->status.list, signalStatus);
        }

        try {
            SsmEncodedMessage encodedSSM;
            MessageFrameMessage frame(ssmPtr);
            encodedSSM.set_data(
                TmxJ2735EncodedMessage<SignalStatusMessage>::encode_j2735_message<
                    codec::uper<MessageFrameMessage>>(frame));
            free(frame.get_j2735_data().get());
            ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());

            encodedSSM.set_flags(IvpMsgFlags_RouteDSRC);
            encodedSSM.addDsrcMetadata(0xE0000015);
            BroadcastMessage(static_cast<tmx::routeable_message &>(encodedSSM));
            PLOG(logINFO) << "SSM (processing) broadcast for " << state.requests.size() << " request(s).";
        } catch (const std::exception &ex) {
            ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());
            PLOG(logERROR) << "Failed to encode/broadcast SSM: " << ex.what();
        }
    }

} /* namespace PriorityPlugin */
