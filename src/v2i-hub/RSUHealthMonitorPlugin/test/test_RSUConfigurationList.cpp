#include <gtest/gtest.h>
#include "RSUConfigurationList.h"
#include <RSURegistrationConfigMessage.h>
#include <TelematicRSUUnit.h>
#include <RSUConfig.h>
#include <RSUEndpoint.h>
#include <RSUSnmpConfig.h>

namespace RSUHealthMonitor
{
    class test_RSUConfigurationList : public ::testing::Test
    {
    public:
        std::shared_ptr<RSUConfigurationList> rsuConfigList = std::make_shared<RSUConfigurationList>();
    };

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"rsuConfigs\": [ { \"event\": \"test event\", \"rsu\": { \"ip\": \"192.168.XX.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authOnlyUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"NTCIP1218\", \"securityLevel\": \"authPriv\" } }, { \"event\": \"test event 2\", \"rsu\": { \"ip\": \"192.168.0.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authPrivUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"RSU4.1\", \"securityLevel\": \"authPriv\" } } ] }";
        rsuConfigList->parseRSUs(rsuConfigsStr);
        ASSERT_EQ(2, rsuConfigList->getConfigs().size());
        std::stringstream ss;
        ss << rsuConfigList->getConfigs()[0];
        std::string expected = "event: test event, rsu.{ip: 192.168.XX.XX, port: 161}, snmp.{user: authOnlyUser, authProtocol: SHA-512, privacyProtocol: AES-256, authPassPhrase: dummy123, privacyPassPhrase: dummy123, rsuMibVersion: NTCIP1218, securityLevel: authPriv}";
        ASSERT_EQ(expected, ss.str());
    }
    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_MalformatJSON)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"rsuConfigs\" { \"event\": \"test event\", \"rsu\": { \"ip\": \"192.168.XX.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authOnlyUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"NTCIP1218\", \"securityLevel\": \"authPriv\" } }, { \"rsu\": { \"ip\": \"192.168.0.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authPrivUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"RSU4.1\", \"securityLevel\": \"authPriv\" } } ] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_RSUS)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"ERROR\": [ { \"event\": \"test event\", \"rsu\": { \"ip\": \"192.168.XX.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authOnlyUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"NTCIP1218\", \"securityLevel\": \"authPriv\" } } ] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_SNMPPORT)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"rsuConfigs\": [ { \"event\": \"test event\", \"rsu\": { \"ip\": \"192.168.XX.XX\" }, \"snmp\": { \"user\": \"authOnlyUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"NTCIP1218\", \"securityLevel\": \"authPriv\" } }, { \"rsu\": { \"ip\": \"192.168.0.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authPrivUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"RSU4.1\", \"securityLevel\": \"authPriv\" } } ] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_INVALID_SNMPPORT)
    {
        std::string rsuConfigsStr = "{ \"rsuConfigs\": [ { \"event\": \"test event\", \"rsu\": { \"ip\": \"192.168.XX.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authOnlyUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"NTCIP1218\", \"securityLevel\": \"authPriv\" } }, { \"rsu\": { \"ip\": \"192.168.0.XX\", \"port\": \"INVALID_PORT\" }, \"snmp\": { \"user\": \"authPrivUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"RSU4.1\", \"securityLevel\": \"authPriv\" } } ] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_Security_Level)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"rsuConfigs\": [ { \"event\": \"test event\", \"rsu\": { \"ip\": \"192.168.XX.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authOnlyUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"NTCIP1218\" } }, { \"event\": \"test event 2\", \"rsu\": { \"ip\": \"192.168.0.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authPrivUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"RSU4.1\", \"securityLevel\": \"authPriv\" } } ] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_Ip)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"rsuConfigs\": [ { \"event\": \"test event\", \"rsu\": { \"port\": 161 }, \"snmp\": { \"user\": \"authOnlyUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"NTCIP1218\", \"securityLevel\": \"authPriv\" } }, { \"rsu\": { \"ip\": \"192.168.0.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authPrivUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"RSU4.1\", \"securityLevel\": \"authPriv\" } } ] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_MibVersion)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"rsuConfigs\": [ { \"event\": \"test event\", \"rsu\": { \"ip\": \"192.168.XX.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authOnlyUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"securityLevel\": \"authPriv\" } }, { \"rsu\": { \"ip\": \"192.168.0.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authPrivUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"RSU4.1\", \"securityLevel\": \"authPriv\" } } ] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Invalid_MibVersion)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"rsuConfigs\": [ { \"event\": \"test event\", \"rsu\": { \"ip\": \"192.168.XX.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authOnlyUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"Invalid_NTCIP1218\", \"securityLevel\": \"authPriv\" } }, { \"rsu\": { \"ip\": \"192.168.0.XX\", \"port\": 161 }, \"snmp\": { \"user\": \"authPrivUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"RSU4.1\", \"securityLevel\": \"authPriv\" } } ] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), tmx::TmxException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseRSUsFromMessage_Success)
    {
        // Create RSU configurations
        tmx::messages::RSUSnmpConfig snmpConfig1("AES-256", "authPriv", "SHA-512", "dummy123", "authOnlyUser", "dummy123", "NTCIP1218");
        tmx::messages::RSUEndpoint endpoint1("192.168.1.1", 161);
        tmx::messages::RSUConfig rsuConfig1("add", "test event 1", "operational", endpoint1, snmpConfig1);

        tmx::messages::RSUSnmpConfig snmpConfig2("AES-256", "authPriv", "SHA-512", "dummy456", "authPrivUser", "dummy456", "RSU4.1");
        tmx::messages::RSUEndpoint endpoint2("192.168.1.2", 161);
        tmx::messages::RSUConfig rsuConfig2("update", "test event 2", "operational", endpoint2, snmpConfig2);

        std::vector<tmx::messages::RSUConfig> rsuConfigs = {rsuConfig1, rsuConfig2};
        tmx::messages::RSUConfigList rsuConfigList_msg(rsuConfigs);

        // Create message
        tmx::messages::RSURegistrationConfigMessage msg;
        tmx::messages::TelematicRSUUnit unit("Unit001", 10, 10, 10, 10);
        msg.set_unitConfig(unit);
        msg.set_rsuConfigs(rsuConfigList_msg);
        msg.set_timestamp(1234567890000);

        // Parse message
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        rsuConfigList->parseRSUs(msg);
        ASSERT_EQ(2, rsuConfigList->getConfigs().size());

        // Verify first config
        auto configs = rsuConfigList->getConfigs();
        EXPECT_EQ(configs[0].rsuIp, "192.168.1.1");
        EXPECT_EQ(configs[0].snmpPort, 161);
        EXPECT_EQ(configs[0].user, "authOnlyUser");
        EXPECT_EQ(configs[0].authProtocol, "SHA-512");
        EXPECT_EQ(configs[0].privProtocol, "AES-256");
        EXPECT_EQ(configs[0].authPassPhrase, "dummy123");
        EXPECT_EQ(configs[0].privPassPhrase, "dummy123");
        EXPECT_EQ(configs[0].securityLevel, "authPriv");
        EXPECT_EQ(configs[0].mibVersion, tmx::utils::rsu::RSU_SPEC::NTCIP_1218);
        EXPECT_EQ(configs[0].event, "test event 1");

        // Verify second config
        EXPECT_EQ(configs[1].rsuIp, "192.168.1.2");
        EXPECT_EQ(configs[1].snmpPort, 161);
        EXPECT_EQ(configs[1].user, "authPrivUser");
        EXPECT_EQ(configs[1].authProtocol, "SHA-512");
        EXPECT_EQ(configs[1].privProtocol, "AES-256");
        EXPECT_EQ(configs[1].authPassPhrase, "dummy456");
        EXPECT_EQ(configs[1].privPassPhrase, "dummy456");
        EXPECT_EQ(configs[1].securityLevel, "authPriv");
        EXPECT_EQ(configs[1].mibVersion, tmx::utils::rsu::RSU_SPEC::RSU_4_1);
        EXPECT_EQ(configs[1].event, "test event 2");
    }

    TEST_F(test_RSUConfigurationList, parseRSUsFromMessage_EmptyList)
    {
        // Create message with empty RSU config list
        tmx::messages::RSURegistrationConfigMessage msg;
        tmx::messages::TelematicRSUUnit unit("Unit001", 10, 10, 10, 10);
        msg.set_unitConfig(unit);
        std::vector<tmx::messages::RSUConfig> emptyConfigs;
        tmx::messages::RSUConfigList rsuConfigList_msg(emptyConfigs);
        msg.set_rsuConfigs(rsuConfigList_msg);
        msg.set_timestamp(1234567890000);

        // Parse message
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        ASSERT_NO_THROW(rsuConfigList->parseRSUs(msg));
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseRSUsFromMessage_ClearsExistingConfigs)
    {
        // First add some configs using string parsing
        std::string rsuConfigsStr = "{ \"rsuConfigs\": [ { \"event\": \"old event\", \"rsu\": { \"ip\": \"192.168.0.1\", \"port\": 161 }, \"snmp\": { \"user\": \"oldUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"old123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"old123\", \"rsuMibVersion\": \"NTCIP1218\", \"securityLevel\": \"authPriv\" } } ] }";
        rsuConfigList->parseRSUs(rsuConfigsStr);
        ASSERT_EQ(1, rsuConfigList->getConfigs().size());

        // Now parse message with different configs
        tmx::messages::RSUSnmpConfig snmpConfig("AES-256", "authPriv", "SHA-512", "new123", "newUser", "new123", "RSU4.1");
        tmx::messages::RSUEndpoint endpoint("10.0.0.1", 161);
        tmx::messages::RSUConfig rsuConfig("add", "new event", "operational", endpoint, snmpConfig);
        std::vector<tmx::messages::RSUConfig> rsuConfigs = {rsuConfig};
        tmx::messages::RSUConfigList rsuConfigList_msg(rsuConfigs);

        tmx::messages::RSURegistrationConfigMessage msg;
        tmx::messages::TelematicRSUUnit unit("Unit001", 10, 10, 10, 10);
        msg.set_unitConfig(unit);
        msg.set_rsuConfigs(rsuConfigList_msg);
        msg.set_timestamp(1234567890000);

        // Parse message - should clear old configs
        rsuConfigList->parseRSUs(msg);
        ASSERT_EQ(1, rsuConfigList->getConfigs().size());

        // Verify new config replaced old one
        auto configs = rsuConfigList->getConfigs();
        EXPECT_EQ(configs[0].rsuIp, "10.0.0.1");
        EXPECT_EQ(configs[0].user, "newUser");
        EXPECT_EQ(configs[0].event, "new event");
    }

    TEST_F(test_RSUConfigurationList, parseRSUsFromMessage_InvalidMibVersion)
    {
        // Create config with invalid MIB version
        tmx::messages::RSUSnmpConfig snmpConfig("AES-256", "authPriv", "SHA-512", "dummy123", "testUser", "dummy123", "InvalidMIBVersion");
        tmx::messages::RSUEndpoint endpoint("192.168.1.1", 161);
        tmx::messages::RSUConfig rsuConfig("add", "test event", "operational", endpoint, snmpConfig);
        std::vector<tmx::messages::RSUConfig> rsuConfigs = {rsuConfig};
        tmx::messages::RSUConfigList rsuConfigList_msg(rsuConfigs);

        tmx::messages::RSURegistrationConfigMessage msg;
        tmx::messages::TelematicRSUUnit unit("Unit001", 10, 10, 10, 10);
        msg.set_unitConfig(unit);
        msg.set_rsuConfigs(rsuConfigList_msg);
        msg.set_timestamp(1234567890000);

        // Should throw exception due to invalid MIB version
        ASSERT_THROW(rsuConfigList->parseRSUs(msg), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }
}