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
#include <array>
#include <ctime>
#include <functional>
#include <thread>

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

TEST(FLIRWebsockAsyncClnSessionTest, checkStreamTimingDetectsStall)
{
    auto makeFrame = [](const std::string &dataNumber, const std::string &time) {
        std::stringstream ss("{\"dataNumber\": \"" + dataNumber + "\", \"messageType\": \"Data\", \"time\": \"" + time +
            "\", \"track\": [], \"type\": \"PedestrianPresenceTracking\"}");
        boost::property_tree::ptree pr;
        boost::property_tree::read_json(ss, pr);
        return pr;
    };
    net::io_context ioc;
    auto flirSession = std::make_shared<FLIRWebsockAsyncClnSession>(
        ioc,
        "127.0.0.1",
        "8084",
        90.0,
        "North",
        "api/subscription",
        tmx::utils::WGS84Point(38.95499217, -77.14920953, 0.0)
    );

    // Frames received around a real stall (2026-09-15 17:47:10 UTC): nothing arrived for 3.58 s
    // although the camera produced dataNumber 50652 only 100 ms after 50650.
    flirSession->checkStreamTiming(makeFrame("50645", "2026-09-15T13:47:09.529-04:00"), 1789494429532ULL);
    flirSession->checkStreamTiming(makeFrame("50646", "2026-09-15T13:47:09.629-04:00"), 1789494429634ULL);
    flirSession->checkStreamTiming(makeFrame("50647", "2026-09-15T13:47:09.730-04:00"), 1789494429733ULL);
    flirSession->checkStreamTiming(makeFrame("50648", "2026-09-15T13:47:09.831-04:00"), 1789494429837ULL);
    flirSession->checkStreamTiming(makeFrame("50649", "2026-09-15T13:47:09.932-04:00"), 1789494429936ULL);
    flirSession->checkStreamTiming(makeFrame("50650", "2026-09-15T13:47:10.032-04:00"), 1789494430036ULL);
    flirSession->checkStreamTiming(makeFrame("50652", "2026-09-15T13:47:10.132-04:00"), 1789494433619ULL);

    auto stats = flirSession->getStreamStats();
    EXPECT_EQ(stats.frames, 7u);
    EXPECT_EQ(stats.stalls, 1u);
    EXPECT_EQ(stats.maxStallMs, 3483);
    EXPECT_EQ(stats.missingFrames, 1u);
    EXPECT_EQ(stats.lateFrames, 1u);
    EXPECT_EQ(stats.lastStallStartMs, 1789494430036ULL);
    EXPECT_EQ(stats.lastStallEndMs, 1789494433619ULL);

    // A frame with an unreadable timestamp is skipped without throwing
    EXPECT_NO_THROW(flirSession->checkStreamTiming(makeFrame("50653", "not a time"), 1789494433620ULL));
    EXPECT_EQ(flirSession->getStreamStats().frames, 7u);

    // The threshold can be changed at runtime
    auto config = flirSession->getStreamMonitorConfig();
    config.stallThresholdMs = 1000;
    flirSession->setStreamMonitorConfig(config);
    EXPECT_EQ(flirSession->getStreamMonitorConfig().stallThresholdMs, 1000);
}

namespace
{
    /**
     * @brief Minimal FLIR camera for link tests: accepts WebSocket connections, answers the subscription and then
     * runs the behavior given for that connection, each connection on its own thread.
     */
    class TestCamera
    {
        public:
            using Connection = websocket::stream<tcp::socket>;
            using Behavior = std::function<void(Connection &ws, unsigned int connectionIndex)>;

            /**
             * @param answerPings False for a camera without ping support: the subscription is read as raw bytes,
             * so the ping sent with it is never answered.
             */
            explicit TestCamera(Behavior behavior, unsigned short port = 0, bool answerPings = true)
                : acceptor_(ioc_, tcp::endpoint(net::ip::make_address("127.0.0.1"), port)), behavior_(std::move(behavior)),
                  answerPings_(answerPings)
            {
                thread_ = std::thread([this] { serve(); });
            }

            ~TestCamera()
            {
                stopping_ = true;
                // Closing the acceptor does not wake a blocked accept(): connect once to wake it
                beast::error_code ec;
                tcp::socket wake(ioc_);
                wake.connect(acceptor_.local_endpoint(), ec);
                thread_.join();
                acceptor_.close(ec);
                for (auto &connectionThread : connectionThreads_)
                {
                    connectionThread.join();
                }
            }

            std::string port() const { return std::to_string(acceptor_.local_endpoint().port()); }
            unsigned int connections() const { return connections_.load(); }

