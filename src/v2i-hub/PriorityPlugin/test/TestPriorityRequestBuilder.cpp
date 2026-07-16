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
#include "PriorityRequestBuilder.hpp"

#include <tsc/NTCIP_1211_MIB.h>

using namespace PriorityPlugin;
using namespace tmx::utils;

namespace {
    // Values shared across the builder tests.
    constexpr long TEST_INTERSECTION_ID = 9709;
    constexpr long TEST_LANE = 2;
    constexpr uint8_t TEST_REQUEST_ID = 7;
    constexpr uint8_t TEST_STRATEGY = 3;
    constexpr time_t TEST_NOW_EPOCH = 1000;
    constexpr uint32_t TEST_TTL_SEC = 60;
    constexpr uint16_t TEST_EST_ARRIVAL = 30;
    constexpr uint16_t TEST_EST_DEPARTURE = 40;
    constexpr uint32_t TEST_TIME_OF_REQUEST = 12345;
    constexpr uint64_t TEST_NOW_MS = 1000000;
    const std::vector<uint8_t> TEST_VEHICLE_ID{0xAA, 0xBB, 0xCC, 0xDD};
    const std::string TEST_VEHICLE_KEY = "veh1";

    // Build a minimal J2735 SignalRequestPackage for the builder tests.
    struct SrmPackage {
        SignalRequestPackage pkg{};
        MinuteOfTheYear_t minuteStore = 1;
        DSecond_t secondStore = 2;
        DSecond_t durationStore = 3;

        SrmPackage(long intersectionID, uint8_t requestID, long requestType, long inboundLane = TEST_LANE) {
            pkg.request.id.id = intersectionID;
            pkg.request.requestID = requestID;
            pkg.request.requestType = requestType;
            pkg.request.inBoundLane.present = IntersectionAccessPoint_PR_lane;
            pkg.request.inBoundLane.choice.lane = inboundLane;
        }

        // Attach the optional J2735 ETA fields (minute of year, ms in minute, ms duration).
        void WithEta(long minute, long second, long duration) {
            minuteStore = minute;
            secondStore = second;
            durationStore = duration;
            pkg.minute = &minuteStore;
            pkg.second = &secondStore;
            pkg.duration = &durationStore;
        }

        // Switch the inbound access point to an approach instead of a lane.
        void WithApproach(long approach) {
            pkg.request.inBoundLane.present = IntersectionAccessPoint_PR_approach;
            pkg.request.inBoundLane.choice.approach = approach;
        }
    };
} // namespace

TEST(ApplyPrsPackageTest, FreeSlotInsert) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, 1);
    auto &table = proc.Table();
    std::array<uint32_t, 10> reservice{};

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);

    auto result = ApplyPrsPackage(table, proc, pk.pkg, PrsPackageInput{
        TEST_VEHICLE_ID, /*classType*/ 1, /*classLevel*/ 1, /*newSeq*/ 4, /*role*/ 2,
        /*minOfYear*/ 0, /*msInMinute*/ 0, TEST_NOW_EPOCH, reservice, TEST_TTL_SEC, 
        TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, /*prsBusy*/ false});

    EXPECT_EQ(result.action, PrsPackageResult::Action::Inserted);
    EXPECT_EQ(result.slotIndex, 0u);
    EXPECT_FALSE(result.overrideTriggered);
    EXPECT_EQ(table[0].statusInPRS, RequestStatus::readyQueued);
    EXPECT_EQ(table[0].requestID, TEST_REQUEST_ID);
    EXPECT_EQ(table[0].serviceStrategyNumber, 1);
    EXPECT_EQ(table[0].intersectionID, TEST_INTERSECTION_ID);
    EXPECT_EQ(table[0].sequenceNumber, 4);
    EXPECT_EQ(table[0].vehicleID, TEST_VEHICLE_ID);
    EXPECT_EQ(table[0].timeOfServiceDesiredInPRS, 1030u);
    EXPECT_EQ(table[0].timeOfEstimatedDepartureInPRS, 1040u);
}

