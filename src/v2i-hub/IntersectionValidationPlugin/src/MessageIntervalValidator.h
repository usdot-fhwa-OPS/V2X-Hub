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
#include <string>
#include <tmx/TmxException.hpp>

namespace IntersectionValidation
{

    static constexpr uint64_t CTI_SPAT_INTERVAL_MAX_THRESHOLD_MS = 300;
    static constexpr uint64_t CTI_MAP_INTERVAL_MAX_THRESHOLD_MS = 100;
    static constexpr uint64_t CTI_SPAT_INTERVAL_REQUIRED_MS = 125;
    static constexpr uint64_t CTI_MAP_INTERVAL_REQUIRED_MS = 1000;

    /**
     * @brief Calculate interval between two message timestamps and validate against threshold.
     * @param lastTimestampMs epoch ms of last received message. Pass 0 for first message.
     * @param currentTimestampMs epoch ms of current message.
     * @param thresholdMs maximum allowable interval in ms per CTI 4501.
     * @return interval in ms.
     * @throws tmx::TmxException if interval exceeds threshold.
     */
    uint64_t calculateMessageInterval(uint64_t lastTimestampMs, uint64_t currentTimestampMs, uint64_t thresholdMs);
}