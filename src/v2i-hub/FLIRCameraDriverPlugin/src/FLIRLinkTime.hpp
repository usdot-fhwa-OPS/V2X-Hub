/**
 * Copyright (C) 2025 LEIDOS.
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
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <string>

namespace FLIRCameraDriverPlugin
{
    /**
     * @brief Current host wall-clock time in milliseconds since epoch. Camera link monitoring always uses the real
     * clock, also in simulation, because it measures real network delays.
     */
    inline uint64_t wallClockMs()
    {
        return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    }

    /**
     * @brief Formats a time in milliseconds since epoch as UTC ISO 8601, e.g. 2026-09-15T17:47:10.036Z
     */
    inline std::string formatUtcMs(uint64_t epochMs)
    {
        const auto seconds = static_cast<std::time_t>(epochMs / 1000);
        std::tm utc{};
        gmtime_r(&seconds, &utc);
        // Sized for the widest value of each field, so the output is never truncated
        char buffer[80];
        std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", utc.tm_year + 1900,
            utc.tm_mon + 1, utc.tm_mday, utc.tm_hour, utc.tm_min, utc.tm_sec, static_cast<int>(epochMs % 1000));
        return buffer;
    }
}