TEST(ApplyPrsPackageTest, UpdateExistingEntry) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    auto &table = proc.Table();
    std::array<uint32_t, 10> reservice{};

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);

    // First application
    ApplyPrsPackage(table, proc, pk.pkg, PrsPackageInput{TEST_VEHICLE_ID, 1, 1, 4, 2,
        0, 0, TEST_NOW_EPOCH, reservice, TEST_TTL_SEC, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, false});
    EXPECT_EQ(table[0].statusInPRS, RequestStatus::readyQueued);

    // Same package, but update with new sequence and times
    auto result = ApplyPrsPackage(table, proc, pk.pkg, PrsPackageInput{TEST_VEHICLE_ID, 1, 1, 9, 2,
        0, 0, 2000, reservice, TEST_TTL_SEC, 50, 70, false});

    // Table contains updated values
    EXPECT_EQ(result.action, PrsPackageResult::Action::Updated);
    EXPECT_EQ(result.slotIndex, 0u);
    EXPECT_EQ(table[0].sequenceNumber, 9);
    EXPECT_EQ(table[0].timeOfServiceDesiredInPRS, 2050u);
    EXPECT_EQ(table[0].timeOfEstimatedDepartureInPRS, 2070u);
    // Still a single occupied slot
    EXPECT_EQ(table[1].statusInPRS, RequestStatus::idleNotValid);
}

TEST(ApplyPrsPackageTest, StrategyMissingRejected) {
    PriorityRequestProcessor proc;
    // No lane strategy here
    auto &table = proc.Table();
    std::array<uint32_t, 10> reservice{};

    SrmPackage pk(1234, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);

    auto result = ApplyPrsPackage(table, proc, pk.pkg, PrsPackageInput{TEST_VEHICLE_ID, 1, 1, 4, 2,
        0, 0, TEST_NOW_EPOCH, reservice, TEST_TTL_SEC, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, false});

    EXPECT_EQ(result.action, PrsPackageResult::Action::Rejected);
    EXPECT_EQ(table[result.slotIndex].statusInPRS, RequestStatus::closedStrategyError);
}

TEST(ApplyPrsPackageTest, ReserviceNotMetRejected) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    auto &table = proc.Table();

    // Seed a recent completion for class type 1 via the CO closedCompleted path.
    std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows{};
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].vehicleClassType = 1;
    coRows[0].requestStatusInCO = RequestStatus::closedCompleted;
    proc.ApplyCoStatusUpdates(coRows, 1000);
    EXPECT_EQ(proc.ReserviceLastCompleted(1), 1000u);
    // Reset the seeded slot back to idle so the new request lands cleanly.
    table[0] = PriorityRequestEntry{};

    std::array<uint32_t, 10> reservice{};
    reservice[0] = 300; // class type 1 reservice period (seconds)

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);

    // now (1100) - lastCompleted (1000) = 100 < 300 => reserviceError.
    auto result = ApplyPrsPackage(table, proc, pk.pkg, PrsPackageInput{TEST_VEHICLE_ID, 1, 1, 4, 2,
        0, 0, 1100, reservice, TEST_TTL_SEC, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, false});

    EXPECT_EQ(result.action, PrsPackageResult::Action::Rejected);
    EXPECT_EQ(table[result.slotIndex].statusInPRS, RequestStatus::reserviceError);
}

TEST(ApplyPrsPackageTest, TableFull) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    auto &table = proc.Table();
    // Fill every slot with an active entry that won't match the incoming request
    for (auto &e : table) {
        e.statusInPRS = RequestStatus::readyQueued;
        e.requestID = 200;
        e.vehicleID = {0xFF};
    }
    std::array<uint32_t, 10> reservice{};

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);

    auto result = ApplyPrsPackage(table, proc, pk.pkg, PrsPackageInput{TEST_VEHICLE_ID, 1, 1, 4, 2,
        0, 0, TEST_NOW_EPOCH, reservice, TEST_TTL_SEC, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, false});

    EXPECT_EQ(result.action, PrsPackageResult::Action::TableFull);
}

