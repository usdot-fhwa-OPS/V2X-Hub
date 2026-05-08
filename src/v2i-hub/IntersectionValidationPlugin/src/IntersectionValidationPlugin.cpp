/**
 * Copyright (C) 2026 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at 
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
 
#include "IntersectionValidationPlugin.h"
#include "MessageIntervalValidator.h"
#include "FieldValidation.h"

using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace std;
 
namespace IntersectionValidation
{
    IntersectionValidationPlugin::IntersectionValidationPlugin(const std::string &name): PluginClientClockAware(name),
        _lastMapTimeMs(0),
        _lastSpatTimeMs(0)
    {

        AddMessageFilter<SpatMessage>(this, &IntersectionValidationPlugin::HandleSpatMessage);
        AddMessageFilter<MapDataMessage>(this, &IntersectionValidationPlugin::HandleMapDataMessage);

        SubscribeToMessages();
    }

    void IntersectionValidationPlugin::UpdateConfigSettings()
    {
        // TODO
    }

	void IntersectionValidationPlugin::OnConfigChanged(const char *key, const char *value)
	{
		PluginClient::OnConfigChanged(key, value);
        UpdateConfigSettings();
	}

    void IntersectionValidationPlugin::OnStateChange(IvpPluginState state)
	{
        PluginClientClockAware::OnStateChange(state);

        if (state == IvpPluginState_registered)
        {
            UpdateConfigSettings();
        }
    }

    void IntersectionValidationPlugin::measureMessageInterval(uint64_t &lastTimestampMs, uint64_t requiredThresholdMs, uint64_t maxThresholdMs, const std::string &messageType)
    {
        uint64_t currentTimeMs = PluginClientClockAware::getClock()->nowInMilliseconds();
        uint64_t intervalMs = 0;

        try
        {
            intervalMs = IntersectionValidation::calculateMessageInterval(lastTimestampMs, currentTimeMs, maxThresholdMs);
        }
        catch (const tmx::TmxException &e)
        {
            PLOG(tmx::utils::logWARNING) << messageType << " interval violation: " << e.what();

            TmxEventLogMessage eventLogMsg;
            eventLogMsg.set_level(IvpLogLevel::IvpLogLevel_warn);
            eventLogMsg.set_description(messageType + EVENT_MAX_THRESHOLD +
                                        std::to_string(maxThresholdMs) + " ms. Actual: " + std::to_string(intervalMs) + " ms");
            PluginClient::BroadcastMessage(eventLogMsg);
        }

        if (intervalMs > requiredThresholdMs && intervalMs <= maxThresholdMs)
        {
            PLOG(tmx::utils::logWARNING) << messageType << " interval violation: interval " << intervalMs << " ms";

            TmxEventLogMessage eventLogMsg;
            eventLogMsg.set_level(IvpLogLevel::IvpLogLevel_warn);
            eventLogMsg.set_description(messageType + EVENT_REQUIRED_THRESHOLD +
                                        std::to_string(requiredThresholdMs) + " ms. Actual: " + std::to_string(intervalMs) + " ms");
            PluginClient::BroadcastMessage(eventLogMsg);
        }

        if (messageType == "SPaT")
        {
            PluginClient::SetStatus("SPaT Message Interval (ms)", intervalMs);
        }
        else if (messageType == "MAP")
        {
            PluginClient::SetStatus("MAP Message Interval (ms)", intervalMs);
        }

        lastTimestampMs = currentTimeMs;
    }

    void IntersectionValidationPlugin::validateMessageFields(const std::string &jsonStr,
                                                             const std::string &schemaPath,
                                                             const std::string &eventType,
                                                             const std::string &messageType,
                                                             int intersectionId,
                                                             uint64_t handlerBeginMs)
    {
        uint32_t &passed = (messageType == "SPaT") ? spatValidationPassed : mapValidationPassed;
        uint32_t &failed = (messageType == "SPaT") ? spatFieldValidationErrors : mapFieldValidationErrors;

        FieldValidation result = validateJsonAgainstSchemaFile(jsonStr, schemaPath);
        
        if (messageType == "SPaT")
        {
            PluginClient::SetStatus("SPAT Schema Path configured", "Yes");
        }
        else if (messageType == "MAP")
        {
            PluginClient::SetStatus("MAP Schema Path configured", "Yes");
        }

        if (!result.valid)
        {
            for (const auto &error : result.errors)
            {
                PLOG(logWARNING) << messageType << " field validation failure: " << error;
            }

            uint64_t handlerEndMs = PluginClientClockAware::getClock()->nowInMilliseconds();

            std::vector<MissingDataElement> elements;
            for (const auto &elem : result.errors)
            {
                elements.emplace_back(elem);
            }

            CTI4501ValidationMessage eventMsg;
            eventMsg.set_eventGeneratedAt(handlerEndMs);
            eventMsg.set_eventType(eventType);
            eventMsg.set_intersectionID(intersectionId);
            eventMsg.set_roadRegulatorID(-1);
            eventMsg.set_source(rsuSource);
            eventMsg.set_timePeriod(ProcessingTimePeriod(handlerBeginMs, handlerEndMs));
            eventMsg.set_missingDataElements(elements);

            PluginClient::BroadcastMessage(eventMsg);

            failed++;
        }
        else
        {
            passed++;
        }

        if (messageType == "SPaT")
        {
            PluginClient::SetStatus("SPaT Field Validation Passed", passed);
            PluginClient::SetStatus("SPaT Field Validation Failed", failed);
        }
        else if (messageType == "MAP")
        {
            PluginClient::SetStatus("MAP Field Validation Passed", passed);
            PluginClient::SetStatus("MAP Field Validation Failed", failed);
        }
    }

    void IntersectionValidationPlugin::HandleSpatMessage(SpatMessage &msg, routeable_message &routeableMsg)
    {
        uint64_t handlerBeginMs = PluginClientClockAware::getClock()->nowInMilliseconds();
 
        // Frequency validation
        measureMessageInterval(_lastSpatTimeMs, SPAT_INTERVAL_REQUIRED_MS, SPAT_INTERVAL_MAX_THRESHOLD_MS, "SPaT");
 
        // Field validation
        if (spatSchemaPath.empty())
        {
            PLOG(logWARNING) << "SpatSchemaPath not configured, skipping field validation";
            PluginClient::SetStatus("SPAT Schema Path configured", "No");
            return;
        }

        try
        {
            auto spatData = msg.get_j2735_data();
            auto spatJsonMsg = TmxJ2735Message<SPAT, tmx::JSON>(spatData);
            std::string spatJsonStr = spatJsonMsg.to_string();

            // Get intersection ID from message
            int intersectionId = -1;
            if (spatData && spatData->intersections.list.count > 0 &&
                spatData->intersections.list.array != nullptr)
            {
                intersectionId = static_cast<int>(spatData->intersections.list.array[0]->id.id);
            }
 
            PLOG(logDEBUG) << "SPAT JSON: " << spatJsonStr;
 
            validateMessageFields(spatJsonStr, spatSchemaPath, "SpatMinimumData", "SPaT", intersectionId, handlerBeginMs);
        }
        catch (const std::exception &e)
        {
            PLOG(logERROR) << "Error during SPaT field validation: " << e.what();
        }
    }

    void IntersectionValidationPlugin::HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg)
    {
        uint64_t handlerBeginMs = PluginClientClockAware::getClock()->nowInMilliseconds();
 
        // Frequency validation
        measureMessageInterval(_lastMapTimeMs, MAP_INTERVAL_REQUIRED_MS, MAP_INTERVAL_MAX_THRESHOLD_MS, "MAP");
        
        // Field validation
        if (mapSchemaPath.empty())
        {
            PLOG(logWARNING) << "MapSchemaPath not configured, skipping field validation";
            PluginClient::SetStatus("MAP Schema Path configured", "No");
            return;
        }

        try
        {
            auto mapData = msg.get_j2735_data();
            auto mapJsonMsg = TmxJ2735Message<MapData, tmx::JSON>(mapData);
            std::string mapJsonStr = mapJsonMsg.to_string();
 
            PLOG(logDEBUG) << "MAP JSON: " << mapJsonStr;
 
            // Get intersection ID from message
            int intersectionId = -1;
            if (mapData && mapData->intersections != nullptr &&
                mapData->intersections->list.count > 0 &&
                mapData->intersections->list.array != nullptr)
            {
                intersectionId = static_cast<int>(mapData->intersections->list.array[0]->id.id);
            }
 
            validateMessageFields(mapJsonStr, mapSchemaPath, "MapMinimumData", "MAP", intersectionId, handlerBeginMs);
        }
        catch (const std::exception &e)
        {
            PLOG(logERROR) << "Error during MAP field validation: " << e.what();
        }
    }
}
 
int main(int argc, char *argv[])
{
    return run_plugin<IntersectionValidation::IntersectionValidationPlugin>("IntersectionValidationPlugin", argc, argv);
}