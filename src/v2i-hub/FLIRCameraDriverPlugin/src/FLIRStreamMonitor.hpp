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
#include <optional>

namespace FLIRCameraDriverPlugin
{
    /**
     * @brief A camera frame that spent much longer in transit than the frame before it: the camera produced it on
     * time, but the link held it, and every frame queued behind it, until it was delivered.
     */
    struct LinkStall
    {
        // When the held frame would have arrived with the same transit time as the frame before it
        // (ms since epoch, host clock)
        uint64_t startMs = 0;
        // When the held frame arrived (ms since epoch, host clock)
        uint64_t endMs = 0;
        // Camera time between the frame before and the held frame. Well above the camera's 100 ms frame period,
        // the camera was idle (tracking nothing) before the held frame.
        int64_t cameraGapMs = 0;

        // How much longer the held frame was in transit than the frame before it
        int64_t holdMs() const { return static_cast<int64_t>(endMs) - static_cast<int64_t>(startMs); }
    };

    /**
     * @brief Detects stalls on a camera connection by comparing when frames arrive with when the camera produced
     * them. See docs/CameraLinkMonitoring.md.
     *
     * Each frame's hold is its arrival gap minus its camera-timestamp gap to the frame before it. Each gap uses a
     * single clock, so the camera and host clocks do not need to be synchronized.
     *
     * Not thread safe: use from the camera session's I/O thread only.
     */
    class FLIRStreamMonitor
    {
        public:
            /**
             * @param stallThresholdMs Hold, in ms, from which a frame counts as a stall.
             */
            explicit FLIRStreamMonitor(int64_t stallThresholdMs = 500);

            /**
             * @brief Checks one received data frame against the frame before it.
             * @param recvMs When this host received the frame (ms since epoch, host clock).
             * @param cameraMs The frame's camera timestamp (ms since epoch, camera clock).
             * @return The stall this frame ended, if it was held at least the stall threshold longer in transit
             * than the frame before it.
             */
            std::optional<LinkStall> onFrame(uint64_t recvMs, uint64_t cameraMs);

        private:
            int64_t stallThresholdMs_;
            std::optional<uint64_t> prevRecvMs_;
            uint64_t prevCameraMs_ = 0;
    };
}