TEST(ApplyPrsPackageTest, OverrideDemotesLowerPriorityActiveEntry) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    auto &table = proc.Table();

    // Existing active CO entry of a lower priority class
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].statusInCO = RequestStatus::activeProcessing;
    table[0].vehicleClassType = 7;
    table[0].vehicleClassLevel = 1;
    table[0].requestID = 50;
    table[0].vehicleID = {0xAA};

    std::array<uint32_t, 10> reservice{};

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);

    // Apply higher-priority request with prsBusy => override
    auto result = ApplyPrsPackage(table, proc, pk.pkg, PrsPackageInput{TEST_VEHICLE_ID, 
        /*classType*/ 1, /*classLevel*/ 1, 
        4, 2, 0, 0, TEST_NOW_EPOCH, reservice, TEST_TTL_SEC, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE,
        /*prsBusy*/ true});

    EXPECT_EQ(result.action, PrsPackageResult::Action::Inserted);
    EXPECT_TRUE(result.overrideTriggered);
    EXPECT_EQ(table[0].statusInPRS, RequestStatus::activeOverride);
}

TEST(BuildPrgPackageTest, CancellationPath) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    std::unordered_map<std::string, PrgTrackedRequest> tracked;
    std::unordered_set<long> configured{TEST_INTERSECTION_ID};

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityCancellation);

    auto result = BuildPrgPackage(tracked, configured, proc, pk.pkg, PrgPackageInput{
        TEST_VEHICLE_ID, TEST_VEHICLE_KEY, 1, 1, 0, 0, TEST_TIME_OF_REQUEST,
        TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, TEST_NOW_MS});

    EXPECT_EQ(result.outcome, PrgPackageResult::Outcome::Send);
    EXPECT_TRUE(result.isCancel);
    EXPECT_EQ(result.targetOID, tsc::mib::ntcip1211::PRIORITY_CANCEL_OID);
    EXPECT_EQ(result.encodedPayload.size(), PRIORITY_CANCEL_SIZE);
}

TEST(BuildPrgPackageTest, UpdateWithoutTracker) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    std::unordered_map<std::string, PrgTrackedRequest> tracked; // empty
    std::unordered_set<long> configured{TEST_INTERSECTION_ID};

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequestUpdate);

    auto result = BuildPrgPackage(tracked, configured, proc, pk.pkg, PrgPackageInput{
        TEST_VEHICLE_ID, TEST_VEHICLE_KEY, 1, 1, 0, 0, TEST_TIME_OF_REQUEST,
        TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, TEST_NOW_MS});

    EXPECT_EQ(result.outcome, PrgPackageResult::Outcome::Send);
    EXPECT_FALSE(result.isCancel);
    // No tracker; sends as a new request instead of update
    EXPECT_EQ(result.targetOID, tsc::mib::ntcip1211::PRIORITY_REQUEST_ABSOLUTE_OID);
    EXPECT_EQ(result.encodedPayload.size(), PRIORITY_REQUEST_SIZE);
}

TEST(BuildPrgPackageTest, NewRequestWithTracker) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    std::unordered_map<std::string, PrgTrackedRequest> tracked;
    std::unordered_set<long> configured{TEST_INTERSECTION_ID};

    // Insert sent request
    std::string trackerKey = TEST_VEHICLE_KEY + "|" + std::to_string(TEST_REQUEST_ID)
        + "|" + std::to_string(TEST_INTERSECTION_ID);
    PrgTrackedRequest existing;
    existing.requestID = TEST_REQUEST_ID;
    existing.intersectionID = TEST_INTERSECTION_ID;
    existing.state = PrgRequestState::sent;
    tracked[trackerKey] = existing;

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);

    auto result = BuildPrgPackage(tracked, configured, proc, pk.pkg, PrgPackageInput{
        TEST_VEHICLE_ID, TEST_VEHICLE_KEY, 1, 1, 0, 0, TEST_TIME_OF_REQUEST,
        TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, TEST_NOW_MS});

    EXPECT_EQ(result.outcome, PrgPackageResult::Outcome::Send);
    // Existing tracker; sends update request
    EXPECT_EQ(result.targetOID, tsc::mib::ntcip1211::PRIORITY_UPDATE_ABSOLUTE_OID);
    EXPECT_EQ(result.trackerKey, trackerKey);
}

