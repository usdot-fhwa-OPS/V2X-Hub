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
        uint64_t dataNumber;
    };

    // Frames received by V2X Hub around a real stall (2026-09-15 17:47:10 UTC). The camera kept producing
    // frames every ~100 ms, but nothing arrived for 3.58 s; the backlog was then delivered within ~50 ms.
    // dataNumber 50651 never arrived.
    const std::vector<Frame> recordedStall = {
        {1789494429035ULL, 1789494429031ULL, 50640},
        {1789494429135ULL, 1789494429131ULL, 50641},
        {1789494429234ULL, 1789494429231ULL, 50642},
        {1789494429332ULL, 1789494429329ULL, 50643},
        {1789494429471ULL, 1789494429428ULL, 50644},
        {1789494429532ULL, 1789494429529ULL, 50645},
        {1789494429634ULL, 1789494429629ULL, 50646},
        {1789494429733ULL, 1789494429730ULL, 50647},
        {1789494429837ULL, 1789494429831ULL, 50648},
        {1789494429936ULL, 1789494429932ULL, 50649},
        {1789494430036ULL, 1789494430032ULL, 50650},
        {1789494433619ULL, 1789494430132ULL, 50652},
        {1789494433620ULL, 1789494430232ULL, 50653},
        {1789494433620ULL, 1789494430330ULL, 50654},
        {1789494433620ULL, 1789494430429ULL, 50655},
        {1789494433662ULL, 1789494430530ULL, 50656},
        {1789494433662ULL, 1789494430629ULL, 50657},
        {1789494433662ULL, 1789494430729ULL, 50658},
        {1789494433662ULL, 1789494430832ULL, 50659},
        {1789494433663ULL, 1789494430932ULL, 50660},
        {1789494433663ULL, 1789494431032ULL, 50661},
        {1789494433663ULL, 1789494431132ULL, 50662},
        {1789494433663ULL, 1789494431233ULL, 50663},
        {1789494433664ULL, 1789494431331ULL, 50664},
        {1789494433664ULL, 1789494431431ULL, 50665},
        {1789494433664ULL, 1789494431531ULL, 50666},
        {1789494433664ULL, 1789494431631ULL, 50667},
        {1789494433665ULL, 1789494431733ULL, 50668},
        {1789494433665ULL, 1789494431833ULL, 50669},
        {1789494433665ULL, 1789494431933ULL, 50670},
        {1789494433665ULL, 1789494432033ULL, 50671},
        {1789494433665ULL, 1789494432134ULL, 50672},
        {1789494433666ULL, 1789494432234ULL, 50673},
        {1789494433666ULL, 1789494432332ULL, 50674},
        {1789494433666ULL, 1789494432432ULL, 50675},
        {1789494433666ULL, 1789494432532ULL, 50676},
        {1789494433666ULL, 1789494432632ULL, 50677},
    };

    // Frames received around a real outage that began while the camera was idle (2026-11-09, V2X Hub host lost
    // its network for ~12 minutes). The last frame before it arrived at 15:54:04; the camera then tracked
    // nothing until 15:59:41, so its frames from then on were held and the whole backlog arrived at 16:09:46,
    // up to 604 s late. The backlog itself contains a 354 s idle gap (dataNumber 3170 -> 3206). The next track
    // (dataNumber 3337) arrived on time. A subset of the backlog is used.
    const std::vector<Frame> recordedIdleStartStall = {
        {1794239643637ULL, 1794239643661ULL, 2935},
        {1794239643739ULL, 1794239643762ULL, 2936},
        {1794239643839ULL, 1794239643862ULL, 2937},
        {1794239643939ULL, 1794239643962ULL, 2938},
        {1794239644039ULL, 1794239644062ULL, 2939},
        {1794239644139ULL, 1794239644163ULL, 2940},
        {1794240586460ULL, 1794239981774ULL, 2975},
        {1794240586460ULL, 1794239981876ULL, 2976},
        {1794240586460ULL, 1794239981977ULL, 2977},
        {1794240586460ULL, 1794239982076ULL, 2978},
        {1794240586460ULL, 1794239982177ULL, 2979},
        {1794240586460ULL, 1794239982277ULL, 2980},
        {1794240586464ULL, 1794239982675ULL, 2984},
        {1794240586489ULL, 1794240000994ULL, 3169},
        {1794240586489ULL, 1794240001094ULL, 3170},
        {1794240586489ULL, 1794240355856ULL, 3206},
        {1794240586489ULL, 1794240355955ULL, 3207},
        {1794240586500ULL, 1794240365864ULL, 3307},
        {1794240655522ULL, 1794240655460ULL, 3337},
        {1794240655564ULL, 1794240655560ULL, 3338},
    };

    const uint64_t t0 = 1789494400000ULL;
}

