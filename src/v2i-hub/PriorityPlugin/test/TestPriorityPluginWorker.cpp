/**
 * Copyright (C) 2026 LEIDOS.
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
#include <cstring>
#include "PriorityPluginWorker.hpp"

using namespace PriorityPlugin;

TEST(ExtractVehicleIDTest, EntityIDNull) {
    VehicleID_t id{};
    id.present = VehicleID_PR_entityID;
    id.choice.entityID.buf = nullptr;
    id.choice.entityID.size = 0;

    EXPECT_TRUE(ExtractVehicleID(id).empty());
}

TEST(ExtractVehicleIDTest, EntityIDBytes) {
    uint8_t raw[] = {0xAA, 0xBB, 0xCC, 0xDD};
    VehicleID_t id{};
    id.present = VehicleID_PR_entityID;
    id.choice.entityID.buf = raw;
    id.choice.entityID.size = sizeof(raw);

    auto out = ExtractVehicleID(id);
    EXPECT_EQ(out, (std::vector<uint8_t>{0xAA, 0xBB, 0xCC, 0xDD}));
}

TEST(ExtractVehicleIDTest, StationID) {
    VehicleID_t id{};
    id.present = VehicleID_PR_stationID;
    id.choice.stationID = 0x11223344;

    auto out = ExtractVehicleID(id);
    ASSERT_EQ(out.size(), sizeof(StationID_t));
    StationID_t decoded = 0;
    std::memcpy(&decoded, out.data(), sizeof(decoded));
    EXPECT_EQ(decoded, 0x11223344UL);
}

TEST(ExtractVehicleIDTest, NothingPresent) {
    VehicleID_t id{};
    id.present = VehicleID_PR_NOTHING;

    EXPECT_TRUE(ExtractVehicleID(id).empty());
}

TEST(ComputeMinuteAndMsOfYearTest, EpochZeroIsStartOfYear) {
    auto [minuteOfYear, msInMinute] = ComputeMinuteAndMsOfYear(0);
    EXPECT_EQ(minuteOfYear, 0L);
    EXPECT_EQ(msInMinute, 0L);
}

TEST(ComputeMinuteAndMsOfYearTest, KnownTimestamp) {
    // 2026-01-02 01:01:30 UTC
    constexpr time_t TEST_EPOCH_JAN_2 = 1767315690;
    auto [minuteOfYear, msInMinute] = ComputeMinuteAndMsOfYear(TEST_EPOCH_JAN_2);
    // (1 day) 1*1440 + (1 hour) 1*60 + (1 min) 1 = 1501 minutes, (30 sec) 30000 ms
    EXPECT_EQ(minuteOfYear, 1501L);
    EXPECT_EQ(msInMinute, 30000L);
}

TEST(ComputeEtaOffsetMsTest, FutureEta) {
    // ETA one minute ahead of current time (60s)
    EXPECT_EQ(ComputeEtaOffsetMs(100, 0, 99, 0), 60000L);
    // One min, 500 ms ahead
    EXPECT_EQ(ComputeEtaOffsetMs(100, 500, 99, 0), 60500L);
}

TEST(ComputeEtaOffsetMsTest, SmallNegativeOffsetIsNotWrapped) {
    // One minute in the past stays negative
    EXPECT_EQ(ComputeEtaOffsetMs(99, 0, 100, 0), -60000L);
}

TEST(ComputeEtaOffsetMsTest, DecJanBoundaryWraps) {
    // ETA minute 1 (early Jan) vs current minute 525950 (late Dec)
    // Offset is almost a year in the past, so it wraps forward.
    constexpr long YEAR_MS = 525960L * 60L * 1000L;
    long expected = (1L * 60000L) + YEAR_MS - (525950L * 60000L);
    EXPECT_EQ(ComputeEtaOffsetMs(1, 0, 525950, 0), expected);
    EXPECT_GT(expected, 0L);
}

TEST(ClassifyStaleTrackedRequestTest, OldCanceledIsCleared) {
    EXPECT_EQ(ClassifyStaleTrackedRequest(true, 2, 100), StaleTrackedAction::SendClearAndErase);
    // Canceled takes precedence over TTL eviction
    EXPECT_EQ(ClassifyStaleTrackedRequest(true, 500, 100), StaleTrackedAction::SendClearAndErase);
}

TEST(ClassifyStaleTrackedRequestTest, CanceledTooRecentIsKept) {
    EXPECT_EQ(ClassifyStaleTrackedRequest(true, 1, 100), StaleTrackedAction::Keep);
}

TEST(ClassifyStaleTrackedRequestTest, TtlExpiryEvicts) {
    EXPECT_EQ(ClassifyStaleTrackedRequest(false, 100, 100), StaleTrackedAction::Evict);
    EXPECT_EQ(ClassifyStaleTrackedRequest(false, 101, 100), StaleTrackedAction::Evict);
}

TEST(ClassifyStaleTrackedRequestTest, FreshRequestIsKept) {
    EXPECT_EQ(ClassifyStaleTrackedRequest(false, 99, 100), StaleTrackedAction::Keep);
    EXPECT_EQ(ClassifyStaleTrackedRequest(false, 0, 100), StaleTrackedAction::Keep);
}
