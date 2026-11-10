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
    LinkVerdict classifyLink(const LinkCheckSummary & /*camera*/, const std::vector<LinkCheckSummary> & /*controls*/)
    {
        // TODO(PR 4, verdict): follow the decision chart in docs/CameraLinkMonitoring.md.
        return LinkVerdict::NotChecked;
    }

    std::string verdictName(LinkVerdict /*verdict*/)
    {
        // TODO(PR 4, verdict): short name of each verdict.
        return "not checked";
    }

    std::string describeVerdict(LinkVerdict verdict, const LinkCheckSummary & /*camera*/,
        const std::vector<LinkCheckSummary> & /*controls*/)
    {
        // TODO(PR 4, verdict): name, explanation and "camera answered X of Y, control hosts X of Y".
        return verdictName(verdict) + ".";
    }
}
