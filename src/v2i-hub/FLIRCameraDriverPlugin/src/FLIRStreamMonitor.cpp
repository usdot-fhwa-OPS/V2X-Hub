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

    std::optional<LinkStall> FLIRStreamMonitor::onFrame(uint64_t /*recvMs*/, uint64_t /*cameraMs*/)
    {
        // TODO(PR 2, stall detection): hold = (recvMs - prevRecvMs) - (cameraMs - prevCameraMs). Return a stall
        // from recvMs - hold to recvMs when hold >= stallThresholdMs_, skip frames whose camera time went
        // backwards, then remember this frame as the previous one.
        return std::nullopt;
    }
}
