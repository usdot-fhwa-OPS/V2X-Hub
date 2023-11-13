
#include "SNMPClient.h"
#include "gtest/gtest.h"
#include "RSU_MIB_4_1.h"

using namespace tmx::utils;
using namespace std;
using namespace tmx::utils::rsu41::mib::oid;

namespace unit_test
{
    class test_SNMPClient : public ::testing::Test
    {
    public:
        shared_ptr<snmp_client> scPtr;
        test_SNMPClient()
        {
            uint16_t port = 161;
            scPtr = make_shared<snmp_client>("127.0.0.1", port, "public", "test", "authPriv", "testtesttest", SNMP_VERSION_3, 1000);
        };
    };

    TEST_F(test_SNMPClient, get_port)
    {
        ASSERT_EQ(161, scPtr->get_port());
    }

    TEST_F(test_SNMPClient, SNMPGet)
    {
        ASSERT_THROW(scPtr->SNMPGet(RSU_ID_OID), snmp_client_exception);
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
        snmp_response_obj response;
        response.type = snmp_response_obj::response_type::INTEGER;
        ASSERT_FALSE(scPtr->process_snmp_request(RSU_ID_OID, request_type::GET, response));
        ASSERT_FALSE(scPtr->process_snmp_request(RSU_ID_OID, request_type::SET, response));
        ASSERT_FALSE(scPtr->process_snmp_request(RSU_ID_OID, request_type::OTHER, response));

        response.type = snmp_response_obj::response_type::STRING;
        ASSERT_FALSE(scPtr->process_snmp_request(RSU_ID_OID, request_type::GET, response));
        ASSERT_FALSE(scPtr->process_snmp_request(RSU_ID_OID, request_type::SET, response));
        ASSERT_FALSE(scPtr->process_snmp_request(RSU_ID_OID, request_type::OTHER, response));
    }

}