TEST(BuildPrgPackageTest, NoControllerForIntersection) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    std::unordered_map<std::string, PrgTrackedRequest> tracked;
    std::unordered_set<long> configured; // empty, no controller

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);

    auto result = BuildPrgPackage(tracked, configured, proc, pk.pkg, PrgPackageInput{
        TEST_VEHICLE_ID, TEST_VEHICLE_KEY, 1, 1, 0, 0, TEST_TIME_OF_REQUEST,
        TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, TEST_NOW_MS});

    EXPECT_EQ(result.outcome, PrgPackageResult::Outcome::NoController);
    EXPECT_TRUE(result.signalRequest.rejected);
    EXPECT_TRUE(result.encodedPayload.empty());
}

TEST(ApplyPrsPackageTest, EtaMinutePresentComputesGlobalTimes) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    auto &table = proc.Table();
    std::array<uint32_t, 10> reservice{};

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);
    // Creates 60500ms offset. 5000ms duration puts departure 65500ms out.
    pk.WithEta(10, 500, 5000);

    auto result = ApplyPrsPackage(table, proc, pk.pkg, PrsPackageInput{TEST_VEHICLE_ID, 1, 1, 4, 2,
        /*minOfYear*/ 9, /*msInMinute*/ 0, TEST_NOW_EPOCH, reservice, TEST_TTL_SEC,
        TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, false});

    EXPECT_EQ(result.action, PrsPackageResult::Action::Inserted);
    EXPECT_EQ(table[0].timeOfServiceDesiredInPRS, static_cast<uint32_t>(TEST_NOW_EPOCH) + 60u);
    EXPECT_EQ(table[0].timeOfEstimatedDepartureInPRS, static_cast<uint32_t>(TEST_NOW_EPOCH) + 65u);
}

TEST(ApplyPrsPackageTest, ApproachInboundHasNoLaneStrategy) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    auto &table = proc.Table();
    std::array<uint32_t, 10> reservice{};

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);
    pk.WithApproach(4);

    auto result = ApplyPrsPackage(table, proc, pk.pkg, PrsPackageInput{TEST_VEHICLE_ID, 1, 1, 4, 2,
        0, 0, TEST_NOW_EPOCH, reservice, TEST_TTL_SEC, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, false});

    // Strategy lookup requires a lane; an approach-only request is rejected but recorded
    EXPECT_EQ(result.action, PrsPackageResult::Action::Rejected);
    EXPECT_EQ(table[result.slotIndex].statusInPRS, RequestStatus::closedStrategyError);
    EXPECT_EQ(table[result.slotIndex].inboundPresent, IntersectionAccessPoint_PR_approach);
    EXPECT_EQ(table[result.slotIndex].inboundValue, 4);
}

TEST(BuildPrgPackageTest, ApproachInboundRecorded) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    std::unordered_map<std::string, PrgTrackedRequest> tracked;
    std::unordered_set<long> configured{TEST_INTERSECTION_ID};

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);
    pk.WithApproach(4);

    auto result = BuildPrgPackage(tracked, configured, proc, pk.pkg, PrgPackageInput{
        TEST_VEHICLE_ID, TEST_VEHICLE_KEY, 1, 1, 0, 0, TEST_TIME_OF_REQUEST,
        TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, TEST_NOW_MS});

    // No lane means no strategy, but the approach is still recorded for the SSM
    EXPECT_EQ(result.outcome, PrgPackageResult::Outcome::NoStrategy);
    EXPECT_EQ(result.signalRequest.inboundPresent, IntersectionAccessPoint_PR_approach);
    EXPECT_EQ(result.signalRequest.inboundValue, 4);
}

