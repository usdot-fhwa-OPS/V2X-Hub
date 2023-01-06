#include <gtest/gtest.h>
#include "SNMPClient.h"

namespace unit_test
{
    class SNMPClientTest : public ::testing::Test
    {
    };

    TEST_F(SNMPClientTest, constructor)
    {
        const std::string rsu_ip = "127.0.0.1";
        uint16_t snmp_port = 161;
        std::string securityUser = "dummy";
        std::string authPassPhrase = "dummy"; // Error: passphrase chosen is below the length requirements of the USM (min=8).
        ASSERT_ANY_THROW(SNMPClient(rsu_ip, snmp_port, securityUser, authPassPhrase));
        authPassPhrase = "dummydummy";
        ASSERT_NO_THROW(SNMPClient(rsu_ip, snmp_port, securityUser, authPassPhrase));
    }
    
    TEST_F(SNMPClientTest, SNMPGet)
    {
        const std::string rsu_ip = "127.0.0.1";
        uint16_t snmp_port = 161;
        std::string securityUser = "dummy";
        std::string authPassPhrase = "dummydummy";
        auto snmpClient = SNMPClient(rsu_ip, snmp_port, securityUser, authPassPhrase);
        ASSERT_THROW(snmpClient.SNMPGet("1.0.15628.4.1.8.5.0"), SNMPClientException);
    }
}