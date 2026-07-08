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
#include "SsmBuilder.hpp"

#include <tmx/j2735_messages/SignalStatusMessage.hpp>

using namespace PriorityPlugin;

namespace {
    // Values shared across the SSM builder tests
    constexpr long TEST_INTERSECTION_ID = 9709;
    constexpr uint8_t TEST_REQUEST_ID = 7;
    constexpr uint8_t TEST_MAX_BROADCASTS = 5;
    constexpr time_t TEST_NOW_EPOCH = 1000;
    constexpr uint64_t TEST_NOW_MS = 1000000;
    constexpr uint16_t TEST_EST_ARRIVAL = 30;
    constexpr uint16_t TEST_EST_DEPARTURE = 40;
    constexpr uint8_t TEST_SEQUENCE_NUMBER = 3;
    constexpr long TEST_ETA_MINUTE = 100;
    constexpr long TEST_ETA_SECOND = 5000;
    constexpr long TEST_DURATION = 10000;
    const std::vector<uint8_t> TEST_VEHICLE_ID{0xAA, 0xBB, 0xCC, 0xDD};

    // Reference to SSM sequencing storage and SsmBroadcastState needed for tests
    struct SsmStateFixture {
        uint8_t ssmSequenceCounter = 0;
        std::unordered_map<long, uint8_t> signalStatusSeqByIntersection;
        std::unordered_map<long, std::string> lastSignalStatusKey;

        SsmBroadcastState state() {
            return SsmBroadcastState{ssmSequenceCounter, signalStatusSeqByIntersection, lastSignalStatusKey};
        }
    };

    // Returns the number of signalStatus entries in the SSM's status list
    int SignalStatusCount(const SignalStatusMessage_t *ssm) {
        return ssm ? ssm->status.list.count : 0;
    }

    // Returns the number of packages in the first signalStatus entry
    int FirstSignalStatusPackageCount(const SignalStatusMessage_t *ssm) {
        if (!ssm || ssm->status.list.count == 0) return 0;
        return ssm->status.list.array[0]->sigStatus.list.count;
    }
} // namespace

TEST(MapNTCIPstatusToSSMTest, MapsKnownStatuses) {
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::readyQueued), PrioritizationResponseStatus_requested);
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::activeProcessing), PrioritizationResponseStatus_processing);
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::closedCompleted), PrioritizationResponseStatus_granted);
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::closedStrategyError), PrioritizationResponseStatus_rejected);
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::closedTimeToLiveError), PrioritizationResponseStatus_maxPresence);
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::reserviceError), PrioritizationResponseStatus_reserviceLocked);
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::idleNotValid), PrioritizationResponseStatus_unknown);
}

TEST(BuildSsmFromTableTest, EmptyTableReturnsNull) {
    std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> table{};
    // Missing table assignments 
    SsmStateFixture fix;
    auto st = fix.state();
    auto ssm = BuildSsmFromTable(table, TEST_MAX_BROADCASTS, TEST_NOW_EPOCH, st);
    EXPECT_EQ(ssm, nullptr);
}

TEST(BuildSsmFromTableTest, SingleEntryYieldsOneStatusAndPackage) {
    std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> table{};
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].requestID = TEST_REQUEST_ID;
    table[0].intersectionID = TEST_INTERSECTION_ID;
    table[0].vehicleID = TEST_VEHICLE_ID;
    table[0].timeOfServiceDesiredInPRS = 1030;
    table[0].timeOfEstimatedDepartureInPRS = 1040;

    SsmStateFixture fix;
    auto st = fix.state();
    auto ssm = BuildSsmFromTable(table, TEST_MAX_BROADCASTS, TEST_NOW_EPOCH, st);

    ASSERT_NE(ssm, nullptr);
    EXPECT_EQ(SignalStatusCount(ssm.get()), 1);
    EXPECT_EQ(FirstSignalStatusPackageCount(ssm.get()), 1);
    EXPECT_EQ(ssm->status.list.array[0]->id.id, TEST_INTERSECTION_ID);
    EXPECT_EQ(fix.ssmSequenceCounter, 1);
    EXPECT_EQ(table[0].ssmBroadcastCount, 1);
}

TEST(BuildSsmFromTableTest, BroadcastLimiterCapsRepeats) {
    std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> table{};
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].requestID = TEST_REQUEST_ID;
    table[0].intersectionID = TEST_INTERSECTION_ID;
    table[0].vehicleID = TEST_VEHICLE_ID;

    SsmStateFixture fix;

    // maxBroadcastsPerStatus = 2
    {
        auto st = fix.state();
        EXPECT_NE(BuildSsmFromTable(table, 2, TEST_NOW_EPOCH, st), nullptr); // allowed
    }
    {
        auto st = fix.state();
        EXPECT_NE(BuildSsmFromTable(table, 2, TEST_NOW_EPOCH, st), nullptr); // allowed
    }
    {
        auto st = fix.state();
        EXPECT_EQ(BuildSsmFromTable(table, 2, TEST_NOW_EPOCH, st), nullptr); // capped
    }
    EXPECT_EQ(table[0].ssmBroadcastCount, 2); // 2 broadcasts allowed
}

