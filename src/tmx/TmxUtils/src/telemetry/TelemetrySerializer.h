#pragma once
#include <vector>
#include <map>
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string.hpp>

#include <tmx/messages/message.hpp>

#include "PluginTelemetry.h"
#include "TelemetrySerializerException.h"
#include "TelemetryHeader.h"

namespace tmx::utils::telemetry{

    class TelemetrySerializer
    {
    public:
        TelemetrySerializer()=delete;
        ~TelemetrySerializer()=delete;
        /***
         * @brief Serialize PluginTelemetry object into JSON container (tmx::message_container_type)
         * @param PluginTelemetry an object of PluginTelemetry class
         * @return A JSON container of tmx::message_container_type
         */
        static tmx::message_container_type serializePluginTelemetry(const PluginTelemetry& pluginTelemetry);
        /***
         * @brief Serialize a list of PluginTelemetry object into JSON container (tmx::message_container_type)
         * @param vector<PluginTelemetry> a vector of PluginTelemetry objects
         * @return A JSON container of tmx::message_container_type
         */
        static tmx::message_container_type serializeFullPluginTelemetryPayload(const vector<PluginTelemetry>& pluginTelemetryList);
        /***
         * @brief Serialize a map of string into string object
         * @param map<string, string> A map of string key and string value pairs. The keys are the fields of a telemetry and the values are updated values from a telemetry. 
         * @return A JSON container of tmx::message_container_type
         */
        static tmx::message_container_type serializeUpdatedTelemetry(const map<string, string>& updates);
        /***
         * @brief Convert the map of telemetry updates into string object
         * @param map<string, string> A map of string key and string value pairs. 
         * @return A JSON string object that contains payload and its updated list of telemetry 
         */
        static string composeUpdatedTelemetryPayload(const map<string, string>& updates);
        /***
         * @brief Concat the header JSON string and the payload JSON string to form a complete telemetry JSON string.
         * @param JSON string of header
         * @param JSON string of payload
         * @return A JSON string object that contains a complete telemetry with header and payload 
         */
        static string composeCompleteTelemetry(const string& headerJsonStr, const string& payloadJsonStr);
        /***
         * @brief Serialize a telemetry header into a JSON container (tmx::message_container_type)
         * @param TelemetryHeader object
         * @return A JSON container of tmx::message_container_type
         */
        static tmx::message_container_type serializeTelemetryHeader(const TelemetryHeader& header);
        /***
         * @brief Convert JSON container of tmx::message_container_type to String
         * @param tmx::message_container_type object
         * @return JSON string object
         */
        static string jsonToString(tmx::message_container_type& container);
    };
}