TEST(FLIRStreamMonitorTest, detectsRecordedStall)
{
    FLIRStreamMonitor monitor;
    std::vector<FrameReport> reports;
    for (const auto &frame : recordedStall)
    {
        reports.push_back(monitor.onFrame(frame.recvMs, frame.cameraMs, frame.dataNumber));
    }

    // The first frame after the gap is the one flagged as a stall
    const auto &stall = reports[11];
    EXPECT_EQ(stall.hold, FrameHoldType::Stall);
    EXPECT_EQ(stall.arrivalGapMs, 3583);
    EXPECT_EQ(stall.cameraGapMs, 100);
    EXPECT_EQ(stall.holdMs, 3483);
    EXPECT_EQ(stall.prevRecvMs, 1789494430036ULL);
    EXPECT_EQ(stall.missingFrames, 1u);
    EXPECT_TRUE(stall.late);

    // No other frame is a stall or minor hold
    for (size_t i = 0; i < reports.size(); i++)
    {
        if (i != 11)
        {
            EXPECT_EQ(reports[i].hold, FrameHoldType::None) << "frame index " << i;
        }
    }

    auto stats = monitor.getStats();
    EXPECT_EQ(stats.frames, recordedStall.size());
    EXPECT_EQ(stats.stalls, 1u);
    EXPECT_EQ(stats.minorHolds, 0u);
    EXPECT_EQ(stats.maxStallMs, 3483);
    EXPECT_EQ(stats.lastStallStartMs, 1789494430036ULL);
    EXPECT_EQ(stats.lastStallEndMs, 1789494433619ULL);
    EXPECT_EQ(stats.missingFrames, 1u);
    // Every backlog frame was at least 1 s late (SDSS discarded all 26 of them)
    EXPECT_EQ(stats.lateFrames, 26u);

    // The burst is still open: the track ended with the backlog. It closes once the stream is idle.
    EXPECT_FALSE(monitor.closeIfIdle(1789494433666ULL + 500).has_value());
    auto burst = monitor.closeIfIdle(1789494433666ULL + 1500);
    ASSERT_TRUE(burst.has_value());
    EXPECT_EQ(burst->frames, 26u);
    EXPECT_EQ(burst->firstRecvMs, 1789494433619ULL);
    EXPECT_EQ(burst->lastRecvMs, 1789494433666ULL);
    EXPECT_EQ(burst->firstDataNumber, 50652u);
    EXPECT_EQ(burst->lastDataNumber, 50677u);
    // Baseline latency is 3 ms, first backlog frame was 3487 ms late
    EXPECT_EQ(burst->maxExcessLatencyMs, 3484);
    EXPECT_FALSE(monitor.flush().has_value());
}

TEST(FLIRStreamMonitorTest, steadyStreamHasNoFindings)
{
    FLIRStreamMonitor monitor;
    for (uint64_t i = 0; i < 200; i++)
    {
        uint64_t cameraMs = t0 + i * 100 + (i % 3);   // small producer jitter
        uint64_t recvMs = cameraMs + 4 + (i % 7);     // small network jitter
        auto report = monitor.onFrame(recvMs, cameraMs, 1000 + i);
        EXPECT_EQ(report.hold, FrameHoldType::None);
        EXPECT_FALSE(report.late);
        EXPECT_FALSE(report.completedBurst.has_value());
    }
    auto stats = monitor.getStats();
    EXPECT_EQ(stats.frames, 200u);
    EXPECT_EQ(stats.stalls, 0u);
    EXPECT_EQ(stats.minorHolds, 0u);
    EXPECT_EQ(stats.lateFrames, 0u);
    EXPECT_EQ(stats.missingFrames, 0u);
}

