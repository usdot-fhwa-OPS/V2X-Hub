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
}

TEST(PriorityTypesTest, ConstantsTest) {
    EXPECT_EQ(SERVICE_REQUEST_SIZE, 110u);
    EXPECT_EQ(MAX_SERVICE_REQUESTS, 10u);
    EXPECT_EQ(SERVICE_REQUEST_ROW_SIZE, 10u);
    EXPECT_EQ(SERVICE_REQUEST_BUSY_OFFSET, 100u);
    EXPECT_EQ(MAX_SERVICE_REQUESTS * SERVICE_REQUEST_ROW_SIZE + 1 + 9, SERVICE_REQUEST_SIZE);
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
