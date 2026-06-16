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

#include "PriorityPluginWorker.hpp"

using namespace tmx::utils;

namespace PriorityPlugin {

    std::vector<uint8_t> ExtractVehicleID(const VehicleID_t &id)
    {
        std::vector<uint8_t> vehicleID;
        if (id.present == VehicleID_PR_entityID) {
            const auto *buf = id.choice.entityID.buf;
            auto len = id.choice.entityID.size;
            if (buf && len > 0) {
                vehicleID.assign(buf, buf + len);
            }
        }
        else if (id.present == VehicleID_PR_stationID) {
            vehicleID.resize(sizeof(id.choice.stationID));
            std::memcpy(vehicleID.data(), &id.choice.stationID, vehicleID.size());
        }
        return vehicleID;
    }

    std::pair<long, long> ComputeMinuteAndMsOfYear(time_t nowEpoch)
    {
        struct tm utcNow;
        gmtime_r(&nowEpoch, &utcNow);
        auto minuteOfYear = static_cast<long>(utcNow.tm_yday) * 1440L
                          + static_cast<long>(utcNow.tm_hour) * 60L
                          + static_cast<long>(utcNow.tm_min);
        auto msInMinute = static_cast<long>(utcNow.tm_sec) * 1000L;
        return {minuteOfYear, msInMinute};
    }

    long ComputeEtaOffsetMs(long etaMinuteOfYear, long etaMs,
                            long currentMinuteOfYear, long currentMsInMinute)
    {
        auto etaTotalMs = etaMinuteOfYear * 60L * 1000L + etaMs;
        auto nowTotalMs = currentMinuteOfYear * 60L * 1000L + currentMsInMinute;
        long etaOffsetMs = etaTotalMs - nowTotalMs;
        // Only wrap when the Dec/Jan boundary is the more plausible interpretation
        // (raw offset > half a year in the past). Small negatives from clock skew or
        // latency fall through unchanged so the CO can judge them on its own.
        if (constexpr long YEAR_MS = 525960L * 60L * 1000L; etaOffsetMs < -YEAR_MS / 2) {
            etaOffsetMs += YEAR_MS;
        }
        return etaOffsetMs;
    }

    void LogEtaSkew(long etaOffsetMs)
    {
        long absMs = (etaOffsetMs < 0) ? -etaOffsetMs : etaOffsetMs;
        const char *sign = (etaOffsetMs < 0) ? "past" : "future";

        if (absMs > 30000) {
            PLOG(logERROR)   << "SRM ETA " << absMs << "ms in " << sign << " (check clock sync)";
        }
        else if (absMs > 5000) {
            PLOG(logWARNING) << "SRM ETA " << absMs << "ms in " << sign;
        }
        else if (absMs > 2000) {
            PLOG(logDEBUG)   << "SRM ETA " << absMs << "ms in " << sign;
        }
    }

    std::pair<uint8_t, uint8_t> MapVehicleClass(long role)
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

    StaleTrackedAction ClassifyStaleTrackedRequest(bool isCanceled, long ageSec, long timeToLiveSec)
    {
        if (isCanceled && ageSec >= 2) {
            return StaleTrackedAction::SendClearAndErase;
        }
        if (ageSec >= timeToLiveSec) {
            return StaleTrackedAction::Evict;
        }
        return StaleTrackedAction::Keep;
    }

} /* namespace PriorityPlugin */
