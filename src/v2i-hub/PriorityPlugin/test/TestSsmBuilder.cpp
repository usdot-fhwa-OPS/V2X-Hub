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
#include "SsmBuilder.hpp"

#include <tmx/j2735_messages/SignalStatusMessage.hpp>

using namespace PriorityPlugin;

namespace {
    // Values shared across the SSM builder tests
    constexpr long TEST_INTERSECTION_ID = 9709;
    constexpr long TEST_INTERSECTION_ID_2 = 9945;
    constexpr uint8_t TEST_REQUEST_ID = 7;
    constexpr uint8_t TEST_REQUEST_ID_2 = 8;
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

TEST(MapNTCIPstatusToSSMTest, QueuedAndTransientStatesAreNotTerminal) {
    // Validated but not yet activated by the CO
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::readyQueued), PrioritizationResponseStatus_requested);
    // Overridden requests stay in the queue and may still be serviced
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::readyOverridden), PrioritizationResponseStatus_processing);
    // Override negotiation in flight. The CO may refuse it
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::activeOverride), PrioritizationResponseStatus_processing);
}

TEST(MapNTCIPstatusToSSMTest, ActiveServiceAndCompletionAreGranted) {
    // The intervention is running
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::activeProcessing), PrioritizationResponseStatus_granted);
    // Current timing already satisfies the request; nothing more will happen
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::activeAdjustNotNeeded), PrioritizationResponseStatus_granted);
    // The CO refused the override, so this request's service continues
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::activeNotOverridden), PrioritizationResponseStatus_granted);
    // Service delivered
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::closedCompleted), PrioritizationResponseStatus_granted);
}

TEST(MapNTCIPstatusToSSMTest, CancellationsWatchOtherTraffic) {
    // Cancel request is not a controller refusal
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::activeCancel), PrioritizationResponseStatus_watchOtherTraffic);
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::closedCanceled), PrioritizationResponseStatus_watchOtherTraffic);
}

TEST(MapNTCIPstatusToSSMTest, TerminalErrorsMapToRejectedAndSpecificStatuses) {
    // Genuine CO refusals
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::closedTimerError), PrioritizationResponseStatus_rejected);
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::closedStrategyError), PrioritizationResponseStatus_rejected);
    EXPECT_EQ(MapNTCIPstatusToSSM(RequestStatus::closedFlash), PrioritizationResponseStatus_rejected);
    // Dedicated J2735 statuses
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
    table[0].inboundPresent = IntersectionAccessPoint_PR_lane;
    table[0].inboundValue = 2;

    SsmStateFixture fix;
    auto st = fix.state();
    auto ssm = BuildSsmFromTable(table, TEST_MAX_BROADCASTS, TEST_NOW_EPOCH, st);

    ASSERT_NE(ssm, nullptr);
    EXPECT_EQ(SignalStatusCount(ssm.get()), 1);
    EXPECT_EQ(FirstSignalStatusPackageCount(ssm.get()), 1);
    EXPECT_EQ(ssm->status.list.array[0]->id.id, TEST_INTERSECTION_ID);
    EXPECT_EQ(fix.ssmSequenceCounter, 1);
    EXPECT_EQ(table[0].ssmBroadcastCount, 1);
    auto &pkg = ssm->status.list.array[0]->sigStatus.list.array[0];
    EXPECT_EQ(pkg->inboundOn.present, IntersectionAccessPoint_PR_lane);
    EXPECT_EQ(pkg->inboundOn.choice.lane, 2);
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
    req.inboundPresent = IntersectionAccessPoint_PR_lane;
    req.inboundValue = 2;
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
    auto &pkg = ssm->status.list.array[0]->sigStatus.list.array[0];
    EXPECT_EQ(pkg->status, PrioritizationResponseStatus_rejected);
    EXPECT_EQ(pkg->inboundOn.present, IntersectionAccessPoint_PR_lane);
    EXPECT_EQ(pkg->inboundOn.choice.lane, 2);
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

TEST(BuildSsmFromTableTest, RequesterChoiceVariants) {
    // sizeof(StationID_t) bytes selects the stationID choice
    {
        std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> table{};
        table[0].statusInPRS = RequestStatus::readyQueued;
        table[0].requestID = TEST_REQUEST_ID;
        table[0].intersectionID = TEST_INTERSECTION_ID;
        std::vector<uint8_t> stationBytes(sizeof(StationID_t), 0);
        StationID_t station = 0xAABBCCDD;
        std::memcpy(stationBytes.data(), &station, sizeof(station));
        table[0].vehicleID = stationBytes;

        SsmStateFixture fix;
        auto st = fix.state();
        auto ssm = BuildSsmFromTable(table, TEST_MAX_BROADCASTS, TEST_NOW_EPOCH, st);

        ASSERT_NE(ssm, nullptr);
        auto *requester = ssm->status.list.array[0]->sigStatus.list.array[0]->requester;
        ASSERT_NE(requester, nullptr);
        EXPECT_EQ(requester->id.present, VehicleID_PR_stationID);
        EXPECT_EQ(requester->id.choice.stationID, station);
    }

    // vehicleID left empty leaves the requester choice unset
    {
        std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> table{};
        table[0].statusInPRS = RequestStatus::readyQueued;
        table[0].requestID = TEST_REQUEST_ID;
        table[0].intersectionID = TEST_INTERSECTION_ID;

        SsmStateFixture fix;
        auto st = fix.state();
        auto ssm = BuildSsmFromTable(table, TEST_MAX_BROADCASTS, TEST_NOW_EPOCH, st);

        ASSERT_NE(ssm, nullptr);
        auto *requester = ssm->status.list.array[0]->sigStatus.list.array[0]->requester;
        ASSERT_NE(requester, nullptr);
        EXPECT_EQ(requester->id.present, VehicleID_PR_NOTHING);
    }
}

