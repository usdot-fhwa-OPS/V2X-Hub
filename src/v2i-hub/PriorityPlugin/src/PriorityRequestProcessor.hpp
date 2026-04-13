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
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <utility>
#include <vector>

#include "PluginLog.h"

namespace PriorityPlugin {

    /**
     * @brief Owns the NTCIP 1211 priority request table and implements the 
     *        logic for PRS prioritization processing, storing CO status, and
     *        OER encode/decode.
     */
    class PriorityRequestProcessor {
        public:
            PriorityRequestProcessor() = default;

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
            static std::vector<uint8_t> EncodePriorityRequest(uint8_t requestID, const uint8_t *vehicleID, size_t vehicleIDLen, uint8_t classType, uint8_t classLevel, uint8_t strategyNum, uint16_t timeOfService, uint16_t timeOfDepart, uint32_t timeOfRequest);

            /**
             * @brief Decodes a prsServiceRequest OCTET STRING received from a CO (GET response) into per-row CO status and the coBusy flag.
             * @param data The raw 110-byte buffer.
             * @param rows Array of 10 CoServiceResponseRows.
             * @param coBusy Flag indicating if the CO reports busy.
             * @return true on successful decode, false if data size is invalid.
             */
            static bool DecodeCoServiceResponse(const std::vector<uint8_t> &data, std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> &rows, bool &coBusy);

            /**
             * @brief Maps J2735 BasicVehicleRole to NTCIP 1211 priorityRequestVehicleClassType (1..10) and priorityRequestVehicleClassLevel (1..10).
             * @param role BasicVehicleRole enumeration value from the SRM requestor type.
             * @return std::pair<uint8_t, uint8_t> NTCIP 1211 vehicle class type (1..10) and class level (1..10).
             */
            static std::pair<uint8_t, uint8_t> MapVehicleClass(long role);

            /**
             * @brief Encodes the prsServiceRequest OCTET STRING per NTCIP 1211 CO-MIB 5.2.2.1.
             * @param prsBusy The value to set for the prsBusy flag in the encoded data.
             * @return std::vector<uint8_t> 110-byte OER-encoded buffer.
             */
            std::vector<uint8_t> EncodeServiceRequest(bool prsBusy) const;

            /**
             * @brief NTCIP 1211 4.2.4.1.4 prioritization processing.
             *        Sorts the priority request table entries by class type (highest first),
             *        class level (highest first), then soonest TSD. Expires stale entries.
             *        Must be called while holding _tableMutex.
             * @param now Current epoch seconds.
             */
            void RunPrioritizationProcessing(uint32_t now);

            /**
             * @brief Applies the CO response statuses back into the priority request table
             *        per NTCIP 1211 4.2.4.1.2 step (i) and 4.3.1 state transitions.
             *        Must be called while holding _tableMutex.
             * @param coRows CO response rows.
             * @param now Current epoch seconds used to stamp reservice completion.
             */
            void ApplyCoStatusUpdates(const std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> &coRows, uint32_t now);

            /**
             * @brief Looks up the serviceStrategyNumber for a given intersection and inbound lane.
             * @param intersectionID The intersection ID from the SRM.
             * @param lane The inbound lane ID from the SRM.
             * @return std::optional<uint8_t> The service strategy number if found, std::nullopt otherwise.
             */
            std::optional<uint8_t> LookupStrategy(long intersectionID, long lane) const;

            // Accessors for the priority request table
            std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> &Table() { return _table; }
            const std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> &Table() const { return _table; }

            // Returns the epoch seconds of the last completed request for a given vehicle class type (1..10).
            uint32_t ReserviceLastCompleted(uint8_t classType) const {
                uint8_t idx = (classType >= 1 && classType <= 10) ? (classType - 1) : 9;
                return _reserviceLastCompletedTime[idx];
            }

            // Clears the lane-strategy map
            void ClearLaneStrategyMap() { _laneStrategyMap.clear(); }

            // Sets the strategy number for a given (intersection, lane) in a lane-strategy map.
            void SetLaneStrategy(long intersectionID, long lane, uint8_t strategyNumber) {
                _laneStrategyMap[{intersectionID, lane}] = strategyNumber;
            }

        private:
            // NTCIP 1211 priority request table (5.1.1.1).
            std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> _table{};

            // Last request completion time for each class type, in epoch seconds.
            std::array<uint32_t, 10> _reserviceLastCompletedTime{};

            // Lane-strategy mapping: key = (intersectionID, lane); value = strategyNumber
            std::map<std::pair<long, long>, uint8_t> _laneStrategyMap;
    };

} /* namespace PriorityPlugin */
