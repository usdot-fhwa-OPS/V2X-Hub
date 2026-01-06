#include <gtest/gtest.h>
#include <RSURegistrationConfigMessage.h>

namespace tmx::messages{
    class RSURegistrationConfigMessageTest : public testing::Test{
        protected:
            std::shared_ptr<RSURegistrationConfigMessage> tmxRSURegistrationConfigPtr;
            std::shared_ptr<TelematicRSUUnit> tmxTelematicRSUUnitPtr;

            RSURegistrationConfigMessageTest(){
                tmxRSURegistrationConfigPtr = std::make_shared<RSURegistrationConfigMessage>();
            }


            void SetUp() override {
                //Set Telematic Unit members
                TelematicRSUUnit TelematicRSUUnit("Unit007", 10, 10, 10, 10);
                tmxRSURegistrationConfigPtr->set_unit(TelematicRSUUnit);

                std::string action = "add";
                RSUConfig rsuConfig1("add", "new",
                     RSUEndpoint("127.0.0.1", 161),
                     RSUSnmpConfig());

                RSUConfig rsuConfig2("update", "modified",
                            RSUEndpoint("192.168.0.1", 161),
                            RSUSnmpConfig());

                std::vector<RSUConfig> rsuConfigs = {rsuConfig1, rsuConfig2};

                RSUConfigList rsuConfigList(rsuConfigs);

                tmxRSURegistrationConfigPtr->set_rsuConfigs(rsuConfigList);

                tmxRSURegistrationConfigPtr->set_timestamp(1234567890000);
            }

    };

    TEST_F(RSURegistrationConfigMessageTest, to_string){
        //Convert message to JSON string
        std::string jsonStr = tmxRSURegistrationConfigPtr->to_string();

        // Verify string is not empty
        ASSERT_FALSE(jsonStr.empty());

        std::stringstream ss;
        ss <<jsonStr;
        boost::property_tree::ptree tree;
        boost::property_tree::read_json(ss, tree);

        // Verify telematic unit
        EXPECT_EQ(tree.get<std::string>("unit.unitId"), "Unit007");
        EXPECT_EQ(tree.get<int>("unit.maxConnections"), 10);
        EXPECT_EQ(tree.get<int>("unit.bridgePluginHeartbeatInterval"),10);

        EXPECT_EQ(tree.get<int64_t>("timestamp"),1234567890000);

        std::cout << "JSON Output:\n" << jsonStr << std::endl;

        // Test setting content for message
        std::shared_ptr<RSURegistrationConfigMessage> tmxRSURegistrationConfigPtr2 = std::make_shared<RSURegistrationConfigMessage>();
        tmxRSURegistrationConfigPtr2->set_contents(jsonStr);
        std::string jsonStr2 = tmxRSURegistrationConfigPtr2->to_string();

        EXPECT_EQ(jsonStr, jsonStr2);
    }

    TEST_F(RSURegistrationConfigMessageTest, getters) {
        TelematicRSUUnit unit = tmxRSURegistrationConfigPtr->get_unit();
        EXPECT_EQ(unit.unitId, "Unit007");
        EXPECT_EQ(unit.maxConnections, 10);
        EXPECT_EQ(unit.bridgePluginHeartbeatInterval, 10);

        RSUConfigList configList = tmxRSURegistrationConfigPtr->get_rsuConfigs();
        EXPECT_EQ(configList.rsuConfigs.size(), 2);

        int64_t timestamp = tmxRSURegistrationConfigPtr->get_timestamp();
        EXPECT_EQ(timestamp, 1234567890000);
    }

    TEST_F(RSURegistrationConfigMessageTest, rsu_endpoint_serialization){
        RSUEndpoint endpoint("10.0.0.1", 161);
        RSUSnmpConfig snmpConfig;
        RSUConfig original("add", "registered", endpoint, snmpConfig);

        auto tree = RSUConfig::to_tree(original);
        RSUConfig reconstructed = RSUConfig::from_tree(tree);

        EXPECT_EQ(original.action, reconstructed.action);
        EXPECT_EQ(original.event, reconstructed.event);
        EXPECT_EQ(original.rsu.ip, reconstructed.rsu.ip);
        EXPECT_EQ(original.rsu.port, reconstructed.rsu.port);
    }

}