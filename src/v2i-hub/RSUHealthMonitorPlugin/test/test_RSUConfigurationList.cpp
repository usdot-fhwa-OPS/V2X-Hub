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
        std::string rsuConfigsStr = "{ \"RSUS\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authOnlyUser\", \"RSUMIBVersion\": \"NTCIP1218\" },{ \"RSUIp\": \"192.168.0.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        rsuConfigList->parseRSUs(rsuConfigsStr);
        ASSERT_EQ(2, rsuConfigList->getConfigs().size());
        std::stringstream ss;
        ss << rsuConfigList->getConfigs()[0];
        std::string expected = "RSUIp: 192.168.0.XX, SNMPPort: 161, SecurityLevel:authPriv, AuthProtocol: SHA-512, AuthPassPhrase: dummy123, PrivacyProtocol: AES-256, PrivacyPassPhrase: dummy123, User: authPrivUser, RSUMIBVersion: RSU4.1";
        ASSERT_EQ(expected, ss.str());
    }
    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_MalformatJSON)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"RSUS { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\": \"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authOnlyUser\", \"RSUMIBVersion\": \"NTCIP1218\" },{ \"RSUIp\": \"192.168.0.XX\", \"SNMPPort\": \"162\", \"SecurityLevel\": \"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_RSUS)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"ERROR\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authOnlyUser\", \"RSUMIBVersion\": \"NTCIP1218\" },{ \"RSUIp\": \"192.168.0.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_SNMPPORT)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"RSUS\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort_Missing\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authOnlyUser\", \"RSUMIBVersion\": \"NTCIP1218\" },{ \"RSUIp\": \"192.168.0.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_INVALID_SNMPPORT)
    {
        std::string rsuConfigsStr = "{ \"RSUS\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authOnlyUser\", \"RSUMIBVersion\": \"NTCIP1218\" },{ \"RSUIp\": \"192.168.0.XX\", \"SNMPPort\": \"INVALID_PORT\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_AuthPassPhrase)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"RSUS\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase_Missing\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authOnlyUser\", \"RSUMIBVersion\": \"NTCIP1218\" },{ \"RSUIp\": \"192.168.0.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_User)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"RSUS\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User_Missing\": \"authOnlyUser\", \"RSUMIBVersion\": \"NTCIP1218\" },{ \"RSUIp\": \"192.168.0.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_MibVersion)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"RSUS\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authOnlyUser\", \"RSUMIBVersion_Missing\": \"NTCIP1218\" },{ \"RSUIp\": \"192.168.0.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Invalid_MibVersion)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"RSUS\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authOnlyUser\", \"RSUMIBVersion\": \"Invalid_NTCIP1218\" },{ \"RSUIp\": \"192.168.0.XX\", \"SNMPPort\": \"161\", \"SecurityLevel\":\"authPriv\", \"AuthProtocol\": \"SHA-512\", \"AuthPassPhrase\": \"dummy123\", \"PrivacyProtocol\": \"AES-256\", \"PrivacyPassPhrase\": \"dummy123\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }
}