TEST(ApplyPrsPackageTest, PrsBusyWithNoActiveEntriesDoesNotOverride) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    auto &table = proc.Table();
    std::array<uint32_t, 10> reservice{};

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);

    auto result = ApplyPrsPackage(table, proc, pk.pkg, PrsPackageInput{TEST_VEHICLE_ID, 1, 1, 4, 2,
        0, 0, TEST_NOW_EPOCH, reservice, TEST_TTL_SEC, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE,
        /*prsBusy*/ true});

    EXPECT_EQ(result.action, PrsPackageResult::Action::Inserted);
    EXPECT_FALSE(result.overrideTriggered);
}

TEST(ApplyPrsPackageTest, LowerPriorityDoesNotOverrideActive) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    auto &table = proc.Table();

    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].statusInCO = RequestStatus::activeAdjustNotNeeded;
    table[0].vehicleClassType = 1;
    table[0].vehicleClassLevel = 1;
    table[0].requestID = 50;
    table[0].vehicleID = {0xEE};

    std::array<uint32_t, 10> reservice{};
    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);

    auto result = ApplyPrsPackage(table, proc, pk.pkg, PrsPackageInput{TEST_VEHICLE_ID,
        /*classType*/ 7, /*classLevel*/ 2,
        4, 2, 0, 0, TEST_NOW_EPOCH, reservice, TEST_TTL_SEC, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE,
        /*prsBusy*/ true});

    EXPECT_EQ(result.action, PrsPackageResult::Action::Inserted);
    EXPECT_FALSE(result.overrideTriggered);
    EXPECT_EQ(table[0].statusInPRS, RequestStatus::readyQueued);
}

TEST(BuildPrgPackageTest, EtaMinutePresentComputesRelativeTimes) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    std::unordered_map<std::string, PrgTrackedRequest> tracked;
    std::unordered_set<long> configured{TEST_INTERSECTION_ID};

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);
    pk.WithEta(10, 500, 5000);

    auto result = BuildPrgPackage(tracked, configured, proc, pk.pkg, PrgPackageInput{
        TEST_VEHICLE_ID, TEST_VEHICLE_KEY, 1, 1, /*minOfYear*/ 9, /*msInMinute*/ 0,
        TEST_TIME_OF_REQUEST, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, TEST_NOW_MS});

    EXPECT_EQ(result.outcome, PrgPackageResult::Outcome::Send);
    EXPECT_EQ(result.signalRequest.timeOfService, 60); // expect truncated 60.5s
    EXPECT_EQ(result.signalRequest.timeOfDepart, 65);
    EXPECT_EQ(result.signalRequest.etaMinute, 10);
    EXPECT_EQ(result.signalRequest.etaSecond, 500);
    EXPECT_EQ(result.signalRequest.duration, 5000);
    EXPECT_EQ(result.trackerEntry.requestID, TEST_REQUEST_ID);
    EXPECT_EQ(result.trackerEntry.intersectionID, TEST_INTERSECTION_ID);
    EXPECT_EQ(result.trackerEntry.sentTimeMs, TEST_NOW_MS);
    EXPECT_EQ(result.trackerEntry.state, PrgRequestState::sent);
}

TEST(BuildPrgPackageTest, PastEtaOutOfRangeRejected) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    std::unordered_map<std::string, PrgTrackedRequest> tracked;
    std::unordered_set<long> configured{TEST_INTERSECTION_ID};

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);
    pk.WithEta(9, 0, 0); // one minute in the past relative to minute 10

    auto result = BuildPrgPackage(tracked, configured, proc, pk.pkg, PrgPackageInput{
        TEST_VEHICLE_ID, TEST_VEHICLE_KEY, 1, 1, /*minOfYear*/ 10, /*msInMinute*/ 0,
        TEST_TIME_OF_REQUEST, TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, TEST_NOW_MS});

    // NTCIP 1211 time fields are 1..65535; a past offset is out of range and rejected
    EXPECT_EQ(result.outcome, PrgPackageResult::Outcome::InvalidEta);
    EXPECT_TRUE(result.signalRequest.rejected);
    EXPECT_TRUE(result.encodedPayload.empty());
}

