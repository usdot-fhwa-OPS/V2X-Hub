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
#include "RevisionCounterValidator.h"
#include "ODEForwarding.h"

using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace std;

namespace IntersectionValidation
{
    IntersectionValidationPlugin::IntersectionValidationPlugin(const std::string &name): PluginClientClockAware(name)
    {

        AddMessageFilter<SpatMessage>(this, &IntersectionValidationPlugin::HandleSpatMessage);
        AddMessageFilter<MapDataMessage>(this, &IntersectionValidationPlugin::HandleMapDataMessage);

        SubscribeToMessages();
    }

    void IntersectionValidationPlugin::UpdateConfigSettings()
    {
        // RSU identifier
        GetConfigValue<std::string>("rsuSource", rsuSource);

        // BroadcastRate Time Window
        GetConfigValue<uint64_t>("BroadcastRateTimeWindow", BroadcastRateTimeWindow);

        // ContentValidation Time Window
        GetConfigValue<uint64_t>("ContentValidationTimeWindow", ContentValidationTimeWindow);

         // ContentUnchangedTimeWindow Time Window
        GetConfigValue<uint64_t>("ContentUnchangedTimeWindow", ContentUnchangedTimeWindow);
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

    void IntersectionValidationPlugin::measureMessageInterval(MessageIntervalValidator &validator,
                                                              const std::string &messageType, int intersectionId)
    {
        const uint64_t currentTimeMs = PluginClientClockAware::getClock()->nowInMilliseconds();

        std::optional<IntervalWindowResult> closedWindow;
        try
        {
            closedWindow = validator.recordMessage(currentTimeMs, BroadcastRateTimeWindow, intersectionId);
        }
        catch (const tmx::TmxException &e)
        {
            PLOG(tmx::utils::logWARNING) << messageType << " interval check failed: " << e.what();
            return;
        }

        PluginClient::SetStatusThrottled((messageType + " Message Interval (ms)").c_str(), validator.lastIntervalMs());

        // Since closedWindow is std::optional, this checks to see if closedWindow has a value.
        // Without this check, the dereferencing will be undefined behavior
        if (closedWindow)
        {
            publishBroadcastRateEvent(*closedWindow, messageType);
        }
    }

    void IntersectionValidationPlugin::publishBroadcastRateEvent(const IntervalWindowResult &result,
                                                                 const std::string &messageType)
    {
        // Event type must match the names ODEForwardPlugin routes on, or the event is dropped
        const std::string rateEventType = (messageType == "SPaT") ? "SpatBroadcastRate" : "MapBroadcastRate";
        const std::string &inputTopic = (messageType == "SPaT") ? spatInputTopic : mapInputTopic;

        if (result.intersectionIdMismatch)
        {
            PLOG(tmx::utils::logDEBUG) << messageType << " aggregation window saw more than one intersection ID; "
                                       << "reporting " << result.intersectionId << ", the one that opened it";
        }

        CTI4501ValidationMessage eventMsg;
        eventMsg.set_eventGeneratedAt(PluginClientClockAware::getClock()->nowInMilliseconds());
        eventMsg.set_eventType(rateEventType);
        eventMsg.set_intersectionID(result.intersectionId);
        eventMsg.set_roadRegulatorID(-1);
        eventMsg.set_source(rsuSource);
        eventMsg.set_topicName(inputTopic);
        eventMsg.set_numberOfMessages(static_cast<int>(result.messageCount));
        eventMsg.set_timePeriod(ProcessingTimePeriod(result.windowStartMs, result.windowEndMs));
        PluginClient::BroadcastMessage(eventMsg);

        tmx::messages::TmxEventLogMessage eventLogMsg;
        eventLogMsg.set_level(IvpLogLevel::IvpLogLevel_warn);
        eventLogMsg.set_description(formatBroadcastRateDescription(messageType, result.violationCount,
                                                                  result.windowStartMs, result.windowEndMs));
        BroadcastMessage(eventLogMsg);
    }

    RevisionCounterResult IntersectionValidationPlugin::validateMessage(const std::string &jsonStr,
                                                       const std::string &schemaPath,
                                                       const std::string &fieldEventType,
                                                       const std::string &revisionEventType,
                                                       const std::string &messageType,
                                                       int intersectionId,
                                                       uint64_t handlerBeginMs)
    {
        // Parse the JSON string
        rapidjson::Document doc;
        doc.Parse(jsonStr.c_str());
        if (doc.HasParseError())
        {
            PLOG(logERROR) << "Failed to parse " << messageType << " JSON";
            return {};
        }

        // Load and parse the schema
        std::string schemaStr = loadFileContents(schemaPath);
        rapidjson::Document schemaDoc;
        schemaDoc.Parse(schemaStr.c_str());
        if (schemaDoc.HasParseError())
        {
            PLOG(logERROR) << "Failed to parse " << messageType << " schema";
            return {};
        }

        // Preprocess, remove TMX empty strings, convert
        // string-encoded integers and booleans based on schema
        removeEmptyStrings(doc, doc.GetAllocator());
        convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);

        // First run revision validation so that MinimumData event can also be throttled
        RevisionCounterResult revResult =
            validateRevisionCounters(doc, revisionEventType, messageType, intersectionId);

        // planForwarding() is true when message content changed (or first seen)
        const bool contentChanged = planForwarding(revResult);
        validateMessageFields(doc, schemaDoc, fieldEventType, messageType,
                              intersectionId, handlerBeginMs, contentChanged);

        return revResult;
    }

