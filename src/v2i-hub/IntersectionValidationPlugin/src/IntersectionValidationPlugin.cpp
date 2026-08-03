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

#include <ctime>
#include <cstdio>
#include <rapidjson/pointer.h>

using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace std;

namespace
{
    // Format timestamp as an UTC string for the timestampA/timestampB
    std::string formatIso8601Utc(const std::shared_ptr<fwha_stol::lib::time::CarmaClock> &clock)
    {
        const uint64_t epochMs = clock->nowInMilliseconds();
        const auto secs = static_cast<std::time_t>(epochMs / 1000);
        const auto millis = static_cast<int>(epochMs % 1000);
        std::tm tmUtc{};
        gmtime_r(&secs, &tmUtc);
 
        std::array<char, 32> buf{};
        std::strftime(buf.data(), buf.size(), "%Y-%m-%dT%H:%M:%S", &tmUtc);
 
        std::ostringstream out;
        out << buf.data() << '.' << std::setfill('0') << std::setw(3) << millis << 'Z';
        return out.str();
    }

    // Resolve a local "#/..." $ref against the root schema. Returns nullptr for
    // external or unresolvable refs.
    const rapidjson::Value *resolveRef(const rapidjson::Value &node,
                                       const rapidjson::Document &schemaRoot)
    {
        auto ref = node.FindMember("$ref");
        if (ref == node.MemberEnd() || !ref->value.IsString())
        {
            return &node;
        }

        const std::string refStr = ref->value.GetString();
        if (refStr.empty() || refStr[0] != '#')
        {
            return nullptr;
        }
        if (refStr == "#")
        {
            return &schemaRoot;
        }
        return rapidjson::Pointer(refStr.substr(1).c_str()).Get(schemaRoot);
    }

