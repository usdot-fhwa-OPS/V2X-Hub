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
#include "MessageIntervalValidator.h"
#include "IntersectionValidationUtils.h"

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
        // Interval tracking, one aggregation window per message type
        MessageIntervalValidator _spatIntervalValidator{SPAT_INTERVAL_MAX_MS};
        MessageIntervalValidator _mapIntervalValidator{MAP_INTERVAL_MAX_MS};

        std::string spatSchemaPath = "/var/www/plugins/IntersectionValidationPlugin/resources/spat.schema.json";
        std::string mapSchemaPath = "/var/www/plugins/IntersectionValidationPlugin/resources/map.schema.json";
        uint spatFieldValidationErrors = 0;
        uint spatValidationPassed = 0;
        uint mapFieldValidationErrors = 0;
        uint mapValidationPassed = 0;
        std::string rsuSource; // TODO: Instead of setting the rsu IP here, have the message receiver grab the IP and attach it to the message
        uint64_t BroadcastRateTimeWindow;
        uint64_t ContentValidationTimeWindow;

        uint spatRevisionPassed = 0;
        uint mapRevisionPassed = 0;
        uint spatRevisionFailed = 0;
        uint mapRevisionFailed = 0;

        // Identifier of the measured input stream, reported as topicName on a BroadcastRate event
        std::string spatInputTopic = "topic.ProcessedSpat";
        std::string mapInputTopic = "topic.ProcessedMap";

        std::map<std::string, tmx::messages::CTI4501ValidationMessage> _lastContentValidationMessage;
        std::map<std::string, tmx::messages::CTI4501ValidationMessage> _lastRevisionValidationMessage;



        /**
         * @brief Record a message arrival against its interval validator
         * @param validator interval validator for this message type
         * @param messageType SPat or MAP 
         * @param intersectionId intersection the message belongs to
         */
        void measureMessageInterval(MessageIntervalValidator &validator, const std::string &messageType,
                                    int intersectionId);

        /**
         * @brief Broadcast the CTI4501ValidationMessage and TmxEventLogMessage for a closed
         *        aggregation window
         * @param result counts from the window
         * @param messageType SPat or MAP
         */
        void publishBroadcastRateEvent(const IntervalWindowResult &result, const std::string &messageType);

        /**
         * @brief Parse JSON, preprocess, run both field validation and
         *        revision counter validation on the same preprocessed document.
         *
         * @param jsonStr Raw JSON string from TMX.
         * @param schemaPath Path to the CTI 4501 schema file.
         * @param fieldEventType Event type for field validation failures (e.g. "SpatMinimumData").
         * @param revisionEventType Event type for revision violations (e.g. "SpatMessageCountProgression").
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
         * @param contentChanged Boolean value indicating if the content of the message 
         has changed
         */
        void validateMessageFields(const rapidjson::Document &doc, const rapidjson::Document &schemaDoc,
                                    const std::string &eventType, const std::string &messageType,
                                    int intersectionId, uint64_t handlerBeginMs, bool contentChanged);


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
                                       int intersectionId);

        /**
         * @brief Set IvpMsgFlags_Validated on a routeable message and broadcast it.
         *        Single-sources the validated-flag spelling for both SPaT and MAP.
         */
        void broadcastValidated(tmx::routeable_message &msg);

        // Revision counter validator — stores previous message state and
        // compares against current to detect CTI 4501 revision violations
        RevisionCounterValidator _revisionValidator;

        static inline const std::string EVENT_FIELD_VALIDATION_FAILED = " encountered CTI 4501 MinimumDataEvent: ";
    };
    /** @brief Compare two vectors of missing data elements for equality.
     *  @param a First vector of missing data elements.
     *  @param b Second vector of missing data elements.
     *  @return True if the vectors are equal, false otherwise.
     */
    bool compareMissingDataElements(const std::vector<tmx::messages::MissingDataElement> &a,
                            const std::vector<tmx::messages::MissingDataElement> &b);
    /** @brief Compare two CTI 4501 revision count validation messages for and only returns true if they are equal
     * and revision count is 0 accross the board.
     *  @param a First CTI 4501 validation message.
     *  @param b Second CTI 4501 validation message.
     *  @return True if the messages are equal and revision count is 0, false otherwise.
     *  @note This is used to throttle duplicate CTI 4501 revision count validation events when revision count is never incremented.
     */
    bool compareRevisionValidationMessages(tmx::messages::CTI4501ValidationMessage &a,
                            tmx::messages::CTI4501ValidationMessage &b);
}