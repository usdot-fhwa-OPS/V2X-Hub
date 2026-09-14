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
#include <optional>
#include <string>

namespace IntersectionValidation
{

    static constexpr uint64_t SPAT_INTERVAL_REQUIRED_MS = 125;
    static constexpr uint64_t MAP_INTERVAL_REQUIRED_MS = 1025;

    // Duration of the aggregation window opened by the first interval violation
    static constexpr uint64_t BROADCAST_RATE_WINDOW_MS = 5000;

    /**
     * @brief Outcome of comparing two message timestamps against the required interval
     */
    struct IntervalCheck
    {
        uint64_t intervalMs = 0;
        bool violation = false;
        bool timeWentBackwards = false;
    };

    /**
     * @brief Calculate interval between two message timestamps and compare against threshold
     * @param lastTimestampMs ms of last received message. Pass 0 for first message
     * @param currentTimestampMs ms of current message
     * @param thresholdMs maximum allowable interval in ms per CTI 4501
     * @return interval and whether it violated the threshold
     */
    IntervalCheck evaluateMessageInterval(uint64_t lastTimestampMs, uint64_t currentTimestampMs,
                                          uint64_t thresholdMs) noexcept;

    /**
     * @brief Counts from a closed aggregation window, one BroadcastRate event's worth
     */
    struct IntervalWindowResult
    {
        uint64_t windowStartMs = 0;
        uint64_t windowEndMs = 0;
        uint32_t violationCount = 0;
        uint32_t messageCount = 0;
        int intersectionId = -1;
        bool intersectionIdMismatch = false;
    };

    /**
     * @brief Tracks message intervals for one message type and aggregates violations into
     *        tumbling windows. The first violation opens a window; every violation and
     *        message seen until the window ends is counted into it; closing the window
     *        yields a single IntervalWindowResult to report
     */
    class MessageIntervalValidator
    {
    public:
        explicit MessageIntervalValidator(uint64_t requiredThresholdMs,
                                          uint64_t windowDurationMs = BROADCAST_RATE_WINDOW_MS);

        /**
         * @brief Record the arrival of a message of this type
         * @param currentTimestampMs ms the message was observed
         * @param intersectionId intersection the message belongs to
         * @return the counts of a window that this message closed, if any
         */
        std::optional<IntervalWindowResult> recordMessage(uint64_t currentTimestampMs,
                                                          int intersectionId = -1);

        uint64_t lastIntervalMs() const;
        uint32_t totalViolations() const;
        uint32_t totalEventsEmitted() const;
        uint32_t totalTimeRegressions() const;
        bool windowOpen() const;

    private:
        IntervalWindowResult createWindow();

        const uint64_t _thresholdMs;
        const uint64_t _windowMs;

        uint64_t _lastTimestampMs = 0;
        uint64_t _lastIntervalMs = 0;

        bool _windowOpen = false;
        uint64_t _windowStartMs = 0;
        uint64_t _windowEndMs = 0;
        uint32_t _windowViolations = 0;
        uint32_t _windowMessages = 0;
        int _windowIntersectionId = -1;
        bool _windowIntersectionIdMismatch = false;

        uint32_t _totalViolations = 0;
        uint32_t _totalEvents = 0;
        uint32_t _totalRegressions = 0;
    };

    /**
     * @brief Format time value as a UTC timestamp,
     */
    std::string formatIso8601Utc(uint64_t epochMs);

    /**
     * @brief Build the event log description for a closed BroadcastRate window
     * @param messageType SPaT or MAP message
     * @param violationCount number of interval violations counted in the window
     * @param windowStartMs ms the window opened
     * @param windowEndMs ms the window closed
     */
    std::string formatBroadcastRateDescription(const std::string &messageType, uint32_t violationCount,
                                               uint64_t windowStartMs, uint64_t windowEndMs);
}
