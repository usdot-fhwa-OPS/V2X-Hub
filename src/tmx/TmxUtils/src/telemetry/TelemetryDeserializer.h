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
        static vector<PluginTelemetry> desrializeFullPluginTelemetryPayload(const string& jsonString);
        static PluginTelemetry deserialzePluginTelemetry(const string & jsonString);
        static boost::property_tree::ptree stringToJson( const string& jsonString);
        ~TelemetryDeserializer() = default;
    };

} // namespace tmx::utils::telemetry