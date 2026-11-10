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
#include <gtest/gtest.h>

#include "FLIRLinkVerdict.hpp"

using namespace FLIRCameraDriverPlugin;

namespace
{
    LinkCheckSummary checks(unsigned total, unsigned noAnswer = 0, unsigned refused = 0)
    {
        LinkCheckSummary summary;
        summary.checks = total;
        summary.noAnswer = noAnswer;
        summary.refused = refused;
        return summary;
    }
}

TEST(FLIRLinkVerdictTest, classifyLink)
{
    EXPECT_EQ(classifyLink(checks(0), {checks(17)}), LinkVerdict::NotChecked);

    EXPECT_EQ(classifyLink(checks(17), {checks(17)}), LinkVerdict::PathUp);
    EXPECT_EQ(classifyLink(checks(17), {}), LinkVerdict::PathUp);
    // The camera answering decides; a control host that failed does not change it
    EXPECT_EQ(classifyLink(checks(17), {checks(17, 3)}), LinkVerdict::PathUp);

    EXPECT_EQ(classifyLink(checks(17, 0, 17), {checks(17)}), LinkVerdict::CameraService);

    EXPECT_EQ(classifyLink(checks(17, 6), {checks(17)}), LinkVerdict::CameraLink);
    EXPECT_EQ(classifyLink(checks(17, 6), {checks(17), checks(17)}), LinkVerdict::CameraLink);
    // A camera that did not answer some checks and refused others was unreachable at times
    EXPECT_EQ(classifyLink(checks(17, 3, 2), {checks(17)}), LinkVerdict::CameraLink);

    EXPECT_EQ(classifyLink(checks(17, 6), {}), LinkVerdict::CameraUnreachable);
    // A control host that could not be resolved has no checks
    EXPECT_EQ(classifyLink(checks(17, 6), {checks(0)}), LinkVerdict::CameraUnreachable);

    EXPECT_EQ(classifyLink(checks(17, 6), {checks(17, 5)}), LinkVerdict::HostNetwork);
    EXPECT_EQ(classifyLink(checks(17, 6), {checks(17), checks(17, 1)}), LinkVerdict::HostNetwork);
}

TEST(FLIRLinkVerdictTest, describeVerdict)
{
    EXPECT_EQ(describeVerdict(LinkVerdict::CameraLink, checks(17, 6), {checks(17)}),
        "camera link. The camera stopped answering while the control hosts answered, so packets were lost between "
        "the network and the camera: its network port, its cable or PoE, or its switch port. Link checks during "
        "this time: camera answered 11 of 17, control hosts 17 of 17.");
    EXPECT_EQ(describeVerdict(LinkVerdict::PathUp, checks(17), {}),
        "network path up. The camera answered every link check, so the network path was up and the frames were "
        "held by the camera's software or on this host. Link checks during this time: camera answered 17 of 17.");
    EXPECT_EQ(describeVerdict(LinkVerdict::NotChecked, checks(0), {}),
        "not checked. No link checks of the camera ran during this time.");
}

TEST(FLIRLinkVerdictTest, verdictName)
{
    EXPECT_EQ(verdictName(LinkVerdict::NotChecked), "not checked");
    EXPECT_EQ(verdictName(LinkVerdict::PathUp), "network path up");
    EXPECT_EQ(verdictName(LinkVerdict::CameraService), "camera service down");
    EXPECT_EQ(verdictName(LinkVerdict::CameraLink), "camera link");
    EXPECT_EQ(verdictName(LinkVerdict::CameraUnreachable), "camera unreachable");
    EXPECT_EQ(verdictName(LinkVerdict::HostNetwork), "host network");
}