TEST(BuildSsmFromTableTest, OmitMinuteAndDuration) {
    std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> table{};
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].requestID = TEST_REQUEST_ID;
    table[0].intersectionID = TEST_INTERSECTION_ID;
    table[0].vehicleID = TEST_VEHICLE_ID;
    table[0].inboundPresent = IntersectionAccessPoint_PR_approach;
    table[0].inboundValue = 5;
    // TSD/TED left 0: no minute/second and no duration allocated

    SsmStateFixture fix;
    auto st = fix.state();
    auto ssm = BuildSsmFromTable(table, TEST_MAX_BROADCASTS, TEST_NOW_EPOCH, st);

    ASSERT_NE(ssm, nullptr);
    auto &pkg = ssm->status.list.array[0]->sigStatus.list.array[0];
    EXPECT_EQ(pkg->inboundOn.present, IntersectionAccessPoint_PR_approach);
    EXPECT_EQ(pkg->inboundOn.choice.approach, 5);
    EXPECT_EQ(pkg->minute, nullptr);
    EXPECT_EQ(pkg->duration, nullptr);
}

TEST(BuildSsmFromTableTest, CoActiveState) {
    std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> table{};
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].statusInCO = RequestStatus::activeProcessing;
    table[0].requestID = TEST_REQUEST_ID;
    table[0].intersectionID = TEST_INTERSECTION_ID;
    table[0].vehicleID = TEST_VEHICLE_ID;

    SsmStateFixture fix;
    auto st = fix.state();
    auto ssm = BuildSsmFromTable(table, TEST_MAX_BROADCASTS, TEST_NOW_EPOCH, st);

    ASSERT_NE(ssm, nullptr);
    // CO activeProcessing wins over the PRS readyQueued for the SSM status 
    // Active service is reported as granted
    EXPECT_EQ(ssm->status.list.array[0]->sigStatus.list.array[0]->status, PrioritizationResponseStatus_granted);
    EXPECT_EQ(table[0].ssmLastStatus, RequestStatus::activeProcessing);
}

TEST(BuildSsmFromTableTest, UnchangedContents) {
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
    EXPECT_EQ(fix.signalStatusSeqByIntersection[TEST_INTERSECTION_ID], 1);
    {
        auto st = fix.state();
        BuildSsmFromTable(table, TEST_MAX_BROADCASTS, TEST_NOW_EPOCH, st);
    }
    // signalStatus sequence does not advance, but SSM counter does
    EXPECT_EQ(fix.signalStatusSeqByIntersection[TEST_INTERSECTION_ID], 1);
    EXPECT_EQ(fix.ssmSequenceCounter, 2);
}

TEST(BuildSsmFromTableTest, OneIntersectionEntryOneSignalStatus) {
    std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> table{};
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].requestID = TEST_REQUEST_ID;
    table[0].intersectionID = TEST_INTERSECTION_ID;
    table[0].vehicleID = TEST_VEHICLE_ID;

    table[1].statusInPRS = RequestStatus::readyQueued;
    table[1].requestID = TEST_REQUEST_ID_2;
    table[1].intersectionID = TEST_INTERSECTION_ID;
    table[1].vehicleID = TEST_VEHICLE_ID;

    SsmStateFixture fix;
    auto st = fix.state();
    auto ssm = BuildSsmFromTable(table, TEST_MAX_BROADCASTS, TEST_NOW_EPOCH, st);

    ASSERT_NE(ssm, nullptr);
    EXPECT_EQ(SignalStatusCount(ssm.get()), 1); // 1 intersection, one status
    EXPECT_EQ(FirstSignalStatusPackageCount(ssm.get()), 2); // 2 packages in one status
    EXPECT_EQ(ssm->status.list.array[0]->id.id, TEST_INTERSECTION_ID);
    auto &packages = ssm->status.list.array[0]->sigStatus.list;
    EXPECT_EQ(packages.array[0]->requester->request, TEST_REQUEST_ID);
    EXPECT_EQ(packages.array[1]->requester->request, TEST_REQUEST_ID_2);
}

