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
#include <string>
#include <vector>

#include "FLIRLinkChecker.hpp"

namespace FLIRCameraDriverPlugin
{
    /**
     * @brief Where camera frames were held, judged from the link checks that ran at the time. See the decision
     * chart in docs/CameraLinkMonitoring.md.
     */
    enum class LinkVerdict
    {
        // No camera checks ran during the time (link checks disabled, or the time was shorter than one check)
        NotChecked,
        // The camera answered every check: the network path was up, so frames were held in software
        PathUp,
        // The camera's network answered but refused connections: its web service was down
        CameraService,
        // The camera stopped answering while every control host answered
        CameraLink,
        // The camera stopped answering, and there were no control host checks to compare with
        CameraUnreachable,
        // The camera and control hosts stopped answering together: the fault is on this host's side
        HostNetwork
    };

    /**
     * @brief Classifies a time window from the camera's and the control hosts' checks during it.
     */
    LinkVerdict classifyLink(const LinkCheckSummary &camera, const std::vector<LinkCheckSummary> &controls);

    /**
     * @brief Short name of a verdict for status, e.g. "camera link".
     */
    std::string verdictName(LinkVerdict verdict);

    /**
     * @brief The verdict's name, what it means and the check counts, as one or more sentences for the log.
     */
    std::string describeVerdict(LinkVerdict verdict, const LinkCheckSummary &camera,
        const std::vector<LinkCheckSummary> &controls);
}
