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

#include "PriorityRequestBuilder.hpp"

#include <algorithm>
#include <tsc/NTCIP_1211_MIB.h>

using namespace tmx::utils;

namespace PriorityPlugin {

    PrsPackageResult ApplyPrsPackage(std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> &table, const PriorityRequestProcessor &processor, const SignalRequestPackage &pkg,const PrsPackageInput &input)
    {
        PrsPackageResult result;

        auto requestID = static_cast<uint8_t>(pkg.request.requestID);
        long intersectionID = pkg.request.id.id;

        // Compute global TSD and TED
        long etaOffsetMs = 0;
        if (pkg.minute) {
            etaOffsetMs = ComputeEtaOffsetMs(static_cast<long>(*pkg.minute), pkg.second ? static_cast<long>(*pkg.second) : 0, input.currentMinuteOfYear, input.currentMsInMinute);
            LogEtaSkew(etaOffsetMs);
        }

        uint32_t globalTSD;
        uint32_t globalTED;
        if (pkg.minute) {
            globalTSD = static_cast<uint32_t>(input.nowEpoch) + static_cast<uint32_t>(etaOffsetMs / 1000L);
            auto departOffsetMs = etaOffsetMs;
            if (pkg.duration) {
                departOffsetMs += static_cast<long>(*pkg.duration);
            }
            globalTED = static_cast<uint32_t>(input.nowEpoch) + static_cast<uint32_t>(departOffsetMs / 1000L);
        }
        else {
            globalTSD = static_cast<uint32_t>(input.nowEpoch) + static_cast<uint32_t>(input.estimatedArrivalTime);
            globalTED = static_cast<uint32_t>(input.nowEpoch) + static_cast<uint32_t>(input.estimatedDepartureTime);
        }

        // Get strategy number for this intersection and inBound lane
        long inBoundLane = (pkg.request.inBoundLane.present == IntersectionAccessPoint_PR_lane) ? static_cast<long>(pkg.request.inBoundLane.choice.lane) : -1;
        auto strategy = processor.LookupStrategy(intersectionID, inBoundLane);

        // Check for an existing entry for this request (per 4.2.3.2 (b))
        if (auto existEntryIt = std::find_if(table.begin(), table.end(), [&](const auto &entry) {
                return entry.statusInPRS != RequestStatus::idleNotValid &&
                       entry.requestID == requestID &&
                       entry.vehicleID == input.vehicleID &&
                       entry.vehicleClassType == input.classType &&
                       entry.vehicleClassLevel == input.classLevel &&
                       strategy && entry.serviceStrategyNumber == *strategy;
            }); existEntryIt != table.end()) 
        {
            // Update existing entry (priority request update per 4.2.3.2)
            existEntryIt->timeOfServiceDesiredInPRS = globalTSD;
            existEntryIt->timeOfEstimatedDepartureInPRS = globalTED;
            existEntryIt->sequenceNumber = input.newSeq;
            result.action = PrsPackageResult::Action::Updated;
            result.slotIndex = static_cast<size_t>(existEntryIt - table.begin());
            PLOG(logINFO) << "Updated priority request table entry " << result.slotIndex
                          << " for requestID=" << static_cast<int>(requestID);
            return result;
        }

        // Find an idle slot per 4.2.3.1 (b)
        auto freeIt = std::find_if(table.begin(), table.end(), [](const auto &entry) {
            return entry.statusInPRS == RequestStatus::idleNotValid;
        });

        if (freeIt == table.end()) {
            PLOG(logWARNING) << "Priority request table full, cannot accept requestID="
                              << static_cast<int>(requestID) << " (buffer full).";
            result.action = PrsPackageResult::Action::TableFull;
            return result;
        }

        result.slotIndex = static_cast<size_t>(freeIt - table.begin());

        // Store contents into the free slot per 4.2.3.1 (c-g)
        auto &entry = *freeIt;
        entry.requestID = requestID;
        entry.vehicleID = input.vehicleID;
        entry.vehicleClassType = input.classType;
        entry.vehicleClassLevel = input.classLevel;
        entry.role = input.role;
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
            entry.timeOfMessage = static_cast<uint32_t>(input.nowEpoch);
            entry.timeToLive = static_cast<uint32_t>(input.nowEpoch) + input.timeToLiveSec;
            entry.intersectionID = intersectionID;
            entry.sequenceNumber = input.newSeq;
            entry.statusInPRS = RequestStatus::closedStrategyError;
            result.action = PrsPackageResult::Action::Rejected;
            return result;
        }
        entry.serviceStrategyNumber = *strategy;
        entry.timeOfServiceDesiredInPRS = globalTSD;
        entry.timeOfEstimatedDepartureInPRS = globalTED;
        entry.timeOfMessage = static_cast<uint32_t>(input.nowEpoch);
        entry.timeToLive = static_cast<uint32_t>(input.nowEpoch) + input.timeToLiveSec;
        entry.intersectionID = intersectionID;
        entry.sequenceNumber = input.newSeq;

