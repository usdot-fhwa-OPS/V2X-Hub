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
#include <algorithm>
#include <boost/property_tree/json_parser.hpp>
#include "PriorityConfiguration.hpp"
#include "PriorityRequestProcessor.hpp"

using namespace PriorityPlugin;

TEST(PriorityTypesTest, RequestStatusValues) {
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::idleNotValid),          1);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::readyQueued),           2);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::readyOverridden),       3);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::activeProcessing),      4);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::activeCancel),          5);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::activeOverride),        6);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::activeNotOverridden),   7);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::closedCanceled),        8);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::reserviceError),        9);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::closedTimeToLiveError), 10);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::closedTimerError),      11);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::closedStrategyError),   12);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::closedCompleted),       13);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::activeAdjustNotNeeded), 14);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::closedFlash),           15);
}

TEST(PriorityTypesTest, PriorityRequestEntries) {
    PriorityRequestEntry entry;
    EXPECT_EQ(entry.vehicleClassType, 10);
    EXPECT_EQ(entry.vehicleClassLevel, 1);
    EXPECT_EQ(entry.serviceStrategyNumber, 0);
    EXPECT_EQ(entry.timeOfServiceDesiredInPRS, 0u);
    EXPECT_EQ(entry.timeOfEstimatedDepartureInPRS, 0u);
    EXPECT_EQ(entry.statusInPRS, RequestStatus::idleNotValid);
    EXPECT_EQ(entry.ssmBroadcastCount, 0);
    EXPECT_EQ(entry.ssmLastStatus, RequestStatus::idleNotValid);
}

TEST(PriorityTypesTest, ConstantsTest) {
    EXPECT_EQ(SERVICE_REQUEST_SIZE, 110u);
    EXPECT_EQ(MAX_SERVICE_REQUESTS, 10u);
    EXPECT_EQ(SERVICE_REQUEST_ROW_SIZE, 10u);
    EXPECT_EQ(SERVICE_REQUEST_BUSY_OFFSET, 100u);
    EXPECT_EQ(MAX_SERVICE_REQUESTS * SERVICE_REQUEST_ROW_SIZE + 1 + 9, SERVICE_REQUEST_SIZE);
}

TEST(PriorityTypesTest, IsReadyXtest) {
    EXPECT_TRUE(IsReadyX(RequestStatus::readyQueued));
    EXPECT_TRUE(IsReadyX(RequestStatus::readyOverridden));
    EXPECT_FALSE(IsReadyX(RequestStatus::idleNotValid));
    EXPECT_FALSE(IsReadyX(RequestStatus::activeProcessing));
    EXPECT_FALSE(IsReadyX(RequestStatus::closedCompleted));
}

TEST(PriorityTypesTest, IsActiveXtest) {
    EXPECT_TRUE(IsActiveX(RequestStatus::activeProcessing));
    EXPECT_TRUE(IsActiveX(RequestStatus::activeCancel));
    EXPECT_TRUE(IsActiveX(RequestStatus::activeOverride));
    EXPECT_TRUE(IsActiveX(RequestStatus::activeNotOverridden));
    EXPECT_TRUE(IsActiveX(RequestStatus::activeAdjustNotNeeded));
    EXPECT_FALSE(IsActiveX(RequestStatus::readyQueued));
    EXPECT_FALSE(IsActiveX(RequestStatus::closedCompleted));
    EXPECT_FALSE(IsActiveX(RequestStatus::idleNotValid));
}

TEST(PriorityTypesTest, IsClosedXtest) {
    EXPECT_TRUE(IsClosedX(RequestStatus::closedCanceled));
    EXPECT_TRUE(IsClosedX(RequestStatus::closedTimeToLiveError));
    EXPECT_TRUE(IsClosedX(RequestStatus::closedTimerError));
    EXPECT_TRUE(IsClosedX(RequestStatus::closedStrategyError));
    EXPECT_TRUE(IsClosedX(RequestStatus::closedCompleted));
    EXPECT_TRUE(IsClosedX(RequestStatus::closedFlash));
    EXPECT_FALSE(IsClosedX(RequestStatus::reserviceError));
    EXPECT_FALSE(IsClosedX(RequestStatus::readyQueued));
    EXPECT_FALSE(IsClosedX(RequestStatus::activeProcessing));
    EXPECT_FALSE(IsClosedX(RequestStatus::idleNotValid));
}

