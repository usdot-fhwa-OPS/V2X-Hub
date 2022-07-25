#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>
#include "jsoncpp/json/json.h"
#include "JsonToJ2735SpatConverter.h"

namespace CARMAStreetsPlugin
{
    class test_JsonToJ2735SpatConverter : public ::testing::Test
    {
    public:
        Json::Value spat_json;
        Json::Value movement_list_json;
        Json::Value manuever_assist_list_json;
        Json::Value movement_events_json;

    protected:
        void SetUp() override
        {
            std::string spat_json_str = "{\"timestamp\":525599,\"name\":\"Signalized intersection\",\"intersections\":[{\"name\":\"WestIntersection\",\"id\":1909,\"revision\":123,\"status\":15,\"moy\":34232,\"time_stamp\":130,\"enabled_lanes\":[1,3,5],\"states\":[{\"movement_name\":\"RightTurn\",\"signal_group\":4,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":1,\"min_end_time\":10,\"max_end_time\":100,\"likely_time\":1000,\"confidence\":2},\"speeds\":[{\"type\":1,\"speed_limit\":4,\"speed_confidence\":3,\"distance\":6,\"class\":5}]}],\"maneuver_assist_list\":[{\"connection_id\":7,\"queue_length\":4,\"available_storage_length\":8,\"wait_on_stop\":true,\"ped_bicycle_detect\":false}]}],\"maneuver_assist_list\":[{\"connection_id\":7,\"queue_length\":4,\"available_storage_length\":8,\"wait_on_stop\":true,\"ped_bicycle_detect\":false}]}]}";
            Json::Reader reader;
            auto parse_success = reader.parse(spat_json_str, spat_json, true);
            if (!parse_success)
            {
                std::cout << "Failed to parse spat json" << std::endl;
            }

            std::string movement_list_str = "{\"states\":[{\"movement_name\":\"RightTurn\",\"signal_group\":4,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":0,\"min_end_time\":0,\"max_end_time\":0,\"likely_time\":0,\"confidence\":0},\"speeds\":[{\"type\":0,\"speed_limit\":4,\"speed_confidence\":1,\"distance\":5,\"class\":5}]}],\"maneuver_assist_list\":[{\"connection_id\":7,\"queue_length\":4,\"available_storage_length\":8,\"wait_on_stop\":true,\"ped_bicycle_detect\":false}]}]}";
            parse_success = reader.parse(movement_list_str, movement_list_json, true);
            if (!parse_success)
            {
                std::cout << "Failed to parse movement list json" << std::endl;
            }

            std::string movement_events_str = "{\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":0,\"min_end_time\":0,\"max_end_time\":0,\"likely_time\":0,\"confidence\":0},\"speeds\":[{\"type\":0,\"speed_limit\":4,\"speed_confidence\":1,\"distance\":5,\"class\":5}]}]}";
            parse_success = reader.parse(movement_events_str, movement_events_json, true);
            if (!parse_success)
            {
                std::cout << "Failed to parse movement events json" << std::endl;
            }

            std::string manuever_assist_list_str = "{\"maneuver_assist_list\":[{\"connection_id\":7,\"queue_length\":4,\"available_storage_length\":8,\"wait_on_stop\":true,\"ped_bicycle_detect\":false}]}";
            parse_success = reader.parse(manuever_assist_list_str, manuever_assist_list_json, true);
            if (!parse_success)
            {
                std::cout << "Failed to parse manuever assist list json" << std::endl;
            }
        }
    };

    TEST_F(test_JsonToJ2735SpatConverter, convertJson2Spat)
    {
        auto spat_ptr = std::make_shared<SPAT>();
        JsonToJ2735SpatConverter converter;
        ASSERT_EQ(spat_ptr->intersections.list.count, 0);
        converter.convertJson2Spat(spat_json, spat_ptr.get());
        ASSERT_EQ(spat_ptr->intersections.list.count, 1);
        ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SPAT, spat_ptr.get());
    }

    TEST_F(test_JsonToJ2735SpatConverter, convertJson2IntersectionStateList)
    {
        if (spat_json["intersections"].isArray())
        {
            auto intersections = (IntersectionStateList *)calloc(1, sizeof(IntersectionStateList));
            JsonToJ2735SpatConverter converter;
            ASSERT_EQ(intersections->list.count, 0);
            converter.convertJson2IntersectionStateList(spat_json["intersections"], intersections);
            ASSERT_EQ(intersections->list.count, 1);
            free(intersections);
        }
    }

    TEST_F(test_JsonToJ2735SpatConverter, convertJson2MovementList)
    {
        if (movement_list_json["states"].isArray())
        {
            auto states = (MovementList_t *)calloc(1, sizeof(MovementList_t));
            JsonToJ2735SpatConverter converter;
            ASSERT_EQ(states->list.count, 0);
            converter.convertJson2MovementList(movement_list_json["states"], states);
            ASSERT_EQ(states->list.count, 1);
            free(states);
        }
    }

    TEST_F(test_JsonToJ2735SpatConverter, convertJson2MovementEventList)
    {
        if (movement_events_json["state_time_speed"].isArray())
        {
            auto state_time_speed = (MovementEventList_t *)calloc(1, sizeof(MovementEventList_t));
            JsonToJ2735SpatConverter converter;
            ASSERT_EQ(state_time_speed->list.count, 0);
            converter.convertJson2MovementEventList(movement_events_json["state_time_speed"], state_time_speed);
            ASSERT_EQ(state_time_speed->list.count, 1);
            free(state_time_speed->list.array[0]->speeds);
            free(state_time_speed->list.array[0]->timing);
            free(state_time_speed);
        }
    }

    TEST_F(test_JsonToJ2735SpatConverter, convertJson2ManeuverAssistList)
    {
        if (manuever_assist_list_json["maneuver_assist_list"].isArray())
        {
            auto maneuver_assist_list = (ManeuverAssistList_t *)calloc(1, sizeof(ManeuverAssistList_t));
            JsonToJ2735SpatConverter converter;
            ASSERT_EQ(maneuver_assist_list->list.count, 0);
            converter.convertJson2ManeuverAssistList(manuever_assist_list_json["maneuver_assist_list"], maneuver_assist_list);
            ASSERT_EQ(maneuver_assist_list->list.count, 1);
            free(maneuver_assist_list);
        }
    }

    TEST_F(test_JsonToJ2735SpatConverter, encodeSpat)
    {
        auto spat_ptr = std::make_shared<SPAT>();
        JsonToJ2735SpatConverter converter;
        converter.convertJson2Spat(spat_json, spat_ptr.get());

        tmx::messages::SpatEncodedMessage encodedSpat;
        converter.encodeSpat(spat_ptr, encodedSpat);
        std::string encoded_spat_str = "00135f68051f5a9e9cfbb0ecd3eb2e441a7774cbcb9e5c7d34efdc07c7d7cbcfa49ddd32f2e7971f4d3bf701dd7d8007842dc00411008182803114b4e7d1d2a75e5b81018fe0002001400c807d0400000f10230018141e07001000881e0700100088";
        ASSERT_EQ(encoded_spat_str, encodedSpat.get_payload_str());
        ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SPAT, spat_ptr.get());
    }
}