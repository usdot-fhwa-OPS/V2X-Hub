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

namespace FLIRCameraDriverPlugin
{
    FLIRStreamMonitor::FLIRStreamMonitor(int64_t stallThresholdMs) : stallThresholdMs_(stallThresholdMs)
    {
    }

    std::optional<LinkStall> FLIRStreamMonitor::onFrame(uint64_t recvMs, uint64_t cameraMs)
    {
        std::optional<LinkStall> stall;
        if (prevRecvMs_)
        {
            const auto arrivalGapMs = static_cast<int64_t>(recvMs) - static_cast<int64_t>(*prevRecvMs_);
            const auto cameraGapMs = static_cast<int64_t>(cameraMs) - static_cast<int64_t>(prevCameraMs_);
            // A camera clock that went backwards makes the comparison meaningless. Otherwise the hold is
            // meaningful across idle gaps too: a frame held by an outage that began while the camera was idle
            // still arrives that much late.
            if (cameraGapMs >= 0 && arrivalGapMs - cameraGapMs >= stallThresholdMs_)
            {
                stall = LinkStall{*prevRecvMs_ + static_cast<uint64_t>(cameraGapMs), recvMs, cameraGapMs};
            }
        }
        prevRecvMs_ = recvMs;
        prevCameraMs_ = cameraMs;
        return stall;
    }
}