TEST(PriorityRequestProcessorTest, TestEncodeServiceRequest) {
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].serviceStrategyNumber = 1;
    table[0].timeOfServiceDesiredInPRS = 0x12345678;
    table[0].timeOfEstimatedDepartureInPRS = 0x12345681;
    table[0].statusInPRS = RequestStatus::readyQueued;

    table[1].serviceStrategyNumber = 2;
    table[1].timeOfServiceDesiredInPRS = 0x00000001;
    table[1].timeOfEstimatedDepartureInPRS = 0x00000002;
    table[1].statusInPRS = RequestStatus::activeProcessing;

    auto buf = proc.EncodeServiceRequest(false);
    EXPECT_EQ(buf.size(), 110u);

    EXPECT_EQ(buf[0], 1);
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
    EXPECT_EQ(buf[100], 0); // prsBusy=false

    for (size_t i = 101; i < 110; i++) {
        EXPECT_EQ(buf[i], 0) << "Reserved byte " << i;
    }
}

TEST(PriorityRequestProcessorTest, PrsBusyFlag) {
    PriorityRequestProcessor proc;
    EXPECT_EQ(proc.EncodeServiceRequest(false)[100], 0);
    EXPECT_EQ(proc.EncodeServiceRequest(true)[100], 1);
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
    data[100] = 1;
    data[101] = 0;

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
    table[0].timeOfServiceDesiredInPRS = 1775846010;
    table[0].timeOfEstimatedDepartureInPRS = 1775846013;
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[1].serviceStrategyNumber = 2;
    table[1].timeOfServiceDesiredInPRS = 1775846020;
    table[1].timeOfEstimatedDepartureInPRS = 1775846025;
    table[1].statusInPRS = RequestStatus::activeProcessing;

    auto buf = proc.EncodeServiceRequest(true);
    std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> decoded;
    bool coBusy = false;
    ASSERT_TRUE(PriorityRequestProcessor::DecodeCoServiceResponse(buf, decoded, coBusy));

    EXPECT_EQ(decoded[0].strategyRequested, 1);
    EXPECT_EQ(decoded[0].requestedTimeOfServiceDesired, 1775846010u);
    EXPECT_EQ(decoded[0].requestedTimeOfEstimatedDeparture, 1775846013u);
    EXPECT_EQ(decoded[0].requestStatusInCO, RequestStatus::readyQueued);
    EXPECT_EQ(decoded[1].strategyRequested, 2);
    EXPECT_EQ(decoded[1].requestedTimeOfServiceDesired, 1775846020u);
    EXPECT_EQ(decoded[1].requestedTimeOfEstimatedDeparture, 1775846025u);
    EXPECT_EQ(decoded[1].requestStatusInCO, RequestStatus::activeProcessing);
    EXPECT_TRUE(coBusy);
}

TEST(PriorityRequestProcessorTest, AllTenRowsFillBuffer) {
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    for (size_t i = 0; i < MAX_SERVICE_REQUESTS; i++) {
        table[i].serviceStrategyNumber = static_cast<uint8_t>(i + 1);
        table[i].timeOfServiceDesiredInPRS = 10 * (i + 1);
        table[i].timeOfEstimatedDepartureInPRS = 20 * (i + 1);
        table[i].statusInPRS = RequestStatus::readyQueued;
    }
    auto buf = proc.EncodeServiceRequest(false);
    for (size_t i = 0; i < MAX_SERVICE_REQUESTS; i++) {
        EXPECT_EQ(buf[i * SERVICE_REQUEST_ROW_SIZE], i + 1) << "Row " << i;
        EXPECT_EQ(buf[i * SERVICE_REQUEST_ROW_SIZE + 9], 2) << "Row " << i << " status";
    }
}

TEST(PriorityRequestProcessorTest, PriorityRequestSizeConstants) {
    EXPECT_EQ(PRIORITY_REQUEST_SIZE, 29u);
    EXPECT_EQ(VEHICLE_ID_FIELD_SIZE, 17u);
}

