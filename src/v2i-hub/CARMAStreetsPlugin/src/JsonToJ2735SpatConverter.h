
#include "jsoncpp/json/json.h"
#include <memory>
#include <chrono>
#include <string>
#include <iostream>
#include <sstream>
#include <tmx/j2735_messages/SpatMessage.hpp>

namespace CARMAStreetsPlugin
{
    class JsonToJ2735SpatConverter
    {
    public:
        JsonToJ2735SpatConverter()= default;
        /**
         * @brief Convert the Json value with spat information info tmx SPAT object.
         * @param json Incoming Json value with spat information that is consumed from a Kafka topic.
         * @param spat Outgoing J2735 spat object that is populated by the json value.
         */
        void convertJson2Spat(const Json::Value& spat_json, SPAT *spat) const;

        /**
         * @brief Convert the Json value with intersection information info J2735 IntersectionStateList object.
         * @param json Incoming Json value with spat information that is consumed from a Kafka topic.
         * @param intersections Outgoing J2735 IntersectionStateList object that is populated by the json value.
         */
        void convertJson2IntersectionStateList(const Json::Value& intersections_json, IntersectionStateList *intersections) const;

        /**
         * @brief Convert the Json value with MovementList information info J2735 MovementList object.
         * @param json Incoming Json value with MovementList information that is consumed from a Kafka topic.
         * @param states Outgoing J2735 MovementList object that is populated by the json value.
         */
        void convertJson2MovementList(const Json::Value& movements_json, MovementList *states) const;

        /**
         * @brief Convert the Json value with MovementEventList information info J2735 MovementEventList object.
         * @param json Incoming Json value with MovementEventList information that is consumed from a Kafka topic.
         * @param state_time_speed Outgoing J2735 MovementEventList object that is populated by the json value.
         */
        void convertJson2MovementEventList(const Json::Value& movement_events_json, MovementEventList *state_time_speed) const;

        /**
         * @brief Convert the Json value with TimeChangeDetails information info J2735 TimeChangeDetails object.
         * @param json Incoming Json value with TimeChangeDetails information that is consumed from a Kafka topic.
         * @param time_change_detail_json Outgoing J2735 TimeChangeDetails object that is populated by the json value.
         */
        void convertJson2TimeChangeDetail(const Json::Value& time_change_detail_json, TimeChangeDetails_t *timing) const;

        /**
         * @brief Convert the Json value with ManeuverAssistList information info J2735 ManeuverAssistList object.
         * @param json Incoming Json value with ManeuverAssistList information that is consumed from a Kafka topic.
         * @param maneuver_assist_list Outgoing J2735 ManeuverAssistList object that is populated by the json value.
         */
        void convertJson2ManeuverAssistList(const Json::Value& maneuver_assist_list_json, ManeuverAssistList *maneuver_assist_list) const;

        /**
         * @brief Convert the Json value with AdvisorySpeedList information info J2735 AdvisorySpeedList object.
         * @param json Incoming Json value with AdvisorySpeedList information that is consumed from a Kafka topic.
         * @param maneuver_assist_list Outgoing J2735 AdvisorySpeedList object that is populated by the json value.
         */
        void convertJson2AdvisorySpeed(const Json::Value& speeds_json, AdvisorySpeedList_t *speeds) const;

        /**
         * @brief Encode the J2735 SPAT object
         * @param spat Incoming J2735 SPAT object.
         * @param encodedSpat Outgoing encoded SPAT object populated by the Incoming SPAT object.
         */
        void encodeSpat(std::unique_ptr<tmx::messages::SpatMessage> &spat_message, tmx::messages::SpatEncodedMessage &encodedSpat) const;

        ~JsonToJ2735SpatConverter() = default;
    };
}