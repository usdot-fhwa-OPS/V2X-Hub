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

#include <array>
#include <cstdint>
#include <ctime>
#include <memory>
#include <string>
#include <unordered_map>

#include <tmx/j2735_messages/SignalStatusMessage.hpp>

namespace PriorityPlugin {

    /**
     * @brief References to the SSM sequencing state shared by both SSM builders.
     */
    struct SsmBroadcastState {
        uint8_t &ssmSequenceCounter; // sequence number for each SSM entry, incremented on SSM build.
        std::unordered_map<long, uint8_t> &signalStatusSeqByIntersection; // Per-intersection signalStatus sequenceNumber.
        std::unordered_map<long, std::string> &lastSignalStatusKey; // Key for latest signalStatus package per intersection.
    };

    /**
     * @brief Maps the NTCIP 1211 priority request/strategy status to the J2735 PrioritizationResponseStatus used in an SSM signalStatus package.
     * @param status the PRS/CO request status
     * @return the corresponding J2735 PrioritizationResponseStatus enumeration value.
     */
    long MapNTCIPstatusToSSM(RequestStatus status);

    /**
     * @brief Builds a SignalStatusMessage reflecting the current PRS priorityRequestTable (PRS mode).
     * @param table                   PRS priority request table
     * @param maxBroadcastsPerStatus  per-status broadcast cap
     * @param nowEpoch                current time in epoch seconds
     * @param ssmState                shared sequencing state
     * @return shared_ptr to the SSM, or nullptr if no entries qualify (caller skips broadcast).
     */
    std::shared_ptr<SignalStatusMessage_t> BuildSsmFromTable(std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> &table, uint8_t maxBroadcastsPerStatus, time_t nowEpoch, SsmBroadcastState &ssmState);

    /**
     * @brief Builds a SignalStatusMessage reflecting the requests in a single requestor state (PRG mode).
     * @param state                   the requestor state
     * @param nowMs                   current time in milliseconds
     * @param estimatedArrivalTime    fallback ETA offset (seconds) when a request has no minute
     * @param estimatedDepartureTime  fallback departure offset (seconds) when a request has no duration
     * @param ssmState                shared sequencing state
     * @return shared_ptr to the SSM, or nullptr if the state has no requests.
     */
    std::shared_ptr<SignalStatusMessage_t> BuildSsmFromRequestor(const RequestorState &state, uint64_t nowMs, uint16_t estimatedArrivalTime, uint16_t estimatedDepartureTime, SsmBroadcastState &ssmState);

} /* namespace PriorityPlugin */
