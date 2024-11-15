
#include "gtest/gtest.h"
#include "telemetry/PluginTelemetry.h"

using namespace tmx::utils::telemetry;
using std::make_shared;
namespace unit_test{
    class PluginTelemetryTest: public ::testing::Test{
        public:
            std::shared_ptr<PluginTelemetry> telemetryPointer;        
            
            void SetUp() override{
                PluginInfo pluginInfo = {
                    "1", //id
                    "CARMACloudPlugin", //name
                    "CARMA cloud plugin for making websocket connection with CARMA cloud.", //description
                    "7.6.0", //version
                };

                PluginInstallation installation = {
                    0, //enabled
                    "/var/www/plugins/CARMACloudPlugin", //path
                    "/bin/CARMACloudPlugin", //exeName
                    "manifest.json", //manifest
                    "500000",//maxMessageInterval
                    "",//commandLineParameters
                };

                PluginInstallation noInstallation = {
                    -1, //external
                    "", //path
                    "", //exeName
                    "", //manifest
                    "",//maxMessageInterval
                    "",//commandLineParameters
                };
                
                telemetryPointer = make_shared<PluginTelemetry>();
                telemetryPointer->setPluginInfo(pluginInfo);
                telemetryPointer->setPluginInstallation(installation);
            }
    };

     TEST_F(PluginTelemetryTest, getPluginInfo){
        ASSERT_EQ("1", telemetryPointer->getPluginInfo().id);
        ASSERT_EQ("7.6.0", telemetryPointer->getPluginInfo().version);
        ASSERT_EQ("CARMA cloud plugin for making websocket connection with CARMA cloud.", telemetryPointer->getPluginInfo().description);
        ASSERT_EQ("CARMACloudPlugin", telemetryPointer->getPluginInfo().name);
    }

    TEST_F(PluginTelemetryTest, getPluginInstallation){
        ASSERT_EQ("/var/www/plugins/CARMACloudPlugin", telemetryPointer->getPluginInstallation().path);
        ASSERT_EQ(0, telemetryPointer->getPluginInstallation().enabled);
        ASSERT_EQ("/bin/CARMACloudPlugin", telemetryPointer->getPluginInstallation().exeName);
        ASSERT_EQ("manifest.json", telemetryPointer->getPluginInstallation().manifest);
        ASSERT_EQ("500000", telemetryPointer->getPluginInstallation().maxMessageInterval);
        ASSERT_EQ("", telemetryPointer->getPluginInstallation().commandLineParameters);
    }

    TEST_F(PluginTelemetryTest, toTree){
        tmx::message_container_type tree;
        ASSERT_TRUE(tree.get_storage().get_tree().empty());
        tree = telemetryPointer->toTree();
        ASSERT_FALSE(tree.get_storage().get_tree().empty());
    }

    TEST_F(PluginTelemetryTest, fromTree){
        auto tree = telemetryPointer->toTree();
        auto newTelemetryPointer = make_shared<PluginTelemetry>();
        ASSERT_EQ("", newTelemetryPointer->getPluginInfo().name);
        newTelemetryPointer->fromTree(tree.get_storage().get_tree());
        ASSERT_EQ("CARMACloudPlugin", newTelemetryPointer->getPluginInfo().name);
    }
}