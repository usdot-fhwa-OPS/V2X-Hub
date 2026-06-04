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

#include "RevisionCounterValidator.h"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <cstring>

namespace IntersectionValidation
{

    int RevisionCounterValidator::extractInt(const rapidjson::Value &value, int defaultVal)
    {
        // Handle both forms of integer representations (ints or strings)
        // so revision counters and intersection IDs are extracted
        if (value.IsInt())
        {
            return value.GetInt();
        }
        if (value.IsString())
        {
            return std::atoi(value.GetString());
        }
        return defaultVal;
    }

    std::string RevisionCounterValidator::createContentHash(const rapidjson::Value &intersection)
    {
        // Strip timestamp fields
        rapidjson::Document tempDoc;
        rapidjson::Value stripped = stripTimestamps(intersection, tempDoc.GetAllocator());

        // Remove the revision field itself for message content comparison
        if (stripped.HasMember("revision"))
        {
            stripped.RemoveMember("revision");
        }

        // Convert the JSON to a string
        return toJSONString(stripped);
    }

    bool RevisionCounterValidator::checkRevisionViolation(int intersectionId, int currentRevision,
                                                          const std::string_view &contentHash,
                                                          const IntersectionState &prevState,
                                                          const std::string &messageType,
                                                          RevisionCounterResult &result)
    {
        result.comparisonPerformed = true;
        // Compare current content against stored previous content
        bool contentChanged = (contentHash != prevState.contentHash);

        if (bool revisionChanged = (currentRevision != prevState.revision); contentChanged && !revisionChanged)
        {
            // CTI 4501 violation: message content changed but revision counter did not increase
            result.valid = false;
            result.violations.push_back(
                messageType + " intersection " + std::to_string(intersectionId) +
                " content changed but revision counter did not increment (revision=" +
                std::to_string(currentRevision) + ")");
        }
        else if (!contentChanged && revisionChanged)
        {
            // CTI 4501 violation: message content is identical to the previous
            // message but the revision counter increased
            result.valid = false;
            result.violations.push_back(
                messageType + " intersection " + std::to_string(intersectionId) +
                " content unchanged but revision counter incremented (prev=" +
                std::to_string(prevState.revision) +
                ", current=" + std::to_string(currentRevision) + ")");
        }

        return contentChanged;
    }

