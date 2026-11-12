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
#include "FLIRLinkVerdict.hpp"

namespace FLIRCameraDriverPlugin
{
    LinkVerdict classifyLink(const LinkCheckSummary &camera, const std::vector<LinkCheckSummary> &controls)
    {
        if (camera.checks == 0)
        {
            return LinkVerdict::NotChecked;
        }
        if (camera.noAnswer == 0)
        {
            return camera.refused > 0 ? LinkVerdict::CameraService : LinkVerdict::PathUp;
        }
        bool controlChecked = false;
        bool controlFailed = false;
        for (const auto &control : controls)
        {
            if (control.checks > 0)
            {
                controlChecked = true;
                controlFailed = controlFailed || control.noAnswer > 0;
            }
        }
        if (!controlChecked)
        {
            return LinkVerdict::CameraUnreachable;
        }
        return controlFailed ? LinkVerdict::HostNetwork : LinkVerdict::CameraLink;
    }

    std::string verdictName(LinkVerdict verdict)
    {
        switch (verdict)
        {
            case LinkVerdict::NotChecked:
                return "not checked";
            case LinkVerdict::PathUp:
                return "network path up";
            case LinkVerdict::CameraService:
                return "camera service down";
            case LinkVerdict::CameraLink:
                return "camera link";
            case LinkVerdict::CameraUnreachable:
                return "camera unreachable";
            case LinkVerdict::HostNetwork:
                return "host network";
        }
        return "unknown";
    }

    std::string describeVerdict(LinkVerdict verdict, const LinkCheckSummary &camera,
        const std::vector<LinkCheckSummary> &controls)
    {
        std::string explanation;
        switch (verdict)
        {
            case LinkVerdict::NotChecked:
                explanation = "No link checks of the camera ran during this time.";
                break;
            case LinkVerdict::PathUp:
                explanation = "The camera answered every link check, so the network path was up and the frames were "
                    "held by the camera's software or on this host.";
                break;
            case LinkVerdict::CameraService:
                explanation = "The camera's network answered, but the camera refused connections, so its web service "
                    "was down.";
                break;
            case LinkVerdict::CameraLink:
                explanation = "The camera stopped answering while the control hosts answered, so packets were lost "
                    "between the network and the camera: its network port, its cable or PoE, or its switch port.";
                break;
            case LinkVerdict::CameraUnreachable:
                explanation = "The camera stopped answering. Set LinkCheckControlHosts to tell a fault on the camera's "
                    "link from one on this host's side of the network.";
                break;
            case LinkVerdict::HostNetwork:
                explanation = "The camera and the control hosts stopped answering together, so the fault is on this "
                    "host's side: its network port, cable or switch, or equipment it shares with the camera.";
                break;
        }

        std::string text = verdictName(verdict) + ". " + explanation;
        if (camera.checks > 0)
        {
            text += " Link checks during this time: camera answered " + std::to_string(camera.answered()) + " of " +
                std::to_string(camera.checks);
            unsigned controlChecks = 0;
            unsigned controlAnswered = 0;
            for (const auto &control : controls)
            {
                controlChecks += control.checks;
                controlAnswered += control.answered();
            }
            if (controlChecks > 0)
            {
                text += ", control hosts " + std::to_string(controlAnswered) + " of " + std::to_string(controlChecks);
            }
            text += ".";
        }
        return text;
    }
}
