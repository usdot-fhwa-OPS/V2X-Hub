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
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <mutex>
#include <map>
#include <memory>
#include <PluginClientClockAware.h>
#include <jsoncpp/json/json.h>
#include <CTI4501ValidationMessage.h>
#include "RevisionCounterValidator.h"

#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <tmx/j2735_messages/SpatMessage.hpp>



namespace IntersectionValidation
{

    class IntersectionValidationPlugin : public tmx::utils::PluginClientClockAware
    {
    public:
        explicit IntersectionValidationPlugin(const std::string &name);
        ~IntersectionValidationPlugin() override = default;

        void UpdateConfigSettings();

        // Message handlers
        void HandleSpatMessage(tmx::messages::SpatMessage &msg, tmx::routeable_message &routeableMsg);
        void HandleMapDataMessage(tmx::messages::MapDataMessage &msg, tmx::routeable_message &routeableMsg);

    protected:
        void OnConfigChanged(const char *key, const char *value) override;
        void OnStateChange(IvpPluginState state) override;

    private:
        // Interval tracking
        uint64_t _lastMapTimeMs = 0;
        uint64_t _lastSpatTimeMs = 0;

        std::string spatSchemaPath = "/var/www/plugins/IntersectionValidationPlugin/resources/spat.schema.json";
        std::string mapSchemaPath = "/var/www/plugins/IntersectionValidationPlugin/resources/map.schema.json";
        uint spatFieldValidationErrors = 0;
        uint spatValidationPassed = 0;
        uint mapFieldValidationErrors = 0;
        uint mapValidationPassed = 0;
        std::string rsuSource;

        uint spatRevisionPassed = 0;
        uint mapRevisionPassed = 0;
        uint spatRevisionFailed = 0;
        uint mapRevisionFailed = 0;

        /**
         * @brief Measure message interval and broadcast TmxEventLogMessage if threshold exceeded.
         * @param lastTimestampMs reference to stored timestamp for this message type (updated in place).
         * @param requiredThresholdMs required threshold in ms.
         * @param maxThresholdMs maximum threshold in ms.
         * @param messageType label for logging (e.g. "SPaT", "MAP").
         */
        void measureMessageInterval(uint64_t &lastTimestampMs, uint64_t requiredThresholdMs, uint64_t maxThresholdMs, const std::string &messageType);

        /**
         * @brief Parse JSON, preprocess, run both field validation and
         *        revision counter validation on the same preprocessed document.
         *
         * @param jsonStr Raw JSON string from TMX.
         * @param schemaPath Path to the CTI 4501 schema file.
         * @param fieldEventType Event type for field validation failures (e.g. "SpatMinimumData").
         * @param revisionEventType Event type for revision violations (e.g. "SpatRevisionCounter").
         * @param messageType Display name ("SPaT" or "MAP").
         * @param intersectionId Intersection ID for event messages.
         * @param handlerBeginMs Timestamp when the handler started.
         * @return Result from revision counter validation check (including intersection info for revision changes)
         */
        RevisionCounterResult validateMessage(const std::string &jsonStr, const std::string &schemaPath,
                             const std::string &fieldEventType, const std::string &revisionEventType,
                             const std::string &messageType, int intersectionId,
                             uint64_t handlerBeginMs);

        /**
         * @brief Validate JSON string against schema file, and update plugin status and broadcast CTI4501ValidationMessage if validation fails.
         * @param doc The preprocessed JSON document to validate.
         * @param schemaDoc The JSON Schema document to validate against.
         * @param eventType The event type to use in the CTI4501ValidationMessage if validation fails.
         * @param messageType The message type label for logging and status updates (e.g. "SPaT", "MAP").
         * @param intersectionId The intersection ID to include in the CTI4501ValidationMessage if validation fails.
         * @param handlerBeginMs Timestamp in milliseconds when message handling began
         */
        void validateMessageFields(const rapidjson::Document &doc, const rapidjson::Document &schemaDoc,
                                    const std::string &eventType, const std::string &messageType,
                                    int intersectionId, uint64_t handlerBeginMs);

        /**
         * @brief Validate revision counters increase when message changes
         * @param doc The preprocessed JSON document to validate.
         * @param eventType The event type to use in the CTI4501ValidationMessage if validation fails.
         * @param messageType The message type label for logging and status updates (e.g. "SPaT", "MAP").
         * @param intersectionId The intersection ID to include in the CTI4501ValidationMessage if validation fails.
         * @param handlerBeginMs Timestamp in milliseconds when message handling began
         * @return Result from revision counter validation check
         */
        RevisionCounterResult validateRevisionCounters(const rapidjson::Document &doc,
                                       const std::string &eventType, const std::string &messageType,
                                       int intersectionId, uint64_t handlerBeginMs);

        /**
         * @brief Set IvpMsgFlags_Validated on a routeable message and broadcast it.
         *        Single-sources the validated-flag spelling for both SPaT and MAP.
         */
        void broadcastValidated(tmx::routeable_message &msg);

        /**
         * @brief Apply per-intersection revision corrections to the SPaT message and
         *        re-broadcast the validated SPaT with IvpMsgFlags_Validated set.
         */
        void forwardValidatedSpat(tmx::messages::SpatMessage &msg, tmx::routeable_message &routeableMsg,
                                  const std::shared_ptr<SPAT> &spatDataRef,
                                  const std::map<int, int> &corrections);

        /**
         * @brief Apply intersection and message revision corrections MAP ASN.1 message, then re-broadcast
         *        the validated MAP with IvpMsgFlags_Validated set.
         * @param msgRevisionCorrection corrected msgIssueRevision, or -1 for none.
         */
        void forwardValidatedMap(tmx::messages::MapDataMessage &msg, tmx::routeable_message &routeableMsg,
                                 const std::shared_ptr<MapData> &mapDataRef,
                                 const std::map<int, int> &corrections,
                                 int msgRevisionCorrection);

        // Revision counter validator — stores previous message state and
        // compares against current to detect CTI 4501 revision violations
        RevisionCounterValidator _revisionValidator;

        // CTI 4501 thresholds
        static constexpr uint64_t SPAT_INTERVAL_MAX_THRESHOLD_MS = 300;
        static constexpr uint64_t MAP_INTERVAL_MAX_THRESHOLD_MS = 100;
        static constexpr uint64_t SPAT_INTERVAL_REQUIRED_MS = 125;
        static constexpr uint64_t MAP_INTERVAL_REQUIRED_MS = 1000;

        static inline const std::string EVENT_MAX_THRESHOLD = " Message interval exceeded CTI 4501 maximum threshold of ";
        static inline const std::string EVENT_REQUIRED_THRESHOLD = " Message interval exceeded CTI 4501 required threshold of ";
        
        static inline const std::string EVENT_FIELD_VALIDATION_FAILED = " Message failed CTI 4501 field validation: ";
    };
}