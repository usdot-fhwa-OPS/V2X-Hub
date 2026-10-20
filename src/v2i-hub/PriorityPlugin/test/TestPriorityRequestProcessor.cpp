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
#include <array>
#include <vector>
#include "PriorityPluginWorker.hpp"
#include "PriorityRequestProcessor.hpp"

using namespace PriorityPlugin;

namespace {
    // Values shared across the encode tests
    constexpr uint8_t TEST_VEHICLE_ID[] = {0xAA, 0xBB, 0xCC, 0xDD};
    constexpr uint8_t TEST_REQUEST_ID = 0x11;
    constexpr uint8_t TEST_CLASS_TYPE = 1;
    constexpr uint8_t TEST_CLASS_LEVEL = 3;
    constexpr uint8_t TEST_STRATEGY_NUM = 7;
    constexpr uint16_t TEST_TIME_OF_SERVICE = 0x1234;
    constexpr uint16_t TEST_TIME_OF_DEPART = 0x5678;
    constexpr uint32_t TEST_TIME_OF_REQUEST = 0x12345678;
    constexpr uint32_t TEST_PRS_TSD = 0x12345678;
    constexpr uint32_t TEST_PRS_TED = 0x12345681;
    constexpr uint32_t TEST_EPOCH_TSD = 1775846010;
    constexpr uint32_t TEST_EPOCH_TED = 1775846020;
} // namespace

TEST(PriorityRequestProcessorTest, TestEncodeServiceRequest) {
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].serviceStrategyNumber = TEST_STRATEGY_NUM;
    table[0].timeOfServiceDesiredInPRS = TEST_PRS_TSD;
    table[0].timeOfEstimatedDepartureInPRS = TEST_PRS_TED;
    table[0].statusInPRS = RequestStatus::readyQueued;

    table[1].serviceStrategyNumber = 2;
    table[1].timeOfServiceDesiredInPRS = 0x00000001;
    table[1].timeOfEstimatedDepartureInPRS = 0x00000002;
    table[1].statusInPRS = RequestStatus::activeProcessing;

    auto buf = proc.EncodeServiceRequest(false);
    EXPECT_EQ(buf.size(), SERVICE_REQUEST_SIZE);

    EXPECT_EQ(buf[0], TEST_STRATEGY_NUM);
    EXPECT_EQ(buf[1], 0x12);
    EXPECT_EQ(buf[2], 0x34);
    EXPECT_EQ(buf[3], 0x56);
    EXPECT_EQ(buf[4], 0x78);
    EXPECT_EQ(buf[5], 0x12);
    EXPECT_EQ(buf[6], 0x34);
    EXPECT_EQ(buf[7], 0x56);
    EXPECT_EQ(buf[8], 0x81);
    EXPECT_EQ(buf[9], 2); // readyQueued

    EXPECT_EQ(buf[10], 2);
    EXPECT_EQ(buf[14], 0x01);
    EXPECT_EQ(buf[18], 0x02);
    EXPECT_EQ(buf[19], 4); // activeProcessing
    EXPECT_EQ(buf[SERVICE_REQUEST_BUSY_OFFSET], 0); // prsBusy=false

    for (size_t i = SERVICE_REQUEST_BUSY_OFFSET + 1; i < SERVICE_REQUEST_SIZE; i++) {
        EXPECT_EQ(buf[i], 0) << "Reserved byte " << i;
    }
}

TEST(PriorityRequestProcessorTest, TestDecodeCoServiceResponse) {
    std::vector<uint8_t> data(SERVICE_REQUEST_SIZE, 0);
    data[0] = 1;
    uint32_t tsd = 5;
    data[1] = (tsd >> 24) & 0xFF; data[2] = (tsd >> 16) & 0xFF;
    data[3] = (tsd >> 8) & 0xFF; data[4] = tsd & 0xFF;
    uint32_t ted = 8;
    data[5] = (ted >> 24) & 0xFF; data[6] = (ted >> 16) & 0xFF;
    data[7] = (ted >> 8) & 0xFF; data[8] = ted & 0xFF;
    data[9] = 4;
    data[19] = 2;
    data[SERVICE_REQUEST_BUSY_OFFSET] = 1;
    data[SERVICE_REQUEST_BUSY_OFFSET + 1] = 0;

    std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> rows;
    bool coBusy = false;
    ASSERT_TRUE(PriorityRequestProcessor::DecodeCoServiceResponse(data, rows, coBusy));
    EXPECT_EQ(rows[0].strategyRequested, 1);
    EXPECT_EQ(rows[0].requestedTimeOfServiceDesired, 5u);
    EXPECT_EQ(rows[0].requestedTimeOfEstimatedDeparture, 8u);
    EXPECT_EQ(rows[0].requestStatusInCO, RequestStatus::activeProcessing);
    EXPECT_EQ(rows[1].requestStatusInCO, RequestStatus::readyQueued);
    EXPECT_TRUE(coBusy);
}

