#include "JsonToJ2735SpatConverter.h"

namespace CARMAStreetsPlugin
{
    void JsonToJ2735SpatConverter::convertJson2Spat(const Json::Value &spat_json, SPAT *spat) const
    {
        ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SPAT, spat);
        std::string name = spat_json["name"].asString();
        int32_t timestamp = spat_json["timestamp"].asInt();

        // Parse spat name field
        spat->name = (DescriptiveName_t *)calloc(1, sizeof(DescriptiveName_t));
        spat->name->buf = (uint8_t *)calloc(1, name.length());
        spat->name->size = name.length();
        memcpy(spat->name->buf, name.c_str(), name.length());

        spat->timeStamp = (MinuteOfTheYear_t *)calloc(1, sizeof(MinuteOfTheYear_t));
        *spat->timeStamp = timestamp;

        // Parse intersections
        if (spat_json["intersections"].isArray())
        {
            convertJson2IntersectionStateList(spat_json["intersections"], &spat->intersections);
        }
    }

    void JsonToJ2735SpatConverter::convertJson2IntersectionStateList(const Json::Value &intersections_json, IntersectionStateList *intersections) const
    {
        for (const auto &int_json : intersections_json)
        {
            auto intersection = (IntersectionState_t *)calloc(1, sizeof(IntersectionState_t));
            // intersection name
            auto name = int_json["name"].asString();
            auto intersection_name = (DescriptiveName_t *)calloc(1, sizeof(DescriptiveName_t));
            intersection_name->size = name.length();
            intersection_name->buf = (uint8_t *)calloc(1, name.length());
            memcpy(intersection_name->buf, name.c_str(), name.length());

            // intersection id
            auto id = int_json["id"].asInt();
            intersection->id.id = id;
            intersection->name = intersection_name;

            // revision
            auto revision = int_json["revision"].asInt();
            intersection->revision = revision;

            // moy
            auto moy_int = int_json["moy"].asInt();
            intersection->moy = (MinuteOfTheYear_t *)calloc(1, sizeof(MinuteOfTheYear_t));
            *intersection->moy = moy_int;

            // timestamp
            intersection->timeStamp = (DSecond_t *)calloc(1, sizeof(DSecond_t));
            *intersection->timeStamp = int_json["time_stamp"].asInt();

            // status
            auto status = static_cast<int16_t>(int_json["status"].asInt());
            intersection->status.buf = (uint8_t *)calloc(2, sizeof(uint8_t));
            intersection->status.size = 2 * sizeof(uint8_t);
            intersection->status.bits_unused = 0;
            intersection->status.buf[1] = static_cast<int8_t>(status);
            intersection->status.buf[0] = (status >> 8);

            // enabled_lanes
            const auto& enabled_lanes_json = int_json["enabled_lanes"];
            if (enabled_lanes_json.isArray())
            {
                intersection->enabledLanes = (EnabledLaneList_t *)calloc(1, sizeof(EnabledLaneList_t));
                for (const auto &laneId : enabled_lanes_json)
                {
                    auto lane = (LaneID_t *)calloc(1, sizeof(LaneID_t));
                    *lane = laneId.asInt();
                    ASN_SEQUENCE_ADD(&intersection->enabledLanes->list, lane);
                }
            }

            // Movements
            if (int_json["states"].isArray())
            {
                convertJson2MovementList(int_json["states"], &intersection->states);
            }

            // Manuever Assist List
            if (int_json["maneuver_assist_list"].isArray())
            {
                intersection->maneuverAssistList = (ManeuverAssistList_t *)calloc(1, sizeof(ManeuverAssistList_t));
                convertJson2ManeuverAssistList(int_json["maneuver_assist_list"], intersection->maneuverAssistList);
            }
            ASN_SEQUENCE_ADD(&intersections->list, intersection);
        }
    }
    void JsonToJ2735SpatConverter::convertJson2MovementList(const Json::Value &movements_json, MovementList *states) const
    {
        for (const auto &movement_json : movements_json)
        {
            auto state = (MovementState_t *)calloc(1, sizeof(MovementState_t));

            auto movement_name_str = movement_json["movement_name"].asString();
            state->movementName = (DescriptiveName_t *)calloc(1, sizeof(DescriptiveName_t));
            state->movementName->size = movement_name_str.length();
            state->movementName->buf = (uint8_t *)calloc(1, movement_name_str.length());
            memcpy(state->movementName->buf, movement_name_str.c_str(), movement_name_str.length());

            state->signalGroup = movement_json["signal_group"].asInt();

            if (movement_json["state_time_speed"].isArray())
            {
                convertJson2MovementEventList(movement_json["state_time_speed"], &state->state_time_speed);
            }

            if (movement_json["maneuver_assist_list"].isArray())
            {
                state->maneuverAssistList = (ManeuverAssistList_t *)calloc(1, sizeof(ManeuverAssistList_t));
                convertJson2ManeuverAssistList(movement_json["maneuver_assist_list"], state->maneuverAssistList);
            }
            ASN_SEQUENCE_ADD(&states->list, state);
        }
    }

    void JsonToJ2735SpatConverter::convertJson2MovementEventList(const Json::Value &movement_event_list_json, MovementEventList *state_time_speed) const
    {
        for (const auto &m_event : movement_event_list_json)
        {
            auto movement_event = (MovementEvent_t *)calloc(1, sizeof(MovementEvent_t));
            movement_event->eventState = m_event["event_state"].asInt();

            // Timing
            const auto& timing_json = m_event["timing"];
            movement_event->timing = (TimeChangeDetails_t *)calloc(1, sizeof(TimeChangeDetails_t));
            convertJson2TimeChangeDetail(timing_json, movement_event->timing);

            // speeds
            const auto& speeds_json = m_event["speeds"];
            if (speeds_json.isArray())
            {
                movement_event->speeds = (AdvisorySpeedList_t *)calloc(1, sizeof(AdvisorySpeedList_t));
                convertJson2AdvisorySpeed(speeds_json, movement_event->speeds);
            }

            ASN_SEQUENCE_ADD(&state_time_speed->list, movement_event);
        }
    }

    void JsonToJ2735SpatConverter::convertJson2TimeChangeDetail(const Json::Value &time_change_detail_json, TimeChangeDetails_t *timing) const
    {
        timing->startTime = (DSRC_TimeMark_t *)calloc(1, sizeof(DSRC_TimeMark_t));
        *timing->startTime = time_change_detail_json["start_time"].asInt();

        timing->minEndTime = time_change_detail_json["min_end_time"].asInt();

        timing->maxEndTime = (DSRC_TimeMark_t *)calloc(1, sizeof(DSRC_TimeMark_t));
        *timing->maxEndTime = time_change_detail_json["max_end_time"].asInt();

        timing->likelyTime = (DSRC_TimeMark_t *)calloc(1, sizeof(DSRC_TimeMark_t));
        *timing->likelyTime = time_change_detail_json["likely_time"].asInt();

        timing->nextTime = (DSRC_TimeMark_t *)calloc(1, sizeof(DSRC_TimeMark_t));
        *timing->nextTime = time_change_detail_json["next_time"].asInt();

        timing->confidence = (TimeIntervalConfidence_t *)calloc(1, sizeof(TimeIntervalConfidence_t));
        *timing->confidence = time_change_detail_json["confidence"].asInt();
    }

    void JsonToJ2735SpatConverter::convertJson2AdvisorySpeed(const Json::Value &speeds_json, AdvisorySpeedList_t *speeds) const
    {
        for (const auto &speed_json : speeds_json)
        {
            auto speed = (AdvisorySpeed_t *)calloc(1, sizeof(AdvisorySpeed_t));
            speed->type = speed_json["type"].asInt();

            speed->speed = (SpeedAdvice_t *)calloc(1, sizeof(SpeedAdvice_t));
            *speed->speed = speed_json["speed_limit"].asInt();

            speed->confidence = (SpeedConfidence_t *)calloc(1, sizeof(SpeedConfidence_t));
            *speed->confidence = speed_json["speed_confidence"].asInt();

            speed->Class = (RestrictionClassID_t *)calloc(1, sizeof(RestrictionClassID_t));
            *speed->Class = speed_json["class"].asInt();

            speed->distance = (ZoneLength_t *)calloc(1, sizeof(ZoneLength_t));
            *speed->distance = speed_json["distance"].asInt();

            ASN_SEQUENCE_ADD(&speeds->list, speed);
        }
    }

    void JsonToJ2735SpatConverter::convertJson2ManeuverAssistList(const Json::Value &maneuver_assist_list_json, ManeuverAssistList_t *maneuver_assist_list) const
    {
        for (const auto &masst : maneuver_assist_list_json)
        {
            auto maneuver_assist = (ConnectionManeuverAssist_t *)calloc(1, sizeof(ConnectionManeuverAssist_t));
            maneuver_assist->connectionID = masst["connection_id"].asInt();

            maneuver_assist->queueLength = (ZoneLength_t *)calloc(1, sizeof(ZoneLength_t));
            *maneuver_assist->queueLength = masst["queue_length"].asInt();

            maneuver_assist->availableStorageLength = (ZoneLength_t *)calloc(1, sizeof(ZoneLength_t));
            *maneuver_assist->availableStorageLength = masst["available_storage_length"].asInt();

            maneuver_assist->waitOnStop = (WaitOnStopline_t *)calloc(1, sizeof(WaitOnStopline_t));
            *maneuver_assist->waitOnStop = masst["wait_on_stop"].asBool() ? 1 : 0;

            maneuver_assist->pedBicycleDetect = (PedestrianBicycleDetect_t *)calloc(1, sizeof(PedestrianBicycleDetect_t));
            *maneuver_assist->pedBicycleDetect = masst["ped_bicycle_detect"].asBool() ? 1 : 0;

            ASN_SEQUENCE_ADD(&maneuver_assist_list->list, maneuver_assist);
        }
    }

    void JsonToJ2735SpatConverter::encodeSpat(const std::shared_ptr<SPAT> &spat_ptr, tmx::messages::SpatEncodedMessage &encodedSpat) const
    {
        tmx::messages::MessageFrameMessage frame(spat_ptr);
        encodedSpat.set_data(tmx::messages::TmxJ2735EncodedMessage<SPAT>::encode_j2735_message<tmx::messages::codec::uper<tmx::messages::MessageFrameMessage>>(frame));
        free(frame.get_j2735_data().get());
    }
}