TEST(FLIRStreamMonitorTest, singleRetransmissionIsMinorHold)
{
    FLIRStreamMonitor monitor;
    for (uint64_t i = 0; i < 10; i++)
    {
        monitor.onFrame(t0 + i * 100 + 4, t0 + i * 100, i);
    }
    // One frame held ~200 ms, as with a single TCP retransmission
    auto report = monitor.onFrame(t0 + 1000 + 204, t0 + 1000, 10);
    EXPECT_EQ(report.hold, FrameHoldType::Minor);
    EXPECT_EQ(report.holdMs, 200);
    EXPECT_FALSE(report.late);

    auto stats = monitor.getStats();
    EXPECT_EQ(stats.minorHolds, 1u);
    EXPECT_EQ(stats.stalls, 0u);
}

TEST(FLIRStreamMonitorTest, idleGapIsNotAStall)
{
    FLIRStreamMonitor monitor;
    for (uint64_t i = 0; i < 10; i++)
    {
        monitor.onFrame(t0 + i * 100 + 4, t0 + i * 100, i);
    }
    // Camera went quiet for 30 s (nothing tracked); its sequence number moved on meanwhile
    auto report = monitor.onFrame(t0 + 30900 + 4, t0 + 30900, 500);
    EXPECT_EQ(report.hold, FrameHoldType::None);
    EXPECT_EQ(report.missingFrames, 0u);
    EXPECT_FALSE(report.late);

    auto stats = monitor.getStats();
    EXPECT_EQ(stats.stalls, 0u);
    EXPECT_EQ(stats.missingFrames, 0u);
}

TEST(FLIRStreamMonitorTest, clockOffsetDoesNotMakeFramesLate)
{
    FLIRStreamMonitor monitor;
    // Camera clock 5 s behind the host
    for (uint64_t i = 0; i < 50; i++)
    {
        auto report = monitor.onFrame(t0 + i * 100 + 5004, t0 + i * 100, i);
        EXPECT_FALSE(report.late);
        EXPECT_EQ(report.latencyMs, 5004);
    }
    EXPECT_EQ(monitor.getStats().lateFrames, 0u);
}

TEST(FLIRStreamMonitorTest, burstClosedByOnTimeFrame)
{
    FLIRStreamMonitor monitor;
    for (uint64_t i = 0; i <= 10; i++)
    {
        monitor.onFrame(t0 + i * 100 + 4, t0 + i * 100, i);
    }
    // Nothing arrives for ~2 s, then frames 11..30 (produced 100 ms apart) arrive together
    const uint64_t burstRecv = t0 + 3004;
    std::optional<LateBurstSummary> burst;
    unsigned int stalls = 0;
    for (uint64_t i = 11; i <= 30; i++)
    {
        auto report = monitor.onFrame(burstRecv, t0 + i * 100, i);
        if (report.hold == FrameHoldType::Stall)
        {
            stalls++;
        }
        if (report.completedBurst)
        {
            burst = report.completedBurst;
            // Closed by the first backlog frame that is no longer late (latency 404 ms)
            EXPECT_EQ(i, 26u);
            EXPECT_FALSE(report.late);
        }
    }
    EXPECT_EQ(stalls, 1u);
    ASSERT_TRUE(burst.has_value());
    // Frames 11..25 were at least 500 ms late
    EXPECT_EQ(burst->frames, 15u);
    EXPECT_EQ(burst->firstDataNumber, 11u);
    EXPECT_EQ(burst->lastDataNumber, 25u);
    EXPECT_EQ(burst->maxExcessLatencyMs, 1900);
}

