#include <gtest/gtest.h>
#include "FLIRConfigurations.hpp"
using std::string;

TEST(FLIRConfigurationsTest, parseFLIRConfigs)
{
    PedestrianPlugin::FLIRConfigurations flirConfigs;
    string configs_str = R"(
            [
                {
                    "WebSocketHost": "127.0.0.1",
                    "WebSocketPort": "8080",
                    "HostString": "api/subscription",
                    "FLIRCameraRotation": 90.0,
                    "FLIRCameraViewName": "North"
                },
                {
                    "WebSocketHost": "127.0.0.1",
                    "WebSocketPort": "8081",
                    "HostString": "api/subscription",
                    "FLIRCameraRotation": 180.0,
                    "FLIRCameraViewName": "South"
                }
            ]
    )";
    flirConfigs.parseFLIRConfigs(configs_str);
    vector<PedestrianPlugin::FLIRConfiguration> configs = flirConfigs.getConfigs();
    EXPECT_EQ(configs.size(), 2);
    EXPECT_EQ(configs[0].socketIp, "127.0.0.1");   
    EXPECT_EQ(configs[0].socketPort, "8080");
    EXPECT_EQ(configs[0].apiSubscription, "api/subscription");
    EXPECT_EQ(configs[0].FLIRCameraRotation, 90.0);
    EXPECT_EQ(configs[0].FLIRCameraViewName, "North");
    EXPECT_EQ(configs[1].socketIp, "127.0.0.1");
    EXPECT_EQ(configs[1].socketPort, "8081");
    EXPECT_EQ(configs[1].apiSubscription, "api/subscription");
    EXPECT_EQ(configs[1].FLIRCameraRotation, 180.0);
    EXPECT_EQ(configs[1].FLIRCameraViewName, "South");
    string actual = flirConfigs.toString();
    string expectedToStr = "{\"\":{\"WebSocketHost\":\"127.0.0.1\",\"WebSocketPort\":\"8080\",\"HostString\":\"api\\/subscription\",\"FLIRCameraRotation\":\"90.0\",\"FLIRCameraViewName\":\"North\"},\"\":{\"WebSocketHost\":\"127.0.0.1\",\"WebSocketPort\":\"8081\",\"HostString\":\"api\\/subscription\",\"FLIRCameraRotation\":\"180.0\",\"FLIRCameraViewName\":\"South\"}}";
    EXPECT_EQ(expectedToStr, actual);
}

