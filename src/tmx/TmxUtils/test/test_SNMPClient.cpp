
#include <MockSNMPClient.h>
#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <rsu/RSU_MIB_4_1.h>
#include <rsu/NTCIP_1218_MIB.h>

using namespace tmx::utils;
using namespace std;
using namespace tmx::utils::rsu::mib::rsu41;
using namespace tmx::utils::rsu::mib::ntcip1218;
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
            scPtr = make_shared<mock_snmp_client>("127.0.0.1", port, "public", "test", "authPriv", "SHA-512", "test1234", "AES-256", "testtesttest", SNMP_VERSION_3, 1000);
        }
    };

    TEST_F(test_SNMPClient, constructor_error)
    {
        EXPECT_THROW(snmp_client("127.0.0.1", port, "public", "test", "authPriv", "test", SNMP_VERSION_3, 1000), snmp_client_exception);
        EXPECT_NO_THROW(snmp_client("127.0.0.1", port, "public", "test", "authPriv", "SHA", "test1234", "DES", "test1234", SNMP_VERSION_3, 1000));
        EXPECT_NO_THROW(snmp_client("127.0.0.1", port, "public", "test", "authPriv", "SHA-224", "test1234", "AES", "testtesttest", SNMP_VERSION_3, 1000));
        EXPECT_NO_THROW(snmp_client("127.0.0.1", port, "public", "test", "authNoPriv", "SHA-256", "test1234", "AES-128", "testtesttest", SNMP_VERSION_3, 1000));
        EXPECT_NO_THROW(snmp_client("127.0.0.1", port, "public", "test", "authNoPriv", "SHA-384", "test1234", "AES-192", "testtesttest", SNMP_VERSION_1, 1000));
        EXPECT_NO_THROW(snmp_client("127.0.0.1", port, "public", "test", "authNoPriv", "SHA-512", "test1234", "AES-192-Cisco", "testtesttest", SNMP_VERSION_2c, 1000));
        EXPECT_NO_THROW(snmp_client("127.0.0.1", port, "public", "test", "authNoPriv", "SHA-512", "test1234", "AES-256-Cisco", "testtesttest", SNMP_VERSION_2c, 1000));
        EXPECT_NO_THROW(snmp_client("127.0.0.1", port, "public", "test", "", "SHA-512", "test1234", "AES-256", "testtesttest", SNMP_VERSION_3, 1000));
        EXPECT_THROW(snmp_client("127.0.0.1", port, "public", "test", "authNoPriv", "dummy", "test1234", "AES-256", "testtesttest", SNMP_VERSION_3, 1000), snmp_client_exception);
        EXPECT_THROW(snmp_client("127.0.0.1", port, "public", "test", "authPriv", "SHA-512", "test1234", "dummy", "testtesttest", SNMP_VERSION_3, 1000), snmp_client_exception);
        EXPECT_THROW(snmp_client("127.0.XX.XX", port, "public", "test", "", "SHA-512", "test1234", "AES-256", "testtesttest", SNMP_VERSION_3, 1000), snmp_client_exception);
    }



    TEST_F(test_SNMPClient, simpleTest)
    {
        auto client = std::make_unique<snmp_client>("127.0.0.1", 161, "public", "test", "authPriv", "SHA-512", "test1234", "AES-256", "test1234", SNMP_VERSION_3, 1000);
        EXPECT_EQ("127.0.0.1", client->get_ip());
        EXPECT_EQ(161, client->get_port());
        snmp_request request {
            RSU_ID_OID,
            's',
            "RSU4.1"
        };
        vector<snmp_request> requests = {request};
        EXPECT_FALSE(client->process_snmp_set_requests(requests));

    }

    TEST_F(test_SNMPClient, process_snmp_set_requests_async)
    {
        // SNMPv2c on purpose. SNMPv3 cannot build a message until USM has discovered the agent engine ID,
        // so with no agent listening the send would be rejected before a packet is ever written and the
        // success path would never be reached. v2c has no discovery step, so the send is a plain sendto.
        auto client = std::make_unique<snmp_client>("127.0.0.1", 161, "public", "test", "", "SHA", "test1234", "AES", "test1234", SNMP_VERSION_2c, 1000);
        snmp_request request {
            RSU_ID_OID,
            's',
            "RSU4.1"
        };
        vector<snmp_request> requests = {request};
        // Nothing is listening, so no response will ever come back. The send must still succeed and return
        // without waiting for one.
        EXPECT_TRUE(client->process_snmp_set_requests_async(requests));

        // The timeout callback logs at logWARNING, which the default ERROR reporting level suppresses.
        // Raise it so a request being expired is actually visible in the test output.
        auto previous_level = tmx::utils::FILELog::ReportingLevel();
        tmx::utils::FILELog::ReportingLevel() = tmx::utils::logWARNING;

        // Each further send pumps the session before adding its own request. The client is built with a
        // 1000 microsecond timeout and net-snmp retries 5 times, so roughly 6ms of real time has to pass
        // before a request can expire. Sleeping between sends lets that happen, which drives the pending
        // requests through their retries and into the timeout callback. That callback is the only thing
        // that frees the PDUs handed off to net-snmp, so this is the path worth exercising.
        for (int i = 0; i < 10; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            EXPECT_NO_THROW(client->process_snmp_set_requests_async(requests));
        }
        tmx::utils::FILELog::ReportingLevel() = previous_level;
    }

    TEST_F(test_SNMPClient, process_snmp_set_requests_async_send_failure)
    {
        // SNMPv3 against an address with no agent. Engine ID discovery cannot complete, so net-snmp
        // rejects the send and process_snmp_set_requests_async reports the failure rather than throwing.
        auto client = std::make_unique<snmp_client>("127.0.0.1", 161, "public", "test", "authPriv", "SHA-512", "test1234", "AES-256", "test1234", SNMP_VERSION_3, 1000);
        snmp_request request {
            RSU_ID_OID,
            's',
            "RSU4.1"
        };
        vector<snmp_request> requests = {request};
        EXPECT_FALSE(client->process_snmp_set_requests_async(requests));
    }

    TEST_F(test_SNMPClient, process_snmp_set_requests_async_invalid_oid)
    {
        auto client = std::make_unique<snmp_client>("127.0.0.1", 161, "public", "test", "authPriv", "SHA-512", "test1234", "AES-256", "test1234", SNMP_VERSION_3, 1000);
        snmp_request request {
            "INVALID OID",
            's',
            "RSU4.1"
        };
        vector<snmp_request> requests = {request};
        EXPECT_THROW(client->process_snmp_set_requests_async(requests), snmp_client_exception);
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

        snmp_client scClient("127.0.0.1", port, "public", "test", "authPriv", "SHA-512", "test1234", "AES-256", "testtesttest", SNMP_VERSION_3, 1000);
        scClient.process_snmp_request(RSU_ID_OID, request_type::GET, reqponseRSUID);
        scClient.process_snmp_request(RSU_ID_OID, request_type::SET, reqponseRSUID);
        scClient.process_snmp_request(RSU_ID_OID, request_type::OTHER, reqponseRSUID);
        scClient.process_snmp_request("INVALID OID", request_type::GET, reqponseRSUID);

        scClient.process_snmp_request(RSU_MODE, request_type::GET, reqponseMode);
        scClient.process_snmp_request(RSU_MODE, request_type::SET, reqponseMode);
        scClient.process_snmp_request(RSU_MODE, request_type::OTHER, reqponseMode);
    }

}