TEST(FLIRStreamMonitorTest, dataNumberResetIsNotMissing)
{
    FLIRStreamMonitor monitor;
    monitor.onFrame(t0 + 4, t0, 90000);
    monitor.onFrame(t0 + 104, t0 + 100, 90001);
    // Camera restarted its sequence (e.g. after a reboot or resubscription)
    auto report = monitor.onFrame(t0 + 204, t0 + 200, 1);
    EXPECT_EQ(report.missingFrames, 0u);
    // Frames without a sequence number are handled
    report = monitor.onFrame(t0 + 304, t0 + 300, std::nullopt);
    EXPECT_EQ(report.missingFrames, 0u);
    EXPECT_EQ(monitor.getStats().missingFrames, 0u);
}

TEST(FLIRStreamMonitorTest, configurableThreshold)
{
    StreamMonitorConfig config;
    config.stallThresholdMs = 300;
    FLIRStreamMonitor monitor(config);
    for (uint64_t i = 0; i < 10; i++)
    {
        monitor.onFrame(t0 + i * 100 + 4, t0 + i * 100, i);
    }
    auto report = monitor.onFrame(t0 + 1000 + 404, t0 + 1000, 10);
    EXPECT_EQ(report.hold, FrameHoldType::Stall);
    EXPECT_TRUE(report.late);
    EXPECT_EQ(monitor.getConfig().stallThresholdMs, 300);

    config.stallThresholdMs = 1000;
    monitor.setConfig(config);
    report = monitor.onFrame(t0 + 1100 + 404 + 400, t0 + 1100, 11);
    EXPECT_EQ(report.hold, FrameHoldType::Minor);
}

TEST(FLIRStreamMonitorTest, detectsStallThatBeganWhileIdle)
{
    FLIRStreamMonitor monitor;
    std::vector<FrameReport> reports;
    for (const auto &frame : recordedIdleStartStall)
    {
        reports.push_back(monitor.onFrame(frame.recvMs, frame.cameraMs, frame.dataNumber));
    }

    // The first backlog frame is a stall even though the camera had been idle (camera gap 337 s) before it
    const auto &stall = reports[6];
    EXPECT_EQ(stall.hold, FrameHoldType::Stall);
    EXPECT_EQ(stall.arrivalGapMs, 942321);
    EXPECT_EQ(stall.cameraGapMs, 337611);
    EXPECT_EQ(stall.holdMs, 604710);
    // dataNumber advances while the camera is idle, so the jump 2940 -> 2975 is not counted as missing
    EXPECT_EQ(stall.missingFrames, 0u);
    EXPECT_TRUE(stall.late);

    // Every backlog frame is late, including those after the window of on-time frames emptied and those after
    // the idle gap inside the backlog
    for (size_t i = 6; i <= 17; i++)
    {
        EXPECT_TRUE(reports[i].late) << "frame index " << i;
        EXPECT_FALSE(reports[i].completedBurst.has_value()) << "frame index " << i;
    }

    // The next track arrives on time and closes the burst, which covers the whole backlog
    EXPECT_FALSE(reports[18].late);
    EXPECT_EQ(reports[18].hold, FrameHoldType::None);
    ASSERT_TRUE(reports[18].completedBurst.has_value());
    const auto &burst = *reports[18].completedBurst;
    EXPECT_EQ(burst.frames, 12u);
    EXPECT_EQ(burst.firstDataNumber, 2975u);
    EXPECT_EQ(burst.lastDataNumber, 3307u);
    EXPECT_EQ(burst.firstRecvMs, 1794240586460ULL);
    EXPECT_EQ(burst.lastRecvMs, 1794240586500ULL);
    // Baseline latency -24 ms (camera clock slightly ahead), first backlog frame 604686 ms
    EXPECT_EQ(burst.maxExcessLatencyMs, 604710);
    EXPECT_FALSE(reports[19].late);

    auto stats = monitor.getStats();
    EXPECT_EQ(stats.stalls, 1u);
    EXPECT_EQ(stats.lateFrames, 12u);
    EXPECT_EQ(stats.maxStallMs, 604710);
    EXPECT_EQ(stats.lastStallStartMs, 1794239644139ULL);
    EXPECT_EQ(stats.lastStallEndMs, 1794240586460ULL);
    EXPECT_EQ(stats.baselineShifts, 0u);
}