    // Recursively compare the document against the schema and collect every required
    // property that is absent
    void collectMissingRequired(const rapidjson::Value &schemaNode,
                                const rapidjson::Value &instance,
                                const rapidjson::Document &schemaRoot,
                                const std::string &jsonPath,
                                const std::string &schemaPath,
                                std::vector<MissingDataElement> &out,
                                int depth = 0)
    {
        constexpr int MAX_DEPTH = 64; // guards against $ref cycles
        if (depth > MAX_DEPTH)
        {
            return;
        }

        // Following a $ref re-bases the reported schema reference on the ref target,
        // so entries cite "#/$defs/J2735TimeMark" rather than the full inline path.
        std::string resolvedSchemaPath = schemaPath;
        if (schemaNode.IsObject())
        {
            auto ref = schemaNode.FindMember("$ref");
            if (ref != schemaNode.MemberEnd() && ref->value.IsString())
            {
                resolvedSchemaPath = ref->value.GetString();
            }
        }

        const rapidjson::Value *schema = resolveRef(schemaNode, schemaRoot);
        if (schema == nullptr || !schema->IsObject())
        {
            return;
        }

        if (instance.IsObject())
        {
            auto properties = schema->FindMember("properties");

            // Required properties absent from this object
            auto required = schema->FindMember("required");
            if (required != schema->MemberEnd() && required->value.IsArray())
            {
                for (const auto &name : required->value.GetArray())
                {
                    if (!name.IsString() || instance.HasMember(name.GetString()))
                    {
                        continue;
                    }

                    // Prefer the missing property's own schema reference
                    std::string elementSchemaPath =
                        resolvedSchemaPath + "/properties/" + name.GetString();
                    if (properties != schema->MemberEnd() && properties->value.IsObject())
                    {
                        auto propSchema = properties->value.FindMember(name.GetString());
                        if (propSchema != properties->value.MemberEnd() &&
                            propSchema->value.IsObject())
                        {
                            auto propRef = propSchema->value.FindMember("$ref");
                            if (propRef != propSchema->value.MemberEnd() &&
                                propRef->value.IsString())
                            {
                                elementSchemaPath = propRef->value.GetString();
                            }
                        }
                    }

                    out.emplace_back(jsonPath + "." + name.GetString() +
                                     " is missing (" + elementSchemaPath + ")");
                }
            }

            // Descend into the properties that are actually present
            if (properties != schema->MemberEnd() && properties->value.IsObject())
            {
                for (auto it = properties->value.MemberBegin();
                     it != properties->value.MemberEnd(); ++it)
                {
                    auto child = instance.FindMember(it->name);
                    if (child != instance.MemberEnd())
                    {
                        collectMissingRequired(it->value, child->value, schemaRoot,
                                               jsonPath + "." + it->name.GetString(),
                                               resolvedSchemaPath + "/properties/" +
                                                   it->name.GetString(),
                                               out, depth + 1);
                    }
                }
            }
        }

        // Descend into array elements
        if (instance.IsArray())
        {
            auto items = schema->FindMember("items");
            if (items != schema->MemberEnd() && items->value.IsObject())
            {
                rapidjson::SizeType index = 0;
                for (const auto &element : instance.GetArray())
                {
                    collectMissingRequired(items->value, element, schemaRoot,
                                           jsonPath + "[" + std::to_string(index) + "]",
                                           resolvedSchemaPath + "/items",
                                           out, depth + 1);
                    ++index;
                }
            }
        }

        // allOf applies every subschema, so its required constraints all hold
        auto allOf = schema->FindMember("allOf");
        if (allOf != schema->MemberEnd() && allOf->value.IsArray())
        {
            rapidjson::SizeType index = 0;
            for (const auto &sub : allOf->value.GetArray())
            {
                collectMissingRequired(sub, instance, schemaRoot, jsonPath,
                                       resolvedSchemaPath + "/allOf/" + std::to_string(index),
                                       out, depth + 1);
                ++index;
            }
        }
    }
}

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
        // RSU identifier
        GetConfigValue<std::string>("rsuSource", rsuSource);
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

    void IntersectionValidationPlugin::measureMessageInterval(uint64_t &lastTimestampMs, uint64_t requiredThresholdMs, uint64_t maxThresholdMs, const std::string &messageType, int intersectionId)
    {
        uint64_t currentTimeMs = PluginClientClockAware::getClock()->nowInMilliseconds();
        uint64_t intervalMs = 0;

        // BroadcastRate event derived from the message interval
        const std::string rateEventType = (messageType == "SPaT") ? "SpatBroadcastRate" : "MapBroadcastRate";
        const std::string &inputTopic = (messageType == "SPaT") ? spatInputTopic : mapInputTopic;

        auto emitBroadcastRate = [&]() {
            CTI4501ValidationMessage eventMsg;
            eventMsg.set_eventGeneratedAt(currentTimeMs);
            eventMsg.set_eventType(rateEventType);
            eventMsg.set_intersectionID(intersectionId);
            eventMsg.set_roadRegulatorID(-1);
            eventMsg.set_source(rsuSource);
            eventMsg.set_topicName(inputTopic);
            eventMsg.set_numberOfMessages(2); // the two messages bounding this interval
            eventMsg.set_timePeriod(ProcessingTimePeriod(lastTimestampMs, currentTimeMs));
            PluginClient::BroadcastMessage(eventMsg);
        };

        try
        {
            intervalMs = IntersectionValidation::calculateMessageInterval(lastTimestampMs, currentTimeMs, maxThresholdMs);
        }
        catch (const tmx::TmxException &e)
        {
            PLOG(tmx::utils::logWARNING) << messageType << " interval violation: " << e.what();

            // Calculate interval if there is an exception thrown
            if (lastTimestampMs != 0)
            {
                intervalMs = currentTimeMs - lastTimestampMs;
            }

            // Hard violation: interval exceeded the CTI 4501 maximum threshold.
            emitBroadcastRate();
        }

        if (intervalMs > requiredThresholdMs && intervalMs <= maxThresholdMs)
        {
            PLOG(tmx::utils::logWARNING) << messageType << " interval violation: interval " << intervalMs << " ms";

            // Soft violation: interval exceeded the CTI 4501 required threshold.
            emitBroadcastRate();
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

        // Run both validations on the same preprocessed document
        validateMessageFields(doc, schemaDoc, fieldEventType, messageType, intersectionId, handlerBeginMs);
        return validateRevisionCounters(doc, revisionEventType, messageType, intersectionId);
    }

    void IntersectionValidationPlugin::validateMessageFields(const rapidjson::Document &doc,
                                                             const rapidjson::Document &schemaDoc,
                                                             const std::string &eventType,
                                                             const std::string &messageType,
                                                             int intersectionId,
                                                             uint64_t handlerBeginMs)
    {
        uint32_t &passed = (messageType == "SPaT") ? spatValidationPassed : mapValidationPassed;
        uint32_t &failed = (messageType == "SPaT") ? spatFieldValidationErrors : mapFieldValidationErrors;

        // Run schema validation directly on the pre-processed document
        rapidjson::SchemaDocument schema(schemaDoc);
        rapidjson::SchemaValidator validator(schema);

        if (messageType == "SPaT")
        {
            PluginClient::SetStatus("SPaT Schema Path configured", "Yes");
        }
        else if (messageType == "MAP")
        {
            PluginClient::SetStatus("MAP Schema Path configured", "Yes");
        }

        if (!doc.Accept(validator))
        {
            // One entry per missing element, formatted to match conflictmonitor's own
            // minimum-data strings.
            std::vector<MissingDataElement> elements;
            collectMissingRequired(schemaDoc, doc, schemaDoc, "$", "#", elements);

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

            for (const auto &element : elements)
            {
                PLOG(logWARNING) << messageType << " field validation failure: " << element.value;
            }

            uint64_t handlerEndMs = PluginClientClockAware::getClock()->nowInMilliseconds();

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
            PluginClient::SetStatus("SPaT Field Validation Passed", static_cast<int>(passed));
            PluginClient::SetStatus("SPaT Field Validation Failed", static_cast<int>(failed));
        }
        else if (messageType == "MAP")
        {
            PluginClient::SetStatus("MAP Field Validation Passed", static_cast<int>(passed));
            PluginClient::SetStatus("MAP Field Validation Failed", static_cast<int>(failed));
        }
    }

    RevisionCounterResult IntersectionValidationPlugin::validateRevisionCounters(const rapidjson::Document &doc,
                                                                const std::string &eventType,
                                                                const std::string &messageType,
                                                                [[maybe_unused]] int intersectionId)
    {
        const std::string currentTimestamp = formatIso8601Utc(getClock());
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
            PluginClient::BroadcastMessage(eventMsg);
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
            PluginClient::SetStatus("SPaT Revision Validation Passed", static_cast<int>(passed));
            PluginClient::SetStatus("SPaT Revision Validation Failed", static_cast<int>(failed));
        }
        else if (messageType == "MAP")
        {
            PluginClient::SetStatus("MAP Revision Validation Passed", static_cast<int>(passed));
            PluginClient::SetStatus("MAP Revision Validation Failed", static_cast<int>(failed));
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
            PluginClient::SetStatus("SPaT Schema Path configured", "No");
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

            measureMessageInterval(_lastSpatTimeMs, SPAT_INTERVAL_REQUIRED_MS,
                                   SPAT_INTERVAL_MAX_THRESHOLD_MS, "SPaT", intersectionId);
 
            // Convert to full MessageFrame JSON
            auto spatJsonMsg = TmxJ2735Message<MessageFrame, tmx::JSON>(spatData);
            std::string spatJsonStr = spatJsonMsg.to_string();

            // Parse, preprocess, validate
            RevisionCounterResult revResult = validateMessage(spatJsonStr, spatSchemaPath, "SpatMinimumData",
                                                              "SpatMessageCountProgression", "SPaT", intersectionId, handlerBeginMs);

            if (planForwarding(revResult))
                broadcastValidated(routeableMsg);
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
            PluginClient::SetStatus("MAP Schema Path configured", "No");
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

            measureMessageInterval(_lastMapTimeMs, MAP_INTERVAL_REQUIRED_MS,
                                   MAP_INTERVAL_MAX_THRESHOLD_MS, "MAP", intersectionId);
 
            auto mapJsonMsg = TmxJ2735Message<MessageFrame, tmx::JSON>(mapData);
            std::string mapJsonStr = mapJsonMsg.to_string();
 
            // Parse, preprocess, validate
            RevisionCounterResult revResult = validateMessage(mapJsonStr, mapSchemaPath, "MapMinimumData",
                                                              "MapMessageCountProgression", "MAP", intersectionId, handlerBeginMs);

            if (planForwarding(revResult))
                broadcastValidated(routeableMsg);
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