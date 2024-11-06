#include <gtest/gtest.h>
#include "TelemetrySerializer.h"

using namespace tmx::utils::telemetry;
namespace unit_test{
    class TelemetrySerializerTest: public ::testing::Test
    {
    public:
        shared_ptr<PluginTelemetry> telemetryPointer;
        
        ~TelemetrySerializerTest() override {};
        
        void SetUp() override{
            PluginInfo pluginInfo = {
                "1", //id
                "CARMACloudPlugin", //name
                "CARMA cloud plugin for making websocket connection with CARMA cloud.", //description
                "7.6.0", //version
            };

            PluginInstallation installation = {
                "Disabled", //enabled
                "/var/www/plugins/CARMACloudPlugin", //path
                "/bin/CARMACloudPlugin", //exeName
                "manifest.json", //manifest
                "500000",//maxMessageInterval
                "",//commandLineParameters
            };
            
            telemetryPointer = make_shared<PluginTelemetry>(pluginInfo, installation);
        }
        void TearDown() override{}
    };

    TEST_F(TelemetrySerializerTest, getPluginInfo){
        ASSERT_EQ("1", telemetryPointer->getPluginInfo().id);
        ASSERT_EQ("7.6.0", telemetryPointer->getPluginInfo().version);
        ASSERT_EQ("CARMA cloud plugin for making websocket connection with CARMA cloud.", telemetryPointer->getPluginInfo().description);
        ASSERT_EQ("CARMACloudPlugin", telemetryPointer->getPluginInfo().name);
    }

    TEST_F(TelemetrySerializerTest, getPluginInstallation){
        ASSERT_EQ("/var/www/plugins/CARMACloudPlugin", telemetryPointer->getPluginInstallation().path);
        ASSERT_EQ("Disabled", telemetryPointer->getPluginInstallation().enabled);
        ASSERT_EQ("/bin/CARMACloudPlugin", telemetryPointer->getPluginInstallation().exeName);
        ASSERT_EQ("manifest.json", telemetryPointer->getPluginInstallation().manifest);
        ASSERT_EQ("500000", telemetryPointer->getPluginInstallation().maxMessageInterval);
        ASSERT_EQ("", telemetryPointer->getPluginInstallation().commandLineParameters);
    }

    TEST_F(TelemetrySerializerTest, serializePluginTelemetryList){
        vector<PluginTelemetry> pluginTelemetryList;
        pluginTelemetryList.push_back(*telemetryPointer.get());
        string serializedTelemetry =  TelemetrySerializer::serializePluginTelemetryList(pluginTelemetryList);
        string expected = "[{\"commandLineParameters\" : \"\",\"description\" : \"CARMA cloud plugin for making websocket connection with CARMA cloud.\",\"enabled\" : \"Disabled\",\"exeName\" : \"/bin/CARMACloudPlugin\",\"id\" : \"1\",\"manifest\" : \"manifest.json\",\"maxMessageInterval\" : \"500000\",\"name\" : \"CARMACloudPlugin\",\"path\" : \"/var/www/plugins/CARMACloudPlugin\",\"version\" : \"7.6.0\"}]";
        ASSERT_EQ(expected, serializedTelemetry);
    }
}