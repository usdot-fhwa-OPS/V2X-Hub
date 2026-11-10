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
#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/post.hpp>
#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <PluginLog.h>
#include <TmxLog.h>
#include <WGS84Point.h>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <queue>
#include <atomic>
#include <optional>

#include <SensorDetectedObject.h>

#include "FLIRPedestrianPresenceTrackingProcessor.hpp"
#include "FLIRStreamMonitor.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace pt = boost::property_tree;    // from <boost/property_tree/ptree.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

namespace FLIRCameraDriverPlugin
{
    // Sends a WebSocket message and prints the response
    class FLIRWebsockAsyncClnSession : public std::enable_shared_from_this<FLIRWebsockAsyncClnSession>
    {   
        private:
            // All handlers run on this strand
            net::strand<net::io_context::executor_type> strand_;
            tcp::resolver resolver_;
            // Replaced with a fresh stream for every connection attempt (a websocket stream cannot be reused)
            std::optional<websocket::stream<beast::tcp_stream>> ws_;
            // Delays the next connection attempt after a failure
            net::steady_timer reconnectTimer_;
            beast::flat_buffer buffer_;
            std::string host_;
            std::string port_;
            // The camera rotation angle in degrees from East
            double cameraRotation_;
            std::string sensorId;

            std::string endpoint_;
            std::string pedPresenceTrackingReq = R"(
                {
                    "messageType": "Subscription",
                    "subscription": {
                        "type": "Data",
                        "action": "Subscribe",
                        "inclusions": [
                        {
                            "type": "PedestrianPresenceTracking"
                        }
                        ]
                    }
                    }
                )";
            std::queue<tmx::messages::SensorDetectedObject> msgQueue;

            std::mutex _msgLock;
            std::atomic<unsigned int> numLastDetections_{0};
            std::atomic<long long> lastDetectionTime_{0};

            // Health status of the FLIR camera
            std::atomic<bool> isHealthy_;
            // Dropped pedestiran Count
            std::atomic<unsigned int> droppedPedCount_{0};
            // Reference location for the sensor. All detection positions are expressed in cartesian coordinates relative to this point.
            tmx::utils::WGS84Point sensorRefPoint;
            // Compares frame arrival times with camera timestamps to detect stalls on the camera link
            FLIRStreamMonitor streamMonitor_;

            // Link watchdog timeout (ms), see setLinkTimeoutMs. 0 disables the watchdog.
            std::atomic<int64_t> linkTimeoutMs_{500};
            // Set by stop(): no further reconnects
            std::atomic<bool> stopping_{false};
            // Successful reconnects after a connection that had been delivering data was lost
            std::atomic<uint64_t> reconnects_{0};
            // Connection state, only used on the strand
            bool receivedOnConnection_ = false;   // a message arrived on the current connection
            // The camera has answered a WebSocket ping. Until it has, silence cannot be told apart from an idle
            // camera, so the watchdog is not enforced.
            bool cameraAnswersPings_ = false;
            bool noPingReplyLogged_ = false;
            std::chrono::steady_clock::time_point connectedAt_;
            uint64_t lastMessageMs_ = 0;          // receive time of the last message (host clock)
            uint64_t linkLostMs_ = 0;             // last message before the current outage, 0 if none
            unsigned int failedAttempts_ = 0;     // connection attempts that failed since the last message
            std::chrono::steady_clock::time_point connectStart_;
            std::chrono::milliseconds lastConnectDuration_{-1};  // TCP connect time of the last connection, -1 if none

            /**
             * @brief Logs a summary of a run of frames that arrived late (the backlog delivered after a stall).
             */
            void logLateBurst(const LateBurstSummary &burst) const;

            /**
             * @brief Starts the next connection attempt after the given delay.
             */
            void scheduleReconnect(std::chrono::milliseconds delay);

            /**
             * @brief WebSocket timeouts for the current settings: the link timeout limits the handshake, and once the
             * camera has answered a ping it is also the idle timeout of the watchdog.
             */
            websocket::stream_base::timeout linkTimeouts() const;

            /**
             * @brief Time limit for a TCP connect. While the network is down, the SYN of an attempt is lost and the
             * kernel would only resend it after 1 s, 2 s, 4 s, ...; abandoning the attempt and starting a new one
             * sends a new SYN sooner, so the connection is made soon after the network comes back. The limit is 10x
             * the last successful connect time (a LAN connect takes about 1 ms), at least 250 ms and at most the link
             * timeout; until a connect has succeeded, the link timeout.
             */
            std::chrono::milliseconds connectTimeout() const;

        public:
            /**
             * @brief Pause between a failed connection attempt and the next attempt. Short, so a
             * new connection is made soon after the network comes back; failed attempts are cheap.
             */
            static constexpr std::chrono::milliseconds reconnectDelay{100};

            /**
             * @brief Current host wall-clock time in milliseconds since epoch.
             */
            static uint64_t wallClockMs();

            // Resolver and socket require an io_context
            explicit FLIRWebsockAsyncClnSession(net::io_context& ioc, const std::string &host, const std::string &port, double cameraRotation,const std::string &sensorId, const std::string& endpoint, const tmx::utils::WGS84Point& point)
                : strand_(net::make_strand(ioc)), resolver_(strand_), ws_(std::in_place, strand_), reconnectTimer_(strand_), host_(host), port_(port), cameraRotation_(cameraRotation), sensorId(sensorId), endpoint_(endpoint), sensorRefPoint(point){
                    isHealthy_.store(false);
                };

