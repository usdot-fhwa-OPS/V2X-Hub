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
    namespace
    {
        void evaluateProgression(IntersectionChangeInfo &change, int prevRevision,
                                 const MsgProgressionCtx &ctx)
        {
            const bool revisionProperIncrement = ((prevRevision + 1) % 128 == change.currentRevision);

            if (ctx.mapStyle)
            {
                // Check intersection revision AND message-level msgIssueRevision
                const bool revisionBad = change.contentChanged && !revisionProperIncrement;
                
                if (const bool msgRevisionBad = change.contentChanged && ctx.hasMsgPrev &&
                        ((ctx.prevMsgRevision + 1) % 128 != ctx.currentMsgRevision); !revisionBad && !msgRevisionBad)
                {
                    return;
                }
                change.progressionViolation = true;
                if (revisionBad)
                {
                    change.progressionCountA = prevRevision;
                    change.progressionCountB = change.currentRevision;
                }
                else
                {
                    change.progressionCountA = ctx.prevMsgRevision;
                    change.progressionCountB = ctx.currentMsgRevision;
                }
                return;
            }

            const bool noChange = (!change.contentChanged && !change.revisionChanged);

            if (const bool validIncrement = (change.contentChanged && revisionProperIncrement); noChange || validIncrement)
            {
                return;
            }
            change.progressionViolation = true;
            change.progressionCountA = prevRevision;
            change.progressionCountB = change.currentRevision;
        }
    } // namespace


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

    void RevisionCounterValidator::processIntersectionArray(const rapidjson::Value &intersections,
                                                            std::unordered_map<int, IntersectionState> &prevStates,
                                                            const std::string &messageType,
                                                            RevisionCounterResult &result,
                                                            bool &anyChanged,
                                                            const std::string &currentTimestamp,
                                                            const MsgProgressionCtx &progressionCtx)
    {
        for (rapidjson::SizeType i = 0; i < intersections.Size(); ++i)
        {
            const auto &intersection = intersections[i];

            if (!intersection.HasMember("id") || !intersection["id"].HasMember("id"))
            {
                continue;
            }

            // Values are already integers after convertNumericStrings
            int intersectionId = intersection["id"]["id"].GetInt();
            int currentRevision = intersection.HasMember("revision")
                                      ? intersection["revision"].GetInt()
                                      : -1;

            std::string contentHash = createContentHash(intersection);

            IntersectionChangeInfo change;
            change.id = intersectionId;
            change.currentRevision = currentRevision;
            change.timestampB = currentTimestamp;

            if (auto prevIt = prevStates.find(intersectionId); prevIt == prevStates.end())
            {
                // First time we've seen this intersection: nothing to compare against,
                // so no progression event (matches conflictmonitor requiring a prior state).
                change.contentChanged = true;
                anyChanged = true;
            }
            else
            {
                change.contentChanged = checkRevisionViolation(intersectionId, currentRevision, contentHash,
                                                               prevIt->second, messageType, result);
                change.revisionChanged = (currentRevision != prevIt->second.revision);
                change.timestampA = prevIt->second.timestamp;
                anyChanged = anyChanged || change.contentChanged;

                // MessageCountProgression mirror (SPaT/MAP rule lives in evaluateProgression).
                evaluateProgression(change, prevIt->second.revision, progressionCtx);
            }

            result.intersectionChanges.push_back(change);
            prevStates[intersectionId] = {currentRevision, contentHash, currentTimestamp};
        }
    }

    RevisionCounterResult RevisionCounterValidator::validateSpatRevision(const rapidjson::Document &doc,
                                                                         const std::string &currentTimestamp)
    {
        RevisionCounterResult result;

        if (doc.HasParseError())
        {
            result.valid = false;
            result.violations.emplace_back("Failed to parse SPaT JSON");
            return result;
        }

        // Navigate to intersections array
        if (!doc.HasMember("value") || !doc["value"].HasMember("SPAT") ||
            !doc["value"]["SPAT"].HasMember("intersections") ||
            !doc["value"]["SPAT"]["intersections"].IsArray())
        {
            return result;
        }

        bool anyChanged = false; 
        processIntersectionArray(doc["value"]["SPAT"]["intersections"], _prevSpatIntersections,
                                 "SPaT", result, anyChanged, currentTimestamp);

        return result;
    }

    RevisionCounterResult RevisionCounterValidator::validateMapRevision(const rapidjson::Document &doc,
                                                                        const std::string &currentTimestamp)
    {
        RevisionCounterResult result;

        if (doc.HasParseError())
        {
            result.valid = false;
            result.violations.emplace_back("Failed to parse MAP JSON");
            return result;
        }

        if (!doc.HasMember("value") || !doc["value"].HasMember("MapData"))
        {
            return result;
        }

        const auto &mapData = doc["value"]["MapData"];

        // Values are already integers after convertNumericStrings
        int currentMsgRevision = mapData.HasMember("msgIssueRevision")
                                     ? mapData["msgIssueRevision"].GetInt()
                                     : -1;

        bool anyIntersectionChanged = false;

        if (mapData.HasMember("intersections") && mapData["intersections"].IsArray())
        {
            anyIntersectionChanged = validateMapIntersections(
                mapData["intersections"], result, currentTimestamp, currentMsgRevision);
        }

        validateMsgIssueRevision(currentMsgRevision, anyIntersectionChanged, result);

        // Capture message-level revision state
        result.currentMsgRevision = currentMsgRevision;
        if (_hasMapPrevious)
        {
            result.hasMsgRevision     = true;
            result.msgRevisionChanged = (currentMsgRevision != _prevMapMessage.msgIssueRevision);
        }

        _prevMapMessage.msgIssueRevision = currentMsgRevision;
        _hasMapPrevious = true;

        return result;
    }

    bool RevisionCounterValidator::validateMapIntersections(const rapidjson::Value &intersections,
                                                            RevisionCounterResult &result,
                                                            const std::string &currentTimestamp,
                                                            int currentMsgRevision)
    {
        bool anyChanged = false;
        // _prevMapMessage still holds the PREVIOUS msgIssueRevision here; it is not
        // updated until after this call returns, so it is safe to pass as prevMsgRevision.
        processIntersectionArray(intersections, _prevMapIntersections, "MAP", result, anyChanged,
                                 currentTimestamp,
                                 MsgProgressionCtx{/*mapStyle=*/true, _hasMapPrevious,
                                                   _prevMapMessage.msgIssueRevision, currentMsgRevision});
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