#pragma once
#include <vector>
#include <jsoncpp/json/json.h>

#include "PluginTelemetry.h"
#include "TelemetryDeserializerException.h"

using namespace std;

namespace tmx::utils::telemetry
{
    class TelemetryDeserializer
    {
    private:
        static PluginTelemetry  populatePluginTelemetry(const Json::Value& value);
    public:
        TelemetryDeserializer() = default;
        static vector<PluginTelemetry> desrializePluginTelemetryList(const string& jsonString);
        static PluginTelemetry deserialzePluginTelemetry(const string & jsonString);
        static Json::Value stringToJson( const string& jsonString);
        ~TelemetryDeserializer() = default;
    };

} // namespace tmx::utils::telemetry