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

#include "MessageFrequencyValidator.h"

namespace IntersectionValidation
{

    uint64_t calculateMessageInterval(uint64_t lastTimestampMs, uint64_t currentTimestampMs, uint64_t thresholdMs)
    {
        if (lastTimestampMs == 0)
        {
            return 0;
        }

        if (currentTimestampMs < lastTimestampMs)
        {
            throw tmx::TmxException("Current timestamp is earlier than last timestamp");
        }

        uint64_t intervalMs = currentTimestampMs - lastTimestampMs;

        if (intervalMs > thresholdMs)
        {
            throw tmx::TmxException("Message interval " + std::to_string(intervalMs) +
                                    " ms exceeded CTI 4501 maximum threshold of " + std::to_string(thresholdMs) + " ms");
        }

        return intervalMs;
    }

}