    void IntersectionValidationPlugin::validateMessageFields(const rapidjson::Document &doc,
                                                             const rapidjson::Document &schemaDoc,
                                                             const std::string &eventType,
                                                             const std::string &messageType,
                                                             int intersectionId,
                                                             uint64_t handlerBeginMs,
                                                             bool contentChanged)
    {
        uint32_t &passed = (messageType == "SPaT") ? spatValidationPassed : mapValidationPassed;
        uint32_t &failed = (messageType == "SPaT") ? spatFieldValidationErrors : mapFieldValidationErrors;

        // Run schema validation directly on the pre-processed document
        rapidjson::SchemaDocument schema(schemaDoc);
        rapidjson::SchemaValidator validator(schema);

        if (!doc.Accept(validator))
        {
            // The validation failure is real regardless of throttling, so count it.
            failed++;

            if (!contentChanged)
            {
                // Identical to previous message for this intersection
                // MinimumDataEvent already fired for this message
                PLOG(logDEBUG) << messageType
                               << " minimum-data unchanged from previous message; "
                                  "throttling duplicate event";
            }
            else
            {
                // One entry per missing element, formatted to match conflictmonitor's own
                // minimum-data strings.
                std::vector<MissingDataElement> elements;
                for (const auto &field : IntersectionValidation::collectMissingRequiredFields(schemaDoc, doc))
                {
                    elements.emplace_back(field);
                }

                if (elements.empty())
                {
                    // Validation failed for a reason other than a missing required property
                    rapidjson::StringBuffer docSb;
                    rapidjson::StringBuffer schemaSb;
                    const char *keyword = validator.GetInvalidSchemaKeyword();
                    validator.GetInvalidDocumentPointer().StringifyUriFragment(docSb);
                    validator.GetInvalidSchemaPointer().StringifyUriFragment(schemaSb);
                    elements.emplace_back(std::string(docSb.GetString()) + " failed " +
                                          (keyword ? keyword : "validation") + " (" +
                                          schemaSb.GetString() + ")");
                }



                uint64_t handlerEndMs = PluginClientClockAware::getClock()->nowInMilliseconds();

                // Check if last validation errors are equal to current validation errors 

                if (_lastContentValidationMessage.find(messageType) == _lastContentValidationMessage.end() ||  // No previous validation errors for this message type
                    !compareMissingDataElements(elements, _lastContentValidationMessage[messageType].get_missingDataElements()) ||  // Current validation errors different from previous
                    (handlerEndMs - _lastContentValidationMessage[messageType].get_eventGeneratedAt()) > ContentValidationTimeWindow) // Current validation errors the same as previous but outside of the throttling time window
                {
                    // TODO: This currently only supports 1 to 1 intersection to v2xhub mapping
                    // Update to support multiple intersections per v2xhub in the future
                    CTI4501ValidationMessage eventMsg;
                    eventMsg.set_eventGeneratedAt(handlerEndMs);
                    eventMsg.set_eventType(eventType);
                    eventMsg.set_intersectionID(intersectionId);
                    eventMsg.set_roadRegulatorID(-1);
                    eventMsg.set_source(rsuSource);
                    eventMsg.set_timePeriod(ProcessingTimePeriod(handlerBeginMs, handlerEndMs));
                    eventMsg.set_missingDataElements(elements);

                    PLOG(logWARNING) << messageType << " encountered CTI 4501 MinimumDataEvent: " << eventMsg.to_string();


                    PluginClient::BroadcastMessage(eventMsg);

                    // EventLog Message
                    tmx::messages::TmxEventLogMessage eventLogMsg;
                    eventLogMsg.set_level(IvpLogLevel::IvpLogLevel_error);
                    eventLogMsg.set_description(messageType + EVENT_FIELD_VALIDATION_FAILED + eventMsg.to_string());
                    BroadcastMessage(eventLogMsg);
                    // Store last validation errors for each unique message type
                    _lastContentValidationMessage[messageType] = eventMsg;
                }
                else
                {
                    PLOG(logDEBUG) << messageType << " minimum-data unchanged from previous message; "
                                   << "throttling duplicate event";
                }
            }
        }
        else
        {
            passed++;
        }

        if (messageType == "SPaT")
        {
            PluginClient::SetStatusThrottled("SPaT Schema Path configured", "Yes");
            PluginClient::SetStatusThrottled("SPaT Field Validation Passed", static_cast<int>(passed));
            PluginClient::SetStatusThrottled("SPaT Field Validation Failed", static_cast<int>(failed));
        }
        else if (messageType == "MAP")
        {
            PluginClient::SetStatusThrottled("MAP Field Validation Passed", static_cast<int>(passed));
            PluginClient::SetStatusThrottled("MAP Field Validation Failed", static_cast<int>(failed));
            PluginClient::SetStatusThrottled("MAP Schema Path configured", "Yes");
        }
    }

