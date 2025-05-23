/**
 * Copyright (C) 2025 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
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
                    "Endpoint": "api/subscription",
                    "CameraRotation": 90.0,
                    "SensorId": "North"
                },
                {
                    "WebSocketHost": "127.0.0.1",
                    "WebSocketPort": "8081",
                    "Endpoint": "api/subscription",
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

