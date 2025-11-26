#include "MessageLogger.hpp"

namespace JSONMessageLoggerPlugin{
    std::string routeableMessageToJsonString(tmx::routeable_message &message) {
        // Convert routeable message to J2735 encoded message
        tmx::messages::TmxJ2735EncodedMessage<tmx::messages::MessageFrameMessage> rMsg = 
            message.get_payload<tmx::messages::TmxJ2735EncodedMessage<tmx::messages::MessageFrameMessage>>();
        // Decode Encode J2735 Message
        auto j2735Data = rMsg.decode_j2735_message().get_j2735_data();
        // Convert J2735 data to TmxJ2735Message for JSON serialization
        auto j2735Message = tmx::messages::TmxJ2735Message<MessageFrame_t, tmx::JSON>(j2735Data);
        // Serial J2735 message to JSON
        std::string jsonPayloadStr = j2735Message.to_string();
        // Get timestamp
        auto current_time = std::chrono::system_clock::now();
        std::string timestamp = tmx::utils::Clock::ToUtcPreciseTimeString(current_time);
        return timestamp + ": " + jsonPayloadStr;
    }

    void logRouteableMessage( tmx::routeable_message & msg, boost::log::sources::severity_channel_logger< boost::log::trivial::severity_level , std::string>& logger ) {
        std::string jsonPayloadStr;
        jsonPayloadStr = routeableMessageToJsonString(msg);
        if ( !jsonPayloadStr.empty() ) {
            BOOST_LOG_SEV(logger, boost::log::trivial::info) << jsonPayloadStr;
        }
        else {
            throw tmx::TmxException("Conversion of msg " + msg.to_string() + " to JSON resulted in empty string failed!");
        }
    }
}