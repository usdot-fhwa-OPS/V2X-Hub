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

#include <cstdint>
#include <ctime>
#include <cstring>
#include <utility>
#include <vector>

#include <PluginLog.h>
#include <tmx/j2735_messages/SignalRequestMessage.hpp>

namespace PriorityPlugin {

    /**
     * @brief Action returned by ClassifyStaleTrackedRequest for a single tracked PRG request.
     */
    enum class StaleTrackedAction : uint8_t {
        Keep,
        SendClearAndErase,
        Evict
    };

    /**
     * @brief Extracts the vehicle identifier bytes from an SRM RequestorDescription's VehicleID
     *        choice. Returns an empty vector if the choice is neither entityID nor stationID.
     * @param id the J2735 VehicleID from the SRM requestor.
     * @return std::vector<uint8_t> J2735 vehicle identifier bytes
     */
    std::vector<uint8_t> ExtractVehicleID(const VehicleID_t &id);

    /**
     * @brief Splits an epoch timestamp into the global minute-of-year and millisecond-in-minute used by SRM ETA math.
     * @param nowEpoch UTC seconds since the epoch.
     * @return std::pair<long, long> {minuteOfYear, msInMinute}
     */
    std::pair<long, long> ComputeMinuteAndMsOfYear(time_t nowEpoch);

    /**
     * @brief Computes the SRM ETA offset (positive = future, negative = past) in milliseconds.
     *        Wraps for the Dec/Jan year boundary when the raw offset exceeds half a year in the past.
     * @param etaMinuteOfYear J2735 SignalRequestPackage.minute value.
     * @param etaMs           J2735 SignalRequestPackage.second value (milliseconds), or 0 when absent.
     * @param currentMinuteOfYear current minute of year derived from system time.
     * @param currentMsInMinute   current millisecond in minute derived from system time.
     * @return long etaOffsetMs
     */
    long ComputeEtaOffsetMs(long etaMinuteOfYear, long etaMs, long currentMinuteOfYear, long currentMsInMinute);

    /**
     * @brief Logs an SRM ETA skew warning at the appropriate severity for the given offset magnitude.
     */
    void LogEtaSkew(long etaOffsetMs);

    /**
     * @brief Maps J2735 BasicVehicleRole to NTCIP 1211 priorityRequestVehicleClassType (1..10) and priorityRequestVehicleClassLevel (1..10).
     * @param role BasicVehicleRole enumeration value from the SRM requestor type.
     * @return std::pair<uint8_t, uint8_t> NTCIP 1211 vehicle class type (1..10) and class level (1..10).
     */
    std::pair<uint8_t, uint8_t> MapVehicleClass(long role);

    /**
     * @brief Classifies a tracked PRG request as Keep, SendClearAndErase, or Evict based on its
     *        canceled state and elapsed time since send.
     * @param isCanceled    true if the tracked request is in the canceled state.
     * @param ageSec        seconds elapsed since the request was sent.
     * @param timeToLiveSec seconds before a non-canceled tracked request is considered stale.
     */
    StaleTrackedAction ClassifyStaleTrackedRequest(bool isCanceled, long ageSec, long timeToLiveSec);

} /* namespace PriorityPlugin */
