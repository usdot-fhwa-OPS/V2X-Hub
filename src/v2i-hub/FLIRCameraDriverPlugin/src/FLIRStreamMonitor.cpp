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
#include "FLIRStreamMonitor.hpp"

#include <algorithm>
#include <cstdio>
#include <ctime>

namespace FLIRCameraDriverPlugin
{
    FLIRStreamMonitor::FLIRStreamMonitor(const StreamMonitorConfig &config) : config_(config)
    {
    }

    FrameReport FLIRStreamMonitor::onFrame(uint64_t recvMs, uint64_t cameraMs, std::optional<uint64_t> dataNumber)
    {
        std::scoped_lock lock{mutex_};
        FrameReport report;
        report.recvMs = recvMs;
        report.cameraMs = cameraMs;
        report.dataNumber = dataNumber;
        report.latencyMs = static_cast<int64_t>(recvMs) - static_cast<int64_t>(cameraMs);
        report.excessLatencyMs = report.latencyMs - updateBaseline(recvMs, report.latencyMs);
        stats_.frames++;

        // True when this frame and the previous one belong to the same continuous stream of camera frames,
        // as opposed to a new track starting after the camera went quiet.
        bool continuous = false;
        if (hasPrevious_)
        {
            report.hasPrevious = true;
            report.prevRecvMs = prevRecvMs_;
            report.prevCameraMs = prevCameraMs_;
            report.prevDataNumber = prevDataNumber_;
            report.arrivalGapMs = static_cast<int64_t>(recvMs) - static_cast<int64_t>(prevRecvMs_);
            report.cameraGapMs = static_cast<int64_t>(cameraMs) - static_cast<int64_t>(prevCameraMs_);
            report.holdMs = report.arrivalGapMs - report.cameraGapMs;
            continuous = report.cameraGapMs >= 0 && report.cameraGapMs <= config_.trackGapMs;

            if (continuous)
            {
                if (report.holdMs >= config_.stallThresholdMs)
                {
                    report.hold = FrameHoldType::Stall;
                    stats_.stalls++;
                    stats_.maxStallMs = std::max(stats_.maxStallMs, report.holdMs);
                    stats_.lastStallStartMs = prevRecvMs_;
                    stats_.lastStallEndMs = recvMs;
                }
                else if (report.holdMs >= config_.minorHoldThresholdMs)
                {
                    report.hold = FrameHoldType::Minor;
                    stats_.minorHolds++;
                }

                if (dataNumber && prevDataNumber_ && *dataNumber > *prevDataNumber_ + 1)
                {
                    report.missingFrames = *dataNumber - *prevDataNumber_ - 1;
                    stats_.missingFrames += report.missingFrames;
                }
            }
        }

        // A track starting after an idle period ends any burst left open from the previous track.
        if (!continuous && openBurst_)
        {
            report.completedBurst = closeBurst();
        }

        report.late = report.excessLatencyMs >= config_.stallThresholdMs;
        if (report.late)
        {
            stats_.lateFrames++;
            if (!openBurst_)
            {
                LateBurstSummary burst;
                burst.firstRecvMs = recvMs;
                burst.firstCameraMs = cameraMs;
                burst.firstDataNumber = dataNumber;
                openBurst_ = burst;
            }
            openBurst_->lastRecvMs = recvMs;
            openBurst_->lastCameraMs = cameraMs;
            openBurst_->lastDataNumber = dataNumber;
            openBurst_->frames++;
            openBurst_->maxExcessLatencyMs = std::max(openBurst_->maxExcessLatencyMs, report.excessLatencyMs);
        }
        else if (openBurst_)
        {
            report.completedBurst = closeBurst();
        }

        hasPrevious_ = true;
        prevRecvMs_ = recvMs;
        prevCameraMs_ = cameraMs;
        prevDataNumber_ = dataNumber;
        return report;
    }

    std::optional<LateBurstSummary> FLIRStreamMonitor::closeIfIdle(uint64_t nowMs)
    {
        std::scoped_lock lock{mutex_};
        if (openBurst_ && hasPrevious_ && static_cast<int64_t>(nowMs) - static_cast<int64_t>(prevRecvMs_) > config_.trackGapMs)
        {
            return closeBurst();
        }
        return std::nullopt;
    }

    std::optional<LateBurstSummary> FLIRStreamMonitor::flush()
    {
        std::scoped_lock lock{mutex_};
        return closeBurst();
    }

    StreamMonitorStats FLIRStreamMonitor::getStats() const
    {
        std::scoped_lock lock{mutex_};
        return stats_;
    }

    void FLIRStreamMonitor::setConfig(const StreamMonitorConfig &config)
    {
        std::scoped_lock lock{mutex_};
        config_ = config;
    }

    StreamMonitorConfig FLIRStreamMonitor::getConfig() const
    {
        std::scoped_lock lock{mutex_};
        return config_;
    }

    int64_t FLIRStreamMonitor::updateBaseline(uint64_t recvMs, int64_t latencyMs)
    {
        // The baseline is the lowest latency seen recently: frames that crossed the link without delay.
        // It absorbs any constant clock offset between camera and host.
        while (!latencyWindow_.empty() &&
               static_cast<int64_t>(recvMs) - static_cast<int64_t>(latencyWindow_.front().first) > config_.baselineWindowMs)
        {
            latencyWindow_.pop_front();
        }
        int64_t baseline = latencyMs;
        if (!latencyWindow_.empty())
        {
            baseline = std::min_element(latencyWindow_.begin(), latencyWindow_.end(),
                [](const auto &a, const auto &b) { return a.second < b.second; })->second;
        }
        else if (lastBaselineMs_)
        {
            // Nothing recent to compare against (e.g. a long stall): keep the previous baseline.
            baseline = *lastBaselineMs_;
        }
        lastBaselineMs_ = baseline;
        latencyWindow_.emplace_back(recvMs, latencyMs);
        return baseline;
    }

    std::optional<LateBurstSummary> FLIRStreamMonitor::closeBurst()
    {
        auto burst = openBurst_;
        openBurst_.reset();
        return burst;
    }

    std::string FLIRStreamMonitor::formatUtcMs(uint64_t epochMs)
    {
        auto seconds = static_cast<std::time_t>(epochMs / 1000);
        std::tm utc{};
        gmtime_r(&seconds, &utc);
        // Sized for the widest values the compiler allows for each field, so the output is never truncated
        char buffer[80];
        std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
            utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday, utc.tm_hour, utc.tm_min, utc.tm_sec,
            static_cast<int>(epochMs % 1000));
        return buffer;
    }
}
