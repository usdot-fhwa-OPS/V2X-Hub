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

#include "PriorityRequestProcessor.hpp"

#include <algorithm>
#include <cstring>
#include <vector>

using namespace tmx::utils;

namespace PriorityPlugin {

    std::vector<uint8_t> PriorityRequestProcessor::EncodePriorityRequest(uint8_t requestID, const uint8_t *vehicleID, size_t vehicleIDLen, uint8_t classType, uint8_t classLevel, uint8_t strategyNum, uint16_t timeOfService, uint16_t timeOfDepart, uint32_t timeOfRequest)
    {
        std::vector<uint8_t> buf(PRIORITY_REQUEST_SIZE, 0);
        buf[0] = requestID;
        if (vehicleID && vehicleIDLen > 0) {
            size_t copyLen = std::min(vehicleIDLen, VEHICLE_ID_FIELD_SIZE);
            size_t padOffset = VEHICLE_ID_FIELD_SIZE - copyLen;
            std::memcpy(&buf[1 + padOffset], vehicleID, copyLen);
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

    std::vector<uint8_t> PriorityRequestProcessor::EncodePriorityUpdate(uint8_t requestID, const uint8_t *vehicleID, size_t vehicleIDLen, uint8_t classType, uint8_t classLevel, uint8_t strategyNum, uint16_t timeOfService, uint16_t timeOfDepart, uint32_t timeOfRequest)
    {
        return EncodePriorityRequest(requestID, vehicleID, vehicleIDLen, classType, classLevel, strategyNum, timeOfService, timeOfDepart, timeOfRequest);
    }

    std::vector<uint8_t> PriorityRequestProcessor::EncodePriorityCancel(uint8_t requestID, const uint8_t *vehicleID, size_t vehicleIDLen, uint8_t classType, uint8_t classLevel, uint8_t strategyNum)
    {
        std::vector<uint8_t> buf(PRIORITY_CANCEL_SIZE, 0);
        buf[0] = requestID;
        if (vehicleID && vehicleIDLen > 0) {
            size_t copyLen = std::min(vehicleIDLen, VEHICLE_ID_FIELD_SIZE);
            size_t padOffset = VEHICLE_ID_FIELD_SIZE - copyLen;
            std::memcpy(&buf[1 + padOffset], vehicleID, copyLen);
        }
        buf[18] = classType;
        buf[19] = classLevel;
        buf[20] = strategyNum;
        return buf;
    }

    std::vector<uint8_t> PriorityRequestProcessor::EncodePriorityClear(uint8_t requestID, const uint8_t *vehicleID, size_t vehicleIDLen, uint8_t classType, uint8_t classLevel, uint8_t strategyNum)
    {
        return EncodePriorityCancel(requestID, vehicleID, vehicleIDLen, classType, classLevel, strategyNum);
    }

    bool PriorityRequestProcessor::DecodeCoServiceResponse(const std::vector<uint8_t> &data, std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> &rows, bool &coBusy)
    {
        if (data.size() < SERVICE_REQUEST_SIZE) {
            return false;
        }
        PLOG(logDEBUG4) << "Decoding CO service response (" << data.size() << " bytes)";

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
            }
            else {
                rows[i].requestStatusInCO = RequestStatus::idleNotValid;
            }
        }

        coBusy = (data[SERVICE_REQUEST_BUSY_OFFSET] != 0);
        return true;
    }

    std::pair<uint8_t, uint8_t> PriorityRequestProcessor::MapVehicleClass(long role)
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

