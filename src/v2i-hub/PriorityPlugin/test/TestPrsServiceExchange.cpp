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
#include <gmock/gmock.h>
#include <MockSNMPClient.h>

#include "PrsServiceExchange.hpp"

#include <tsc/NTCIP_1211_MIB.h>

using namespace PriorityPlugin;
using namespace tmx::utils;
using testing::_;
using testing::DoAll;
using testing::Invoke;
using testing::Return;

namespace {
    const std::string SERVICE_OID = tsc::mib::ntcip1211::NTCIP1211_PRS_SERVICE_REQUEST_OID;
    constexpr long TEST_INTERSECTION_ID = 9709;

    std::shared_ptr<unit_test::mock_snmp_client> MakeMockClient() {
        return std::make_shared<unit_test::mock_snmp_client>("", 0, "", "", "", "");
    }

    struct ExchangeFixture {
        PriorityRequestProcessor processor;
        std::mutex tableMutex;
        std::vector<uint8_t> lastSent;
        bool prsBusy = false;
        std::atomic<bool> running{true};
        std::shared_ptr<fwha_stol::lib::time::CarmaClock> clock =
            std::make_shared<fwha_stol::lib::time::CarmaClock>(false);

        ServiceExchangeContext ctx{processor, tableMutex, lastSent, prsBusy};

        ExchangeResult Run(const std::shared_ptr<snmp_client> &client) {
            return DoOneServiceExchange(ctx, client, TEST_INTERSECTION_ID, clock, running);
        }
    };

    std::vector<char> CoResponse(bool coBusy, uint8_t row0Status = 0) {
        std::vector<char> data(SERVICE_REQUEST_SIZE, 0);
        data[9] = static_cast<char>(row0Status);
        data[SERVICE_REQUEST_BUSY_OFFSET] = coBusy ? 1 : 0;
        return data;
    }

    // gmock action: fill the GET response object with the given payload.
    auto RespondWith(const std::vector<char> &payload, bool success = true) {
        return Invoke([payload, success](const std::string &, const request_type &, snmp_response_obj &val) {
            val.val_string = payload;
            return success;
        });
    }

    // Assign one readyQueued row
    void ReadyRequest(PriorityRequestProcessor &processor) {
        auto &entry = processor.Table()[0];
        entry.requestID = 7;
        entry.serviceStrategyNumber = 3;
        entry.timeOfServiceDesiredInPRS = 2000;
        entry.timeOfEstimatedDepartureInPRS = 2010;
        entry.intersectionID = TEST_INTERSECTION_ID;
        entry.statusInPRS = RequestStatus::readyQueued;
    }
} // namespace

TEST(SnmpHelpersTest, NullClientFails) {
    std::vector<uint8_t> data{1, 2, 3};
    EXPECT_FALSE(SnmpSet(nullptr, SERVICE_OID, data));
    EXPECT_FALSE(SnmpGet(nullptr, SERVICE_OID, data));
}

TEST(SnmpHelpersTest, SetSendsPayload) {
    auto mock = MakeMockClient();
    std::vector<char> captured;
    EXPECT_CALL(*mock, process_snmp_request(SERVICE_OID, request_type::SET, _))
        .WillOnce(Invoke([&captured](const std::string &, const request_type &, const snmp_response_obj &val) {
            captured = val.val_string;
            return true;
        }));

    std::vector<uint8_t> data{0x01, 0x02, 0xFF};
    EXPECT_TRUE(SnmpSet(mock, SERVICE_OID, data));
    EXPECT_EQ(captured, (std::vector<char>{0x01, 0x02, '\xFF'}));
}

TEST(SnmpHelpersTest, SetFailureReturnsFalse) {
    auto mock = MakeMockClient();
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::SET, _)).WillOnce(Return(false));
    std::vector<uint8_t> data{1};
    EXPECT_FALSE(SnmpSet(mock, SERVICE_OID, data));
}

TEST(SnmpHelpersTest, GetReturnsResponseBytes) {
    auto mock = MakeMockClient();
    EXPECT_CALL(*mock, process_snmp_request(SERVICE_OID, request_type::GET, _))
        .WillOnce(RespondWith({0x0A, 0x0B}));

    std::vector<uint8_t> data;
    EXPECT_TRUE(SnmpGet(mock, SERVICE_OID, data));
    EXPECT_EQ(data, (std::vector<uint8_t>{0x0A, 0x0B}));
}

TEST(SnmpHelpersTest, GetFailureReturnsFalse) {
    auto mock = MakeMockClient();
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::GET, _)).WillOnce(Return(false));
    std::vector<uint8_t> data;
    EXPECT_FALSE(SnmpGet(mock, SERVICE_OID, data));
}