            static std::string dataFrame(unsigned int dataNumber)
            {
                const auto nowMs = FLIRWebsockAsyncClnSession::wallClockMs();
                const auto seconds = static_cast<std::time_t>(nowMs / 1000);
                std::tm utc{};
                gmtime_r(&seconds, &utc);
                char time[64];
                std::snprintf(time, sizeof(time), "%04d-%02d-%02dT%02d:%02d:%02d.%03d+00:00", utc.tm_year + 1900,
                    utc.tm_mon + 1, utc.tm_mday, utc.tm_hour, utc.tm_min, utc.tm_sec, static_cast<int>(nowMs % 1000));
                return "{\"dataNumber\": \"" + std::to_string(dataNumber) + "\", \"messageType\": \"Data\", \"time\": \"" +
                    time + "\", \"track\": [{\"angle\": \"255.0\", \"class\": \"Pedestrian\", \"iD\": \"1234\", "
                    "\"latitude\": \"38.95504354\", \"longitude\": \"-77.14934177\", \"speed\": \"1.1\", \"x\": \"-2.6\", "
                    "\"y\": \"19.9\"}], \"type\": \"PedestrianPresenceTracking\"}";
            }

            static void sendFrames(Connection &ws, unsigned int count)
            {
                for (unsigned int i = 1; i <= count; i++)
                {
                    ws.write(net::buffer(dataFrame(i)));
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }

        private:
            void serve()
            {
                while (!stopping_)
                {
                    beast::error_code ec;
                    tcp::socket socket(ioc_);
                    acceptor_.accept(socket, ec);
                    if (ec || stopping_)
                    {
                        return;
                    }
                    connectionThreads_.emplace_back([this, socket = std::move(socket)]() mutable
                    {
                        try
                        {
                            Connection ws(std::move(socket));
                            ws.accept();
                            if (answerPings_)
                            {
                                beast::flat_buffer request;
                                ws.read(request);
                            }
                            else
                            {
                                std::array<char, 1024> raw;
                                ws.next_layer().read_some(net::buffer(raw));
                            }
                            ws.write(net::buffer(std::string(
                                R"({"messageType": "Subscription", "subscription": {"returnValue": "OK", "type": "Data"}})")));
                            behavior_(ws, connections_++);
                        }
                        catch (const std::exception &)
                        {
                            // Client went away
                        }
                    });
                }
            }

            net::io_context ioc_;
            tcp::acceptor acceptor_;
            Behavior behavior_;
            bool answerPings_;
            std::atomic<unsigned int> connections_{0};
            std::atomic<bool> stopping_{false};
            std::thread thread_;
            std::vector<std::thread> connectionThreads_;
    };

    /**
     * @brief Runs a session against a local camera on its own I/O thread.
     */
    struct RunningSession
    {
        net::io_context ioc;
        std::shared_ptr<FLIRWebsockAsyncClnSession> session;
        std::thread io;

        RunningSession(const std::string &port, int64_t linkTimeoutMs)
        {
            session = std::make_shared<FLIRWebsockAsyncClnSession>(ioc, "127.0.0.1", port, 90.0, "Test",
                "/api/subscriptions", tmx::utils::WGS84Point(38.95499217, -77.14920953, 0.0));
            session->setLinkTimeoutMs(linkTimeoutMs);
            session->run();
            io = std::thread([this] { ioc.run(); });
        }

        ~RunningSession()
        {
            stop();
        }

        void stop()
        {
            session->stop();
            if (io.joinable())
            {
                io.join();
            }
        }

