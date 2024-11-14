#pragma once
#include <vector>
#include <map>
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string.hpp>

#include <tmx/messages/message.hpp>

#include "PluginTelemetry.h"
#include "TelemetrySerializerException.h"
#include "TelemetryHeader.h"
#include "TelemetryMetadata.h"

namespace tmx::utils::telemetry{
    class TelemetrySerializer
    {  
    public:
        TelemetrySerializer()=delete;
        ~TelemetrySerializer()=delete;
        /***
         * @brief Serialize a list of generic Telemetry object into JSON container (tmx::message_container_type)
         * @param vector<T> a vector of telemetry objects
         * @return A JSON container of tmx::message_container_type
         */
        template <typename T>
        static tmx::message_container_type serializeFullTelemetryPayload(const vector<T>& telemetryList){
            tmx::message_container_type outputContainer;
            boost::property_tree::ptree ptArray;
            for(const auto& telemetry: telemetryList){
                auto output = telemetry.toTree();
                ptArray.push_back(boost::property_tree::ptree::value_type("",output.get_storage().get_tree()));
            }
            outputContainer.get_storage().get_tree().put_child(PAYLOAD_STRING, ptArray);
            return outputContainer;
        }
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
        static tmx::message_container_type composeUpdatedTelemetryPayload(const map<string, tmx::message_container_type>& updates);
        /***
         * @brief Combine JSON containers and convert the merged container into a JSON string.
         * @param JSON tmx::message_container_type of header
         * @param JSON tmx::message_container_type of payload
         * @return A JSON string object that contains a complete telemetry with header and payload 
         */
        static string composeCompleteTelemetry(tmx::message_container_type& header, tmx::message_container_type& payload);
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
        static string treeToString(tmx::message_container_type& container);
    };
}