TEST(PriorityRequestProcessorTest, TestInvalidStatusDefault) {
    std::vector<uint8_t> data(SERVICE_REQUEST_SIZE, 0);
    data[9]  = 0;
    data[19] = 16;
    data[29] = 255;
    data[39] = 0;
    data[49] = 16;
    data[59] = 255;
    data[69] = 0;
    data[79] = 16;
    data[89] = 255;
    data[99] = 0;

    std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> rows;
    bool coBusy = false;
    ASSERT_TRUE(PriorityRequestProcessor::DecodeCoServiceResponse(data, rows, coBusy));
    for (size_t i = 0; i < MAX_SERVICE_REQUESTS; i++) {
        EXPECT_EQ(rows[i].requestStatusInCO, RequestStatus::idleNotValid);
    }
}

TEST(PriorityRequestProcessorTest, DecodeCoServiceResponseShortBuffer) {
    std::vector<uint8_t> data(50, 0);
    std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> rows;
    bool coBusy = false;
    EXPECT_FALSE(PriorityRequestProcessor::DecodeCoServiceResponse(data, rows, coBusy));
}

TEST(PriorityRequestProcessorTest, EncodeDecodePrsServiceRequest) {
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].serviceStrategyNumber = 1;
    table[0].timeOfServiceDesiredInPRS = TEST_EPOCH_TSD;
    table[0].timeOfEstimatedDepartureInPRS = TEST_EPOCH_TSD + 3;
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[1].serviceStrategyNumber = 2;
    table[1].timeOfServiceDesiredInPRS = TEST_EPOCH_TED;
    table[1].timeOfEstimatedDepartureInPRS = TEST_EPOCH_TED + 5;
    table[1].statusInPRS = RequestStatus::activeProcessing;

    auto buf = proc.EncodeServiceRequest(true);
    std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> decoded;
    bool coBusy = false;
    ASSERT_TRUE(PriorityRequestProcessor::DecodeCoServiceResponse(buf, decoded, coBusy));

    EXPECT_EQ(decoded[0].strategyRequested, 1);
    EXPECT_EQ(decoded[0].requestedTimeOfServiceDesired, TEST_EPOCH_TSD);
    EXPECT_EQ(decoded[0].requestedTimeOfEstimatedDeparture, TEST_EPOCH_TSD + 3);
    EXPECT_EQ(decoded[0].requestStatusInCO, RequestStatus::readyQueued);
    EXPECT_EQ(decoded[1].strategyRequested, 2);
    EXPECT_EQ(decoded[1].requestedTimeOfServiceDesired, TEST_EPOCH_TED);
    EXPECT_EQ(decoded[1].requestedTimeOfEstimatedDeparture, TEST_EPOCH_TED + 5);
    EXPECT_EQ(decoded[1].requestStatusInCO, RequestStatus::activeProcessing);
    EXPECT_TRUE(coBusy);
}

TEST(PriorityRequestProcessorTest, TestEncodePriorityRequest) {
    auto buf = PriorityRequestProcessor::EncodePriorityRequest(
        TEST_REQUEST_ID,
        TEST_VEHICLE_ID, sizeof(TEST_VEHICLE_ID),
        TEST_CLASS_TYPE,
        TEST_CLASS_LEVEL,
        TEST_STRATEGY_NUM,
        TEST_TIME_OF_SERVICE,
        TEST_TIME_OF_DEPART,
        TEST_TIME_OF_REQUEST);

    EXPECT_EQ(buf.size(), PRIORITY_REQUEST_SIZE);
    EXPECT_EQ(buf[0], TEST_REQUEST_ID);
    for (size_t i = 1; i <= 13; i++) EXPECT_EQ(buf[i], 0); // left-padding; assumed to be vehicle VIN in 1211, but we use SRM vehicle ID, which is shorter
    EXPECT_EQ(buf[14], TEST_VEHICLE_ID[0]);
    EXPECT_EQ(buf[15], TEST_VEHICLE_ID[1]);
    EXPECT_EQ(buf[16], TEST_VEHICLE_ID[2]);
    EXPECT_EQ(buf[17], TEST_VEHICLE_ID[3]);
    EXPECT_EQ(buf[18], TEST_CLASS_TYPE);
    EXPECT_EQ(buf[19], TEST_CLASS_LEVEL);
    EXPECT_EQ(buf[20], TEST_STRATEGY_NUM);
    EXPECT_EQ(buf[21], 0x12);
    EXPECT_EQ(buf[22], 0x34);
    EXPECT_EQ(buf[23], 0x56);
    EXPECT_EQ(buf[24], 0x78);
    EXPECT_EQ(buf[25], 0x12);
    EXPECT_EQ(buf[26], 0x34);
    EXPECT_EQ(buf[27], 0x56);
    EXPECT_EQ(buf[28], 0x78);
}

TEST(PriorityRequestProcessorTest, EncodePriorityRequestVehicleIDTruncated) {
    std::vector<uint8_t> vehId(25, 0x55);
    auto buf = PriorityRequestProcessor::EncodePriorityRequest(
        1, vehId.data(), vehId.size(), 1, 1, 1, 1, 1, 1);
    for (size_t i = 1; i <= VEHICLE_ID_FIELD_SIZE; i++) {
        EXPECT_EQ(buf[i], 0x55) << "Truncated slot " << i;
    }
    EXPECT_EQ(buf[18], 1);
}

