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
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

namespace FLIRCameraDriverPlugin
{
    /**
     * @brief Thresholds used by FLIRStreamMonitor.
     */
    struct StreamMonitorConfig
    {
        /**
         * A frame that spent at least this long longer in transit than the frame before it is reported as a
         * link stall. Frames whose latency is at least this much above the recent baseline are counted as late.
         * 500 ms matches the freshness limit the CARMA Streets sensor data sharing service applies before
         * discarding detections.
         */
        int64_t stallThresholdMs = 500;
        /**
         * Holds from this value up to stallThresholdMs are counted as minor holds. A hold of about 200 ms is
         * the signature of a single TCP retransmission (Linux minimum retransmission timeout), so a rising
         * minor-hold count is an early sign of packet loss on the camera link.
         */
        int64_t minorHoldThresholdMs = 150;
        /**
         * Camera timestamps further apart than this mean the camera stopped sending because nothing was
         * being tracked. Missing dataNumbers are only counted across shorter gaps. Holds are still measured
         * across idle periods (the camera gap is subtracted), so an outage that begins while nothing is tracked
         * is caught by the first frame the camera produces during it. A pause in arrivals longer than this
         * also ends a late burst.
         */
        int64_t trackGapMs = 1000;
        /**
         * Time window used to estimate the normal camera-to-plugin latency, which includes any clock offset
         * between the camera and this host. Late frames are kept out of the estimate, so the backlog after a
         * stall does not become the new normal. A latency that stays late at a steady level for this long is
         * adopted as the new baseline, since that is a clock offset change rather than held frames.
         */
        int64_t baselineWindowMs = 10000;
    };

    /**
     * @brief How much longer a frame spent in transit than the frame before it.
     */
    enum class FrameHoldType
    {
        None,
        Minor,
        Stall
    };

    /**
     * @brief Summary of a run of consecutive late frames, i.e. the backlog delivered after a stall.
     */
    struct LateBurstSummary
    {
        // Receive time of the first late frame (ms since epoch, host clock)
        uint64_t firstRecvMs = 0;
        // Receive time of the last late frame (ms since epoch, host clock)
        uint64_t lastRecvMs = 0;
        // Camera timestamps of the first and last late frame (ms since epoch, camera clock)
        uint64_t firstCameraMs = 0;
        uint64_t lastCameraMs = 0;
        // Number of late frames in the burst
        unsigned int frames = 0;
        // Largest latency above baseline seen in the burst
        int64_t maxExcessLatencyMs = 0;
        std::optional<uint64_t> firstDataNumber;
        std::optional<uint64_t> lastDataNumber;
    };

    /**
     * @brief Timing analysis of one received frame.
     */
    struct FrameReport
    {
        // When this host received the frame (ms since epoch, host clock)
        uint64_t recvMs = 0;
        // Camera timestamp of the frame (ms since epoch, camera clock)
        uint64_t cameraMs = 0;
        std::optional<uint64_t> dataNumber;
        // recvMs - cameraMs. Includes any clock offset between camera and host.
        int64_t latencyMs = 0;
        // latencyMs minus the recent baseline latency. Independent of a constant clock offset.
        int64_t excessLatencyMs = 0;

        bool hasPrevious = false;
        uint64_t prevRecvMs = 0;
        uint64_t prevCameraMs = 0;
        std::optional<uint64_t> prevDataNumber;
        // recvMs - prevRecvMs
        int64_t arrivalGapMs = 0;
        // cameraMs - prevCameraMs
        int64_t cameraGapMs = 0;
        // arrivalGapMs - cameraGapMs: how much longer this frame was in transit than the previous one
        int64_t holdMs = 0;
        FrameHoldType hold = FrameHoldType::None;
        // Frames skipped according to dataNumber, counted only while the camera was tracking continuously
        uint64_t missingFrames = 0;
        // excessLatencyMs >= stallThresholdMs
        bool late = false;
        // The latency had stayed late at a steady level for baselineWindowMs, so it was adopted as the new
        // baseline (camera/host clock offset changed). The late burst counted until now was discarded.
        bool baselineShift = false;
        // Late burst that ended just before this frame, if any
        std::optional<LateBurstSummary> completedBurst;
    };

