
#include "MockSNMPClient.h"
#include "gtest/gtest.h"
#include "RSU_MIB_4_1.h"

using namespace tmx::utils;
using namespace std;
using namespace tmx::utils::rsu41::mib::oid;
using testing::_;
using testing::Action;
using testing::DoDefault;
using testing::Return;
using testing::SetArgReferee;
using testing::Throw;

namespace unit_test
{
    class test_SNMPClient : public ::testing::Test
    {
    public:
        shared_ptr<mock_snmp_client> scPtr;
        uint16_t port = 161;
        test_SNMPClient()
        {
            scPtr = make_shared<mock_snmp_client>("127.0.0.1", port, "public", "test", "authPriv", "testtesttest", SNMP_VERSION_3, 1000);
        }
    };

    TEST_F(test_SNMPClient, constructor_error)
    {
        ASSERT_THROW(mock_snmp_client("127.0.0.1", port, "public", "test", "authPriv", "test", SNMP_VERSION_3, 1000), snmp_client_exception);
    }

    TEST_F(test_SNMPClient, get_port)
    {
        ASSERT_EQ(161, scPtr->get_port());
    }

    TEST_F(test_SNMPClient, SNMPGet)
    {
        ON_CALL(*scPtr, SNMPGet("Invalid OID")).WillByDefault(Throw(snmp_client_exception("Invalid OID")));
        ASSERT_THROW(scPtr->SNMPGet("Invalid OID"), snmp_client_exception);

        EXPECT_CALL(*scPtr, SNMPGet(RSU_ID_OID)).WillOnce(Return("RSU 4.1"));
        scPtr->SNMPGet(RSU_ID_OID);
    }

    TEST_F(test_SNMPClient, log_error)
    {
        snmp_pdu response;
        ASSERT_NO_THROW(scPtr->log_error(STAT_ERROR, request_type::GET, &response));
        ASSERT_NO_THROW(scPtr->log_error(STAT_ERROR, request_type::SET, &response));
        ASSERT_NO_THROW(scPtr->log_error(STAT_SUCCESS, request_type::OTHER, &response));
        ASSERT_NO_THROW(scPtr->log_error(STAT_TIMEOUT, request_type::OTHER, &response));
    }

    TEST_F(test_SNMPClient, process_snmp_request)
    {
        snmp_response_obj reqponseRSUID;
        string rsuId = "RSU4.1";
        vector<char> rsuId_c;
        copy(rsuId.begin(), rsuId.end(), back_inserter(rsuId_c));
        reqponseRSUID.val_string = rsuId_c;
        reqponseRSUID.type = snmp_response_obj::response_type::STRING;
        EXPECT_CALL(*scPtr, process_snmp_request(RSU_ID_OID, request_type::GET, _)).WillRepeatedly(testing::DoAll(SetArgReferee<2>(reqponseRSUID), Return(true)));
        EXPECT_CALL(*scPtr, process_snmp_request(RSU_ID_OID, request_type::SET, _)).WillRepeatedly(testing::DoAll(SetArgReferee<2>(reqponseRSUID), Return(true)));

        snmp_response_obj reqponseMode;
        reqponseMode.val_int = 2;
        reqponseMode.type = snmp_response_obj::response_type::INTEGER;
        EXPECT_CALL(*scPtr, process_snmp_request(RSU_MODE, request_type::GET, _)).WillRepeatedly(testing::DoAll(SetArgReferee<2>(reqponseMode), Return(true)));
        EXPECT_CALL(*scPtr, process_snmp_request(RSU_MODE, request_type::SET, _)).WillRepeatedly(testing::DoAll(SetArgReferee<2>(reqponseRSUID), Return(true)));

        snmp_response_obj reqponseInvalidOID;
        EXPECT_CALL(*scPtr, process_snmp_request("Invalid OID", request_type::GET, _)).WillRepeatedly(testing::DoAll(SetArgReferee<2>(reqponseInvalidOID), Return(false)));
        EXPECT_CALL(*scPtr, process_snmp_request("Invalid OID", request_type::SET, _)).WillRepeatedly(testing::DoAll(SetArgReferee<2>(reqponseInvalidOID), Return(false)));

        snmp_response_obj response;
        scPtr->process_snmp_request(RSU_ID_OID, request_type::GET, response);
        scPtr->process_snmp_request(RSU_ID_OID, request_type::SET, response);
        scPtr->process_snmp_request(RSU_MODE, request_type::GET, response);
        scPtr->process_snmp_request(RSU_MODE, request_type::SET, response);
        scPtr->process_snmp_request("Invalid OID", request_type::GET, response);
        scPtr->process_snmp_request("Invalid OID", request_type::SET, response);

        snmp_client scClient("127.0.0.1", port, "public", "test", "authPriv", "testtesttest", SNMP_VERSION_3, 1000);
        scClient.process_snmp_request(RSU_ID_OID,request_type::GET,reqponseRSUID);
        scClient.process_snmp_request(RSU_ID_OID,request_type::SET,reqponseRSUID);
        scClient.process_snmp_request(RSU_ID_OID,request_type::OTHER,reqponseRSUID);
        scClient.process_snmp_request("INVALID OID",request_type::GET,reqponseRSUID);

        scClient.process_snmp_request(RSU_MODE,request_type::GET,reqponseMode);
        scClient.process_snmp_request(RSU_MODE,request_type::SET,reqponseMode);
        scClient.process_snmp_request(RSU_MODE,request_type::OTHER,reqponseMode);
    }

}