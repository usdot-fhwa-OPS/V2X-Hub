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
#include <vector>

#include "FLIRStreamMonitor.hpp"

using namespace FLIRCameraDriverPlugin;

namespace
{
    struct Frame
    {
        uint64_t recvMs;
        uint64_t cameraMs;
    };

    // Frames received around a stall recorded in the field on 2026-09-15 (ms since epoch). The camera produced
    // a frame about every 100 ms, but nothing arrived for 3583 ms after 17:47:10.036 UTC; then the held frames
    // arrived together.
    const std::vector<Frame> fieldStall = {
        {1789494429035ULL, 1789494429031ULL},
        {1789494429135ULL, 1789494429131ULL},
        {1789494429234ULL, 1789494429231ULL},
        {1789494429332ULL, 1789494429329ULL},
        {1789494429471ULL, 1789494429428ULL},
        {1789494429532ULL, 1789494429529ULL},
        {1789494429634ULL, 1789494429629ULL},
        {1789494429733ULL, 1789494429730ULL},
        {1789494429837ULL, 1789494429831ULL},
        {1789494429936ULL, 1789494429932ULL},
        {1789494430036ULL, 1789494430032ULL},
        {1789494433619ULL, 1789494430132ULL},
        {1789494433620ULL, 1789494430232ULL},
        {1789494433620ULL, 1789494430330ULL},
        {1789494433620ULL, 1789494430429ULL},
        {1789494433662ULL, 1789494430530ULL},
        {1789494433662ULL, 1789494430629ULL},
        {1789494433662ULL, 1789494430729ULL},
        {1789494433662ULL, 1789494430832ULL},
        {1789494433663ULL, 1789494430932ULL},
    };

    std::vector<LinkStall> feed(FLIRStreamMonitor &monitor, const std::vector<Frame> &frames, int64_t cameraOffsetMs = 0)
    {
        std::vector<LinkStall> stalls;
        for (const auto &frame : frames)
        {
            if (auto stall = monitor.onFrame(frame.recvMs, frame.cameraMs + cameraOffsetMs))
            {
                stalls.push_back(*stall);
            }
        }
        return stalls;
    }
}

TEST(FLIRStreamMonitorTest, detectsFieldStall)
{
    FLIRStreamMonitor monitor;
    const auto stalls = feed(monitor, fieldStall);
    // Only the first held frame is a stall: the frames queued behind it arrived with it, so their own holds
    // are negative.
    ASSERT_EQ(stalls.size(), 1u);
    EXPECT_EQ(stalls[0].startMs, 1789494430136ULL);
    EXPECT_EQ(stalls[0].endMs, 1789494433619ULL);
    EXPECT_EQ(stalls[0].holdMs(), 3483);
    EXPECT_EQ(stalls[0].cameraGapMs, 100);
}

TEST(FLIRStreamMonitorTest, onTimeFramesWithJitterAreNotStalls)
{
    FLIRStreamMonitor monitor;
    // The frames before the stall, including one held 40 ms longer in transit than the frame before it
    const std::vector<Frame> onTime(fieldStall.begin(), fieldStall.begin() + 11);
    EXPECT_TRUE(feed(monitor, onTime).empty());
}

TEST(FLIRStreamMonitorTest, cameraClockOffsetDoesNotMatter)
{
    // The same frames with the camera clock 5 h 1.234 s ahead of, and then behind, the host clock
    for (const int64_t offsetMs : {18001234LL, -18001234LL})
    {
        FLIRStreamMonitor monitor;
        const auto stalls = feed(monitor, fieldStall, offsetMs);
        ASSERT_EQ(stalls.size(), 1u);
        EXPECT_EQ(stalls[0].startMs, 1789494430136ULL);
        EXPECT_EQ(stalls[0].holdMs(), 3483);
    }
}

TEST(FLIRStreamMonitorTest, threshold)
{
    FLIRStreamMonitor monitor(500);
    EXPECT_FALSE(monitor.onFrame(1000, 0));
    // Arrival gap 599 ms, camera gap 100 ms: held 499 ms
    EXPECT_FALSE(monitor.onFrame(1599, 100));
    // Arrival gap 600 ms, camera gap 100 ms: held 500 ms
    const auto stall = monitor.onFrame(2199, 200);
    ASSERT_TRUE(stall);
    EXPECT_EQ(stall->startMs, 1699u);
    EXPECT_EQ(stall->endMs, 2199u);
    EXPECT_EQ(stall->holdMs(), 500);

    FLIRStreamMonitor sensitive(200);
    sensitive.onFrame(1000, 0);
    EXPECT_TRUE(sensitive.onFrame(1300, 100));
}

TEST(FLIRStreamMonitorTest, stallAfterIdle)
{
    FLIRStreamMonitor monitor;
    monitor.onFrame(10000, 10000);
    // The camera tracked nothing for 30 s. The next frame it produced was held 3 s by an outage that began
    // during the idle time.
    const auto stall = monitor.onFrame(43000, 40000);
    ASSERT_TRUE(stall);
    EXPECT_EQ(stall->startMs, 40000u);
    EXPECT_EQ(stall->endMs, 43000u);
    EXPECT_EQ(stall->cameraGapMs, 30000);

    // Tracking resuming after idle time without an outage is not a stall
    FLIRStreamMonitor quiet;
    quiet.onFrame(10000, 10000);
    EXPECT_FALSE(quiet.onFrame(40005, 40000));
}

TEST(FLIRStreamMonitorTest, cameraClockGoingBackwardsIsSkipped)
{
    FLIRStreamMonitor monitor;
    monitor.onFrame(1000, 5000);
    // The camera clock stepped back 1 s: the hold cannot be measured
    EXPECT_FALSE(monitor.onFrame(1100, 4000));
    // Measured again from the next frame on
    EXPECT_FALSE(monitor.onFrame(1200, 4100));
    EXPECT_TRUE(monitor.onFrame(2000, 4200));
}