            /**
             * @brief Set the link watchdog timeout. Once the camera has answered a ping, the connection is dropped and
             * re-established when nothing (no data and no reply to a ping) has arrived from the camera for this long.
             * It also limits the WebSocket handshake and the TCP connect (see connectTimeout()). 0 disables the
             * watchdog (silence is waited out indefinitely, connection attempts limited to 30 s). Applies from the
             * next connection.
             */
            void setLinkTimeoutMs(int64_t timeoutMs);
            int64_t getLinkTimeoutMs() const;

            /**
             * @brief Number of times the connection was re-established after a connection that had been delivering
             * data was lost.
             */
            uint64_t getReconnectCount() const;

            /**
             * @brief Close the connection and stop reconnecting. Thread safe.
             */
            void stop();

            /**
             * @brief Reports a failure with any of the websocket functions below and schedules a reconnect
             * @param ec the error code for the specific function
             * @param what description of the error
             */
            void fail(beast::error_code ec, const std::string& what);

            /**
             * @brief Start the asynchronous web socket connection to the camera. Each function will call the
             * function below it.
             * @param host ip address of camera to connect to
             * @param port port to connect to
             * @param cameraRotation calculated camera rotation
             */
            void run();
            
            /**
             * @brief Lookup the domain name of the IP address from run function.
             * @param ec error code containing information describing resolve issue
             * @param results result of domain name lookup
             */
            void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
            
            /**
             * @brief Configures websocket settings and initiates handshake
             * @param ec error code containing information describing connection issue 
             */
            void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);

            /**
             * @brief Performs the websocket handshake and calls write function
             * @param ec error code containing information describing handshake issue
             */
            void on_handshake(beast::error_code ec);

            /**
             * @brief Sends the subscription request json to the camera and calls read function for camera response
             * @param ec error code containing information describing issue with json send
             * @param bytes_transferred the bytes of the json
             */
            void on_write(beast::error_code ec, std::size_t bytes_transferred);

            /**
             * @brief Used to read in all messages from the camera and parse out desired fields
             * @param ec error code containing information describing issue with reading camera data
             * @param bytes_transferred the bytes of the received camera data
             */
            void on_read(beast::error_code ec, std::size_t bytes_transferred);
            
            /**
             * @brief Closes the websocket connection to the camera
             * @param ec error code containing information describing issue with closing websocket
             */
            void on_close(beast::error_code ec);

            /**
             * @brief Get method for queue containing message(s) for all tracked pedestrians. Copies the queue into
             * a temporary queue and returns temporary queue. Clears the original queue.
             * @return The message queue.
             */
            std::queue<tmx::messages::SensorDetectedObject> getMsgQueue();
            /**
             * @brief Clears the message queue
             */
            void clearMsgQueue();

            /**
             * @brief Handles messages of type "Subscription" received from the FLIR camera
             * @param pr Property tree containing the parsed JSON message
             */
            void handleSubscriptionMessage(const pt::ptree& pr) const;

            /**
             * @brief Handles messages of type "Data" received from the FLIR camera
             * @param pr Property tree containing the parsed JSON message
             */
            void handleDataMessage(const pt::ptree& pr);

            /**
             * @brief Compares when a "Data" frame arrived with its camera timestamp and logs link stalls,
             * minor holds, missing frames and late-frame bursts. Never throws.
             * @param pr Property tree containing the parsed JSON message
             * @param recvMs Time the frame was received, in ms since epoch (host wall clock)
             */
            void checkStreamTiming(const pt::ptree& pr, uint64_t recvMs);

            /**
             * @brief Reports a late-frame burst that ended together with a track, once no frame has arrived for
             * a while. Call periodically.
             */
            void checkStreamIdle();

            /**
             * @brief Get camera link statistics since the session was created.
             */
            StreamMonitorStats getStreamStats() const;

            /**
             * @brief Get the thresholds used for camera link monitoring.
             */
            StreamMonitorConfig getStreamMonitorConfig() const;

            /**
             * @brief Set the thresholds used for camera link monitoring.
             */
            void setStreamMonitorConfig(const StreamMonitorConfig &config);

            /**
             * @brief Processes an array of pedestrian presence tracking objects from the FLIR camera.
             * @param pr The property tree containing the pedestrian presence tracking objects.
             * @param cameraRotation The rotation of the camera in degrees clockwise from North (NED).
             * @param cameraViewName The name of the camera view.
             * @return A queue of processed SensorDetectedObject messages.
             * @throws FLIRCameraDriverException if fails to parse the tracking objects.
             */
            std::queue<tmx::messages::SensorDetectedObject> processPedestrianPresenceTrackingObjects(const boost::property_tree::ptree& pr);

            /**
             * @brief Get the health status of the FLIR camera
             */
            bool isHealthy() const;

            /**
             * @brief Get the dropped pedestrian count
             */
            unsigned int getDroppedPedCount() const;
            /**
             * @brief Clear the dropped pedestrian count
             */
            void clearDroppedPedCount();

            /**
             * @brief Get the last detection time
             * @return The last detection time in milliseconds since epoch.
             * 
             */
            long long getLastDetectionTime() const;
            /**
             * @brief Get the number of last detections
             * @return The number of last detections.
             */
            unsigned int getNumLastDetections() const;

            std::string getSensorId() const;
    };
}