    std::vector<uint8_t> PriorityRequestProcessor::EncodeServiceRequest(bool prsBusy) const
    {
        std::vector<uint8_t> buf(SERVICE_REQUEST_SIZE, 0);

        for (size_t i = 0; i < MAX_SERVICE_REQUESTS; i++) {
            const auto &entry = _table[i];
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
        buf[SERVICE_REQUEST_BUSY_OFFSET] = prsBusy ? 1 : 0;

        // Bytes 101-109: reserved (already zero)
        return buf;
    }

    void PriorityRequestProcessor::CheckForOverride()
    {
        PriorityRequestEntry *activeEntry = nullptr;
        // Check for active entries in the CO
        for (auto &entry : _table) {
            if (IsActiveX(entry.statusInCO)) {
                activeEntry = &entry;
                break;
            }
        }
        if (!activeEntry) {
            return;
        }

        for (const auto &entry : _table) {
            if (&entry == activeEntry) continue;
            // Check if any readyQueued entry has higher priority than the active entry
            if (entry.statusInPRS != RequestStatus::readyQueued) {
                continue;
            }
            // Determine if this entry has higher priority than the active entry
            if (entry.vehicleClassType < activeEntry->vehicleClassType ||
                (entry.vehicleClassType == activeEntry->vehicleClassType &&
                 entry.vehicleClassLevel < activeEntry->vehicleClassLevel)) {
                activeEntry->statusInPRS = RequestStatus::activeOverride;
            }
            break;
        }
    }

    void PriorityRequestProcessor::RunPrioritizationProcessing(uint32_t now)
    {
        // a) If priorityRequestStatusInPRS is 'readyX' or 'closedX',
        //    AND priorityRequestTimeToLive >= GLO.globalTime, then reset the entire
        //    priorityRequestTableEntry to its default value state (statusInPRS = idleNotValid).
        for (auto &entry : _table) {
            if (entry.statusInPRS == RequestStatus::idleNotValid) continue;

            if (bool readyOrClosed = IsReadyX(entry.statusInPRS) || IsClosedX(entry.statusInPRS);
                readyOrClosed && entry.timeToLive > 0 && now >= entry.timeToLive) {
                entry = PriorityRequestEntry{}; // reset this entry to default
                continue;
            }

            // b) If priorityRequestTimeOfServiceDesiredInPRS >
            //    priorityRequestTimeToLive, set statusInPRS to 'closedTimeToLiveError'.
            if (entry.timeToLive > 0 &&
                entry.timeOfServiceDesiredInPRS > entry.timeToLive) {
                entry.statusInPRS = RequestStatus::closedTimeToLiveError;
            }
        }

        // c) If none of the entries in the priorityRequestTable is 'activeX' at the CO,
        //    reorder by priority. Otherwise, check if a higher-priority readyQueued
        //    request should override the active one.
        bool hasActive = false;
        for (const auto &entry : _table) {
            if (IsActiveX(entry.statusInCO)) {
                hasActive = true;
                break;
            }
        }

        if (hasActive) {
            CheckForOverride();
            return;
        }

        // (i) Build a sortable index of readyQueued entries
        struct SortEntry {
            size_t idx;
            uint8_t classType;
            uint8_t classLevel;
            uint32_t tsd;
        };
        std::vector<SortEntry> queued;
        for (size_t i = 0; i < MAX_SERVICE_REQUESTS; i++) {
            if (_table[i].statusInPRS == RequestStatus::readyQueued) {
                queued.push_back({i,
                    _table[i].vehicleClassType,
                    _table[i].vehicleClassLevel,
                    _table[i].timeOfServiceDesiredInPRS});
            }
        }

        // Sort ascending by classType, classLevel, then TSD
        std::sort(queued.begin(), queued.end(), [](const SortEntry &a, const SortEntry &b) {
            if (a.classType != b.classType) return a.classType < b.classType;
            if (a.classLevel != b.classLevel) return a.classLevel < b.classLevel;
            return a.tsd < b.tsd;
        });

        // Reorder the table per sub-steps (i-iv):
        // (i)   readyQueued (pre-sorted by priority above)
        // (ii)  readyOverridden
        // (iii) closedX
        // (iv)  idleNotValid
        std::vector<size_t> readyOverridden;
        std::vector<size_t> closedEntries;
        std::vector<size_t> idleEntries;
        for (size_t i = 0; i < MAX_SERVICE_REQUESTS; i++) {
            auto s = _table[i].statusInPRS;
            if (s == RequestStatus::readyQueued) continue;
            if (s == RequestStatus::readyOverridden) readyOverridden.push_back(i);
            else if (s == RequestStatus::idleNotValid) idleEntries.push_back(i);
            else closedEntries.push_back(i);
        }

        std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> reordered;
        size_t pos = 0;
        for (const auto &se : queued) {
            if (pos < MAX_SERVICE_REQUESTS) {
                reordered[pos] = _table[se.idx];
                ++pos;
            }
        }
        for (auto idx : readyOverridden) {
            if (pos < MAX_SERVICE_REQUESTS) {
                reordered[pos] = _table[idx];
                ++pos;
            }
        }
        for (auto idx : closedEntries) {
            if (pos < MAX_SERVICE_REQUESTS) {
                reordered[pos] = _table[idx];
                ++pos;
            }
        }
        for (auto idx : idleEntries) {
            if (pos < MAX_SERVICE_REQUESTS) {
                reordered[pos] = _table[idx];
                ++pos;
            }
        }
        while (pos < MAX_SERVICE_REQUESTS) {
            reordered[pos] = PriorityRequestEntry{};
            ++pos;
        }
        _table = reordered;
    }

    void PriorityRequestProcessor::ApplyCoStatusUpdates(const std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> &coRows, uint32_t now)
    {
        for (size_t i = 0; i < MAX_SERVICE_REQUESTS; i++) {
            auto &entry = _table[i];
            const auto &coRow = coRows[i];

            if (entry.statusInPRS == RequestStatus::idleNotValid) continue;

            RequestStatus coStatus = coRow.requestStatusInCO;
            entry.statusInCO = coStatus;

            switch (coStatus) {
                // CO-owned states, tracked in statusInCO above.
                case RequestStatus::activeProcessing:
                case RequestStatus::activeAdjustNotNeeded:
                case RequestStatus::activeCancel:
                case RequestStatus::activeNotOverridden:
                    PLOG(logDEBUG) << "Row " << i << ": CO reports " << static_cast<int>(coStatus);
                    break;

                // readyQueued/readyOverridden -> closedTimerError ("CO says TSD & TED <> criteria")
                case RequestStatus::closedTimerError:
                    if (IsReadyX(entry.statusInPRS)) {
                        entry.statusInPRS = RequestStatus::closedTimerError;
                        PLOG(logDEBUG) << "Row " << i << ": closedTimerError";
                    }
                    break;

                // readyQueued/readyOverridden -> closedStrategyError ("CO says bad strategy")
                case RequestStatus::closedStrategyError:
                    if (IsReadyX(entry.statusInPRS)) {
                        entry.statusInPRS = RequestStatus::closedStrategyError;
                        PLOG(logDEBUG) << "Row " << i << ": closedStrategyError";
                    }
                    break;

                // readyQueued/readyOverridden -> closedFlash ("CO says controller in flash")
                case RequestStatus::closedFlash:
                    if (IsReadyX(entry.statusInPRS)) {
                        entry.statusInPRS = RequestStatus::closedFlash;
                        PLOG(logDEBUG) << "Row " << i << ": closedFlash";
                    }
                    break;

                // readyQueued/readyOverridden/activeCancel-in-CO -> closedCanceled ("Cancel Received")
                case RequestStatus::closedCanceled:
                    if (IsReadyX(entry.statusInPRS)) {
                        entry.statusInPRS = RequestStatus::closedCanceled;
                        PLOG(logDEBUG) << "Row " << i << ": closedCanceled";
                    }
                    break;

                // readyX held in PRS status for the whole active phase
                // Trigger is the CO's status. Accept it if the PRS has not already closed the entry. ("CO says it finished")
                case RequestStatus::closedCompleted:
                    if (IsReadyX(entry.statusInPRS) || entry.statusInPRS == RequestStatus::activeOverride) {
                        entry.statusInPRS = RequestStatus::closedCompleted;
                        // Reset reservice timer (4.2.4.1.3 (f))
                        uint8_t classIdx = (entry.vehicleClassType >= 1 && entry.vehicleClassType <= 10) ? (entry.vehicleClassType - 1) : 9;
                        _reserviceLastCompletedTime[classIdx] = now;
                        PLOG(logDEBUG) << "Row " << i << ": closedCompleted";
                    }
                    break;

                // activeOverride -> readyOverridden ("CO can terminate early")
                case RequestStatus::readyOverridden:
                    if (entry.statusInPRS == RequestStatus::activeOverride) {
                        entry.statusInPRS = RequestStatus::readyOverridden;
                        PLOG(logDEBUG) << "Row " << i << ": readyOverridden";
                    }
                    break;

                // activeOverride -> readyQueued ("CO can terminate early")
                case RequestStatus::readyQueued:
                    if (entry.statusInPRS == RequestStatus::activeOverride) {
                        entry.statusInPRS = RequestStatus::readyQueued;
                        PLOG(logDEBUG) << "Row " << i << ": readyQueued";
                    }
                    break;

                default:
                    break;
            }
        }
    }

    std::optional<uint8_t> PriorityRequestProcessor::LookupStrategy(long intersectionID, long lane) const
    {
        if (lane >= 0) {
            auto it = _laneStrategyMap.find({intersectionID, lane});
            if (it != _laneStrategyMap.end()) {
                return it->second;
            }
        }
        return std::nullopt;
    }

} /* namespace PriorityPlugin */
