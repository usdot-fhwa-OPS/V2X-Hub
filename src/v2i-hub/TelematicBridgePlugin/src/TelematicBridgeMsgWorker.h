#pragma once
#include <tmx/messages/routeable_message.hpp>
#include "jsoncpp/json/json.h"


namespace TelematicBridge
{
    
    /**
     * @brief create JSON payload from given IVP message
     * @param IVPMessage V2xHub interval exchanged message
     * @return JSON value
     */
    Json::Value IvpMessageToJson(tmx::routeable_message &msg)
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
} // TelematicBridge