        template <typename Condition>
        bool waitFor(Condition condition, std::chrono::milliseconds limit = std::chrono::milliseconds(5000))
        {
            const auto deadline = std::chrono::steady_clock::now() + limit;
            while (std::chrono::steady_clock::now() < deadline)
            {
                if (condition())
                {
                    return true;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            return condition();
        }
    };
}

TEST(FLIRWebsockAsyncClnSessionTest, linkTimeoutSetting)
{
    net::io_context ioc;
    auto flirSession = std::make_shared<FLIRWebsockAsyncClnSession>(ioc, "127.0.0.1", "8084", 90.0, "North",
        "api/subscription", tmx::utils::WGS84Point(38.95499217, -77.14920953, 0.0));
    EXPECT_EQ(flirSession->getLinkTimeoutMs(), 500);
    flirSession->setLinkTimeoutMs(200);
    EXPECT_EQ(flirSession->getLinkTimeoutMs(), 200);
    EXPECT_EQ(flirSession->getReconnectCount(), 0u);
    EXPECT_FALSE(flirSession->isHealthy());
}

TEST(FLIRWebsockAsyncClnSessionTest, reconnectsWhenCameraClosesConnection)
{
    // First connection: a few frames, then the camera closes it. Later connections stream normally.
    TestCamera camera([](TestCamera::Connection &ws, unsigned int connection)
    {
        if (connection == 0)
        {
            TestCamera::sendFrames(ws, 3);
            ws.close(websocket::close_code::going_away);
            return;
        }
        TestCamera::sendFrames(ws, 1000);
    });
    RunningSession running(camera.port(), 500);

    ASSERT_TRUE(running.waitFor([&] { return running.session->getReconnectCount() == 1; }));
    EXPECT_EQ(camera.connections(), 2u);
    const auto framesAfterReconnect = running.session->getStreamStats().frames;
    EXPECT_TRUE(running.waitFor([&] { return running.session->getStreamStats().frames >= framesAfterReconnect + 3; }));
    EXPECT_TRUE(running.session->isHealthy());
}

TEST(FLIRWebsockAsyncClnSessionTest, reconnectsWhenLinkStalls)
{
    // First connection: a few frames, then the camera goes silent without closing the connection and does not
    // answer pings, as during a link outage. Later connections stream normally.
    TestCamera camera([](TestCamera::Connection &ws, unsigned int connection)
    {
        if (connection == 0)
        {
            TestCamera::sendFrames(ws, 3);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            return;
        }
        TestCamera::sendFrames(ws, 1000);
    });
    RunningSession running(camera.port(), 500);

    ASSERT_TRUE(running.waitFor([&] { return running.session->getStreamStats().frames >= 3; }));
    const auto stallStart = std::chrono::steady_clock::now();
    ASSERT_TRUE(running.waitFor([&] { return running.session->getReconnectCount() == 1; }));
    // Detected about LinkTimeoutMs after the last frame, not after the 2 s the silent connection stays open
    EXPECT_LT(std::chrono::steady_clock::now() - stallStart, std::chrono::milliseconds(1500));
    EXPECT_EQ(camera.connections(), 2u);
    // Frames flow again on the new connection
    const auto framesAfterReconnect = running.session->getStreamStats().frames;
    EXPECT_TRUE(running.waitFor([&] { return running.session->getStreamStats().frames >= framesAfterReconnect + 3; }));
    EXPECT_TRUE(running.session->isHealthy());
}

TEST(FLIRWebsockAsyncClnSessionTest, idleCameraIsNotAStall)
{
    // The camera sends a few frames and then nothing (nothing tracked) but keeps reading, so it answers pings
    TestCamera camera([](TestCamera::Connection &ws, unsigned int)
    {
        TestCamera::sendFrames(ws, 3);
        beast::flat_buffer buffer;
        for (;;)
        {
            ws.read(buffer);
            buffer.consume(buffer.size());
        }
    });
    RunningSession running(camera.port(), 500);

    ASSERT_TRUE(running.waitFor([&] { return running.session->getStreamStats().frames >= 3; }));
    // Idle for six link timeouts: pings are answered, so the connection is kept
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    EXPECT_EQ(running.session->getReconnectCount(), 0u);
    EXPECT_EQ(camera.connections(), 1u);
    EXPECT_TRUE(running.session->isHealthy());
}

TEST(FLIRWebsockAsyncClnSessionTest, connectsWhenCameraComesUpLater)
{
    // Find a free port, then leave it closed so the first connection attempts are refused
    unsigned short port;
    {
        net::io_context ioc;
        tcp::acceptor probe(ioc, tcp::endpoint(net::ip::make_address("127.0.0.1"), 0));
        port = probe.local_endpoint().port();
    }
    RunningSession running(std::to_string(port), 500);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_FALSE(running.session->isHealthy());

    TestCamera camera([](TestCamera::Connection &ws, unsigned int) { TestCamera::sendFrames(ws, 1000); }, port);
    EXPECT_TRUE(running.waitFor([&] { return running.session->getStreamStats().frames >= 3; }));
    EXPECT_TRUE(running.session->isHealthy());
    // Not a reconnect: the link had never delivered data
    EXPECT_EQ(running.session->getReconnectCount(), 0u);
    // Disconnect before the camera goes away
    running.stop();
}

TEST(FLIRWebsockAsyncClnSessionTest, watchdogOffWhenCameraDoesNotAnswerPings)
{
    // A camera that never answers pings: a few frames and then idle. Silence cannot be told apart from a stall,
    // so the connection must be kept rather than dropped every time the camera goes idle.
    TestCamera camera([](TestCamera::Connection &ws, unsigned int)
    {
        TestCamera::sendFrames(ws, 3);
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }, 0, false);
    RunningSession running(camera.port(), 500);

    ASSERT_TRUE(running.waitFor([&] { return running.session->getStreamStats().frames >= 3; }));
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    EXPECT_EQ(running.session->getReconnectCount(), 0u);
    EXPECT_EQ(camera.connections(), 1u);
    running.stop();
}
