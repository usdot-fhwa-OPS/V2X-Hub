#include "JsonToJ2735SpatConverter.h"

namespace CARMAStreetsPlugin
{
    void JsonToJ2735SpatConverter::convertJson2Spat(const Json::Value spat_json, SPAT *spat) const
    {
        std::string name = spat_json["name"].asString();
        int32_t timestamp = spat_json["timestamp"].asInt();

        // Parse spat name field
        auto spat_name = (DescriptiveName_t *)calloc(1, sizeof(DescriptiveName_t));
        spat_name->buf = (uint8_t *)calloc(1, name.length());
        spat_name->size = name.length();
        memcpy(spat_name->buf, name.c_str(), name.length());
        spat->name = spat_name;

        auto min_of_year_timestamp = (MinuteOfTheYear_t *)calloc(1, sizeof(MinuteOfTheYear_t));
        *min_of_year_timestamp = timestamp;
        spat->timeStamp = min_of_year_timestamp;

        // Parse intersections
        if (spat_json["intersections"].isArray())
        {
            auto intersections = (IntersectionStateList *)calloc(1, sizeof(IntersectionStateList));
            convertJson2IntersectionStateList(spat_json["intersections"], intersections);
            spat->intersections = *intersections;
        }
    }

    void JsonToJ2735SpatConverter::convertJson2IntersectionStateList(const Json::Value intersections_json, IntersectionStateList *intersections) const
    {
        for (auto int_json : intersections_json)
        {
            IntersectionState_t *intersection = (IntersectionState_t *)calloc(1, sizeof(IntersectionState_t));
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
            auto moy = (MinuteOfTheYear_t *)calloc(1, sizeof(MinuteOfTheYear_t));
            *moy = moy_int;
            intersection->moy = moy;

            // timestamp
            auto time_stamp = int_json["time_stamp"];
            auto timeStamp = (DSecond_t *)calloc(1, sizeof(DSecond_t));
            *timeStamp = time_stamp.asInt();
            intersection->timeStamp = timeStamp;

            // status
            int16_t status_int = int_json["status"].asInt();
            intersection->status.buf = (uint8_t *)calloc(2, sizeof(uint8_t));
            intersection->status.size = 2 * sizeof(uint8_t);
            intersection->status.bits_unused = 0;
            intersection->status.buf[1] = status_int;
            intersection->status.buf[0] = (status_int >> 8);

            // enabled_lanes
            auto enabled_lanes_json = int_json["enabled_lanes"];
            if (enabled_lanes_json.isArray())
            {
                auto enabled_lanes = (EnabledLaneList_t *)calloc(1, sizeof(EnabledLaneList_t));
                for (auto laneId : enabled_lanes_json)
                {
                    LaneID_t *lane = (LaneID_t *)calloc(1, sizeof(LaneID_t));
                    *lane = laneId.asInt();
                    ASN_SEQUENCE_ADD(&enabled_lanes->list, lane);
                }
                intersection->enabledLanes = enabled_lanes;
            }

            // Movements
            if (int_json["states"].isArray())
            {
                auto states = (MovementList_t *)calloc(1, sizeof(MovementList_t));
                convertJson2MovementList(int_json["states"], states);
                intersection->states = *states;
            }

            // Manuever Assist List
            if (int_json["maneuver_assist_list"].isArray())
            {
                auto maneuver_assist_list = (ManeuverAssistList_t *)calloc(1, sizeof(ManeuverAssistList_t));
                convertJson2ManeuverAssistList(int_json["maneuver_assist_list"], maneuver_assist_list);
                intersection->maneuverAssistList = maneuver_assist_list;
            }
            ASN_SEQUENCE_ADD(&intersections->list, intersection);
        }
    }
    void JsonToJ2735SpatConverter::convertJson2MovementList(const Json::Value movements_json, MovementList *states) const
    {
        for (auto movement_json : movements_json)
        {
            auto state = (MovementState_t *)calloc(1, sizeof(MovementState_t));

            auto movement_name_str = movement_json["movement_name"].asString();
            auto movement_name = (DescriptiveName_t *)calloc(1, sizeof(DescriptiveName_t));
            movement_name->size = movement_name_str.length();
            movement_name->buf = (uint8_t *)calloc(1, movement_name_str.length());
            memcpy(movement_name->buf, movement_name_str.c_str(), movement_name_str.length());
            state->movementName = movement_name;

            auto sg_id = movement_json["signal_group"].asInt();
            state->signalGroup = sg_id;

            if (movement_json["state_time_speed"].isArray())
            {
                auto state_time_speed = (MovementEventList_t *)calloc(1, sizeof(MovementEventList_t));
                convertJson2MovementEventList(movement_json["state_time_speed"], state_time_speed);
                state->state_time_speed = *state_time_speed;
            }

            if (movement_json["maneuver_assist_list"].isArray())
            {
                auto maneuver_assist_list = (ManeuverAssistList_t *)calloc(1, sizeof(ManeuverAssistList_t));
                convertJson2ManeuverAssistList(movement_json["maneuver_assist_list"], maneuver_assist_list);
                state->maneuverAssistList = maneuver_assist_list;
            }
            ASN_SEQUENCE_ADD(&states->list, state);
        }
    }