TEST(PriorityRequestProcessorTest, EncodePriorityRequestNullVehicleID) {
    auto buf = PriorityRequestProcessor::EncodePriorityRequest(
        9, nullptr, 0, 2, 2, 2, 0, 0, 0);
    EXPECT_EQ(buf[0], 9);
    for (size_t i = 1; i <= 17; i++) EXPECT_EQ(buf[i], 0);
    EXPECT_EQ(buf[18], 2);
}

TEST(PriorityRequestProcessorTest, TestEncodePriorityUpdate) {
    auto buf = PriorityRequestProcessor::EncodePriorityUpdate(
        TEST_REQUEST_ID,
        TEST_VEHICLE_ID, sizeof(TEST_VEHICLE_ID),
        TEST_CLASS_TYPE,
        TEST_CLASS_LEVEL,
        TEST_STRATEGY_NUM,
        TEST_TIME_OF_SERVICE,
        TEST_TIME_OF_DEPART,
        TEST_TIME_OF_REQUEST);

    // Update is identical to Request encoding (29 bytes, same layout)
    EXPECT_EQ(buf.size(), PRIORITY_REQUEST_SIZE);
    auto reqBuf = PriorityRequestProcessor::EncodePriorityRequest(
        TEST_REQUEST_ID, TEST_VEHICLE_ID, sizeof(TEST_VEHICLE_ID),
        TEST_CLASS_TYPE, TEST_CLASS_LEVEL, TEST_STRATEGY_NUM,
        TEST_TIME_OF_SERVICE, TEST_TIME_OF_DEPART, TEST_TIME_OF_REQUEST);
    EXPECT_EQ(buf, reqBuf);
}

TEST(PriorityRequestProcessorTest, TestEncodePriorityCancel) {
    auto buf = PriorityRequestProcessor::EncodePriorityCancel(
        TEST_REQUEST_ID,
        TEST_VEHICLE_ID, sizeof(TEST_VEHICLE_ID),
        TEST_CLASS_TYPE,
        TEST_CLASS_LEVEL,
        TEST_STRATEGY_NUM);

    EXPECT_EQ(buf.size(), PRIORITY_CANCEL_SIZE);
    EXPECT_EQ(buf[0], TEST_REQUEST_ID);
    // Vehicle ID right-padded in 17-byte field
    for (size_t i = 1; i <= 13; i++) EXPECT_EQ(buf[i], 0);
    EXPECT_EQ(buf[14], TEST_VEHICLE_ID[0]);
    EXPECT_EQ(buf[15], TEST_VEHICLE_ID[1]);
    EXPECT_EQ(buf[16], TEST_VEHICLE_ID[2]);
    EXPECT_EQ(buf[17], TEST_VEHICLE_ID[3]);
    EXPECT_EQ(buf[18], TEST_CLASS_TYPE);
    EXPECT_EQ(buf[19], TEST_CLASS_LEVEL);
    EXPECT_EQ(buf[20], TEST_STRATEGY_NUM);
}

TEST(PriorityRequestProcessorTest, TestEncodePriorityClear) {
    auto cancelBuf = PriorityRequestProcessor::EncodePriorityCancel(
        TEST_REQUEST_ID, TEST_VEHICLE_ID, sizeof(TEST_VEHICLE_ID),
        TEST_CLASS_TYPE, TEST_CLASS_LEVEL, TEST_STRATEGY_NUM);
    auto clearBuf = PriorityRequestProcessor::EncodePriorityClear(
        TEST_REQUEST_ID, TEST_VEHICLE_ID, sizeof(TEST_VEHICLE_ID),
        TEST_CLASS_TYPE, TEST_CLASS_LEVEL, TEST_STRATEGY_NUM);

    // Clear is identical encoding to Cancel (21 bytes); different OID at call site
    EXPECT_EQ(clearBuf.size(), PRIORITY_CANCEL_SIZE);
    EXPECT_EQ(clearBuf, cancelBuf);
}

TEST(PriorityRequestProcessorTest, EncodePriorityCancelNullVehicleID) {
    auto buf = PriorityRequestProcessor::EncodePriorityCancel(
        9, nullptr, 0, 2, 2, 2);
    EXPECT_EQ(buf.size(), PRIORITY_CANCEL_SIZE);
    EXPECT_EQ(buf[0], 9);
    for (size_t i = 1; i <= 17; i++) EXPECT_EQ(buf[i], 0);
    EXPECT_EQ(buf[18], 2);
    EXPECT_EQ(buf[19], 2);
    EXPECT_EQ(buf[20], 2);
}

