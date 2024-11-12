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
        static tmx::message_container_type serializePluginTelemetry(const PluginTelemetry& pluginTelemetry);
        static tmx::message_container_type serializeFullPluginTelemetryPayload(const vector<PluginTelemetry>& pluginTelemetryList);
        static tmx::message_container_type serializeUpdatedTelemetry(const map<string, string>& updates);
        static string composeUpdatedTelemetryPayload(const map<string, string>& updates);
        static string composeCompleteTelemetry(const string& headerJsonStr, const string& payloadJsonStr);
        static tmx::message_container_type serializeTelemetryHeader(const TelemetryHeader& header);
        static string jsonToString(tmx::message_container_type& container);
    };
}