    RevisionCounterResult IntersectionValidationPlugin::validateRevisionCounters(const rapidjson::Document &doc,
                                                                const std::string &eventType,
                                                                const std::string &messageType,
                                                                [[maybe_unused]] int intersectionId)
    {
        const std::string currentTimestamp = formatIso8601Utc(getClock()->nowInMilliseconds());
        RevisionCounterResult result = (messageType == "SPaT")
                                           ? _revisionValidator.validateSpatRevision(doc, currentTimestamp)
                                           : _revisionValidator.validateMapRevision(doc, currentTimestamp);

        const uint64_t handlerEndMs = PluginClientClockAware::getClock()->nowInMilliseconds();
        for (const auto &change : result.intersectionChanges)
        {
            if (!change.progressionViolation)
            {
                continue;
            }
            // Check if last revision validation error had 0 message count and current validation error has 0 count.
            // If both are 
            CTI4501ValidationMessage eventMsg;
            eventMsg.set_eventGeneratedAt(handlerEndMs);
            eventMsg.set_eventType(eventType); // Spat/MapMessageCountProgression
            eventMsg.set_intersectionID(change.id);
            eventMsg.set_roadRegulatorID(-1);
            eventMsg.set_source(rsuSource);
            eventMsg.set_messageType(messageType); // "SPaT" / "MAP"
            eventMsg.set_messageCountA(change.progressionCountA);
            eventMsg.set_messageCountB(change.progressionCountB);
            eventMsg.set_timestampA(change.timestampA);
            eventMsg.set_timestampB(change.timestampB);

            if (_lastRevisionValidationMessage.find(messageType) == _lastRevisionValidationMessage.end() ||  // No previous validation errors for this message type
                !compareRevisionValidationMessages(eventMsg, _lastRevisionValidationMessage[messageType]) || // Current validation errors different from previous or none zero counts
                (handlerEndMs - _lastRevisionValidationMessage[messageType].get_eventGeneratedAt()) > ContentValidationTimeWindow) // Current validation errors the same and zero counts but outside of the throttling time window
            {
                PLOG(logWARNING) << messageType << " encountered CTI 4501 MessageCountProgressionEvent: " << eventMsg.to_string();
                PluginClient::BroadcastMessage(eventMsg);
                // EventLog Message
                tmx::messages::TmxEventLogMessage eventLogMsg;
                eventLogMsg.set_level(IvpLogLevel::IvpLogLevel_error);
                eventLogMsg.set_description(messageType + " intersection " + std::to_string(change.id) +
                                            " message count did not progress per CTI 4501 (countA=" +
                                            std::to_string(change.progressionCountA) + ", countB=" +
                                            std::to_string(change.progressionCountB) + ")");
                BroadcastMessage(eventLogMsg);

                _lastRevisionValidationMessage[messageType] = eventMsg;
            }
            else
            {
                PLOG(logDEBUG) << messageType << " message count progression unchanged from previous message; "
                               << "throttling duplicate event";
                continue;
            }
                       
        }

        // CTI 4501 revision validity bookkeeping (no event emitted here).
        uint32_t &passed = (messageType == "SPaT") ? spatRevisionPassed : mapRevisionPassed;
        uint32_t &failed = (messageType == "SPaT") ? spatRevisionFailed : mapRevisionFailed;

        if (!result.valid)
        {
            for (const auto &violation : result.violations)
            {
                PLOG(logWARNING) << messageType << " revision counter violation: " << violation;
            }
            failed++;
        }
        else if (result.comparisonPerformed)
        {
            passed++;
        }

        if (messageType == "SPaT")
        {
            PluginClient::SetStatusThrottled("SPaT Revision Validation Passed", static_cast<int>(passed));
            PluginClient::SetStatusThrottled("SPaT Revision Validation Failed", static_cast<int>(failed));
        }
        else if (messageType == "MAP")
        {
            PluginClient::SetStatusThrottled("MAP Revision Validation Passed", static_cast<int>(passed));
            PluginClient::SetStatusThrottled("MAP Revision Validation Failed", static_cast<int>(failed));
        }

        return result;
    }