        // Check reservice timer per 4.2.3.1 (h)
        uint8_t classIdx = (input.classType >= 1 && input.classType <= 10) ? (input.classType - 1) : 9;
        auto reservicePeriod = input.reserviceClassTime[classIdx];
        if (auto lastCompleted = processor.ReserviceLastCompleted(input.classType);
            reservicePeriod > 0 && lastCompleted > 0 &&
            (static_cast<uint32_t>(input.nowEpoch) - lastCompleted) < reservicePeriod) {
            entry.statusInPRS = RequestStatus::reserviceError;
            result.action = PrsPackageResult::Action::Rejected;
            PLOG(logINFO) << "Reservice period not met for class " << static_cast<int>(input.classType)
                          << ", setting reserviceError for slot " << result.slotIndex;
            return result;
        }

        entry.statusInPRS = RequestStatus::readyQueued;
        result.action = PrsPackageResult::Action::Inserted;
        PLOG(logINFO) << "Accepted priority request into slot " << result.slotIndex
                      << " as readyQueued for requestID=" << static_cast<int>(requestID)
                      << " intersection=" << intersectionID;

        // Set request status per 4.2.3.1 (i) - check for override of active entries in the CO.
        if (!input.prsBusy) {
            return result;
        }
        for (auto &other : table) {
            if (&other == &entry) {
                continue;
            }
            bool isActive = other.statusInCO == RequestStatus::activeProcessing ||
                            other.statusInCO == RequestStatus::activeAdjustNotNeeded;
            bool isLowerClass = input.classType < other.vehicleClassType ||
                                (input.classType == other.vehicleClassType && input.classLevel < other.vehicleClassLevel);
            if (isActive && isLowerClass) {
                other.statusInPRS = RequestStatus::activeOverride;
                result.overrideTriggered = true;
                PLOG(logINFO) << "New request overrode active entry in slot: " << (&other - &table[0])
                              << " with lower priority class. Marking overridden entry as activeOverride.";
            }
        }
        return result;
    }

    PrgPackageResult BuildPrgPackage(const std::unordered_map<std::string, PrgTrackedRequest> &trackedRequests, const std::unordered_set<long> &configuredIntersectionIDs, const PriorityRequestProcessor &processor,const SignalRequestPackage &pkg, const PrgPackageInput &input)
    {
        PrgPackageResult result;

        auto requestID = static_cast<uint8_t>(pkg.request.requestID);
        auto requestType = pkg.request.requestType;
        long intersectionID = pkg.request.id.id;

        long etaOffsetMs = 0;
        if (pkg.minute) {
            etaOffsetMs = ComputeEtaOffsetMs(static_cast<long>(*pkg.minute), pkg.second ? static_cast<long>(*pkg.second) : 0, input.currentMinuteOfYear, input.currentMsInMinute);
            LogEtaSkew(etaOffsetMs);
        }

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
            timeOfService = input.estimatedArrivalTime;
            timeOfDepart = input.estimatedDepartureTime;
        }

        long inBoundLane = (pkg.request.inBoundLane.present == IntersectionAccessPoint_PR_lane) ? static_cast<long>(pkg.request.inBoundLane.choice.lane) : -1;
        auto strategy = processor.LookupStrategy(intersectionID, inBoundLane);

        uint8_t inboundPresent = pkg.request.inBoundLane.present;
        long inboundValue = 0;
        if (inboundPresent == IntersectionAccessPoint_PR_lane) {
            inboundValue = pkg.request.inBoundLane.choice.lane;
        }
        else if (inboundPresent == IntersectionAccessPoint_PR_approach) {
            inboundValue = pkg.request.inBoundLane.choice.approach;
        }

        long etaMin = pkg.minute ? static_cast<long>(*pkg.minute) : 0;
        long etaSec = pkg.second ? static_cast<long>(*pkg.second) : 0;
        long duration = pkg.duration ? static_cast<long>(*pkg.duration) : 0;

        result.signalRequest = {requestID, intersectionID, requestType, timeOfService, timeOfDepart, false, inboundPresent, inboundValue, etaMin, etaSec, duration};

        if (!strategy.has_value()) {
            PLOG(logWARNING) << "No lane strategy mapping for IntersectionID="
                             << intersectionID << " Lane=" << inBoundLane
                             << ", rejecting requestID=" << static_cast<int>(requestID);
            result.outcome = PrgPackageResult::Outcome::NoStrategy;
            result.signalRequest.rejected = true;
            return result;
        }

        if (configuredIntersectionIDs.find(intersectionID) == configuredIntersectionIDs.end()) {
            PLOG(logWARNING) << "No controller configured for IntersectionID=" << intersectionID;
            result.outcome = PrgPackageResult::Outcome::NoController;
            result.signalRequest.rejected = true;
            return result;
        }

        // Composite tracker key for this request
        std::string trackerKey = input.vehicleKey + "|" + std::to_string(requestID) + "|" + std::to_string(intersectionID);
        result.trackerKey = trackerKey;

        // Branch on J2735 requestType per NTCIP 1211 4.2.3.1-4.2.3.4
        if (requestType == PriorityRequestType_priorityCancellation) {
            // requestType 3: Cancel - 21-byte payload to prgPriorityCancel OID
            result.encodedPayload = PriorityRequestProcessor::EncodePriorityCancel(
                requestID, input.vehicleID.data(), input.vehicleID.size(),
                input.classType, input.classLevel, *strategy);
            result.targetOID = tsc::mib::ntcip1211::NTCIP1211_PRIORITY_CANCEL_OID;
            result.isCancel = true;
            PLOG(logDEBUG) << "PRG cancel for requestID=" << static_cast<int>(requestID);
        }
        else if (requestType == PriorityRequestType_priorityRequestUpdate) {
            // requestType 2: Explicit update
            auto trackerIt = trackedRequests.find(trackerKey);
            if (trackerIt == trackedRequests.end()) {
                // No existing tracked request - send as new request with a warning
                PLOG(logWARNING) << "Update requested but no tracked request found for requestID="
                                 << static_cast<int>(requestID) << ", sending as new request.";
                result.encodedPayload = PriorityRequestProcessor::EncodePriorityRequest(
                    requestID, input.vehicleID.data(), input.vehicleID.size(),
                    input.classType, input.classLevel, *strategy,
                    timeOfService, timeOfDepart, input.timeOfRequest);
                result.targetOID = tsc::mib::ntcip1211::NTCIP1211_PRIORITY_REQUEST_ABSOLUTE_OID;
            }
            else {
                result.encodedPayload = PriorityRequestProcessor::EncodePriorityUpdate(
                    requestID, input.vehicleID.data(), input.vehicleID.size(),
                    input.classType, input.classLevel, *strategy,
                    timeOfService, timeOfDepart, input.timeOfRequest);
                result.targetOID = tsc::mib::ntcip1211::NTCIP1211_PRIORITY_UPDATE_ABSOLUTE_OID;
            }
        }
        else {
            // requestType 0 (reserved) or 1 (new request):
            // Check tracker to decide new vs update
            auto trackerIt = trackedRequests.find(trackerKey);
            if (trackerIt != trackedRequests.end() && trackerIt->second.state == PrgRequestState::sent) {
                // Existing tracked request in sent state - send as update
                result.encodedPayload = PriorityRequestProcessor::EncodePriorityUpdate(
                    requestID, input.vehicleID.data(), input.vehicleID.size(),
                    input.classType, input.classLevel, *strategy,
                    timeOfService, timeOfDepart, input.timeOfRequest);
                result.targetOID = tsc::mib::ntcip1211::NTCIP1211_PRIORITY_UPDATE_ABSOLUTE_OID;
            }
            else {
                // No existing entry or canceled - send as new request
                result.encodedPayload = PriorityRequestProcessor::EncodePriorityRequest(
                    requestID, input.vehicleID.data(), input.vehicleID.size(),
                    input.classType, input.classLevel, *strategy,
                    timeOfService, timeOfDepart, input.timeOfRequest);
                result.targetOID = tsc::mib::ntcip1211::NTCIP1211_PRIORITY_REQUEST_ABSOLUTE_OID;
            }
        }

        result.outcome = PrgPackageResult::Outcome::Send;

        // Tracker entry the caller applies after a successful non-cancel SET.
        result.trackerEntry.requestID = requestID;
        result.trackerEntry.intersectionID = intersectionID;
        result.trackerEntry.vehicleID = input.vehicleID;
        result.trackerEntry.classType = input.classType;
        result.trackerEntry.classLevel = input.classLevel;
        result.trackerEntry.strategyNumber = *strategy;
        result.trackerEntry.sentTimeMs = input.nowMs;
        result.trackerEntry.state = PrgRequestState::sent;

        return result;
    }

} /* namespace PriorityPlugin */
