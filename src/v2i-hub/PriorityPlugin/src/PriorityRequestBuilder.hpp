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
#include "PriorityPluginWorker.hpp"
#include "PriorityTypes.hpp"

#include <array>
#include <cstdint>
#include <ctime>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <tmx/j2735_messages/SignalRequestMessage.hpp>

namespace PriorityPlugin {

    /**
     * @brief Result of applying one SRM signal request package to the PRS priority request table.
     *        The caller holds _tableMutex and updates any status/UI effects (e.g. SkippedMessages) (PRS mode).
     */
    struct PrsPackageResult {
        enum class Action : uint8_t {
            Updated,    // Existing entry updated (priority request update).
            Inserted,   // New entry stored as readyQueued.
            Rejected,   // Stored with a closed/error status (strategy missing or reservice locked).
            TableFull   // No free slot; nothing stored.
        };

        Action action = Action::TableFull;
        size_t slotIndex = 0;            // Index into the table for action
        bool overrideTriggered = false;  // True if any active entry is set to activeOverride.
    };

    /**
     * @brief Result of deciding how to convert one SRM signal request package into an outgoing NTCIP 1211 priority request (PRG mode).
     */
    struct PrgPackageResult {
        enum class Outcome : uint8_t {
            Send,           // Send encodedPayload to targetOID, then apply the tracker mutation on success.
            NoStrategy,     // No lane-strategy mapping; nothing to send (request rejected).
            NoController    // No controller configured for the intersection; nothing to send (request rejected).
        };

        Outcome outcome = Outcome::NoStrategy;
        std::vector<uint8_t> encodedPayload;  // OER-encoded request/update/cancel
        std::string targetOID;                // NTCIP 1211 OID to SET
        bool isCancel = false;                // True if this is a priority cancel
        SignalRequest signalRequest;          // The request to record in RequestorState
        std::string trackerKey;               // Composite tracker key for this request
        PrgTrackedRequest trackerEntry;       // Tracker entry to store on a successful non-cancel SET
    };

    /**
     * @brief Request inputs for ApplyPrsPackage.
     */
    struct PrsPackageInput {
        const std::vector<uint8_t> &vehicleID;      // decoded vehicle ID bytes
        uint8_t classType = 0;                      // mapped NTCIP 1211 vehicle class type (1..10)
        uint8_t classLevel = 0;                     // mapped NTCIP 1211 vehicle class level (1..10)
        uint8_t newSeq = 0;                         // SRM sequence number
        long role = 0;                              // BasicVehicleRole
        long currentMinuteOfYear = 0;               // current minute of year
        long currentMsInMinute = 0;                 // current millisecond in minute
        time_t nowEpoch = 0;                        // current time in epoch seconds
        const std::array<uint32_t, 10> &reserviceClassTime;  // per-class reservice periods (seconds)
        uint32_t timeToLiveSec = 0;                 // seconds a stored request remains valid
        uint16_t estimatedArrivalTime = 0;          // fallback time-of-service when the package has no minute
        uint16_t estimatedDepartureTime = 0;        // fallback time-of-departure when the package has no minute
        bool prsBusy = false;                       // true while prioritization is in progress (enables override)
    };

    /**
     * @brief Request inputs for BuildPrgPackage.
     */
    struct PrgPackageInput {
        const std::vector<uint8_t> &vehicleID;   // decoded vehicle ID bytes
        const std::string &vehicleKey;           // string key for this vehicle ID
        uint8_t classType = 0;                   // mapped NTCIP 1211 vehicle class type (1..10)
        uint8_t classLevel = 0;                  // mapped NTCIP 1211 vehicle class level (1..10)
        long currentMinuteOfYear = 0;            // current minute of year
        long currentMsInMinute = 0;              // current millisecond in minute
        uint32_t timeOfRequest = 0;              // time of request to embed (epoch seconds)
        uint16_t estimatedArrivalTime = 0;       // fallback time-of-service when the package has no minute
        uint16_t estimatedDepartureTime = 0;     // fallback time-of-departure when the package has no minute
        uint64_t nowMs = 0;                      // current utc time in milliseconds (stamps the tracker entry)
    };


    /**
     * @brief Applies a request package to the PRS priority request table. 
     * @param table      the PRS priority request table (caller holds the table mutex)
     * @param processor  provides lane-strategy lookup and reservice timestamps
     * @param pkg        the J2735 signal request package to process
     * @param input      per-request context (vehicle, class, timing, TTL, override state)
     * @return PrsPackageResult describing what happened to the table.
     */
    PrsPackageResult ApplyPrsPackage( std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> &table, const PriorityRequestProcessor &processor, const SignalRequestPackage &pkg, const PrsPackageInput &input);

    /**
     * @brief Decides how to convert one SRM signal request package into an outgoing NTCIP 1211
     *        priority request, update, or cancel (PRG mode). Performs no SNMP IO; the caller
     *        sends encodedPayload to targetOID and, on success, applies the tracker mutation and
     *        records signalRequest. Unit-testable: supply inputs, assert on the returned result.
     * @param trackedRequests           map of composite key to currently tracked PRG requests.
     * @param configuredIntersectionIDs intersection IDs that have a configured controller.
     * @param processor                 provides lane-strategy lookup (read-only).
     * @param pkg                       the J2735 signal request package to process.
     * @param input                     per-request context (vehicle, class, timing, estimates).
     * @return PrgPackageResult describing what to send and how to update tracking.
     */
    PrgPackageResult BuildPrgPackage( const std::unordered_map<std::string, PrgTrackedRequest> &trackedRequests, const std::unordered_set<long> &configuredIntersectionIDs, const PriorityRequestProcessor &processor, const SignalRequestPackage &pkg, const PrgPackageInput &input);

} /* namespace PriorityPlugin */
