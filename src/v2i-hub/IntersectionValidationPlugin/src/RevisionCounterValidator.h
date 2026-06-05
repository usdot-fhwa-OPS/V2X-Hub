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
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <rapidjson/document.h>

namespace IntersectionValidation
{

    /**
     * @brief Result of a revision counter validation check.
     */
    struct RevisionCounterResult
    {
        bool valid = true;
        bool comparisonPerformed = false;
        std::vector<std::string> violations;
    };

    /**
     * @brief Validates that SPaT and MAP revision counters follow CTI 4501 rules.
     *
     *        CTI 4501 requires:
     *        - Revision counters increment when message content changes (excluding timestamps)
     *        - Revision counters stay the same when content hasn't changed
     *
     *        This class stores the previous message state per intersection ID and
     *        compares it against the current message to detect violations.
     */
    class RevisionCounterValidator
    {
    public:
        RevisionCounterValidator() = default;

        /**
         * @brief Validate SPaT intersection revision counters. For each intersection in SPaT message,
         * 
         * - Strip timestamp field for the comparison
         * - If content is changed, check if revision was increased
         * - If content is not changed, check if revision stayed the same
         * - Update the internal state with the current message for next comparison
         *
         * @param doc The full SPaT MessageFrame JSON document.
         * @return RevisionCounterResult with any violations found.
         */
        RevisionCounterResult validateSpatRevision(const rapidjson::Document &doc);

        /**
         * @brief Validate MAP message-level and intersection-level revision counters.
         *
         * - Check if any intersesction level content has changed
         * - Check message-level msgIssueRevision based on whether any intersection content changed
         * - Updates internal state with curent message for next comparison
         *
         * @param doc The full MAP MessageFrame JSON document.
         * @return RevisionCounterResult with any violations found.
         */
        RevisionCounterResult validateMapRevision(const rapidjson::Document &doc);

    private:
        /**
         * @brief Stored state for a previous intersection message.
         */
        struct IntersectionState
        {
            int revision = -1;
            std::string contentHash; // JSON content with timestamps stripped
        };

        /**
         * @brief Stored state for previous MAP message-level data.
         */
        struct MapMessageState
        {
            int msgIssueRevision = -1;
        };

        // Previous SPaT intersection states, keyed by intersection ID
        std::unordered_map<int, IntersectionState> _prevSpatIntersections;

        // Previous MAP intersection states, keyed by intersection ID
        std::unordered_map<int, IntersectionState> _prevMapIntersections;

        // Previous MAP message-level state
        MapMessageState _prevMapMessage;
        bool _hasMapPrevious = false;

        /**
         * @brief Create a deep copy of a JSON value, stripping timestamp fields.
         *
         *        Removes fields named "timeStamp" at any
         *        depth so that content comparison ignores time-varying fields.
         *
         * @param value The JSON value to copy and strip.
         * @param allocator The allocator for the copy.
         * @return A new JSON value with timestamp fields removed.
         */
        static rapidjson::Value stripTimestamps(const rapidjson::Value &value,
                                                 rapidjson::Document::AllocatorType &allocator);

        /**
         * @brief Convert a JSON value to a string for comparison.
         * @param value The JSON value to convert.
         * @return A JSON string representation.
         */
        static std::string toJSONString(const rapidjson::Value &value);

        /**
         * @brief Create a content hash for an intersection, stripping timestamps
         *        and the revision field itself.
         * @param intersection The intersection JSON value.
         * @return A JSON string for content comparison.
         */
        static std::string createContentHash(const rapidjson::Value &intersection);

        /**
         * @brief Compare current intersection state against previous and report violations.
         * @param intersectionId The intersection ID for violation messages.
         * @param currentRevision The current revision counter value.
         * @param contentHash The current content hash.
         * @param prevState The previous intersection state.
         * @param messageType "SPaT" or "MAP" for violation messages.
         * @param result The result to append violations to.
         * @return true if content changed, false otherwise.
         */
        static bool checkRevisionViolation(int intersectionId, int currentRevision,
                                                          const std::string_view &contentHash,
                                                          const IntersectionState &prevState,
                                                          const std::string &messageType,
                                                          RevisionCounterResult &result);

        /**
         * @brief Validate revision counters for each intersection in a MAP message.
         * @param intersections The intersections JSON array.
         * @param result The result to append violations to.
         * @return true if any intersection content changed.
         */
        bool validateMapIntersections(const rapidjson::Value &intersections,
                                      RevisionCounterResult &result);

        /**
         * @brief Validate the message-level msgIssueRevision counter.
         * @param currentMsgRevision Current msgIssueRevision value.
         * @param anyIntersectionChanged Whether any intersection content changed.
         * @param result The result to append violations to.
         */
        void validateMsgIssueRevision(int currentMsgRevision,
                                       bool anyIntersectionChanged,
                                       RevisionCounterResult &result) const;
    };

}