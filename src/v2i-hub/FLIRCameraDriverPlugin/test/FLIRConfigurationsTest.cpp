#include <gtest/gtest.h>
#include "FLIRConfigurations.hpp"
using std::string;

TEST(FLIRConfigurationsTest, parseFLIRConfigs)
{
    FLIRCameraDriverPlugin::FLIRConfigurations flirConfigs;
    string configs_str = R"(
            [
                {
                    "WebSocketHost": "127.0.0.1",
                    "WebSocketPort": "8080",
                    "HostString": "api/subscription",
                    "CameraRotation": 90.0,
                    "SensorId": "North"
                },
                {
                    "WebSocketHost": "127.0.0.1",
                    "WebSocketPort": "8081",
                    "HostString": "api/subscription",
                    "CameraRotation": 180.0,
                    "SensorId": "South"
                }
            ]
    )";
    flirConfigs.parseFLIRConfigs(configs_str);
    vector<FLIRCameraDriverPlugin::FLIRConfiguration> configs = flirConfigs.getConfigs();
    EXPECT_EQ(configs.size(), 2);
    EXPECT_EQ(configs[0].socketIp, "127.0.0.1");   
    EXPECT_EQ(configs[0].socketPort, "8080");
    EXPECT_EQ(configs[0].apiSubscription, "api/subscription");
    EXPECT_EQ(configs[0].cameraRotation, 90.0);
    EXPECT_EQ(configs[0].sensorId, "North");
    EXPECT_EQ(configs[1].socketIp, "127.0.0.1");
    EXPECT_EQ(configs[1].socketPort, "8081");
    EXPECT_EQ(configs[1].apiSubscription, "api/subscription");
    EXPECT_EQ(configs[1].cameraRotation, 180.0);
    EXPECT_EQ(configs[1].sensorId, "South");
}

