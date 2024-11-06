#pragma once
#include <vector>
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string.hpp>

#include "PluginTelemetry.h"
#include "TelemetrySerializerException.h"

namespace tmx::utils::telemetry{

    class TelemetrySerializer
    {
    private:
        static string jsonToString(Json::Value root);
    public:
        TelemetrySerializer()=delete;
        ~TelemetrySerializer()=delete;
        static string serializePluginTelemetryList(vector<PluginTelemetry> pluginTelemetryList);
    };
}