TEST(PriorityRequestProcessorTest, MapVehicleClassEmergencyGroup) {
    using P = std::pair<uint8_t, uint8_t>;
    EXPECT_EQ(MapVehicleClass(6),  (P{1, 1}));
    EXPECT_EQ(MapVehicleClass(12), (P{1, 2}));
    EXPECT_EQ(MapVehicleClass(13), (P{1, 3}));
    EXPECT_EQ(MapVehicleClass(14), (P{1, 4}));
    EXPECT_EQ(MapVehicleClass(5),  (P{1, 5}));
    EXPECT_EQ(MapVehicleClass(7),  (P{1, 6}));
    EXPECT_EQ(MapVehicleClass(11), (P{1, 7}));
}

TEST(PriorityRequestProcessorTest, MapVehicleClassTransitGroup) {
    using P = std::pair<uint8_t, uint8_t>;
    EXPECT_EQ(MapVehicleClass(1),  (P{3, 1}));
    EXPECT_EQ(MapVehicleClass(16), (P{3, 2}));
}

TEST(PriorityRequestProcessorTest, MapVehicleClassWorkGroup) {
    using P = std::pair<uint8_t, uint8_t>;
    EXPECT_EQ(MapVehicleClass(15), (P{5, 1}));
    EXPECT_EQ(MapVehicleClass(4),  (P{5, 2}));
}

TEST(PriorityRequestProcessorTest, MapVehicleClassTruckGroup) {
    using P = std::pair<uint8_t, uint8_t>;
    EXPECT_EQ(MapVehicleClass(3),  (P{7, 1}));
    EXPECT_EQ(MapVehicleClass(2),  (P{7, 2}));
    EXPECT_EQ(MapVehicleClass(9),  (P{7, 3}));
    EXPECT_EQ(MapVehicleClass(17), (P{7, 4}));
    EXPECT_EQ(MapVehicleClass(18), (P{7, 5}));
}

TEST(PriorityRequestProcessorTest, MapVehicleClassDefault) {
    using P = std::pair<uint8_t, uint8_t>;
    EXPECT_EQ(MapVehicleClass(0),   (P{10, 1}));
    EXPECT_EQ(MapVehicleClass(99),  (P{10, 1}));
    EXPECT_EQ(MapVehicleClass(-1),  (P{10, 1}));
}

TEST(PriorityRequestProcessorTest, LookupStrategyTest) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(9709, 1, 1); // intersectionID, lane, strategyNumber
    proc.SetLaneStrategy(9945, 2, 2);
    proc.SetLaneStrategy(106,  1, 3);
    EXPECT_EQ(proc.LookupStrategy(9709, 1).value(), 1);
    EXPECT_EQ(proc.LookupStrategy(9945, 2).value(), 2);
    EXPECT_EQ(proc.LookupStrategy(106, 1).value(), 3);
    EXPECT_FALSE(proc.LookupStrategy(100, 3).has_value());
    EXPECT_FALSE(proc.LookupStrategy(300, 1).has_value());
    // A negative lane never matches, even for a configured intersection
    EXPECT_FALSE(proc.LookupStrategy(9709, -1).has_value());
}

TEST(PriorityRequestProcessorTest, ClearLaneStrategyMap) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(1, 1, 42);
    EXPECT_TRUE(proc.LookupStrategy(1, 1).has_value());
    proc.ClearLaneStrategyMap();
    EXPECT_FALSE(proc.LookupStrategy(1, 1).has_value());
}

TEST(PriorityRequestProcessorTest, RunPrioritizationProcessingTTLReached) {
    // Test step (a)
    // TTL reached for now (1000), given timeToLive of 1000 and 900.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].timeToLive = 1000;
    table[0].requestID = 42;
    table[1].statusInPRS = RequestStatus::readyOverridden;
    table[1].timeToLive = 900;

    proc.RunPrioritizationProcessing(1000);

    const auto &t = proc.Table();
    EXPECT_EQ(t[0].statusInPRS, RequestStatus::idleNotValid);
    EXPECT_EQ(t[0].requestID, 0);
    EXPECT_EQ(t[1].statusInPRS, RequestStatus::idleNotValid);
    EXPECT_EQ(t[1].requestID, 0);
}

TEST(PriorityRequestProcessorTest, RunPrioritizationProcessingClosedTimeToLive) {
    // Test steps (a-b) with closedTimeToLiveError status.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].timeToLive = 500;
    table[0].timeOfServiceDesiredInPRS = 600;
    table[0].requestID = 7;

    proc.RunPrioritizationProcessing(100);

    bool found = false;
    for (const auto &e : proc.Table()) {
        if (e.requestID == 7) {
            EXPECT_EQ(e.statusInPRS, RequestStatus::closedTimeToLiveError);
            found = true;
        }
    }
    EXPECT_TRUE(found);
}

TEST(PriorityRequestProcessorTest, RunPrioritizationProcessingDoesNotOverwriteClosedX) {
    // Step (b) must not relabel an entry that is already in a closedX state.
    // A closedStrategyError with TSD > TTL should stay closedStrategyError.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].statusInPRS = RequestStatus::closedStrategyError;
    table[0].timeToLive = 500;
    table[0].timeOfServiceDesiredInPRS = 600;
    table[0].requestID = 11;

    proc.RunPrioritizationProcessing(100);

    bool found = false;
    for (const auto &e : proc.Table()) {
        if (e.requestID == 11) {
            EXPECT_EQ(e.statusInPRS, RequestStatus::closedStrategyError);
            found = true;
        }
    }
    EXPECT_TRUE(found);
}

