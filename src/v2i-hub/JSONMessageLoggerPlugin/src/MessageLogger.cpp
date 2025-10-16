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
        std::string json_payload_str = j2735Message.to_string();

        return json_payload_str;
       
         
    }

    void logRouteableMessage( tmx::routeable_message & msg, boost::log::sources::severity_channel_logger< boost::log::trivial::severity_level , std::string>& logger ) {
        std::string json_payload_str;
        json_payload_str = routeableMessageToJsonString(msg);
        if ( !json_payload_str.empty() ) {
            if ( msg.get_flags() & IvpMsgFlags_RouteDSRC ) {
                BOOST_LOG_SEV(logger, boost::log::trivial::info) << json_payload_str;
            }
            else {
                BOOST_LOG_SEV(logger, boost::log::trivial::info) << json_payload_str;
            }
        }
        else {
            throw tmx::TmxException("Conversion of msg " + msg.to_string() + " to JSON resulted in empty string failed!");
        }
    }
}