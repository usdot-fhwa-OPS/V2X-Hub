#pragma once
#include <tmx/messages/routeable_message.hpp>
#include <tmx/messages/TmxJ2735.hpp>
#include <tmx/messages/TmxJ2735Codec.hpp>
#include <PluginLog.h>
#include "TelematicBridgeException.h"
#include "jsoncpp/json/json.h"
#include <RSURegistrationConfigMessage.h>


namespace TelematicBridge
{

    /**
     * @brief Create Json::Value from a rtmx::routeable_message
     * @param msg tmx::routeable_message
     * @return JSON value
     */
    Json::Value routeableMessageToJsonValue(tmx::routeable_message &msg);

    /**
     * @brief Create Json::Value from a rtmx::routeable_message
     * @param msg tmx::routeable_message
     * @return JSON value
     */
    bool jsonValueToRouteableMessage(const Json::Value& json, tmx::messages::RSURegistrationConfigMessage& msg);
    /**
     * @brief Servialize a J2735 routeable message into JSON using
     * stol-j2735 library JER encoding functionality.
     * @note: This function assumes the input routeable_message is a J2735 encoded message.
     * @param msg tmx::routeable_message
     * @return JSON string of the J2735 message payload
     */
    std::string j2735MessageToJson(tmx::routeable_message &msg);
    /**
     * @brief Convert a JSON string to a Json::Value to include in an larger JSON object.
     * @param str The JSON string to convert
     * @return Json::Value containing the parsed JSON data
     * @throws
     */
    Json::Value stringToJsonValue(const std::string &str);

    /**
     * @brief Convert a Json::Value to a JSON string.
     * @param json The Json::Value to convert
     * @return JSON string containing the parsed JSON data
     * @throws
     */
    std::string jsonValueToString(const Json::Value& json);

    /**
     * @brief Trim leading and ending spaces from a given string.
     * @param str string value to be trimmed.
     * @return Trimmed string value.
     */
    std::string trim(std::string str);
} // TelematicBridge