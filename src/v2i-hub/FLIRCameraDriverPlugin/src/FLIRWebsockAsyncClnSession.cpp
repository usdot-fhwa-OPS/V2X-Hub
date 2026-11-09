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
    }

    void FLIRWebsockAsyncClnSession::fail(beast::error_code ec, const std::string& what)
    {
        isHealthy_.store(false);
        PLOG(logERROR) << what << ": " << ec.message();
        // Report any backlog that was still arriving when the connection failed
        if (auto burst = streamMonitor_.flush())
        {
            logLateBurst(*burst);
        }
    }

    void FLIRWebsockAsyncClnSession::run()
    {
	    // Save these for later
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
        isHealthy_.store(true);
        // Set the timeout for the operation
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(ws_).async_connect(
            results,
            beast::bind_front_handler(
                &FLIRWebsockAsyncClnSession::on_connect,
                 shared_from_this()));
     }


    void FLIRWebsockAsyncClnSession::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
    {
        if(ec)
            return fail(ec, "connect");
        isHealthy_.store(true);
        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        beast::get_lowest_layer(ws_).expires_never();      

        // Set suggested timeout settings for the websocket
        ws_.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::client));

        // Set a decorator to change the User-Agent of the handshake
        ws_.set_option(websocket::stream_base::decorator(
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
        ws_.async_handshake(host_, endpoint_,
            beast::bind_front_handler(
                &FLIRWebsockAsyncClnSession::on_handshake,
                shared_from_this()));
    }

    void FLIRWebsockAsyncClnSession::on_handshake(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "handshake");
        isHealthy_.store(true);
        // Send the message
        ws_.async_write(
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
        isHealthy_.store(true);
        // Read a message into our buffer,
        // this should be the response to the subscribe message
        ws_.async_read(
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
        // parse the buffer
        std::stringstream ss;
	    ptree pr;
	    std::string text(beast::buffers_to_string(buffer_.data()));
        ss<<text;

        PLOG(logDEBUG) << "Received: " << text;

        try {
	        pt::read_json(ss, pr);
        } catch(const ptree_error &e) {
            PLOG(logERROR) << "Error converting json to p tree: " << e.what();
        }

        std::string messageType = pr.get_child("messageType").get_value<string>();

        if (messageType.compare("Subscription") == 0)
        {
            handleSubscriptionMessage(pr);
        }
        else if (messageType.compare("Data") == 0)
        {
            checkStreamTiming(pr, recvMs);
            handleDataMessage(pr);
        }
        else
        {
            PLOG(logDEBUG) << "Received unknown message: " << text.c_str();
        }

        // need to clear the buffer after reading the message
        buffer_.consume(buffer_.size());  

        // this will read data subscribed to
        ws_.async_read(
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

            if (report.hold == FrameHoldType::Stall)
            {
                PLOG(logWARNING) << "FLIR link stall on " << sensorId << ": no data for " << report.arrivalGapMs
                    << " ms while the camera kept producing frames " << report.cameraGapMs
                    << " ms apart, so frames were held " << report.holdMs << " ms in transit. Stall from "
                    << FLIRStreamMonitor::formatUtcMs(report.prevRecvMs) << " to " << FLIRStreamMonitor::formatUtcMs(report.recvMs)
                    << ", dataNumber " << toString(report.prevDataNumber) << " -> " << toString(report.dataNumber)
                    << " (" << report.missingFrames << " missing), latency of the first frame after the stall "
                    << report.latencyMs << " ms.";
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
}
