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
#include <gmock/gmock.h>
#include "FLIRWebsockAsyncClnSession.hpp" 

using namespace FLIRCameraDriverPlugin;

TEST(FLIRWebsockAsyncClnSessionTest, testHandleDataMessage) {
    std::string json = R"(
        {
            "dataNumber": "199262",
            "messageType": "Data",
            "time": "2025-05-20T13:47:35.092-04:00",
            "track": [
                {
                "angle": "255.00000000",
                "class": "Pedestrian",
                "iD": "2910604",
                "latitude": "38.95504354",
                "longitude": "-77.14934177",
                "speed": "1.12053359",
                "x": "-2.66856720",
                "y": "19.92332193"
                },
                {
                "angle": "246.00000000",
                "class": "Pedestrian",
                "iD": "2910927",
                "latitude": "38.95503376",
                "longitude": "-77.14937300",
                "speed": "4.96141529",
                "x": "-3.02072971",
                "y": "22.81371546"
                }
            ],
            "type": "PedestrianPresenceTracking"
            }
    
    )";
    std::stringstream ss(json);
    boost::property_tree::ptree pr;
    try {
        boost::property_tree::read_json(ss, pr);
    } catch(const boost::property_tree::ptree_error &e) {
        GTEST_FAIL() << "Error converting json to p tree: " << e.what();
    }

    // The io_context is required for all I/O
    net::io_context ioc;

    // Create a session and run it
    auto flirSession = std::make_shared<FLIRWebsockAsyncClnSession>(
        ioc, 
        "127.0.0.1", 
        "8084", 
        255, 
        "FLIRNorth",
        "api/subscription",
        tmx::utils::WGS84Point(38.95499217, -77.14920953, 0.0) // Example reference point
    ); 

    flirSession->handleDataMessage(pr);
    EXPECT_EQ(flirSession->getMsgQueue().size(), 2);
    flirSession->clearMsgQueue();
    EXPECT_EQ(flirSession->getMsgQueue().size(), 0);
} 

TEST(FLIRWebsockAsyncClnSessionTest, testRun) {
    std::string json = R"(
        {
            "dataNumber": "199262",
            "messageType": "Data",
            "time": "2025-05-20T13:47:35.092-04:00",
            "track": [
                {
                "angle": "255.00000000",
                "class": "Pedestrian",
                "iD": "2910604",
                "latitude": "38.95504354",
                "longitude": "-77.14934177",
                "speed": "1.12053359",
                "x": "-2.66856720",
                "y": "19.92332193"
                },
                {
                "angle": "246.00000000",
                "class": "Pedestrian",
                "iD": "2910927",
                "latitude": "38.95503376",
                "longitude": "-77.14937300",
                "speed": "4.96141529",
                "x": "-3.02072971",
                "y": "22.81371546"
                }
            ],
            "type": "PedestrianPresenceTracking"
            }
    
    )";
    std::stringstream ss(json);
    boost::property_tree::ptree pr;
    try {
        boost::property_tree::read_json(ss, pr);
    } catch(const boost::property_tree::ptree_error &e) {
        GTEST_FAIL() << "Error converting json to p tree: " << e.what();
    }

    // The io_context is required for all I/O
    net::io_context ioc;

    // Create a session and run it
    auto flirSession = std::make_shared<FLIRWebsockAsyncClnSession>(
        ioc, 
        "127.0.0.1", 
        "8084", 
        255, 
        "FLIRNorth",
        "api/subscription",
        tmx::utils::WGS84Point(38.95499217, -77.14920953, 0.0) // Example reference point
    ); 

    EXPECT_NO_THROW(flirSession->run());
    
}