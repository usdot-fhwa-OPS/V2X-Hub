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

#include "MessageIntervalValidator.h"

#include <array>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace IntersectionValidation
{

    IntervalCheck evaluateMessageInterval(uint64_t lastTimestampMs, uint64_t currentTimestampMs,
                                          uint64_t thresholdMs) noexcept
    {
        IntervalCheck check;

        if (lastTimestampMs == 0)
        {
            return check;
        }

        if (currentTimestampMs < lastTimestampMs)
        {
            check.timeWentBackwards = true;
            return check;
        }

        check.intervalMs = currentTimestampMs - lastTimestampMs;
        check.violation = check.intervalMs > thresholdMs;

        return check;
    }

    MessageIntervalValidator::MessageIntervalValidator(uint64_t requiredThresholdMs, uint64_t windowDurationMs)
        : _thresholdMs(requiredThresholdMs), _windowMs(windowDurationMs)
    {
    }

    IntervalWindowResult MessageIntervalValidator::createWindow()
    {
        IntervalWindowResult result;
        result.windowStartMs = _windowStartMs;
        result.windowEndMs = _windowEndMs;
        result.violationCount = _windowViolations;
        result.messageCount = _windowMessages;
        result.intersectionId = _windowIntersectionId;
        result.intersectionIdMismatch = _windowIntersectionIdMismatch;

        _windowOpen = false;
        _windowStartMs = 0;
        _windowEndMs = 0;
        _windowViolations = 0;
        _windowMessages = 0;
        _windowIntersectionId = -1;
        _windowIntersectionIdMismatch = false;
        ++_totalEvents;

        return result;
    }

    std::optional<IntervalWindowResult> MessageIntervalValidator::recordMessage(uint64_t currentTimestampMs,
                                                                               int intersectionId)
    {
        const IntervalCheck check = evaluateMessageInterval(_lastTimestampMs, currentTimestampMs, _thresholdMs);
        _lastIntervalMs = check.intervalMs;

        if (check.timeWentBackwards)
        {
            ++_totalRegressions;
        }

        // Close an expired window
        std::optional<IntervalWindowResult> closed;
        if (_windowOpen && currentTimestampMs >= _windowEndMs)
        {
            closed = createWindow();
        }

        _lastTimestampMs = currentTimestampMs;

        if (check.violation)
        {
            ++_totalViolations;

            if (_windowOpen)
            {
                ++_windowViolations;
                ++_windowMessages;
                if (intersectionId != _windowIntersectionId)
                {
                    _windowIntersectionIdMismatch = true;
                }
            }
            else
            {
                _windowOpen = true;
                _windowStartMs = currentTimestampMs;
                _windowEndMs = currentTimestampMs + _windowMs;
                _windowViolations = 1;
                _windowMessages = 1;
                _windowIntersectionId = intersectionId;
                _windowIntersectionIdMismatch = false;
            }
        }
        else if (_windowOpen)
        {
            ++_windowMessages;
        }

        return closed;
    }

    uint64_t MessageIntervalValidator::lastIntervalMs() const
    {
        return _lastIntervalMs;
    }

    uint32_t MessageIntervalValidator::totalViolations() const
    {
        return _totalViolations;
    }

    uint32_t MessageIntervalValidator::totalEventsEmitted() const
    {
        return _totalEvents;
    }

    uint32_t MessageIntervalValidator::totalTimeRegressions() const
    {
        return _totalRegressions;
    }

    bool MessageIntervalValidator::windowOpen() const
    {
        return _windowOpen;
    }

    std::string formatIso8601Utc(uint64_t epochMs)
    {
        const auto secs = static_cast<std::time_t>(epochMs / 1000);
        const auto millis = static_cast<int>(epochMs % 1000);
        std::tm tmUtc{};
        gmtime_r(&secs, &tmUtc);

        std::array<char, 32> buf{};
        std::strftime(buf.data(), buf.size(), "%Y-%m-%dT%H:%M:%S", &tmUtc);

        std::ostringstream out;
        out << buf.data() << '.' << std::setfill('0') << std::setw(3) << millis << 'Z';
        return out.str();
    }

    std::string formatBroadcastRateDescription(const std::string &messageType, uint32_t violationCount,
                                               uint64_t windowStartMs, uint64_t windowEndMs)
    {
        return "There has been " + std::to_string(violationCount) + " BroadcastRateEvents for " +
               messageType + " messages between " + formatIso8601Utc(windowStartMs) + " and " +
               formatIso8601Utc(windowEndMs);
    }

}
