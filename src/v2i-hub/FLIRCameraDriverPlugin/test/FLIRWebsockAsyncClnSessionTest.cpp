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
#include <thread>
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
    EXPECT_EQ(flirSession->getNumLastDetections(), 2);
    EXPECT_EQ(flirSession->getLastDetectionTime(), 1747763255092);
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

TEST(FLIRWebsockAsyncClnSessionTest, processPedestrianPresenceTrackingObjects)
{
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
        90.0, 
        "North",
        "api/subscription",
        tmx::utils::WGS84Point(38.95499217, -77.14920953, 0.0) // Example reference point
    ); 
    
    std::queue<tmx::messages::SensorDetectedObject> msgQueue = flirSession->processPedestrianPresenceTrackingObjects(pr);

    EXPECT_EQ(msgQueue.size(), 2);
    tmx::messages::SensorDetectedObject obj = msgQueue.front();
    EXPECT_EQ(flirSession->getNumLastDetections(), 2);
    EXPECT_EQ(flirSession->getLastDetectionTime(), 1747763255092);
    EXPECT_NEAR(obj.get_timestamp(), 1747763255092, 1);
    std::string proj_string = "+proj=tmerc +lat_0=38.9549921700 +lon_0=-77.1492095300 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs +axis=enu";
    EXPECT_EQ( obj.get_projString(), proj_string);
}

TEST(FLIRPedestrianPresenceTrackingProcessorTest, processPedestrianPresenceTrackingObjectsTimestamp)
{
    std::string json = R"(
            {
            "dataNumber": "203438",
            "messageType": "Data",
            "time": "2025-05-20T19:56:43.461-04:00",
            "track": [
                {
                "angle": "52.00000000",
                "class": "Pedestrian",
                "iD": "2923009",
                "latitude": "38.95504334",
                "longitude": "-77.14948274",
                "speed": "7.70469952",
                "x": "0.46391721",
                "y": "31.70375721"
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
        90.0, 
        "North",
        "api/subscription",
        tmx::utils::WGS84Point(38.95499217, -77.14920953, 0.0) // Example reference point
    ); 
    std::queue<tmx::messages::SensorDetectedObject> msgQueue = flirSession->processPedestrianPresenceTrackingObjects(pr);

    EXPECT_EQ(msgQueue.size(), 1);
    tmx::messages::SensorDetectedObject obj = msgQueue.front();
    EXPECT_EQ(flirSession->getNumLastDetections(), 1);
    EXPECT_EQ(flirSession->getLastDetectionTime(), 1747785403461);
    EXPECT_NEAR(obj.get_timestamp(), 1747785403461, 1);
    std::string proj_string = "+proj=tmerc +lat_0=38.9549921700 +lon_0=-77.1492095300 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs +axis=enu";
    EXPECT_EQ( obj.get_projString(), proj_string);
}


TEST(FLIRWebsockAsyncClnSessionTest, checkStreamTimingQueuesStalls)
{
    auto makeFrame = [](const std::string &time) {
        std::stringstream ss("{\"dataNumber\": \"1\", \"messageType\": \"Data\", \"time\": \"" + time +
            "\", \"track\": [], \"type\": \"PedestrianPresenceTracking\"}");
        boost::property_tree::ptree pr;
        boost::property_tree::read_json(ss, pr);
        return pr;
    };
    net::io_context ioc;
    const tmx::utils::WGS84Point refPoint(38.95499217, -77.14920953, 0.0);
    auto flirSession = std::make_shared<FLIRWebsockAsyncClnSession>(ioc, "127.0.0.1", "8084", 90.0, "North",
        "api/subscription", refPoint);

    // The last on-time frame before a stall recorded in the field and the first held frame. The camera's time
    // field is local time (UTC-4); the receive times are UTC.
    flirSession->checkStreamTiming(makeFrame("2026-09-15T13:47:10.032-04:00"), 1789494430036ULL);
    flirSession->checkStreamTiming(makeFrame("2026-09-15T13:47:10.132-04:00"), 1789494433619ULL);
    // A frame whose time cannot be parsed is skipped without throwing
    EXPECT_NO_THROW(flirSession->checkStreamTiming(makeFrame("not a time"), 1789494433620ULL));

    const auto stalls = flirSession->takeStalls();
    ASSERT_EQ(stalls.size(), 1u);
    EXPECT_EQ(stalls[0].startMs, 1789494430136ULL);
    EXPECT_EQ(stalls[0].endMs, 1789494433619ULL);
    EXPECT_EQ(stalls[0].holdMs(), 3483);
    // Taking the stalls clears them
    EXPECT_TRUE(flirSession->takeStalls().empty());

    // With a 5 s threshold the same frames are not a stall
    auto lenientSession = std::make_shared<FLIRWebsockAsyncClnSession>(ioc, "127.0.0.1", "8084", 90.0, "North",
        "api/subscription", refPoint, 5000);
    lenientSession->checkStreamTiming(makeFrame("2026-09-15T13:47:10.032-04:00"), 1789494430036ULL);
    lenientSession->checkStreamTiming(makeFrame("2026-09-15T13:47:10.132-04:00"), 1789494433619ULL);
    EXPECT_TRUE(lenientSession->takeStalls().empty());
}

TEST(FLIRWebsockAsyncClnSessionTest, lastReceiveCountsPings)
{
    net::io_context ioc;
    tcp::acceptor acceptor(ioc, tcp::endpoint(net::ip::make_address("127.0.0.1"), 0));
    // A camera that accepts the subscription, then sends nothing but a WebSocket ping, like an idle FLIR camera
    std::thread camera([&acceptor] {
        websocket::stream<tcp::socket> ws(acceptor.accept());
        ws.accept();
        beast::flat_buffer request;
        ws.read(request);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        ws.ping({});
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        beast::error_code ec;
        ws.close(websocket::close_code::normal, ec);
    });

    auto flirSession = std::make_shared<FLIRWebsockAsyncClnSession>(ioc, "127.0.0.1",
        std::to_string(acceptor.local_endpoint().port()), 90.0, "North", "/api/subscriptions",
        tmx::utils::WGS84Point(38.95499217, -77.14920953, 0.0));
    EXPECT_EQ(flirSession->getLastReceiveMs(), 0u);
    flirSession->run();
    std::thread io([&ioc] { ioc.run(); });

    // Set when the connection is established
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    const auto connectedMs = flirSession->getLastReceiveMs();
    EXPECT_NE(connectedMs, 0u);
    // Updated by the ping alone
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_GE(flirSession->getLastReceiveMs(), connectedMs + 250);

    camera.join();
    ioc.stop();
    io.join();
}