TEST(PriorityRequestProcessorTest, TestEncodePriorityRequest) {
    uint8_t vehId[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    auto buf = PriorityRequestProcessor::EncodePriorityRequest(
        /*requestID*/   0x11,
        vehId, sizeof(vehId),
        /*classType*/   1,
        /*classLevel*/  3,
        /*strategyNum*/ 7,
        /*TOS*/         0x1234,
        /*TOD*/         0x5678,
        /*TOR*/         0x12345678);

    ASSERT_EQ(buf.size(), PRIORITY_REQUEST_SIZE);
    EXPECT_EQ(buf[0], 0x11);
    for (size_t i = 1; i <= 13; i++) EXPECT_EQ(buf[i], 0); // left-padding; assumed to be vehicle VIN in 1211, but we use SRM vehicle ID, which is shorter
    EXPECT_EQ(buf[14], 0xAA);
    EXPECT_EQ(buf[15], 0xBB);
    EXPECT_EQ(buf[16], 0xCC);
    EXPECT_EQ(buf[17], 0xDD);
    EXPECT_EQ(buf[18], 1);
    EXPECT_EQ(buf[19], 3);
    EXPECT_EQ(buf[20], 7);
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
    uint8_t vehId[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    auto buf = PriorityRequestProcessor::EncodePriorityUpdate(
        /*requestID*/   0x11,
        vehId, sizeof(vehId),
        /*classType*/   1,
        /*classLevel*/  3,
        /*strategyNum*/ 7,
        /*TOS*/         0x1234,
        /*TOD*/         0x5678,
        /*TOR*/         0x12345678);

    // Update is identical to Request encoding (29 bytes, same layout)
    ASSERT_EQ(buf.size(), PRIORITY_REQUEST_SIZE);
    auto reqBuf = PriorityRequestProcessor::EncodePriorityRequest(
        0x11, vehId, sizeof(vehId), 1, 3, 7, 0x1234, 0x5678, 0x12345678);
    EXPECT_EQ(buf, reqBuf);
}

TEST(PriorityRequestProcessorTest, TestEncodePriorityCancel) {
    uint8_t vehId[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    auto buf = PriorityRequestProcessor::EncodePriorityCancel(
        /*requestID*/   0x11,
        vehId, sizeof(vehId),
        /*classType*/   1,
        /*classLevel*/  3,
        /*strategyNum*/ 7);

    ASSERT_EQ(buf.size(), PRIORITY_CANCEL_SIZE);
    EXPECT_EQ(buf[0], 0x11);
    // Vehicle ID right-padded in 17-byte field
    for (size_t i = 1; i <= 13; i++) EXPECT_EQ(buf[i], 0);
    EXPECT_EQ(buf[14], 0xAA);
    EXPECT_EQ(buf[15], 0xBB);
    EXPECT_EQ(buf[16], 0xCC);
    EXPECT_EQ(buf[17], 0xDD);
    EXPECT_EQ(buf[18], 1);  // classType
    EXPECT_EQ(buf[19], 3);  // classLevel
    EXPECT_EQ(buf[20], 7);  // strategyNum
}

TEST(PriorityRequestProcessorTest, TestEncodePriorityClear) {
    uint8_t vehId[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    auto cancelBuf = PriorityRequestProcessor::EncodePriorityCancel(
        0x11, vehId, sizeof(vehId), 1, 3, 7);
    auto clearBuf = PriorityRequestProcessor::EncodePriorityClear(
        0x11, vehId, sizeof(vehId), 1, 3, 7);

    // Clear is identical encoding to Cancel (21 bytes); different OID at call site
    ASSERT_EQ(clearBuf.size(), PRIORITY_CANCEL_SIZE);
    EXPECT_EQ(clearBuf, cancelBuf);
}

TEST(PriorityRequestProcessorTest, EncodePriorityCancelNullVehicleID) {
    auto buf = PriorityRequestProcessor::EncodePriorityCancel(
        9, nullptr, 0, 2, 2, 2);
    ASSERT_EQ(buf.size(), PRIORITY_CANCEL_SIZE);
    EXPECT_EQ(buf[0], 9);
    for (size_t i = 1; i <= 17; i++) EXPECT_EQ(buf[i], 0);
    EXPECT_EQ(buf[18], 2);
    EXPECT_EQ(buf[19], 2);
    EXPECT_EQ(buf[20], 2);
}

TEST(PriorityRequestProcessorTest, CancelSizeConstants) {
    EXPECT_EQ(PRIORITY_CANCEL_SIZE, 21u);
}

TEST(PriorityRequestProcessorTest, MapVehicleClassEmergencyGroup) {
    using P = std::pair<uint8_t, uint8_t>;
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(6),  (P{1, 1}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(12), (P{1, 2}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(13), (P{1, 3}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(14), (P{1, 4}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(5),  (P{1, 5}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(7),  (P{1, 6}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(11), (P{1, 7}));
}

TEST(PriorityRequestProcessorTest, MapVehicleClassTransitGroup) {
    using P = std::pair<uint8_t, uint8_t>;
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(1),  (P{3, 1}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(16), (P{3, 2}));
}

TEST(PriorityRequestProcessorTest, MapVehicleClassWorkGroup) {
    using P = std::pair<uint8_t, uint8_t>;
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(15), (P{5, 1}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(4),  (P{5, 2}));
}

TEST(PriorityRequestProcessorTest, MapVehicleClassTruckGroup) {
    using P = std::pair<uint8_t, uint8_t>;
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(3),  (P{7, 1}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(9),  (P{7, 2}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(17), (P{7, 3}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(18), (P{7, 4}));
}

TEST(PriorityRequestProcessorTest, MapVehicleClassDefault) {
    using P = std::pair<uint8_t, uint8_t>;
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(0),   (P{10, 1}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(2),   (P{10, 1}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(99),  (P{10, 1}));
    EXPECT_EQ(PriorityRequestProcessor::MapVehicleClass(-1),  (P{10, 1}));
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
}

TEST(PriorityRequestProcessorTest, LookupStrategyNegativeLaneTest) {
    PriorityRequestProcessor proc;
    proc.SetLaneStrategy(100, 1, 5); // intersectionID, lane, strategyNumber
    EXPECT_FALSE(proc.LookupStrategy(100, -1).has_value());
}

TEST(PriorityRequestProcessorTest, LookupStrategyEmptyMap) {
    PriorityRequestProcessor proc;
    EXPECT_FALSE(proc.LookupStrategy(100, 1).has_value());
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

TEST(PriorityRequestProcessorTest, EncodeServiceRequestIdenticalState) {
    // The exchange loop gates to suppress redundant SETs and keeps a CO from 
    // reprocessing a closed request. Check that two encodes of the same table 
    // state produce an identical output.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].serviceStrategyNumber = 1;
    table[0].timeOfServiceDesiredInPRS = 1775846010;
    table[0].timeOfEstimatedDepartureInPRS = 1775846020;
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].requestID = 42;

    auto first = proc.EncodeServiceRequest(false);
    auto second = proc.EncodeServiceRequest(false);
    EXPECT_EQ(first, second);
}

TEST(PriorityRequestProcessorTest, EncodeServiceRequestChangesWhenCoClosesRequest) {
    // When the CO reports closedCompleted, ApplyCoStatusUpdates changes statusInPRS to closedCompleted. 
    // The re-encoded payload must differ from the previous SET so the PRS acknowledges closure to the CO.
    PriorityRequestProcessor proc;
    auto &table = proc.Table();
    table[0].serviceStrategyNumber = 1;
    table[0].timeOfServiceDesiredInPRS = 1775846010;
    table[0].timeOfEstimatedDepartureInPRS = 1775846020;
    table[0].statusInPRS = RequestStatus::readyQueued;
    table[0].requestID = 42;

    auto before = proc.EncodeServiceRequest(false);

    std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows{};
    coRows[0].strategyRequested = 1;
    coRows[0].requestedTimeOfServiceDesired = 1775846010;
    coRows[0].requestedTimeOfEstimatedDeparture = 1775846020;
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
    table[0].timeOfServiceDesiredInPRS = 1775846010;
    table[0].timeOfEstimatedDepartureInPRS = 1775846020;
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

TEST(PriorityConfigurationTest, ParseTscConfigurationList) {
    const std::string json = R"([{"IntersectionID": 9709, "IP": "192.168.55.92", "Port": 161}])";
    auto configs = parseTscConfigurationList(json);
    ASSERT_EQ(configs.size(), 1u);
    EXPECT_EQ(configs[0].intersectionID, 9709);
    EXPECT_EQ(configs[0].ip, "192.168.55.92");
    EXPECT_EQ(configs[0].port, 161);
}

TEST(PriorityConfigurationTest, ParseTscConfigurationListEmptyStringArray) {
    auto configs = parseTscConfigurationList("");
    EXPECT_TRUE(configs.empty());
    configs = parseTscConfigurationList("[]");
    EXPECT_TRUE(configs.empty());
}

TEST(PriorityConfigurationTest, ParseTscConfigurationListMultipleEntries) {
    const std::string json = R"([
        {"IntersectionID": 9709, "IP": "192.168.55.91", "Port": 161},
        {"IntersectionID": 9945, "IP": "192.168.55.92", "Port": 162}
    ])";
    auto configs = parseTscConfigurationList(json);
    ASSERT_EQ(configs.size(), 2u);
    EXPECT_EQ(configs[0].intersectionID, 9709);
    EXPECT_EQ(configs[0].ip, "192.168.55.91");
    EXPECT_EQ(configs[0].port, 161);
    EXPECT_EQ(configs[1].intersectionID, 9945);
    EXPECT_EQ(configs[1].ip, "192.168.55.92");
    EXPECT_EQ(configs[1].port, 162);
}

TEST(PriorityConfigurationTest, ParseTscConfigurationListMalformedJson) {
    EXPECT_THROW(parseTscConfigurationList("not json"),
                 boost::property_tree::json_parser_error);
    EXPECT_THROW(parseTscConfigurationList(R"([{"IntersectionID": 1,)"),
                 boost::property_tree::json_parser_error);
}

TEST(PriorityConfigurationTest, ParseTscConfigurationListMissingField) {
    // IP missing
    const std::string json = R"([{"IntersectionID": 1, "Port": 161}])";
    EXPECT_THROW(parseTscConfigurationList(json), boost::property_tree::ptree_error);
}

TEST(PriorityConfigurationTest, ParseLaneStrategyMapping) {
    const std::string json = R"([{"IntersectionID": 9709, "Lane": 2, "Strategy": 1}])";
    auto entries = parseLaneStrategyMapping(json);
    ASSERT_EQ(entries.size(), 1u);
    EXPECT_EQ(entries[0].intersectionID, 9709);
    EXPECT_EQ(entries[0].lane, 2);
    EXPECT_EQ(entries[0].strategy, 1);
}

TEST(PriorityConfigurationTest, ParseLaneStrategyMappingEmptyString) {
    auto entries = parseLaneStrategyMapping("");
    EXPECT_TRUE(entries.empty());
}

TEST(PriorityConfigurationTest, ParseLaneStrategyMappingMultipleEntries) {
    const std::string json = R"([
        {"IntersectionID": 101, "Lane": 3, "Strategy": 1},
        {"IntersectionID": 102, "Lane": 5, "Strategy": 2}
    ])";
    auto entries = parseLaneStrategyMapping(json);
    ASSERT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0].intersectionID, 101);
    EXPECT_EQ(entries[0].lane, 3);
    EXPECT_EQ(entries[0].strategy, 1);
    EXPECT_EQ(entries[1].intersectionID, 102);
    EXPECT_EQ(entries[1].lane, 5);
    EXPECT_EQ(entries[1].strategy, 2);
}

TEST(PriorityConfigurationTest, ParseLaneStrategyMappingStrategyBoundaries) {
    // Strategy values at the edges of the [1, 255] range are accepted.
    const std::string json = R"([
        {"IntersectionID": 1, "Lane": 1, "Strategy": 1},
        {"IntersectionID": 2, "Lane": 2, "Strategy": 255}
    ])";
    auto entries = parseLaneStrategyMapping(json);
    ASSERT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0].strategy, 1);
    EXPECT_EQ(entries[1].strategy, 255);
}

TEST(PriorityConfigurationTest, ParseLaneStrategyMappingStrategyOutOfRangeSkipped) {
    // Entries with Strategy outside [1, 255] are skipped.
    const std::string json = R"([
        {"IntersectionID": 1, "Lane": 1, "Strategy": 0},
        {"IntersectionID": 2, "Lane": 2, "Strategy": 50},
        {"IntersectionID": 3, "Lane": 3, "Strategy": 256},
        {"IntersectionID": 4, "Lane": 4, "Strategy": -1}
    ])";
    auto entries = parseLaneStrategyMapping(json);
    ASSERT_EQ(entries.size(), 1u);
    EXPECT_EQ(entries[0].intersectionID, 2);
    EXPECT_EQ(entries[0].lane, 2);
    EXPECT_EQ(entries[0].strategy, 50);
}

TEST(PriorityConfigurationTest, ParseLaneStrategyMappingMalformedJsonThrows) {
    EXPECT_THROW(parseLaneStrategyMapping("{not json"),
                 boost::property_tree::json_parser_error);
}

TEST(PriorityConfigurationTest, ParseLaneStrategyMappingMissingFieldThrows) {
    // Strategy missing - ptree throws on node.get<>()
    const std::string json = R"([{"IntersectionID": 1, "Lane": 2}])";
    EXPECT_THROW(parseLaneStrategyMapping(json), boost::property_tree::ptree_error);
}

TEST(PriorityConfigurationTest, ParseReserviceClassTimes) {
    auto result = parseReserviceClassTimes("0,0,0,0,0,0,0,0,0,0");
    for (size_t i = 0; i < ReserviceClassTimesSize; i++) {
        EXPECT_EQ(result[i], 0u) << "index " << i;
    }
}

TEST(PriorityConfigurationTest, ParseReserviceClassTimesEmptyString) {
    auto result = parseReserviceClassTimes("");
    for (size_t i = 0; i < ReserviceClassTimesSize; i++) {
        EXPECT_EQ(result[i], 0u) << "index " << i;
    }
}

TEST(PriorityConfigurationTest, ParseReserviceClassTimesAllSlotsPopulated) {
    auto result = parseReserviceClassTimes("10,20,30,40,50,60,70,80,90,100");
    EXPECT_EQ(result[0], 10u);
    EXPECT_EQ(result[1], 20u);
    EXPECT_EQ(result[2], 30u);
    EXPECT_EQ(result[3], 40u);
    EXPECT_EQ(result[4], 50u);
    EXPECT_EQ(result[5], 60u);
    EXPECT_EQ(result[6], 70u);
    EXPECT_EQ(result[7], 80u);
    EXPECT_EQ(result[8], 90u);
    EXPECT_EQ(result[9], 100u);
}

TEST(PriorityConfigurationTest, ParseReserviceClassTimesPartialFillsRemainingZeros) {
    auto result = parseReserviceClassTimes("5,15,25");
    EXPECT_EQ(result[0], 5u);
    EXPECT_EQ(result[1], 15u);
    EXPECT_EQ(result[2], 25u);
    for (size_t i = 3; i < ReserviceClassTimesSize; i++) {
        EXPECT_EQ(result[i], 0u) << "index " << i;
    }
}

TEST(PriorityConfigurationTest, ParseReserviceClassTimesIgnoresPastTen) {
    // 12 values; only first 10 should be read.
    auto result = parseReserviceClassTimes("1,2,3,4,5,6,7,8,9,10,11,12");
    EXPECT_EQ(result[0], 1u);
    EXPECT_EQ(result[9], 10u);
}

TEST(PriorityConfigurationTest, ParseReserviceClassTimesInvalidValue) {
    // Logs a warning and leaves index 1 at 0; Surrounding values still parse.
    auto result = parseReserviceClassTimes("7,abc,42");
    EXPECT_EQ(result[0], 7u);
    EXPECT_EQ(result[1], 0u);
    EXPECT_EQ(result[2], 42u);
    for (size_t i = 3; i < ReserviceClassTimesSize; i++) {
        EXPECT_EQ(result[i], 0u) << "index " << i;
    }
}

TEST(PriorityConfigurationTest, ParseReserviceClassTimesEmptyValue) {
    // Trailing comma produces an empty token at idx 2; std::stoul throws invalid_argument.
    auto result = parseReserviceClassTimes("1,2,,4");
    EXPECT_EQ(result[0], 1u);
    EXPECT_EQ(result[1], 2u);
    EXPECT_EQ(result[2], 0u);
    EXPECT_EQ(result[3], 4u);
}
