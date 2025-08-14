#include "TelematicBridgeMsgWorker.h"

namespace TelematicBridge
{
    
    /**
     * @brief Create Json::Value from a rtmx::routeable_message
     * @param msg tmx::routeable_message
     * @return JSON value
     */
    Json::Value routeableMessageToJsonValue(tmx::routeable_message &msg)
    {
        Json::Value json;
        json["type"] = msg.get_type();
        json["subType"] = msg.get_subtype();
        json["source"] = msg.get_source();
        json["sourceId"] = msg.get_sourceId();
        json["flags"] = msg.get_flags();
        json["timestamp"] = msg.get_timestamp();
        json["channel"] = msg.get_dsrcChannel();
        json["psid"] = msg.get_dsrcPsid();
        json["encoding"] = msg.get_encoding();
        json["payload"] = msg.get_payload_str();
        return json;
    }
    /**
     * @brief Servialize a J2735 routeable message into JSON using
     * stol-j2735 library JER encoding functionality.
     * @note: This function assumes the input routeable_message is a J2735 encoded message.
     * @param msg tmx::routeable_message
     * @return JSON string of the J2735 message payload 
     */
    std::string j2735MessageToJson(tmx::routeable_message &msg)
    {
        // Convert routeable message to J2735 encoded message
        tmx::messages::TmxJ2735EncodedMessage<tmx::messages::MessageFrameMessage> rMsg = 
            msg.get_payload<tmx::messages::TmxJ2735EncodedMessage<tmx::messages::MessageFrameMessage>>();
        // Decode Encode J2735 Message
        auto j2735Data = rMsg.decode_j2735_message().get_j2735_data();
        // Convert J2735 data to TmxJ2735Message for JSON serialization
        auto j2735Message = tmx::messages::TmxJ2735Message<MessageFrame_t, tmx::JSON>(j2735Data);
        // Serial J2735 message to JSON
        std::string json_payload_str = j2735Message.to_string();
        FILE_LOG(tmx::utils::LogLevel::logDEBUG2) << "J2735 JSON payload: " << json_payload_str;
        // Free the J2735 data structure
        ASN_STRUCT_FREE(asn_DEF_MessageFrame, j2735Data.get());
        return json_payload_str;
    }
    /**
     * @brief Convert a JSON string to a Json::Value to include in an larger JSON object.
     * @param str The JSON string to convert
     * @return Json::Value containing the parsed JSON data
     * @throws
     */
    Json::Value stringToJsonValue(const std::string &str) {
        // Create a Json::CharReaderBuilder and Json::CharReader to parse the string
        Json::CharReaderBuilder builder;
        Json::Value parsedParam;
        std::string errs;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        Json::Value json;
        // Parse the JSON string
        if (reader->parse(str.data(), str.data() + str.size(), &parsedParam, &errs)) {
            json = parsedParam;
        } else {
            throw TelematicBridgeException("Failed to parse JSON string: " + str + " with errors: " + errs);
        }
        return json;
    }
} // TelematicBridge