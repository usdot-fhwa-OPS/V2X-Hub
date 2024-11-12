#pragma once
#include <vector>
#include <jsoncpp/json/json.h>
#include <tmx/messages/message.hpp>
#include <boost/foreach.hpp>

#include "PluginTelemetry.h"
#include "TelemetryDeserializerException.h"

using namespace std;

namespace tmx::utils::telemetry
{
    class TelemetryDeserializer
    {
    private:
        static PluginTelemetry  populatePluginTelemetry(const boost::property_tree::ptree& value);
    public:
        TelemetryDeserializer() = default;
        /***
         * @brief Deserialize JSON string into a vector of PluginTelemetry objects
         * @param JSON string object 
         * @return Vector of PluginTelemetry objects
         */
        static vector<PluginTelemetry> desrializeFullPluginTelemetryPayload(const string& jsonString);
        /***
         * @brief Deserialize JSON string into PluginTelemetry object
         * @param JSON string object 
         * @return PluginTelemetry object
         */
        static PluginTelemetry deserialzePluginTelemetry(const string & jsonString);
        /***
         * @brief Convert JSON String into boost::property_tree::ptree object
         * @param JSON string object 
         * @return boost::property_tree::ptree
         */
        static boost::property_tree::ptree stringToJson( const string& jsonString);
        ~TelemetryDeserializer() = default;
    };

} // namespace tmx::utils::telemetry