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
            // Convert SPAT to JSON
            auto spatData = msg.get_j2735_data();
            auto spatJsonMsg = TmxJ2735Message<SPAT, tmx::JSON>(spatData);
            std::string spatJsonStr = spatJsonMsg.to_string();
 
            PLOG(logDEBUG) << "SPaT JSON: " << spatJsonStr;
 
            // Validate against schema file
            FieldValidation result = validateJsonAgainstSchemaFile(spatJsonStr, spatSchemaPath);
            PluginClient::SetStatus("Spat Schema Path configured", "Yes");
 
            if (!result.valid)
            {
                for (const auto &error : result.errors)
                {
                    PLOG(logWARNING) << "SPaT field validation failure: " << error;
                }
 
                // Get intersection ID from message
                int intersectionId = -1;
                if (spatData && spatData->intersections.list.count > 0 &&
                    spatData->intersections.list.array != nullptr)
                {
                    intersectionId = static_cast<int>(spatData->intersections.list.array[0]->id.id);
                }
 
                uint64_t handlerEndMs = PluginClientClockAware::getClock()->nowInMilliseconds();
 
                std::vector<MissingDataElement> elements;
                for (const auto &elem : result.errors)
                {
                    elements.emplace_back(elem);
                }
 
                CTI4501ValidationMessage eventMsg;
                eventMsg.set_eventGeneratedAt(handlerEndMs);
                eventMsg.set_eventType("SpatMinimumData");
                eventMsg.set_intersectionID(intersectionId);
                eventMsg.set_roadRegulatorID(-1);
                eventMsg.set_source(rsuSource);
                eventMsg.set_timePeriod(ProcessingTimePeriod(handlerBeginMs, handlerEndMs));
                eventMsg.set_missingDataElements(elements);
 
                PluginClient::BroadcastMessage(eventMsg);
 
                spatFieldValidationErrors++;
                
            }
            else
            {
                spatValidationPassed++;
            }
            PluginClient::SetStatus("SPaT Validation Passed", spatValidationPassed);
            PluginClient::SetStatus("SPaT Validation Failed", spatFieldValidationErrors);
        }
        catch (const std::exception &e)
        {
            PLOG(logERROR) << "Error during SPaT field validation: " << e.what();
        }
    }

    void IntersectionValidationPlugin::HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg)
    {
        measureMessageInterval(_lastMapTimeMs, MAP_INTERVAL_REQUIRED_MS, MAP_INTERVAL_MAX_THRESHOLD_MS, "MAP");
        // TODO: Perform MAP required fields validation
    }
}
 
int main(int argc, char *argv[])
{
    return run_plugin<IntersectionValidation::IntersectionValidationPlugin>("IntersectionValidationPlugin", argc, argv);
}