#include <gtest/gtest.h>
#include "TelemetrySerializer.h"


using namespace tmx::utils::telemetry;
namespace unit_test{
    class TelemetrySerializerTest: public ::testing::Test
    {
    public:
        shared_ptr<PluginTelemetry> telemetryPointer;
        shared_ptr<PluginTelemetry> telemetryPluginInfoOnlyPointer;
        
        ~TelemetrySerializerTest() override {};
        
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
                -1, //enabled
                "", //path
                "", //exeName
                "", //manifest
                "",//maxMessageInterval
                "",//commandLineParameters
            };
            
            telemetryPointer = make_shared<PluginTelemetry>();
            telemetryPluginInfoOnlyPointer = make_shared<PluginTelemetry>();
            telemetryPointer->setPluginInfo(pluginInfo);
            telemetryPluginInfoOnlyPointer->setPluginInfo(pluginInfo);
            telemetryPointer->setPluginInstallation(installation);
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
        ASSERT_EQ(0, telemetryPointer->getPluginInstallation().enabled);
        ASSERT_EQ("/bin/CARMACloudPlugin", telemetryPointer->getPluginInstallation().exeName);
        ASSERT_EQ("manifest.json", telemetryPointer->getPluginInstallation().manifest);
        ASSERT_EQ("500000", telemetryPointer->getPluginInstallation().maxMessageInterval);
        ASSERT_EQ("", telemetryPointer->getPluginInstallation().commandLineParameters);
    }

    TEST_F(TelemetrySerializerTest, serializeFullPluginTelemetry){
        auto telemetryContainer = TelemetrySerializer::serializeFullPluginTelemetry(*telemetryPointer.get());
        string result = TelemetrySerializer::jsonToString(telemetryContainer);
        string expected = "{\"name\":\"CARMACloudPlugin\",\"id\":\"1\",\"description\":\"CARMA cloud plugin for making websocket connection with CARMA cloud.\",\"version\":\"7.6.0\",\"enabled\":\"Disabled\",\"path\":\"manifest.json\",\"exeName\":\"\\/bin\\/CARMACloudPlugin\",\"maxMessageInterval\":\"500000\",\"commandLineParameters\":\"\"}";
        ASSERT_EQ(expected, result);
    }

    TEST_F(TelemetrySerializerTest, serializeFullPluginTelemetryList){
        vector<PluginTelemetry> pluginTelemetryList;
        pluginTelemetryList.push_back(*telemetryPointer.get());
        pluginTelemetryList.push_back(*telemetryPluginInfoOnlyPointer.get());
        auto telemetryContainer = TelemetrySerializer::serializeFullPluginTelemetryList(pluginTelemetryList);
        string result = TelemetrySerializer::jsonToString(telemetryContainer);
        string expected = "{\"payload\":[{\"name\":\"CARMACloudPlugin\",\"id\":\"1\",\"description\":\"CARMA cloud plugin for making websocket connection with CARMA cloud.\",\"version\":\"7.6.0\",\"enabled\":\"Disabled\",\"path\":\"manifest.json\",\"exeName\":\"\\/bin\\/CARMACloudPlugin\",\"maxMessageInterval\":\"500000\",\"commandLineParameters\":\"\"},{\"name\":\"CARMACloudPlugin\",\"id\":\"1\",\"description\":\"CARMA cloud plugin for making websocket connection with CARMA cloud.\",\"version\":\"7.6.0\",\"enabled\":\"External\"}]}";
        ASSERT_EQ(expected, result);
    }

    TEST_F(TelemetrySerializerTest, serializeTelemetryHeader){
        TelemetryHeader header{"Telemetry","List","JsonString",12212};
        auto headerContainer = TelemetrySerializer::serializeTelemetryHeader(header);
        auto result = TelemetrySerializer::jsonToString(headerContainer);
        string expected = "{\"header\":{\"type\":\"Telemetry\",\"subtype\":\"List\",\"encoding\":\"JsonString\",\"timestamp\":\"12212\"}}";
        ASSERT_EQ(expected, result);
    }
}