TEST(FLIRStreamMonitorTest, clockStepBecomesNewBaseline)
{
    FLIRStreamMonitor monitor;
    for (uint64_t i = 0; i < 100; i++)
    {
        monitor.onFrame(t0 + i * 100 + 5, t0 + i * 100, i);
    }
    // The camera clock steps 2 s back and stays there. At the step this looks like a 2 s hold.
    auto report = monitor.onFrame(t0 + 100 * 100 + 2005, t0 + 100 * 100, 100);
    EXPECT_EQ(report.hold, FrameHoldType::Stall);
    EXPECT_TRUE(report.late);
    for (uint64_t i = 101; i < 200; i++)
    {
        report = monitor.onFrame(t0 + i * 100 + 2005, t0 + i * 100, i);
        EXPECT_TRUE(report.late) << "frame " << i;
        EXPECT_FALSE(report.baselineShift) << "frame " << i;
    }
    EXPECT_EQ(monitor.getStats().lateFrames, 100u);

    // After a whole baseline window at the same latency, it becomes the new baseline
    report = monitor.onFrame(t0 + 200 * 100 + 2005, t0 + 200 * 100, 200);
    EXPECT_TRUE(report.baselineShift);
    EXPECT_FALSE(report.late);
    EXPECT_EQ(report.excessLatencyMs, 0);
    EXPECT_EQ(report.hold, FrameHoldType::None);
    EXPECT_FALSE(report.completedBurst.has_value());
    for (uint64_t i = 201; i < 300; i++)
    {
        report = monitor.onFrame(t0 + i * 100 + 2005, t0 + i * 100, i);
        EXPECT_FALSE(report.late) << "frame " << i;
        EXPECT_FALSE(report.baselineShift) << "frame " << i;
    }

    auto stats = monitor.getStats();
    EXPECT_EQ(stats.baselineShifts, 1u);
    // The frames counted late before the shift were not a backlog
    EXPECT_EQ(stats.lateFrames, 0u);
    EXPECT_FALSE(monitor.flush().has_value());
}

TEST(FLIRStreamMonitorTest, slowlyDrainingBacklogIsNotABaselineShift)
{
    FLIRStreamMonitor monitor;
    for (uint64_t i = 0; i < 100; i++)
    {
        monitor.onFrame(t0 + i * 100 + 5, t0 + i * 100, 1000 + i);
    }
    // 30 s outage, then the backlog drains over 15 s: frames produced 100 ms apart arrive 50 ms apart, so the
    // latency keeps falling and never settles at a steady level
    std::optional<FrameReport> last;
    for (uint64_t k = 0; k < 300; k++)
    {
        last = monitor.onFrame(t0 + 40000 + k * 50, t0 + 10000 + k * 100, 1100 + k);
        EXPECT_TRUE(last->late) << "backlog frame " << k;
        EXPECT_FALSE(last->baselineShift) << "backlog frame " << k;
    }
    auto stats = monitor.getStats();
    EXPECT_EQ(stats.stalls, 1u);
    EXPECT_EQ(stats.lateFrames, 300u);
    EXPECT_EQ(stats.baselineShifts, 0u);
    auto burst = monitor.flush();
    ASSERT_TRUE(burst.has_value());
    EXPECT_EQ(burst->frames, 300u);
}

TEST(FLIRStreamMonitorTest, formatUtcMs)
{
    EXPECT_EQ(FLIRStreamMonitor::formatUtcMs(1789494430036ULL), "2026-09-15T17:47:10.036Z");
    EXPECT_EQ(FLIRStreamMonitor::formatUtcMs(0), "1970-01-01T00:00:00.000Z");
}
