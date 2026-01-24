#include <gtest/gtest.h>
#include "RSUConfigurationList.h"

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
}