TEST(PriorityRequestProcessorTest, RunPrioritizationProcessingReorderByPriority) {
    // Test step (c) reordering with three ready requests of different priority.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].vehicleClassType = 7;
    table[0].vehicleClassLevel = 1;
    table[0].timeOfServiceDesiredInPRS = 100;
    table[0].requestID = 1;

    table[1].statusInPRS = RequestStatus::readyQueued;
    table[1].vehicleClassType = 1;
    table[1].vehicleClassLevel = 1;
    table[1].timeOfServiceDesiredInPRS = 200;
    table[1].requestID = 2;

    table[2].statusInPRS = RequestStatus::readyQueued;
    table[2].vehicleClassType = 3;
    table[2].vehicleClassLevel = 2;
    table[2].timeOfServiceDesiredInPRS = 300;
    table[2].requestID = 3;

    proc.RunPrioritizationProcessing(50);

    const auto &t = proc.Table();
    EXPECT_EQ(t[0].requestID, 2);
    EXPECT_EQ(t[1].requestID, 3);
    EXPECT_EQ(t[2].requestID, 1);
}

TEST(PriorityRequestProcessorTest, RunPrioritizationProcessingHigherPriorityOverridesActive) {
    // Test step (c) with a higher priority request overriding an active request.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].statusInCO = RequestStatus::activeProcessing;
    table[0].vehicleClassType = 3;
    table[0].vehicleClassLevel = 2;
    table[0].requestID = 10;

    table[1].statusInPRS = RequestStatus::readyQueued;
    table[1].vehicleClassType = 1;
    table[1].vehicleClassLevel = 1;
    table[1].requestID = 20;

    proc.RunPrioritizationProcessing(50);

    const auto &t = proc.Table();
    EXPECT_EQ(t[0].statusInPRS, RequestStatus::activeOverride);
    EXPECT_EQ(t[0].statusInCO, RequestStatus::activeProcessing);
    EXPECT_EQ(t[1].statusInPRS, RequestStatus::readyQueued);
}

TEST(PriorityRequestProcessorTest, CheckForOverrideClearsWhenHigherPriorityWithdraws) {
    // CO is in activeX state in and PRS previously asserted activeOverride. 
    // If higher priority readyQueued entry goes away, CheckForOverride must
    // revert statusInPRS to readyQueued.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].statusInPRS = RequestStatus::activeOverride;
    table[0].statusInCO = RequestStatus::activeNotOverridden;
    table[0].vehicleClassType = 3;
    table[0].vehicleClassLevel = 2;
    table[0].requestID = 10;

    proc.RunPrioritizationProcessing(50);

    const auto &t = proc.Table();
    EXPECT_EQ(t[0].statusInPRS, RequestStatus::readyQueued);
    EXPECT_EQ(t[0].statusInCO, RequestStatus::activeNotOverridden);
}

TEST(PriorityRequestProcessorTest, CheckForOverrideClearsWhenOnlyLowerPriorityQueued) {
    // A lower priority readyQueued entry does not justify maintaining activeOverride 
    // on the CO's active entry. Only a higher priority readyQueued entry does.
    // So, activeOverride should clear in this scenario.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].statusInPRS = RequestStatus::activeOverride;
    table[0].statusInCO = RequestStatus::activeProcessing;
    table[0].vehicleClassType = 1;
    table[0].vehicleClassLevel = 1;
    table[0].requestID = 10;

    table[1].statusInPRS = RequestStatus::readyQueued;
    table[1].vehicleClassType = 7;
    table[1].vehicleClassLevel = 2;
    table[1].requestID = 20;

    proc.RunPrioritizationProcessing(50);

    const auto &t = proc.Table();
    EXPECT_EQ(t[0].statusInPRS, RequestStatus::readyQueued);
    EXPECT_EQ(t[0].statusInCO, RequestStatus::activeProcessing);
}

TEST(PriorityRequestProcessorTest, CheckForOverrideAcrossTicks) {
    // PRS asserts activeOverride when a higher priority readyQueued
    // is placed and a CO entry is active. PRS then reverts on a later tick once
    // that request is withdrawn. Guards against activeOverride getting stuck in
    // statusInPRS after the triggering condition clears.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].statusInCO = RequestStatus::activeProcessing;
    table[0].vehicleClassType = 3;
    table[0].vehicleClassLevel = 2;
    table[0].requestID = 10;

    table[1].statusInPRS = RequestStatus::readyQueued;
    table[1].vehicleClassType = 1;
    table[1].vehicleClassLevel = 1;
    table[1].requestID = 20;

    proc.RunPrioritizationProcessing(50);
    EXPECT_EQ(proc.Table()[0].statusInPRS, RequestStatus::activeOverride);

    // Higher priority request withdraws, CO remains active in the original entry.
    proc.Table()[1] = PriorityRequestEntry{};

    proc.RunPrioritizationProcessing(51);

    const auto &t = proc.Table();
    EXPECT_EQ(t[0].statusInPRS, RequestStatus::readyQueued);
    EXPECT_EQ(t[0].statusInCO, RequestStatus::activeProcessing);
    EXPECT_EQ(t[0].requestID, 10);
}

