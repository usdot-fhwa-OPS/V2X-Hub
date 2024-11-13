#include <gtest/gtest.h>
#include "TelemetryDeserializer.h"

using namespace tmx::utils::telemetry;
namespace unit_test{
    class TelemetryDeserializerTest: public ::testing::Test
    {
    public:
        ~TelemetryDeserializerTest() override {};
    };

    TEST_F(TelemetryDeserializerTest, desrializeFullPluginTelemetryPayload){
        string jsonString = "{\"payload\":[{\"name\":\"CARMACloudPlugin\",\"id\":\"1\",\"description\":\"CARMA cloud plugin for making websocket connection with CARMA cloud.\",\"version\":\"7.6.0\",\"enabled\":\"Disabled\",\"path\":\"manifest.json\",\"exeName\":\"\\/bin\\/CARMACloudPlugin\",\"maxMessageInterval\":\"500000\",\"commandLineParameters\":\"\"},{\"name\":\"CARMACloudPlugin2\",\"id\":\"2\",\"description\":\"CARMA cloud plugin for making websocket connection with CARMA cloud.\",\"version\":\"7.6.0\",\"enabled\":\"External\"}]}";
        auto jsonContainer = TelemetryDeserializer::stringToJson(jsonString);
        auto telemetryList =  TelemetryDeserializer::desrializeFullTelemetryPayload<PluginTelemetry>(jsonContainer);
        ASSERT_EQ(2, telemetryList.size());
        
        ASSERT_EQ("CARMA cloud plugin for making websocket connection with CARMA cloud.",telemetryList.begin()->getPluginInfo().description);
        ASSERT_EQ("1",telemetryList.begin()->getPluginInfo().id);
        ASSERT_EQ("CARMACloudPlugin",telemetryList.begin()->getPluginInfo().name);
        ASSERT_EQ("7.6.0",telemetryList.begin()->getPluginInfo().version);
        ASSERT_EQ(0,telemetryList.begin()->getPluginInstallation().enabled);
        ASSERT_EQ("manifest.json",telemetryList.begin()->getPluginInstallation().path);
        ASSERT_EQ("/bin/CARMACloudPlugin",telemetryList.begin()->getPluginInstallation().exeName);
        ASSERT_EQ("500000",telemetryList.begin()->getPluginInstallation().maxMessageInterval);
        ASSERT_EQ("",telemetryList.begin()->getPluginInstallation().commandLineParameters);

        ASSERT_EQ("2",telemetryList.back().getPluginInfo().id);
        ASSERT_EQ("CARMACloudPlugin2",telemetryList.back().getPluginInfo().name);
        ASSERT_EQ("7.6.0",telemetryList.back().getPluginInfo().version);
        ASSERT_EQ(-1,telemetryList.back().getPluginInstallation().enabled);
        ASSERT_EQ("",telemetryList.back().getPluginInstallation().path);
        ASSERT_EQ("",telemetryList.back().getPluginInstallation().exeName);
        ASSERT_EQ("",telemetryList.back().getPluginInstallation().maxMessageInterval);
        ASSERT_EQ("",telemetryList.back().getPluginInstallation().commandLineParameters);
    }

    TEST_F(TelemetryDeserializerTest, desrializeFullPluginTelemetryPayloadPluginInfoOnly){        
        string jsonString = "{\"payload\":[{\"description\" : \"CARMA cloud plugin for making websocket connection with CARMA cloud.\",\"id\" : \"1\",\"name\" : \"CARMACloudPlugin\",\"version\" : \"7.6.0\"}]}";
        auto jsonContainer = TelemetryDeserializer::stringToJson(jsonString);
        auto telemetryList =  TelemetryDeserializer::desrializeFullTelemetryPayload<PluginTelemetry>(jsonContainer);
        ASSERT_EQ(1, telemetryList.size());
        ASSERT_EQ("CARMA cloud plugin for making websocket connection with CARMA cloud.",telemetryList.begin()->getPluginInfo().description);
        ASSERT_EQ("1",telemetryList.begin()->getPluginInfo().id);
        ASSERT_EQ("CARMACloudPlugin",telemetryList.begin()->getPluginInfo().name);
        ASSERT_EQ("7.6.0",telemetryList.begin()->getPluginInfo().version);
    }

    TEST_F(TelemetryDeserializerTest, desrializeFullPluginTelemetryPayloadEmpty){
        ASSERT_THROW(TelemetryDeserializer::desrializeFullTelemetryPayload<PluginTelemetry>(TelemetryDeserializer::stringToJson("")), tmx::TmxException);
    }

    TEST_F(TelemetryDeserializerTest, desrializeFullPluginTelemetryPayloadNotArray){
        ASSERT_THROW(TelemetryDeserializer::desrializeFullTelemetryPayload<PluginTelemetry>(TelemetryDeserializer::stringToJson("{\"payload\":{\"test\": \"0\"}}")), tmx::TmxException);
    }

    TEST_F(TelemetryDeserializerTest, desrializeFullPluginTelemetryPayloadContentEmpty){
        ASSERT_THROW(TelemetryDeserializer::desrializeFullTelemetryPayload<PluginTelemetry>(TelemetryDeserializer::stringToJson("{\"payload\":[{}]}")), tmx::TmxException);
    }
}