    void IntersectionValidationPlugin::broadcastValidated(routeable_message &msg)
    {
        // set_flags on the encoded message
        msg.set_flags(IvpMsgFlags_Validated);
        PluginClient::BroadcastMessage(msg);
    }

    void IntersectionValidationPlugin::HandleSpatMessage(SpatMessage &msg, routeable_message &routeableMsg)
    {
        // Skip re-broadcasts
        if (routeableMsg.get_flags() & IvpMsgFlags_Validated)
            return;

        uint64_t handlerBeginMs = PluginClientClockAware::getClock()->nowInMilliseconds();

        if (spatSchemaPath.empty())
        {
            PLOG(logWARNING) << "SpatSchemaPath not configured, skipping validation";
            PluginClient::SetStatusThrottled("SPaT Schema Path configured", "No");
            return;
        }
 
        try
        {
            auto spatData = msg.get_j2735_data();
            auto spatDataRef = spatData; // keep alive past JSON conversion

            // Extract intersection ID before JSON conversion
            int intersectionId = -1;
            if (spatData && spatData->intersections.list.count > 0 &&
                spatData->intersections.list.array != nullptr)
            {
                intersectionId = static_cast<int>(spatData->intersections.list.array[0]->id.id);
            }

            measureMessageInterval(_spatIntervalValidator, "SPaT", intersectionId);
 
            // Convert to full MessageFrame JSON
            auto spatJsonMsg = TmxJ2735Message<MessageFrame, tmx::JSON>(spatData);
            std::string spatJsonStr = spatJsonMsg.to_string();

            // Parse, preprocess, validate
            RevisionCounterResult revResult = validateMessage(spatJsonStr, spatSchemaPath, "SpatMinimumData",
                                                              "SpatMessageCountProgression", "SPaT", intersectionId, handlerBeginMs);
            if (planForwarding(revResult)
                || handlerBeginMs - lastBroadcastMessageTime["SPaT"] > ContentUnchangedTimeWindow) {
                // Forward if content changed or if time since last broadcast exceeds the configured window
                broadcastValidated(routeableMsg);
                lastBroadcastMessageTime["SPaT"] = handlerBeginMs;
            }
        }
        catch (const std::exception &e)
        {
            PLOG(logERROR) << "Error during SPaT validation: " << e.what();
        }
    }

    void IntersectionValidationPlugin::HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg)
    {
        // Skip re-broadcasts
        if (routeableMsg.get_flags() & IvpMsgFlags_Validated)
            return;

        uint64_t handlerBeginMs = PluginClientClockAware::getClock()->nowInMilliseconds();

        if (mapSchemaPath.empty())
        {
            PLOG(logWARNING) << "MapSchemaPath not configured, skipping validation";
            PluginClient::SetStatusThrottled("MAP Schema Path configured", "No");
            return;
        }
 
        try
        {
            auto mapData = msg.get_j2735_data();
            auto mapDataRef = mapData;

            // Extract intersection ID before JSON conversion
            int intersectionId = -1;
            if (mapData && mapData->intersections != nullptr &&
                mapData->intersections->list.count > 0 &&
                mapData->intersections->list.array != nullptr)
            {
                intersectionId = static_cast<int>(mapData->intersections->list.array[0]->id.id);
            }

            measureMessageInterval(_mapIntervalValidator, "MAP", intersectionId);
 
            auto mapJsonMsg = TmxJ2735Message<MessageFrame, tmx::JSON>(mapData);
            std::string mapJsonStr = mapJsonMsg.to_string();
 
            // Parse, preprocess, validate
            RevisionCounterResult revResult = validateMessage(mapJsonStr, mapSchemaPath, "MapMinimumData",
                                                              "MapMessageCountProgression", "MAP", intersectionId, handlerBeginMs);

            if (planForwarding(revResult) 
                || handlerBeginMs - lastBroadcastMessageTime["MAP"] > ContentUnchangedTimeWindow) 
            {
                // Forward if content changed or if time since last broadcast exceeds the configured window
                broadcastValidated(routeableMsg);
                lastBroadcastMessageTime["MAP"] = handlerBeginMs;

            }
        }
        catch (const std::exception &e)
        {
            PLOG(logERROR) << "Error during MAP validation: " << e.what();
        }
    }

   
}
int main(int argc, char *argv[])
{
    return run_plugin<IntersectionValidation::IntersectionValidationPlugin>("IntersectionValidationPlugin", argc, argv);
}