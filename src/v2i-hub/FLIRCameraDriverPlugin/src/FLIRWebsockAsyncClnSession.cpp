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
#include "FLIRWebsockAsyncClnSession.hpp"

#include <chrono>

using namespace tmx::utils;
using namespace std;
using namespace boost::property_tree;

namespace FLIRCameraDriverPlugin
{
    namespace
    {
        std::string toString(const std::optional<uint64_t> &value)
        {
            return value ? std::to_string(*value) : std::string("?");
        }

        bool isPowerOfTen(unsigned int value)
        {
            while (value >= 10 && value % 10 == 0)
            {
                value /= 10;
            }
            return value == 1;
        }
    }

    void FLIRWebsockAsyncClnSession::fail(beast::error_code ec, const std::string& what)
    {
        isHealthy_.store(false);
        // Report any backlog that was still arriving when the connection failed
        if (auto burst = streamMonitor_.flush())
        {
            logLateBurst(*burst);
        }
        if (stopping_.load())
        {
            PLOG(logDEBUG) << "FLIR session " << sensorId << " stopped (" << what << ": " << ec.message() << ")";
            return;
        }

        const uint64_t nowMs = wallClockMs();
        if (receivedOnConnection_)
        {
            // A connection that was delivering data was lost
            receivedOnConnection_ = false;
            failedAttempts_ = 0;
            linkLostMs_ = lastMessageMs_;
            PLOG(logWARNING) << "FLIR link to " << sensorId << " lost at " << FLIRStreamMonitor::formatUtcMs(nowMs) << " ("
                << what << ": " << ec.message() << "), " << (nowMs - lastMessageMs_) << " ms after the last message. Reconnecting.";
        }
        else
        {
            failedAttempts_++;
            // Log the 1st, 10th, 100th, ... failed attempt so a long outage does not flood the log
            if (isPowerOfTen(failedAttempts_))
            {
                PLOG(logWARNING) << "Unable to connect to FLIR camera " << sensorId << " at " << host_ << ":" << port_
                    << " (" << what << ": " << ec.message() << "), " << failedAttempts_ << " failed attempt(s). Retrying.";
            }
            else
            {
                PLOG(logDEBUG) << "Unable to connect to FLIR camera " << sensorId << " (" << what << ": " << ec.message()
                    << "), attempt " << failedAttempts_ << ".";
            }
        }
        // The first attempt after losing a working connection starts at once; after a failed attempt, wait a little
        scheduleReconnect(failedAttempts_ == 0 ? std::chrono::milliseconds(0) : reconnectDelay);
    }

    void FLIRWebsockAsyncClnSession::scheduleReconnect(std::chrono::milliseconds delay)
    {
        reconnectTimer_.expires_after(delay);
        reconnectTimer_.async_wait([self = shared_from_this()](beast::error_code ec)
        {
            if (ec || self->stopping_.load())
            {
                return;
            }
            // A websocket stream cannot be reused after a failure: close the old one and start over with a new one
            beast::get_lowest_layer(*self->ws_).close();
            self->ws_.emplace(self->strand_);
            self->buffer_.clear();
            self->run();
        });
    }

    websocket::stream_base::timeout FLIRWebsockAsyncClnSession::linkTimeouts() const
    {
        auto timeouts = websocket::stream_base::timeout::suggested(beast::role_type::client);
        const auto linkTimeout = std::chrono::milliseconds(linkTimeoutMs_.load());
        if (linkTimeout.count() > 0)
        {
            timeouts.handshake_timeout = linkTimeout;
            if (cameraAnswersPings_)
            {
                // Link watchdog. When nothing has arrived for linkTimeout / 2, Beast pings the camera; if nothing at
                // all, not even the reply, arrives within linkTimeout, the read fails with a timeout. An idle camera
                // (nothing tracked) answers the ping, so it is not mistaken for a stall. During a link outage or a TCP
                // retransmission stall the reply cannot arrive, because it is queued behind the frames being held.
                timeouts.idle_timeout = linkTimeout;
                timeouts.keep_alive_pings = true;
            }
        }
        return timeouts;
    }