    /**
     * @brief Running totals since the monitor was created.
     */
    struct StreamMonitorStats
    {
        uint64_t frames = 0;
        uint64_t stalls = 0;
        uint64_t minorHolds = 0;
        uint64_t lateFrames = 0;
        uint64_t missingFrames = 0;
        // Times a steady late latency was adopted as the new baseline (camera/host clock offset changed)
        uint64_t baselineShifts = 0;
        int64_t maxStallMs = 0;
        // Receive times bounding the most recent stall (ms since epoch, host clock). 0 if no stall yet.
        uint64_t lastStallStartMs = 0;
        uint64_t lastStallEndMs = 0;
    };

    /**
     * @brief Detects stalls in a camera data stream by comparing when frames arrive with when the camera
     * produced them.
     *
     * A link stall shows up as a long gap between arrivals while the camera timestamps of the same frames
     * stay close together: the camera kept producing frames but they were held in transit and then delivered
     * as a burst. This check uses differences of camera timestamps and of receive timestamps separately, so it
     * does not depend on the camera and host clocks being synchronized. It also works across idle periods: if
     * the link goes down while nothing is tracked, the first frame produced during the outage arrives with a
     * hold as long as the time it waited.
     *
     * Thread safe: frames may be reported from the I/O thread while statistics are read from another thread.
     */
    class FLIRStreamMonitor
    {
        public:
            explicit FLIRStreamMonitor(const StreamMonitorConfig &config = StreamMonitorConfig());

            /**
             * @brief Analyze one received data frame.
             * @param recvMs Time this host received the frame, in ms since epoch (host wall clock).
             * @param cameraMs Camera timestamp of the frame, in ms since epoch (camera wall clock).
             * @param dataNumber Camera frame sequence number, if present.
             * @return Timing analysis for the frame.
             */
            FrameReport onFrame(uint64_t recvMs, uint64_t cameraMs, std::optional<uint64_t> dataNumber);

            /**
             * @brief Close an open late burst if no frame has arrived for longer than trackGapMs, so a burst at the
             * end of a track is reported without waiting for the next track.
             * @param nowMs Current time in ms since epoch (host wall clock).
             * @return The closed burst, if one was open and idle.
             */
            std::optional<LateBurstSummary> closeIfIdle(uint64_t nowMs);

            /**
             * @brief Close any open late burst, e.g. when the connection drops.
             * @return The closed burst, if one was open.
             */
            std::optional<LateBurstSummary> flush();

            StreamMonitorStats getStats() const;
            void setConfig(const StreamMonitorConfig &config);
            StreamMonitorConfig getConfig() const;

            /**
             * @brief Format a time in ms since epoch as UTC ISO 8601 with milliseconds, e.g. 2026-09-15T17:47:10.036Z
             */
            static std::string formatUtcMs(uint64_t epochMs);

        private:
            /**
             * @brief Baseline latency to compare a frame with: the lowest latency of recent on-time frames.
             */
            int64_t currentBaseline(uint64_t recvMs, int64_t latencyMs);
            /**
             * @brief Feed the baseline estimate. On-time frames go into the window; late frames are tracked
             * separately and only adopted if they stay at a steady level for baselineWindowMs.
             * @return True if a steady late level was adopted as the new baseline.
             */
            bool trackBaseline(uint64_t recvMs, int64_t latencyMs, bool late);
            std::optional<LateBurstSummary> closeBurst();

            mutable std::mutex mutex_;
            StreamMonitorConfig config_;
            StreamMonitorStats stats_;

            bool hasPrevious_ = false;
            uint64_t prevRecvMs_ = 0;
            uint64_t prevCameraMs_ = 0;
            std::optional<uint64_t> prevDataNumber_;

            // (receive time, latency) of recent on-time frames, used for the baseline latency
            std::deque<std::pair<uint64_t, int64_t>> latencyWindow_;
            std::optional<int64_t> lastBaselineMs_;
            // (receive time, latency) of the last baselineWindowMs of the current run of consecutive late frames
            std::deque<std::pair<uint64_t, int64_t>> lateRun_;
            // Receive time of the first frame of the current late run
            std::optional<uint64_t> lateSinceMs_;

            std::optional<LateBurstSummary> openBurst_;
    };
}
