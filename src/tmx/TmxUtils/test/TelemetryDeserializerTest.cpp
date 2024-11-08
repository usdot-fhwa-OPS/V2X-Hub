// #include <gtest/gtest.h>
// #include "TelemetryDeserializer.h"

// using namespace tmx::utils::telemetry;
// namespace unit_test{
//     class TelemetryDeserializerTest: public ::testing::Test
//     {
//     public:
//         shared_ptr<PluginTelemetry> telemetryPointer;
//         shared_ptr<PluginTelemetry> telemetryPluginInfoOnlyPointer;
        
//         ~TelemetryDeserializerTest() override {};
        
//         void SetUp() override{
//             PluginInfo pluginInfo = {
//                 "1", //id
//                 "CARMACloudPlugin", //name
//                 "CARMA cloud plugin for making websocket connection with CARMA cloud.", //description
//                 "7.6.0", //version
//             };

//             PluginInstallation installation = {
//                 0, //enabled
//                 "/var/www/plugins/CARMACloudPlugin", //path
//                 "/bin/CARMACloudPlugin", //exeName
//                 "manifest.json", //manifest
//                 "500000",//maxMessageInterval
//                 "",//commandLineParameters
//             };
            
//             telemetryPointer = make_shared<PluginTelemetry>();
//             telemetryPluginInfoOnlyPointer = make_shared<PluginTelemetry>();
//             telemetryPointer->setPluginInfo(pluginInfo);
//             telemetryPluginInfoOnlyPointer->setPluginInfo(pluginInfo);
//             telemetryPointer->setPluginInstallation(installation);
//         }
//         void TearDown() override{}
//     };

//     TEST_F(TelemetryDeserializerTest, desrializeTelemetryPluginList){
//         string jsonString = "[{\"commandLineParameters\" : \"\",\"description\" : \"CARMA cloud plugin for making websocket connection with CARMA cloud.\",\"enabled\" : \"Disabled\",\"exeName\" : \"/bin/CARMACloudPlugin\",\"id\" : \"1\",\"manifest\" : \"manifest.json\",\"maxMessageInterval\" : \"500000\",\"name\" : \"CARMACloudPlugin\",\"path\" : \"/var/www/plugins/CARMACloudPlugin\",\"version\" : \"7.6.0\"}]";
//         auto telemetryList =  TelemetryDeserializer::desrializePluginTelemetryList(jsonString);
//         ASSERT_TRUE(telemetryList.front().isInstallationSet());
//         ASSERT_TRUE(telemetryList.front().isPluginInfoSet());
//     }

//     TEST_F(TelemetryDeserializerTest, desrializeTelemetryPluginListPluginInfoOnly){        
//         string jsonString = "[{\"description\" : \"CARMA cloud plugin for making websocket connection with CARMA cloud.\",\"id\" : \"1\",\"name\" : \"CARMACloudPlugin\",\"version\" : \"7.6.0\"}]";
//         auto telemetryList =  TelemetryDeserializer::desrializePluginTelemetryList(jsonString);
//         ASSERT_FALSE(telemetryList.front().isInstallationSet());
//         ASSERT_TRUE(telemetryList.front().isPluginInfoSet());
//     }

//     TEST_F(TelemetryDeserializerTest, deserialzePluginTelemetry){        
//         string jsonString = "{\"description\" : \"CARMA cloud plugin for making websocket connection with CARMA cloud.\",\"id\" : \"1\",\"name\" : \"CARMACloudPlugin\",\"version\" : \"7.6.0\"}";
//         auto telemetry =  TelemetryDeserializer::deserialzePluginTelemetry(jsonString);
//         ASSERT_FALSE(telemetry.isInstallationSet());
//         ASSERT_TRUE(telemetry.isPluginInfoSet());
//     }

//     TEST_F(TelemetryDeserializerTest, desrializeTelemetryPluginListEmpty){
//         ASSERT_THROW(TelemetryDeserializer::desrializePluginTelemetryList(""), TelemetryDeserializerException);
//     }

//     TEST_F(TelemetryDeserializerTest, desrializeTelemetryPluginListNotArray){
//         ASSERT_THROW(TelemetryDeserializer::desrializePluginTelemetryList("{}"), TelemetryDeserializerException);
//     }

//     TEST_F(TelemetryDeserializerTest, desrializeTelemetryPluginListContentEmpty){
//         auto telemetryList = TelemetryDeserializer::desrializePluginTelemetryList("[{}]");
//         ASSERT_TRUE(telemetryList.size()==1);
//         ASSERT_FALSE(telemetryList.front().isInstallationSet());
//         ASSERT_FALSE(telemetryList.front().isPluginInfoSet());
//     }
// }