    rapidjson::Value RevisionCounterValidator::stripTimestamps(const rapidjson::Value &value,
                                                               rapidjson::Document::AllocatorType &allocator)
    {
        if (value.IsObject())
        {
            // Skip the timeStamp fields at any level of the JSON hierarchy
            rapidjson::Value result(rapidjson::kObjectType);
            for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it)
            {
                if (it->name.GetString() == std::string("timeStamp"))
                {
                    continue;
                }
                rapidjson::Value keyCopy(it->name, allocator);
                rapidjson::Value valCopy = stripTimestamps(it->value, allocator);
                result.AddMember(keyCopy, valCopy, allocator);
            }
            return result;
        }
        else if (value.IsArray())
        {
            // Recurse into each array element (e.g. intersections, states)
            rapidjson::Value result(rapidjson::kArrayType);
            for (rapidjson::SizeType i = 0; i < value.Size(); ++i)
            {
                result.PushBack(stripTimestamps(value[i], allocator), allocator);
            }
            return result;
        }
        else
        {
            // Primitive value (int, string, bool) — deep copy unchanged
            rapidjson::Value result;
            result.CopyFrom(value, allocator);
            return result;
        }
    }

    // Convert the JSON message for a string for comparison
    std::string RevisionCounterValidator::toJSONString(const rapidjson::Value &value)
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        value.Accept(writer);
        return buffer.GetString();
    }

    RevisionCounterResult RevisionCounterValidator::validateSpatRevision(const std::string &spatJson)
    {
        RevisionCounterResult result;

        // Parse the SPaT MessageFrame JSON
        rapidjson::Document doc;
        doc.Parse(spatJson.c_str());
        if (doc.HasParseError())
        {
            result.valid = false;
            result.violations.emplace_back("Failed to parse SPaT JSON for revision validation");
            return result;
        }

        // Navigate to the intersections array
        // Structure: {"messageId":19, "value":{"SPAT":{"intersections":[...]}}}
        if (!doc.HasMember("value") || !doc["value"].HasMember("SPAT") ||
            !doc["value"]["SPAT"].HasMember("intersections") ||
            !doc["value"]["SPAT"]["intersections"].IsArray())
        {
            return result;
        }

        const auto &intersections = doc["value"]["SPAT"]["intersections"];

        // Validate each intersection's revision counter against its
        // previously stored state. The first time an intersection is seen,
        // no comparison is performed — it just stores the initial state.
        for (rapidjson::SizeType i = 0; i < intersections.Size(); ++i)
        {
            const auto &intersection = intersections[i];

            // Skip intersections without a valid ID
            if (!intersection.HasMember("id") || !intersection["id"].HasMember("id"))
            {
                continue;
            }

            int intersectionId = extractInt(intersection["id"]["id"]);
            int currentRevision = intersection.HasMember("revision")
                                      ? extractInt(intersection["revision"])
                                      : -1;

            // Create content hash excluding timestamps and revision
            std::string contentHash = createContentHash(intersection);

            // Compare against previous state if we've seen this intersection before
            if (auto prevIt = _prevSpatIntersections.find(intersectionId); prevIt != _prevSpatIntersections.end())
            {
                checkRevisionViolation(intersectionId, currentRevision, contentHash,
                                       prevIt->second, "SPaT", result);
            }

            // Store current state for next message comparison
            _prevSpatIntersections[intersectionId] = {currentRevision, contentHash};
        }

        return result;
    }

    RevisionCounterResult RevisionCounterValidator::validateMapRevision(const std::string &mapJson)
    {
        RevisionCounterResult result;

        // Parse the MAP MessageFrame JSON
        rapidjson::Document doc;
        doc.Parse(mapJson.c_str());
        if (doc.HasParseError())
        {
            result.valid = false;
            result.violations.emplace_back("Failed to parse MAP JSON for revision validation");
            return result;
        }

        // Navigate to MapData
        // Structure: {"messageId":18, "value":{"MapData":{...}}}
        if (!doc.HasMember("value") || !doc["value"].HasMember("MapData"))
        {
            return result;
        }

        const auto &mapData = doc["value"]["MapData"];

        // Extract message-level revision counter
        int currentMsgRevision = mapData.HasMember("msgIssueRevision")
                                     ? extractInt(mapData["msgIssueRevision"])
                                     : -1;

        // Validate per-intersection revision counters and track
        // whether any intersection content changed
        bool anyIntersectionChanged = false;

        if (mapData.HasMember("intersections") && mapData["intersections"].IsArray())
        {
            anyIntersectionChanged = validateMapIntersections(
                mapData["intersections"], result);
        }

        // Validate message-level msgIssueRevision based on whether
        // any intersection content changed
        validateMsgIssueRevision(currentMsgRevision, anyIntersectionChanged, result);

        // Update message-level state for next comparison
        _prevMapMessage.msgIssueRevision = currentMsgRevision;
        _hasMapPrevious = true;

        return result;
    }

    bool RevisionCounterValidator::validateMapIntersections(const rapidjson::Value &intersections,
                                                            RevisionCounterResult &result)
    {
        bool anyChanged = false;

        for (rapidjson::SizeType i = 0; i < intersections.Size(); ++i)
        {
            const auto &intersection = intersections[i];

            // Skip intersections without a valid ID
            if (!intersection.HasMember("id") || !intersection["id"].HasMember("id"))
            {
                continue;
            }

            int intersectionId = extractInt(intersection["id"]["id"]);
            int currentRevision = intersection.HasMember("revision")
                                      ? extractInt(intersection["revision"])
                                      : -1;

            // Create content hash excluding timestamps and revision
            std::string contentHash = createContentHash(intersection);

            if (auto prevIt = _prevMapIntersections.find(intersectionId); prevIt != _prevMapIntersections.end())
            {
                // Compare against previous state and check for revision violations
                bool changed = checkRevisionViolation(intersectionId, currentRevision,
                                                      contentHash, prevIt->second, "MAP", result);
                if (changed)
                {
                    anyChanged = true;
                }
            }
            else
            {
                // First time seeing this intersection — treat as new content
                anyChanged = true;
            }

            // Store current state for next message comparison
            _prevMapIntersections[intersectionId] = {currentRevision, contentHash};
        }

        return anyChanged;
    }

    void RevisionCounterValidator::validateMsgIssueRevision(int currentMsgRevision,
                                                            bool anyIntersectionChanged,
                                                            RevisionCounterResult &result)
    const {
        // Skip message-level validation if this is the first MAP message —
        // no previous msgIssueRevision to compare against
        if (!_hasMapPrevious)
        {
            return;
        }
        result.comparisonPerformed = true;

        bool msgRevisionChanged = (currentMsgRevision != _prevMapMessage.msgIssueRevision);

        if (anyIntersectionChanged && !msgRevisionChanged)
        {
            // CTI 4501 violation: at least one intersection's content changed
            // but the message-level revision counter was not incremented
            result.valid = false;
            result.violations.push_back(
                "MAP content changed but msgIssueRevision did not increment (msgIssueRevision=" +
                std::to_string(currentMsgRevision) + ")");
        }
        else if (!anyIntersectionChanged && msgRevisionChanged)
        {
            // CTI 4501 violation: no intersection content changed but the
            // message-level revision counter was incremented
            result.valid = false;
            result.violations.push_back(
                "MAP content unchanged but msgIssueRevision incremented (prev=" +
                std::to_string(_prevMapMessage.msgIssueRevision) +
                ", current=" + std::to_string(currentMsgRevision) + ")");
        }
    }

}