TEST(PriorityRequestProcessorTest, RunPrioritizationProcessingEqualPriorityDoesNotOverrideActive) {
    // Test step (c) with an equal priority request does not override an active request.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].statusInCO = RequestStatus::activeProcessing;
    table[0].vehicleClassType = 3;
    table[0].vehicleClassLevel = 2;
    table[1].statusInPRS = RequestStatus::readyQueued;
    table[1].vehicleClassType = 3;
    table[1].vehicleClassLevel = 2;

    proc.RunPrioritizationProcessing(50);

    EXPECT_EQ(proc.Table()[0].statusInPRS, RequestStatus::readyQueued);
    EXPECT_EQ(proc.Table()[0].statusInCO, RequestStatus::activeProcessing);
}

TEST(PriorityRequestProcessorTest, RunPrioritizationProcessingReorderTiesByTsd) {
    // Test step c (i) reordering with two ready requests of equal priority. Tie broken by time of service desired.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].vehicleClassType = 1;
    table[0].vehicleClassLevel = 1;
    table[0].timeOfServiceDesiredInPRS = 900;
    table[0].requestID = 1;

    table[1].statusInPRS = RequestStatus::readyQueued;
    table[1].vehicleClassType = 1;
    table[1].vehicleClassLevel = 1;
    table[1].timeOfServiceDesiredInPRS = 100;
    table[1].requestID = 2;

    proc.RunPrioritizationProcessing(50);

    const auto &t = proc.Table();
    EXPECT_EQ(t[0].requestID, 2);
    EXPECT_EQ(t[1].requestID, 1);
}

TEST(PriorityRequestProcessorTest, RunPrioritizationProcessingReordering) {
    // Test step c (ii-iv) reordering queue with a mix of ready, active, and closed requests.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].statusInPRS = RequestStatus::closedCompleted;
    table[0].requestID = 1;
    table[1].statusInPRS = RequestStatus::readyQueued;
    table[1].vehicleClassType = 1;
    table[1].vehicleClassLevel = 1;
    table[1].requestID = 2;
    table[2].statusInPRS = RequestStatus::readyOverridden;
    table[2].requestID = 3;

    proc.RunPrioritizationProcessing(50);

    const auto &t = proc.Table();
    EXPECT_EQ(t[0].requestID, 2);
    EXPECT_EQ(t[1].requestID, 3);
    EXPECT_EQ(t[2].requestID, 1);
    EXPECT_EQ(t[3].statusInPRS, RequestStatus::idleNotValid);
}