TEST(BuildSsmFromTableTest, TwoIntersectionEntriesTwoSignalStatuses) {
    std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> table{};
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].requestID = TEST_REQUEST_ID;
    table[0].intersectionID = TEST_INTERSECTION_ID;
    table[0].vehicleID = TEST_VEHICLE_ID;

    table[1].statusInPRS = RequestStatus::readyQueued;
    table[1].requestID = TEST_REQUEST_ID_2;
    table[1].intersectionID = TEST_INTERSECTION_ID_2;
    table[1].vehicleID = TEST_VEHICLE_ID;

    SsmStateFixture fix;
    auto st = fix.state();
    auto ssm = BuildSsmFromTable(table, TEST_MAX_BROADCASTS, TEST_NOW_EPOCH, st);

    ASSERT_NE(ssm, nullptr);
    ASSERT_EQ(SignalStatusCount(ssm.get()), 2); // 2 intersections, 2 statuses
    EXPECT_EQ(ssm->status.list.array[0]->id.id, TEST_INTERSECTION_ID);
    EXPECT_EQ(ssm->status.list.array[0]->sigStatus.list.count, 1);
    EXPECT_EQ(ssm->status.list.array[0]->sigStatus.list.array[0]->requester->request, TEST_REQUEST_ID);
    EXPECT_EQ(ssm->status.list.array[1]->id.id, TEST_INTERSECTION_ID_2);
    EXPECT_EQ(ssm->status.list.array[1]->sigStatus.list.count, 1);
    EXPECT_EQ(ssm->status.list.array[1]->sigStatus.list.array[0]->requester->request, TEST_REQUEST_ID_2);
}

TEST(BuildSsmFromRequestorTest, InboundApproachSet) {
    RequestorState state;
    state.vehicleID = TEST_VEHICLE_ID;
    state.sequenceNumber = TEST_SEQUENCE_NUMBER;
    PriorityPlugin::SignalRequest req{};
    req.requestID = TEST_REQUEST_ID;
    req.intersectionID = TEST_INTERSECTION_ID;
    req.requestType = PriorityRequestType_priorityRequest;
    req.inboundPresent = IntersectionAccessPoint_PR_approach;
    req.inboundValue = 3;
    req.etaMinute = TEST_ETA_MINUTE;
    req.etaSecond = TEST_ETA_SECOND;
    req.duration = TEST_DURATION;
    state.requests.push_back(req);

    SsmStateFixture fix;
    auto st = fix.state();
    auto ssm = BuildSsmFromRequestor(state, TEST_NOW_MS, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, st);

    ASSERT_NE(ssm, nullptr);
    auto &pkg = ssm->status.list.array[0]->sigStatus.list.array[0];
    EXPECT_EQ(pkg->inboundOn.present, IntersectionAccessPoint_PR_approach);
    EXPECT_EQ(pkg->inboundOn.choice.approach, 3);
    EXPECT_EQ(*pkg->minute, TEST_ETA_MINUTE);
    EXPECT_EQ(*pkg->second, TEST_ETA_SECOND);
    EXPECT_EQ(*pkg->duration, TEST_DURATION);
}

TEST(BuildSsmFromRequestorTest, FallbackWhenDepartureNotAfterArrival) {
    RequestorState state;
    state.vehicleID = TEST_VEHICLE_ID;
    state.sequenceNumber = TEST_SEQUENCE_NUMBER;
    PriorityPlugin::SignalRequest req{};
    req.requestID = TEST_REQUEST_ID;
    req.intersectionID = TEST_INTERSECTION_ID;
    req.requestType = PriorityRequestType_priorityRequest;
    req.duration = 0; // fall back to configured estimates
    state.requests.push_back(req);

    SsmStateFixture fix;
    auto st = fix.state();
    // estimatedDepartureTime (20) <= estimatedArrivalTime (30): fallback uses departure alone.
    auto ssm = BuildSsmFromRequestor(state, TEST_NOW_MS, /*arrival*/ 30, /*departure*/ 20, st);

    auto &pkg = ssm->status.list.array[0]->sigStatus.list.array[0];
    ASSERT_NE(pkg->duration, nullptr);
    EXPECT_EQ(*pkg->duration, 20 * 1000);
}

TEST(BuildSsmFromRequestorTest, DurationFromEstimateWhenDepartureAfterArrival) {
    RequestorState state;
    state.vehicleID = TEST_VEHICLE_ID;
    state.sequenceNumber = TEST_SEQUENCE_NUMBER;
    PriorityPlugin::SignalRequest req{};
    req.requestID = TEST_REQUEST_ID;
    req.intersectionID = TEST_INTERSECTION_ID;
    req.requestType = PriorityRequestType_priorityRequest;
    req.duration = 0;
    state.requests.push_back(req);

    SsmStateFixture fix;
    auto st = fix.state();
    auto ssm = BuildSsmFromRequestor(state, TEST_NOW_MS, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, st);

    auto &pkg = ssm->status.list.array[0]->sigStatus.list.array[0];
    ASSERT_NE(pkg->duration, nullptr);
    EXPECT_EQ(*pkg->duration, (TEST_EST_DEPARTURE - TEST_EST_ARRIVAL) * 1000);
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
    auto &pkg = ssm->status.list.array[0]->sigStatus.list.array[0];
    ASSERT_NE(pkg->minute, nullptr); // fallback logic triggered; not null
    EXPECT_EQ(pkg->status, PrioritizationResponseStatus_processing);
}