TEST(BuildPrgPackageTest, UpdateWithTrackerUsesUpdateOid) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    std::unordered_map<std::string, PrgTrackedRequest> tracked;
    std::unordered_set<long> configured{TEST_INTERSECTION_ID};

    std::string trackerKey = TEST_VEHICLE_KEY + "|" + std::to_string(TEST_REQUEST_ID)
        + "|" + std::to_string(TEST_INTERSECTION_ID);
    PrgTrackedRequest existing;
    existing.requestID = TEST_REQUEST_ID;
    existing.intersectionID = TEST_INTERSECTION_ID;
    existing.state = PrgRequestState::sent;
    tracked[trackerKey] = existing;

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequestUpdate);

    auto result = BuildPrgPackage(tracked, configured, proc, pk.pkg, PrgPackageInput{
        TEST_VEHICLE_ID, TEST_VEHICLE_KEY, 1, 1, 0, 0, TEST_TIME_OF_REQUEST,
        TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, TEST_NOW_MS});

    EXPECT_EQ(result.outcome, PrgPackageResult::Outcome::Send);
    EXPECT_EQ(result.targetOID, tsc::mib::ntcip1211::PRIORITY_UPDATE_ABSOLUTE_OID);
    EXPECT_EQ(result.encodedPayload.size(), PRIORITY_REQUEST_SIZE);
}

TEST(BuildPrgPackageTest, CanceledTrackerSendsNewRequest) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(TEST_INTERSECTION_ID, TEST_LANE, TEST_STRATEGY);
    std::unordered_map<std::string, PrgTrackedRequest> tracked;
    std::unordered_set<long> configured{TEST_INTERSECTION_ID};

    std::string trackerKey = TEST_VEHICLE_KEY + "|" + std::to_string(TEST_REQUEST_ID)
        + "|" + std::to_string(TEST_INTERSECTION_ID);
    PrgTrackedRequest existing;
    existing.requestID = TEST_REQUEST_ID;
    existing.intersectionID = TEST_INTERSECTION_ID;
    existing.state = PrgRequestState::canceled;
    tracked[trackerKey] = existing;

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);

    auto result = BuildPrgPackage(tracked, configured, proc, pk.pkg, PrgPackageInput{
        TEST_VEHICLE_ID, TEST_VEHICLE_KEY, 1, 1, 0, 0, TEST_TIME_OF_REQUEST,
        TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, TEST_NOW_MS});

    EXPECT_EQ(result.outcome, PrgPackageResult::Outcome::Send);
    // A canceled tracked request is treated as gone; the SRM starts a new request
    EXPECT_EQ(result.targetOID, tsc::mib::ntcip1211::PRIORITY_REQUEST_ABSOLUTE_OID);
}

TEST(BuildPrgPackageTest, NoStrategyMapping) {
    PriorityRequestProcessor proc;
    // Missing strategy here
    std::unordered_map<std::string, PrgTrackedRequest> tracked;
    std::unordered_set<long> configured{TEST_INTERSECTION_ID};

    SrmPackage pk(TEST_INTERSECTION_ID, TEST_REQUEST_ID, PriorityRequestType_priorityRequest);

    auto result = BuildPrgPackage(tracked, configured, proc, pk.pkg, PrgPackageInput{
        TEST_VEHICLE_ID, TEST_VEHICLE_KEY, 1, 1, 0, 0, TEST_TIME_OF_REQUEST,
        TEST_EST_ARRIVAL, TEST_EST_DEPARTURE, TEST_NOW_MS});

    EXPECT_EQ(result.outcome, PrgPackageResult::Outcome::NoStrategy);
    EXPECT_TRUE(result.signalRequest.rejected);
    EXPECT_TRUE(result.encodedPayload.empty());
}