TEST(PriorityRequestProcessorTest, TestApplyCoStatusUpdates) {
    // CO activeX states are tracked in statusInCO only
    // statusInPRS stays readyX so that byte 9 of the next SET continues to carry a PRS-owned state.
    {
        PriorityRequestProcessor proc;
        auto &table = proc.Table();
        std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;

        table[0].statusInPRS = RequestStatus::readyQueued;
        coRows[0].requestStatusInCO = RequestStatus::activeProcessing;
        table[1].statusInPRS = RequestStatus::readyOverridden;
        coRows[1].requestStatusInCO = RequestStatus::activeAdjustNotNeeded;

        proc.ApplyCoStatusUpdates(coRows, 1000);

        EXPECT_EQ(proc.Table()[0].statusInPRS, RequestStatus::readyQueued);
        EXPECT_EQ(proc.Table()[0].statusInCO, RequestStatus::activeProcessing);
        EXPECT_EQ(proc.Table()[1].statusInPRS, RequestStatus::readyOverridden);
        EXPECT_EQ(proc.Table()[1].statusInCO, RequestStatus::activeAdjustNotNeeded);
    }

    // Ready to closed states
    {
        PriorityRequestProcessor proc;
        auto &table = proc.Table();
        std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;

        table[0].statusInPRS = RequestStatus::readyQueued;
        coRows[0].requestStatusInCO = RequestStatus::closedTimerError;
        table[1].statusInPRS = RequestStatus::readyQueued;
        coRows[1].requestStatusInCO = RequestStatus::closedStrategyError;
        table[2].statusInPRS = RequestStatus::readyQueued;
        coRows[2].requestStatusInCO = RequestStatus::closedFlash;

        proc.ApplyCoStatusUpdates(coRows, 1000);

        const auto &t = proc.Table();
        EXPECT_EQ(t[0].statusInPRS, RequestStatus::closedTimerError);
        EXPECT_EQ(t[1].statusInPRS, RequestStatus::closedStrategyError);
        EXPECT_EQ(t[2].statusInPRS, RequestStatus::closedFlash);
    }

    // closedCanceled accepted while PRS is in readyX
    {
        PriorityRequestProcessor proc;
        auto &table = proc.Table();
        std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;

        table[0].statusInPRS = RequestStatus::readyQueued;
        coRows[0].requestStatusInCO = RequestStatus::closedCanceled;
        table[1].statusInPRS = RequestStatus::readyOverridden;
        coRows[1].requestStatusInCO = RequestStatus::closedCanceled;

        proc.ApplyCoStatusUpdates(coRows, 1000);

        const auto &t = proc.Table();
        EXPECT_EQ(t[0].statusInPRS, RequestStatus::closedCanceled);
        EXPECT_EQ(t[1].statusInPRS, RequestStatus::closedCanceled);
    }

    // closedCompleted from readyQueued and reservice time stamped
    {
        PriorityRequestProcessor proc;
        auto &table = proc.Table();
        std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;

        table[0].statusInPRS = RequestStatus::readyQueued;
        table[0].statusInCO = RequestStatus::activeProcessing;
        table[0].vehicleClassType = 3;
        coRows[0].requestStatusInCO = RequestStatus::closedCompleted;

        proc.ApplyCoStatusUpdates(coRows, 42);

        EXPECT_EQ(proc.Table()[0].statusInPRS, RequestStatus::closedCompleted);
        EXPECT_EQ(proc.Table()[0].statusInCO, RequestStatus::closedCompleted);
        EXPECT_EQ(proc.ReserviceLastCompleted(3), 42u);
    }

    // closedCompleted with invalid classType clamps to last slot
    {
        PriorityRequestProcessor proc;
        auto &table = proc.Table();
        std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;

        table[0].statusInPRS = RequestStatus::readyQueued;
        table[0].vehicleClassType = 99;
        coRows[0].requestStatusInCO = RequestStatus::closedCompleted;

        proc.ApplyCoStatusUpdates(coRows, 555);

        EXPECT_EQ(proc.ReserviceLastCompleted(10), 555u);
    }

    // Given activeOverride, CO responds with readyOverridden or readyQueued. 
    // PRS accepts the override, but activeNotOverridden is tracked only in statusInCO.
    {
        PriorityRequestProcessor proc;
        auto &table = proc.Table();
        std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;

        table[0].statusInPRS = RequestStatus::activeOverride;
        coRows[0].requestStatusInCO = RequestStatus::activeNotOverridden;
        table[1].statusInPRS = RequestStatus::activeOverride;
        coRows[1].requestStatusInCO = RequestStatus::readyOverridden;
        table[2].statusInPRS = RequestStatus::activeOverride;
        coRows[2].requestStatusInCO = RequestStatus::readyQueued;
        table[3].statusInPRS = RequestStatus::readyQueued;
        coRows[3].requestStatusInCO = RequestStatus::readyQueued;

        proc.ApplyCoStatusUpdates(coRows, 1000);

        const auto &t = proc.Table();
        EXPECT_EQ(t[0].statusInPRS, RequestStatus::activeOverride);
        EXPECT_EQ(t[0].statusInCO, RequestStatus::activeNotOverridden);
        EXPECT_EQ(t[1].statusInPRS, RequestStatus::readyOverridden);
        EXPECT_EQ(t[2].statusInPRS, RequestStatus::readyQueued);
        EXPECT_EQ(t[3].statusInPRS, RequestStatus::readyQueued);
    }

    // activeCancel is CO-owned, tracked only in statusInCO
    {
        PriorityRequestProcessor proc;
        auto &table = proc.Table();
        std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;

        table[0].statusInPRS = RequestStatus::readyQueued;
        coRows[0].requestStatusInCO = RequestStatus::activeCancel;
        table[1].statusInPRS = RequestStatus::readyQueued;
        coRows[1].requestStatusInCO = RequestStatus::activeNotOverridden;

        proc.ApplyCoStatusUpdates(coRows, 1000);

        EXPECT_EQ(proc.Table()[0].statusInPRS, RequestStatus::readyQueued);
        EXPECT_EQ(proc.Table()[0].statusInCO, RequestStatus::activeCancel);
        EXPECT_EQ(proc.Table()[1].statusInPRS, RequestStatus::readyQueued);
        EXPECT_EQ(proc.Table()[1].statusInCO, RequestStatus::activeNotOverridden);
    }

    // closedCompleted accepted from activeOverride (PRS overrode, CO finished anyway)
    {
        PriorityRequestProcessor proc;
        auto &table = proc.Table();
        std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;

        table[0].statusInPRS = RequestStatus::activeOverride;
        table[0].vehicleClassType = 2;
        coRows[0].requestStatusInCO = RequestStatus::closedCompleted;

        proc.ApplyCoStatusUpdates(coRows, 77);

        EXPECT_EQ(proc.Table()[0].statusInPRS, RequestStatus::closedCompleted);
        EXPECT_EQ(proc.ReserviceLastCompleted(2), 77u);
    }

    // closedCompleted NOT accepted once the PRS already closed the entry
    {
        PriorityRequestProcessor proc;
        auto &table = proc.Table();
        std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;

        table[0].statusInPRS = RequestStatus::closedCanceled;
        coRows[0].requestStatusInCO = RequestStatus::closedCompleted;

        proc.ApplyCoStatusUpdates(coRows, 1000);

        EXPECT_EQ(proc.Table()[0].statusInPRS, RequestStatus::closedCanceled);
    }

    // PRS-owned CO statuses with no matching transition fall through the default case
    {
        PriorityRequestProcessor proc;
        auto &table = proc.Table();
        std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;

        table[0].statusInPRS = RequestStatus::readyQueued;
        coRows[0].requestStatusInCO = RequestStatus::reserviceError;
        table[1].statusInPRS = RequestStatus::readyQueued;
        coRows[1].requestStatusInCO = RequestStatus::idleNotValid;

        proc.ApplyCoStatusUpdates(coRows, 1000);

        EXPECT_EQ(proc.Table()[0].statusInPRS, RequestStatus::readyQueued);
        EXPECT_EQ(proc.Table()[0].statusInCO, RequestStatus::reserviceError);
        EXPECT_EQ(proc.Table()[1].statusInPRS, RequestStatus::readyQueued);
    }

    // Idle entries do nothing, stay at default idleNotValid state
    {
        PriorityRequestProcessor proc;
        std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;

        for (auto &row : coRows) row.requestStatusInCO = RequestStatus::activeProcessing;

        proc.ApplyCoStatusUpdates(coRows, 1000);

        for (const auto &e : proc.Table()) {
            EXPECT_EQ(e.statusInPRS, RequestStatus::idleNotValid);
            EXPECT_EQ(e.statusInCO, RequestStatus::idleNotValid);
        }
    }
}

