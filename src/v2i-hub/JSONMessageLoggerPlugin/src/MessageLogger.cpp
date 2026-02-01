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
        uint64_t timestamp_ms = message.get_timestamp();

        return  std::to_string(timestamp_ms) + " : "+ jsonPayloadStr;
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

    bool isBSM(const std::string& jsonText) {
        Json::CharReaderBuilder builder;
        builder["collectComments"] = false;

        Json::Value root;
        std::string errs;

        const auto* begin = jsonText.data();
        const auto* end   = begin + jsonText.size();

        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(begin, end, &root, &errs)) {
            // handle parse error (log errs if useful)
            return false;
        }

        return root.isMember("header")
            && root["header"].isObject()
            && root["header"].get("subtype", "").asString() == "BSM";
    }

    bool logRouteableMessage2( tmx::routeable_message & msg, boost::log::sources::severity_channel_logger< boost::log::trivial::severity_level , std::string>& logger ) {
        std::string jsonPayloadStr;
        jsonPayloadStr = routeableMessageToJsonString(msg);
        if ( !jsonPayloadStr.empty() ) {
            BOOST_LOG_SEV(logger, boost::log::trivial::info) << jsonPayloadStr;
        }
        else {
            throw tmx::TmxException("Conversion of msg " + msg.to_string() + " to JSON resulted in empty string failed!");
        }

        if (isBSM(jsonPayloadStr)){
            return true;
        }
        return false;
    }
}