TEST(BuildSsmFromTableTest, StatusTransitionResetsCount) {
    std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> table{};
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].requestID = TEST_REQUEST_ID;
    table[0].intersectionID = TEST_INTERSECTION_ID;
    table[0].vehicleID = TEST_VEHICLE_ID;

    SsmStateFixture fix;
    {
        auto st = fix.state();
        BuildSsmFromTable(table, TEST_MAX_BROADCASTS, TEST_NOW_EPOCH, st);
    }
    EXPECT_EQ(table[0].ssmBroadcastCount, 1);
    EXPECT_EQ(table[0].ssmLastStatus, RequestStatus::readyQueued);

    // Status changes; count resets and increments
    table[0].statusInPRS = RequestStatus::closedCompleted;
    {
        auto st = fix.state();
        BuildSsmFromTable(table, TEST_MAX_BROADCASTS, TEST_NOW_EPOCH, st);
    }
    EXPECT_EQ(table[0].ssmBroadcastCount, 1);
    EXPECT_EQ(table[0].ssmLastStatus, RequestStatus::closedCompleted);
}

TEST(BuildSsmFromRequestorTest, EmptyRequestsReturnsNull) {
    RequestorState state;
    state.vehicleID = TEST_VEHICLE_ID;
    // missing state requests here
    SsmStateFixture fix;
    auto st = fix.state();
    auto ssm = BuildSsmFromRequestor(state, TEST_NOW_MS, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, st);
    EXPECT_EQ(ssm, nullptr);
}

TEST(BuildSsmFromRequestorTest, RejectedRequestMapsToRejectedStatus) {
    RequestorState state;
    state.vehicleID = TEST_VEHICLE_ID;
    state.sequenceNumber = TEST_SEQUENCE_NUMBER;
    PriorityPlugin::SignalRequest req{};
    req.requestID = TEST_REQUEST_ID;
    req.intersectionID = TEST_INTERSECTION_ID;
    req.requestType = PriorityRequestType_priorityRequest;
    req.rejected = true;
    req.etaMinute = TEST_ETA_MINUTE;
    req.etaSecond = TEST_ETA_SECOND;
    req.duration = TEST_DURATION;
    state.requests.push_back(req);

    SsmStateFixture fix;
    auto st = fix.state();
    auto ssm = BuildSsmFromRequestor(state, TEST_NOW_MS, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, st);

    ASSERT_NE(ssm, nullptr);
    ASSERT_EQ(SignalStatusCount(ssm.get()), 1);
    ASSERT_EQ(FirstSignalStatusPackageCount(ssm.get()), 1);
    EXPECT_EQ(ssm->status.list.array[0]->sigStatus.list.array[0]->status, PrioritizationResponseStatus_rejected);
}

TEST(BuildSsmFromRequestorTest, CancellationMapsToWatchOtherTraffic) {
    RequestorState state;
    state.vehicleID = TEST_VEHICLE_ID;
    state.sequenceNumber = TEST_SEQUENCE_NUMBER;
    PriorityPlugin::SignalRequest req{};
    req.requestID = TEST_REQUEST_ID;
    req.intersectionID = TEST_INTERSECTION_ID;
    req.requestType = PriorityRequestType_priorityCancellation;
    req.rejected = false;
    req.etaMinute = TEST_ETA_MINUTE;
    req.etaSecond = TEST_ETA_SECOND;
    req.duration = TEST_DURATION;
    state.requests.push_back(req);

    SsmStateFixture fix;
    auto st = fix.state();
    auto ssm = BuildSsmFromRequestor(state, TEST_NOW_MS, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, st);

    ASSERT_NE(ssm, nullptr);
    EXPECT_EQ(ssm->status.list.array[0]->sigStatus.list.array[0]->status, PrioritizationResponseStatus_watchOtherTraffic);
}

TEST(BuildSsmFromRequestorTest, EtaFallbackWhenMinuteAbsent) {
    RequestorState state;
    state.vehicleID = TEST_VEHICLE_ID;
    state.sequenceNumber = TEST_SEQUENCE_NUMBER;
    PriorityPlugin::SignalRequest req{};
    req.requestID = TEST_REQUEST_ID;
    req.intersectionID = TEST_INTERSECTION_ID;
    req.requestType = PriorityRequestType_priorityRequest;
    req.rejected = false;
    req.etaMinute = 0; // no eta, fallback to now + estimatedArrivalTime
    req.etaSecond = 0;
    req.duration = 0;
    state.requests.push_back(req);

    SsmStateFixture fix;
    auto st = fix.state();
    auto ssm = BuildSsmFromRequestor(state, TEST_NOW_MS, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, st);

    ASSERT_NE(ssm, nullptr);
    auto *pkg = ssm->status.list.array[0]->sigStatus.list.array[0];
    ASSERT_NE(pkg->minute, nullptr); // fallback logic triggered; not null
    EXPECT_EQ(pkg->status, PrioritizationResponseStatus_processing);
}