TEST(PriorityRequestProcessorTest, EncodeServiceRequestChangesWhenCoClosesRequest) {
    // When the CO reports closedCompleted, ApplyCoStatusUpdates changes statusInPRS to closedCompleted. 
    // The re-encoded payload must differ from the previous SET so the PRS acknowledges closure to the CO.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].serviceStrategyNumber = 1;
    table[0].timeOfServiceDesiredInPRS = TEST_EPOCH_TSD;
    table[0].timeOfEstimatedDepartureInPRS = TEST_EPOCH_TED;
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].requestID = 42;

    auto before = proc.EncodeServiceRequest(false);

    std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows{};
    coRows[0].strategyRequested = 1;
    coRows[0].requestedTimeOfServiceDesired = TEST_EPOCH_TSD;
    coRows[0].requestedTimeOfEstimatedDeparture = TEST_EPOCH_TED;
    coRows[0].requestStatusInCO = RequestStatus::closedCompleted;
    proc.ApplyCoStatusUpdates(coRows, 1775846000);

    EXPECT_EQ(proc.Table()[0].statusInPRS, RequestStatus::closedCompleted);

    auto after = proc.EncodeServiceRequest(false);
    EXPECT_NE(before, after);
    EXPECT_EQ(after[9], static_cast<uint8_t>(RequestStatus::closedCompleted));
}

TEST(PriorityRequestProcessorTest, EncodeServiceRequestChangesWhenTtlExpires) {
    // When the TTL for a readyX/closedX entry expires, RunPrioritizationProcessing resets the row to idleNotValid. 
    // The re-encoded payload must differ from the prior SET so the exchange loop clears the row on the CO instead of
    // leaving a stale entry.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].serviceStrategyNumber = 1;
    table[0].timeOfServiceDesiredInPRS = TEST_EPOCH_TSD;
    table[0].timeOfEstimatedDepartureInPRS = TEST_EPOCH_TED;
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].requestID = 42;
    table[0].timeToLive = 1000;

    auto before = proc.EncodeServiceRequest(false);

    proc.RunPrioritizationProcessing(1001); // TTL < now

    EXPECT_EQ(proc.Table()[0].statusInPRS, RequestStatus::idleNotValid);

    auto after = proc.EncodeServiceRequest(false);
    EXPECT_NE(before, after);
    EXPECT_EQ(after[9], static_cast<uint8_t>(RequestStatus::idleNotValid));
}

TEST(PriorityRequestProcessorTest, EncodeServiceRequestUsesStatusInPrsNotStatusInCo) {
    // After the CO has reported activeProcessing, the next EncodeServiceRequest must still serialize
    // the PRS-owned statusInPRS (readyQueued) into byte 9, not the CO-owned statusInCO.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].serviceStrategyNumber = 1;
    table[0].statusInPRS = RequestStatus::readyQueued;

    std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows{};
    coRows[0].requestStatusInCO = RequestStatus::activeProcessing;
    proc.ApplyCoStatusUpdates(coRows, 1000);

    EXPECT_EQ(proc.Table()[0].statusInPRS, RequestStatus::readyQueued);
    EXPECT_EQ(proc.Table()[0].statusInCO, RequestStatus::activeProcessing);

    auto setPayload = proc.EncodeServiceRequest(false);
    EXPECT_EQ(setPayload[9], static_cast<uint8_t>(RequestStatus::readyQueued));
}
