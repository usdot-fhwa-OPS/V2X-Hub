#pragma once
#include <vector>
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
        static string composeFullTelemetry(const string& header, const string& payloadContent);
        static tmx::message_container_type serializeTelemetryHeader(const TelemetryHeader& header);
        static string jsonToString(tmx::message_container_type& container);
    };
}