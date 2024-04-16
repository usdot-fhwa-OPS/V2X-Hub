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
        std::string rsuConfigsStr = "{ \"RSUS\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"AuthPassPhrase\": \"dummy\", \"User\": \"authOnlyUser\", \"RSUMIBVersion\": \"RSU4.1\" },{ \"RSUIp\": \"192.168.00.XX\", \"SNMPPort\": \"162\", \"AuthPassPhrase\": \"tester\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        rsuConfigList->parseRSUs(rsuConfigsStr);
        ASSERT_EQ(2, rsuConfigList->getConfigs().size());
    }
    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_MalformatJSON)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"RSUS { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"AuthPassPhrase\": \"dummy\", \"User\": \"authOnlyUser\", \"RSUMIBVersion\": \"RSU4.1\" },{ \"RSUIp\": \"192.168.00.XX\", \"SNMPPort\": \"162\", \"AuthPassPhrase\": \"tester\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_RSUS)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"ERROR\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"AuthPassPhrase\": \"dummy\", \"User\": \"authOnlyUser\", \"RSUMIBVersion\": \"RSU4.1\" },{ \"RSUIp\": \"192.168.00.XX\", \"SNMPPort\": \"162\", \"AuthPassPhrase\": \"tester\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_SNMPPORT)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"RSUS\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort_Missing\": \"161\", \"AuthPassPhrase\": \"dummy\", \"User\": \"authOnlyUser\", \"RSUMIBVersion\": \"RSU4.1\" },{ \"RSUIp\": \"192.168.00.XX\", \"SNMPPort\": \"162\", \"AuthPassPhrase\": \"tester\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_AuthPassPhrase)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"RSUS\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"AuthPassPhrase_Missing\": \"dummy\", \"User\": \"authOnlyUser\", \"RSUMIBVersion\": \"RSU4.1\" },{ \"RSUIp\": \"192.168.00.XX\", \"SNMPPort\": \"162\", \"AuthPassPhrase\": \"tester\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_User)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"RSUS\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"AuthPassPhrase\": \"dummy\", \"User_Missing\": \"authOnlyUser\", \"RSUMIBVersion\": \"RSU4.1\" },{ \"RSUIp\": \"192.168.00.XX\", \"SNMPPort\": \"162\", \"AuthPassPhrase\": \"tester\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Missing_MibVersion)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"RSUS\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"AuthPassPhrase\": \"dummy\", \"User\": \"authOnlyUser\", \"RSUMIBVersion_Missing\": \"RSU4.1\" },{ \"RSUIp\": \"192.168.00.XX\", \"SNMPPort\": \"162\", \"AuthPassPhrase\": \"tester\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }

    TEST_F(test_RSUConfigurationList, parseAndGetConfigs_Invalid_MibVersion)
    {
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
        std::string rsuConfigsStr = "{ \"RSUS\": [ { \"RSUIp\": \"192.168.XX.XX\", \"SNMPPort\": \"161\", \"AuthPassPhrase\": \"dummy\", \"User\": \"authOnlyUser\", \"RSUMIBVersion\": \"INVALID_RSU4.1\" },{ \"RSUIp\": \"192.168.00.XX\", \"SNMPPort\": \"162\", \"AuthPassPhrase\": \"tester\", \"User\": \"authPrivUser\", \"RSUMIBVersion\": \"RSU4.1\" }] }";
        ASSERT_THROW(rsuConfigList->parseRSUs(rsuConfigsStr), RSUHealthMonitor::RSUConfigurationException);
        ASSERT_EQ(0, rsuConfigList->getConfigs().size());
    }
}