    std::chrono::milliseconds FLIRWebsockAsyncClnSession::connectTimeout() const
    {
        const auto linkTimeout = std::chrono::milliseconds(linkTimeoutMs_.load());
        if (linkTimeout.count() <= 0)
        {
            return std::chrono::milliseconds(30000);
        }
        if (lastConnectDuration_.count() < 0)
        {
            return linkTimeout;
        }
        return std::min(std::max(lastConnectDuration_ * 10, std::chrono::milliseconds(250)), linkTimeout);
    }

    void FLIRWebsockAsyncClnSession::run()
    {
        PLOG(logDEBUG) << "Starting FLIRWebsockAsyncClnSession at " << host_ << ":" << port_;
        // Look up the domain name
        resolver_.async_resolve(
            host_,
            port_,
            beast::bind_front_handler(
                &FLIRWebsockAsyncClnSession::on_resolve,
                shared_from_this()));
    }

    void FLIRWebsockAsyncClnSession::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
    {
        if(ec)
            return fail(ec, "resolve");
        // Limit the TCP connect, see connectTimeout()
        connectStart_ = std::chrono::steady_clock::now();
        beast::get_lowest_layer(*ws_).expires_after(connectTimeout());

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(*ws_).async_connect(
            results,
            beast::bind_front_handler(
                &FLIRWebsockAsyncClnSession::on_connect,
                 shared_from_this()));
     }


    void FLIRWebsockAsyncClnSession::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
    {
        if(ec)
            return fail(ec, "connect");
        lastConnectDuration_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - connectStart_);
        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        beast::get_lowest_layer(*ws_).expires_never();

        connectedAt_ = std::chrono::steady_clock::now();
        ws_->set_option(linkTimeouts());
        ws_->control_callback([this](websocket::frame_type kind, beast::string_view)
        {
            if (kind == websocket::frame_type::pong && !cameraAnswersPings_)
            {
                cameraAnswersPings_ = true;
                ws_->set_option(linkTimeouts());
                PLOG(logINFO) << "FLIR camera " << sensorId << " answers WebSocket pings, link watchdog active.";
            }
        });