    void JsonToJ2735SpatConverter::convertJson2MovementEventList(const Json::Value movement_event_list_json, MovementEventList *state_time_speed) const
    {
        for (auto m_event : movement_event_list_json)
        {
            auto movement_event = (MovementEvent_t *)calloc(1, sizeof(MovementEvent_t));
            movement_event->eventState = m_event["event_state"].asInt();

            // Timing
            auto timing_json = m_event["timing"];
            auto timing = (TimeChangeDetails_t *)calloc(1, sizeof(TimeChangeDetails_t));
            convertJson2TimeChangeDetail(timing_json, timing);
            movement_event->timing = timing;

            // speeds
            auto speeds_json = m_event["speeds"];
            if (speeds_json.isArray())
            {
                auto speeds = (AdvisorySpeedList_t *)calloc(1, sizeof(AdvisorySpeedList_t));
                convertJson2AdvisorySpeed(speeds_json, speeds);
                movement_event->speeds = speeds;
            }

            ASN_SEQUENCE_ADD(&state_time_speed->list, movement_event);
        }
    }

    void JsonToJ2735SpatConverter::convertJson2TimeChangeDetail(const Json::Value time_change_detail_json, TimeChangeDetails_t *timing) const
    {
        auto t_mark_start = (DSRC_TimeMark_t *)calloc(1, sizeof(DSRC_TimeMark_t));
        *t_mark_start = time_change_detail_json["start_time"].asInt();
        timing->startTime = t_mark_start;

        auto t_mark_min_end = (DSRC_TimeMark_t *)calloc(1, sizeof(DSRC_TimeMark_t));
        *t_mark_min_end = time_change_detail_json["min_end_time"].asInt();
        timing->minEndTime = *t_mark_min_end;

        auto t_mark_max_end = (DSRC_TimeMark_t *)calloc(1, sizeof(DSRC_TimeMark_t));
        *t_mark_max_end = time_change_detail_json["max_end_time"].asInt();
        timing->maxEndTime = t_mark_max_end;

        auto t_mark_likely = (DSRC_TimeMark_t *)calloc(1, sizeof(DSRC_TimeMark_t));
        *t_mark_likely = time_change_detail_json["likely_time"].asInt();
        timing->likelyTime = t_mark_likely;

        auto t_mark_next = (DSRC_TimeMark_t *)calloc(1, sizeof(DSRC_TimeMark_t));
        *t_mark_next = time_change_detail_json["next_time"].asInt();
        timing->nextTime = t_mark_next;

        auto t_confidence = (TimeIntervalConfidence_t *)calloc(1, sizeof(TimeIntervalConfidence_t));
        *t_confidence = time_change_detail_json["confidence"].asInt();
        timing->confidence = t_confidence;
    }

    void JsonToJ2735SpatConverter::convertJson2AdvisorySpeed(const Json::Value speeds_json, AdvisorySpeedList_t *speeds) const
    {
        for (auto speed_json : speeds_json)
        {
            auto speed = (AdvisorySpeed_t *)calloc(1, sizeof(AdvisorySpeed_t));
            speed->type = speed_json["type"].asInt();

            auto speed_limit = (SpeedAdvice_t *)calloc(1, sizeof(SpeedAdvice_t));
            *speed_limit = speed_json["speed_limit"].asInt();
            speed->speed = speed_limit;

            auto confidence = (SpeedConfidence_t *)calloc(1, sizeof(SpeedConfidence_t));
            *confidence = speed_json["speed_confidence"].asInt();
            speed->confidence = confidence;

            auto Class = (RestrictionClassID_t *)calloc(1, sizeof(RestrictionClassID_t));
            *Class = speed_json["class"].asInt();
            speed->Class = Class;

            auto distance = (ZoneLength_t *)calloc(1, sizeof(ZoneLength_t));
            *distance = speed_json["distance"].asInt();
            speed->distance = distance;

            ASN_SEQUENCE_ADD(&speeds->list, speed);
        }
    }

    void JsonToJ2735SpatConverter::convertJson2ManeuverAssistList(const Json::Value maneuver_assist_list_json, ManeuverAssistList_t *maneuver_assist_list) const
    {
        for (auto masst : maneuver_assist_list_json)
        {
            auto maneuver_assist = (ConnectionManeuverAssist_t *)calloc(1, sizeof(ConnectionManeuverAssist_t));
            maneuver_assist->connectionID = masst["connection_id"].asInt();

            auto queueLength = (ZoneLength_t *)calloc(1, sizeof(ZoneLength_t));
            *queueLength = masst["queue_length"].asInt();
            maneuver_assist->queueLength = queueLength;

            auto availableStorageLength = (ZoneLength_t *)calloc(1, sizeof(ZoneLength_t));
            *availableStorageLength = masst["available_storage_length"].asInt();
            maneuver_assist->availableStorageLength = availableStorageLength;

            auto waitOnStop = (WaitOnStopline_t *)calloc(1, sizeof(WaitOnStopline_t));
            *waitOnStop = masst["wait_on_stop"].asBool() ? 1 : 0;
            maneuver_assist->waitOnStop = waitOnStop;

            auto pedBicycleDetect = (PedestrianBicycleDetect_t *)calloc(1, sizeof(PedestrianBicycleDetect_t));
            *pedBicycleDetect = masst["ped_bicycle_detect"].asBool() ? 1 : 0;
            maneuver_assist->pedBicycleDetect = pedBicycleDetect;

            ASN_SEQUENCE_ADD(&maneuver_assist_list->list, maneuver_assist);
        }
    }

    void JsonToJ2735SpatConverter::encodeSpat(tmx::messages::SpatMessage &spat_message, tmx::messages::SpatEncodedMessage &encodedSpat) const
    {
        tmx::messages::MessageFrameMessage frame(spat_message.get_j2735_data());
        encodedSpat.set_data(tmx::messages::TmxJ2735EncodedMessage<SPAT>::encode_j2735_message<tmx::messages::codec::uper<tmx::messages::MessageFrameMessage>>(frame));
        free(frame.get_j2735_data().get());
    }
}