TEST(DoOneServiceExchangeTest, InitialSetFailure) {
    ExchangeFixture fix;
    ReadyRequest(fix.processor);
    auto mock = MakeMockClient();
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::SET, _)).WillOnce(Return(false));

    EXPECT_EQ(fix.Run(mock), ExchangeResult::SnmpSetFailed);
    EXPECT_TRUE(fix.lastSent.empty()); // A failed SET is not saved as the CO's state
}

TEST(DoOneServiceExchangeTest, GetFailureIsCoError) {
    ExchangeFixture fix;
    ReadyRequest(fix.processor);
    auto mock = MakeMockClient();
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::SET, _)).WillOnce(Return(true));
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::GET, _)).WillOnce(Return(false));

    EXPECT_EQ(fix.Run(mock), ExchangeResult::CoError);
}

TEST(DoOneServiceExchangeTest, UndecodableResponseIsCoError) {
    ExchangeFixture fix;
    ReadyRequest(fix.processor);
    auto mock = MakeMockClient();
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::SET, _)).WillOnce(Return(true));
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::GET, _))
        .WillOnce(RespondWith(std::vector<char>(10, 0))); // Response shorter than SERVICE_REQUEST_SIZE

    EXPECT_EQ(fix.Run(mock), ExchangeResult::CoError);
}

TEST(DoOneServiceExchangeTest, CoBusyExhaustsRetries) {
    ExchangeFixture fix;
    ReadyRequest(fix.processor);
    auto mock = MakeMockClient();
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::SET, _)).WillOnce(Return(true));
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::GET, _))
        .Times(3)   // CO stays busy for 3 retries
        .WillRepeatedly(RespondWith(CoResponse(true)));

    EXPECT_EQ(fix.Run(mock), ExchangeResult::CoStillBusy);
}

TEST(DoOneServiceExchangeTest, ClearedRunningFlagSkipsPolling) {
    ExchangeFixture fix;
    ReadyRequest(fix.processor);
    fix.running = false;
    auto mock = MakeMockClient();
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::SET, _)).WillOnce(Return(true));
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::GET, _)).Times(0);

    // GET poll never runs, so the CO is still considered busy
    EXPECT_EQ(fix.Run(mock), ExchangeResult::CoStillBusy);
}

TEST(DoOneServiceExchangeTest, CompletedExchangeAcksClosure) {
    ExchangeFixture fix;
    ReadyRequest(fix.processor);
    auto mock = MakeMockClient();

    // Initial SET and GET reports row 0 closedCompleted and CO not busy
    // Prioritization saves the row as closedCompleted and the table is SET again to acknowledge the closure
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::SET, _))
        .Times(2)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::GET, _))
        .WillOnce(RespondWith(CoResponse(false, static_cast<uint8_t>(RequestStatus::closedCompleted))));

    EXPECT_EQ(fix.Run(mock), ExchangeResult::Ok);
    EXPECT_FALSE(fix.prsBusy);
    EXPECT_EQ(fix.processor.Table()[0].statusInPRS, RequestStatus::closedCompleted);
    // Post-prioritization payload is remembered for the next iteration's dedup
    EXPECT_EQ(fix.lastSent, fix.processor.EncodeServiceRequest(false));
}

TEST(DoOneServiceExchangeTest, PostPrioritizationSetFailure) {
    ExchangeFixture fix;
    ReadyRequest(fix.processor);
    auto mock = MakeMockClient();

    EXPECT_CALL(*mock, process_snmp_request(_, request_type::SET, _))
        .WillOnce(Return(true))
        .WillOnce(Return(false)); // mock failure after prioritization
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::GET, _))
        .WillOnce(RespondWith(CoResponse(false, static_cast<uint8_t>(RequestStatus::closedCompleted))));

    EXPECT_EQ(fix.Run(mock), ExchangeResult::SnmpSetFailed);
}

TEST(DoOneServiceExchangeTest, UnchangedTableSkipsBothSets) {
    ExchangeFixture fix;
    // No initial SET
    // CO response all-idle means no post-prioritization SET
    fix.lastSent = fix.processor.EncodeServiceRequest(false);
    auto mock = MakeMockClient();

    EXPECT_CALL(*mock, process_snmp_request(_, request_type::SET, _)).Times(0);
    EXPECT_CALL(*mock, process_snmp_request(_, request_type::GET, _))
        .WillOnce(RespondWith(CoResponse(false)));

    EXPECT_EQ(fix.Run(mock), ExchangeResult::Ok);
}