        // Set a decorator to change the User-Agent of the handshake
        ws_->set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-async");
            }));

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4

        PLOG(logDEBUG) << "host_: " << host_ << "hostString_: " << endpoint_;

        // Perform the websocket handshake
        ws_->async_handshake(host_, endpoint_,
            beast::bind_front_handler(
                &FLIRWebsockAsyncClnSession::on_handshake,
                shared_from_this()));
    }

    void FLIRWebsockAsyncClnSession::on_handshake(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "handshake");
        if (linkTimeoutMs_.load() > 0 && !cameraAnswersPings_)
        {
            // Find out whether the camera answers pings, which the watchdog relies on. Sent before the subscription,
            // so the reply normally arrives before the subscription response and the watchdog is armed from then on.
            ws_->async_ping({}, [self = shared_from_this()](beast::error_code) {});
        }
        // Send the message
        ws_->async_write(
            net::buffer(pedPresenceTrackingReq),
            beast::bind_front_handler(
                &FLIRWebsockAsyncClnSession::on_write,
                shared_from_this()));
    }

    void FLIRWebsockAsyncClnSession::on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");
        // Read a message into our buffer,
        // this should be the response to the subscribe message
        ws_->async_read(
            buffer_,
            beast::bind_front_handler(
                &FLIRWebsockAsyncClnSession::on_read,
                shared_from_this()));
    }

    void FLIRWebsockAsyncClnSession::on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");
        // Arrival time of this frame, taken before any processing
        const uint64_t recvMs = wallClockMs();
        isHealthy_.store(true);
        lastMessageMs_ = recvMs;
        if (!receivedOnConnection_)
        {
            receivedOnConnection_ = true;
            if (linkLostMs_ != 0)
            {
                reconnects_++;
                PLOG(logWARNING) << "FLIR link to " << sensorId << " restored at " << FLIRStreamMonitor::formatUtcMs(recvMs)
                    << ": no messages for " << (recvMs - linkLostMs_) << " ms, " << failedAttempts_
                    << " failed reconnect attempt(s).";
                linkLostMs_ = 0;
            }
            else
            {
                PLOG(logINFO) << "FLIR link to " << sensorId << " connected"
                    << (failedAttempts_ > 0 ? " after " + std::to_string(failedAttempts_) + " failed attempt(s)." : ".");
            }
            failedAttempts_ = 0;
        }
        if (linkTimeoutMs_.load() > 0 && !cameraAnswersPings_ && !noPingReplyLogged_ &&
            std::chrono::steady_clock::now() - connectedAt_ > std::chrono::milliseconds(linkTimeoutMs_.load()))
        {
            noPingReplyLogged_ = true;
            PLOG(logWARNING) << "FLIR camera " << sensorId << " has not answered a WebSocket ping within LinkTimeoutMs, "
                << "so a stalled link cannot be told apart from an idle camera: the link watchdog is off for this camera "
                << "until it answers one.";
        }

        std::string text(beast::buffers_to_string(buffer_.data()));
        PLOG(logDEBUG) << "Received: " << text;
        // A malformed message must not end the read loop (an exception here would escape the I/O thread)
        try
        {
            std::stringstream ss(text);
            ptree pr;
            pt::read_json(ss, pr);
            const auto messageType = pr.get<std::string>("messageType");

            if (messageType == "Subscription")
            {
                handleSubscriptionMessage(pr);
            }
            else if (messageType == "Data")
            {
                checkStreamTiming(pr, recvMs);
                handleDataMessage(pr);
            }
            else
            {
                PLOG(logDEBUG) << "Received unknown message: " << text;
            }
        }
        catch (const std::exception &e)
        {
            PLOG(logERROR) << "Unable to process message from FLIR camera " << sensorId << ": " << e.what();
        }

        // need to clear the buffer after reading the message
        buffer_.consume(buffer_.size());

        // this will read data subscribed to
        ws_->async_read(
        buffer_,
        beast::bind_front_handler(
            &FLIRWebsockAsyncClnSession::on_read,
            shared_from_this()));
    }

    void FLIRWebsockAsyncClnSession::handleSubscriptionMessage(const pt::ptree& pr) const
    {
        /* Example FLIR subscription response json:
        Received:  {"messageType": "Subscription", "subscription": {"returnValue": "OK", "type": "Data"}}
        */

        processSubscriptionMessage(pr);
    }

    std::queue<tmx::messages::SensorDetectedObject> FLIRWebsockAsyncClnSession::processPedestrianPresenceTrackingObjects(const boost::property_tree::ptree& pr)
    {
        std::queue<tmx::messages::SensorDetectedObject> msgQueue;
        uint64_t timestamp = timeStringParser(pr.get_child("time").data());
        for (const auto& [key, value] : pr.get_child("track"))
        {
            try {
            // Process the pedestrian presence tracking object
                tmx::messages::SensorDetectedObject obj = processPedestrianPresenceTrackingObject(value, timestamp, cameraRotation_, sensorId, sensorRefPoint);
                msgQueue.push(obj);
            } 
            catch (const FLIRCameraDriverException& e) {
                std::stringstream ss;
                boost::property_tree::write_json(ss, value);
                FILE_LOG(tmx::utils::LogLevel::logERROR) << "Skipping track " << ss.str() <<"! Error processing pedestrian presence tracking object: " << e.what();
                // Increment dropped pedestrian count
                droppedPedCount_.fetch_add(1);
            }
            
        }
        // Set the number of last detections
        numLastDetections_.store(static_cast<unsigned int>(msgQueue.size()));
        // Set the last detection time
        lastDetectionTime_.store(timestamp);
        return msgQueue;
    }

    void FLIRWebsockAsyncClnSession::handleDataMessage(const pt::ptree& pr)
    {
        /* Example received pedestrian tracking data:
        {"dataNumber": "473085", "messageType": "Data", "time": "2022-04-20T15:25:51.001-04:00",
        "track": [{"angle": "263.00000000", "class": "Pedestrian", "iD": "15968646", "latitude": "38.95499217",
        "longitude": "-77.14920953", "speed": "1.41873741", "x": "0.09458912", "y": "14.80903757"}], "type": "PedestrianPresenceTracking"}
        */
       
        auto newDetections = processPedestrianPresenceTrackingObjects(pr);
        std::scoped_lock lock{_msgLock};
        while( !newDetections.empty() )
        {
            auto obj = newDetections.front();
            newDetections.pop();
            msgQueue.push(obj);
        }
       

    }

    void FLIRWebsockAsyncClnSession::on_close(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "close");
        isHealthy_.store(false);
        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }

    std::queue<tmx::messages::SensorDetectedObject> FLIRWebsockAsyncClnSession::getMsgQueue()
    {
        std::scoped_lock lock{_msgLock};
        return msgQueue;
    }

    void FLIRWebsockAsyncClnSession::clearMsgQueue()
    {
        std::scoped_lock lock{_msgLock};
        msgQueue = std::queue<tmx::messages::SensorDetectedObject>();
    }

    bool FLIRWebsockAsyncClnSession::isHealthy() const{
        return isHealthy_.load();
    }
    /**
     * @brief Get the dropped pedestrian count
     */
    unsigned int FLIRWebsockAsyncClnSession::getDroppedPedCount() const {
        return droppedPedCount_.load();
    }
    /**
     * @brief Clear the dropped pedestrian count
     */
    void FLIRWebsockAsyncClnSession::clearDroppedPedCount() {
        droppedPedCount_.store(0);
    }

    std::string FLIRWebsockAsyncClnSession::getSensorId() const {
        return sensorId;
    }

    long long FLIRWebsockAsyncClnSession::getLastDetectionTime() const {
        return lastDetectionTime_.load();
    }   

    unsigned int FLIRWebsockAsyncClnSession::getNumLastDetections() const {
        return numLastDetections_.load();
    }

    uint64_t FLIRWebsockAsyncClnSession::wallClockMs()
    {
        return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    }

    void FLIRWebsockAsyncClnSession::checkStreamTiming(const pt::ptree& pr, uint64_t recvMs)
    {
        try
        {
            const uint64_t cameraMs = timeStringParser(pr.get<std::string>("time"));
            std::optional<uint64_t> dataNumber;
            if (auto dataNumberStr = pr.get_optional<std::string>("dataNumber"))
            {
                try
                {
                    dataNumber = std::stoull(*dataNumberStr);
                }
                catch (const std::exception &)
                {
                    // Leave the sequence number unknown; the timing checks still apply
                }
            }

            const auto report = streamMonitor_.onFrame(recvMs, cameraMs, dataNumber);

            // A burst that ended before this frame is reported first, to keep the log in time order
            if (report.completedBurst)
            {
                logLateBurst(*report.completedBurst);
            }

            if (report.baselineShift)
            {
                PLOG(logINFO) << "FLIR baseline latency on " << sensorId << " moved to " << report.latencyMs
                    << " ms at " << FLIRStreamMonitor::formatUtcMs(report.recvMs) << ": frames stayed at that latency for "
                    << streamMonitor_.getConfig().baselineWindowMs << " ms, so the camera/host clock offset changed. "
                    << "Frames counted as late since the change were not a backlog and are no longer counted.";
            }

            if (report.hold == FrameHoldType::Stall)
            {
                // Keep the first sentence in this format: scripts/flir_stream_probe.py analyze parses it
                PLOG(logWARNING) << "FLIR link stall on " << sensorId << ": no data for " << report.arrivalGapMs
                    << " ms while the camera kept producing frames " << report.cameraGapMs
                    << " ms apart, so frames were held " << report.holdMs << " ms in transit. Stall from "
                    << FLIRStreamMonitor::formatUtcMs(report.prevRecvMs) << " to " << FLIRStreamMonitor::formatUtcMs(report.recvMs)
                    << ", dataNumber " << toString(report.prevDataNumber) << " -> " << toString(report.dataNumber)
                    << " (" << report.missingFrames << " missing), latency of the first frame after the stall "
                    << report.latencyMs << " ms."
                    << (report.cameraGapMs > streamMonitor_.getConfig().trackGapMs
                        ? " The camera was idle (nothing tracked) before this frame, so the link was already down "
                          "when tracking resumed."
                        : "");
            }
            else if (report.hold == FrameHoldType::Minor)
            {
                PLOG(logINFO) << "FLIR link minor hold on " << sensorId << ": frame held " << report.holdMs
                    << " ms in transit at " << FLIRStreamMonitor::formatUtcMs(report.recvMs) << ", dataNumber "
                    << toString(report.dataNumber) << ". About 200 ms usually means one TCP retransmission.";
            }

            if (report.missingFrames > 0 && report.hold != FrameHoldType::Stall)
            {
                PLOG(logDEBUG) << "FLIR camera frames missing on " << sensorId << ": dataNumber "
                    << toString(report.prevDataNumber) << " -> " << toString(report.dataNumber) << " ("
                    << report.missingFrames << " missing) at " << FLIRStreamMonitor::formatUtcMs(report.recvMs) << ".";
            }
        }
        catch (const std::exception &e)
        {
            PLOG(logDEBUG) << "Unable to check stream timing for " << sensorId << ": " << e.what();
        }
    }

    void FLIRWebsockAsyncClnSession::checkStreamIdle()
    {
        if (auto burst = streamMonitor_.closeIfIdle(wallClockMs()))
        {
            logLateBurst(*burst);
        }
    }

    void FLIRWebsockAsyncClnSession::logLateBurst(const LateBurstSummary &burst) const
    {
        PLOG(logWARNING) << "FLIR late burst on " << sensorId << ": " << burst.frames
            << " frame(s) arrived at least " << streamMonitor_.getConfig().stallThresholdMs
            << " ms later than normal (worst " << burst.maxExcessLatencyMs << " ms). Received from "
            << FLIRStreamMonitor::formatUtcMs(burst.firstRecvMs) << " to " << FLIRStreamMonitor::formatUtcMs(burst.lastRecvMs)
            << " (" << (burst.lastRecvMs - burst.firstRecvMs) << " ms), produced by the camera from "
            << FLIRStreamMonitor::formatUtcMs(burst.firstCameraMs) << " to " << FLIRStreamMonitor::formatUtcMs(burst.lastCameraMs)
            << ", dataNumber " << toString(burst.firstDataNumber) << " -> " << toString(burst.lastDataNumber)
            << ". Detections this late are likely discarded downstream as stale.";
    }

    StreamMonitorStats FLIRWebsockAsyncClnSession::getStreamStats() const
    {
        return streamMonitor_.getStats();
    }

    StreamMonitorConfig FLIRWebsockAsyncClnSession::getStreamMonitorConfig() const
    {
        return streamMonitor_.getConfig();
    }

    void FLIRWebsockAsyncClnSession::setStreamMonitorConfig(const StreamMonitorConfig &config)
    {
        streamMonitor_.setConfig(config);
    }

    void FLIRWebsockAsyncClnSession::setLinkTimeoutMs(int64_t timeoutMs)
    {
        linkTimeoutMs_.store(timeoutMs);
    }

    int64_t FLIRWebsockAsyncClnSession::getLinkTimeoutMs() const
    {
        return linkTimeoutMs_.load();
    }

    uint64_t FLIRWebsockAsyncClnSession::getReconnectCount() const
    {
        return reconnects_.load();
    }

    void FLIRWebsockAsyncClnSession::stop()
    {
        stopping_.store(true);
        net::post(strand_, [self = shared_from_this()]
        {
            self->reconnectTimer_.cancel();
            if (self->ws_)
            {
                beast::get_lowest_layer(*self->ws_).close();
            }
        });
    }
}
