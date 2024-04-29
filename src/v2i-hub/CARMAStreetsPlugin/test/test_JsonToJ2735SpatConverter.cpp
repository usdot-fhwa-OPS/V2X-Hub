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
            std::string spat_json_str="{\"time_stamp\":316554,\"name\":\"\",\"intersections\":[{\"name\":\"WestIntersection\",\"id\":9001,\"revision\":120,\"status\":0,\"moy\":316554,\"time_stamp\":8609,\"states\":[{\"movement_name\":\"\",\"signal_group\":8,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":32486,\"min_end_time\":32558,\"max_end_time\":32558,\"confidence\":0}}]},{\"movement_name\":\"\",\"signal_group\":7,\"state_time_speed\":[{\"event_state\":6,\"timing\":{\"start_time\":32486,\"min_end_time\":32518,\"max_end_time\":32518,\"confidence\":0}}]},{\"movement_name\":\"\",\"signal_group\":2,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":32486,\"min_end_time\":32838,\"max_end_time\":32838,\"confidence\":0}}]},{\"movement_name\":\"\",\"signal_group\":1,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":32486,\"min_end_time\":32698,\"max_end_time\":32698,\"confidence\":0}}]},{\"movement_name\":\"\",\"signal_group\":3,\"state_time_speed\":[{\"event_state\":6,\"timing\":{\"start_time\":32486,\"min_end_time\":32518,\"max_end_time\":32518,\"confidence\":0}}]},{\"movement_name\":\"\",\"signal_group\":4,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":32486,\"min_end_time\":32558,\"max_end_time\":32558,\"confidence\":0}}]},{\"movement_name\":\"\",\"signal_group\":5,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":32486,\"min_end_time\":32698,\"max_end_time\":32698,\"confidence\":0}}]},{\"movement_name\":\"\",\"signal_group\":6,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":32486,\"min_end_time\":32838,\"max_end_time\":32838,\"confidence\":0}}]}]}]}";
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
        std::string encoded_spat_str = "0013808f44d48a0383ebe5e7d24eee997973cb8fa69dfb84653e000013522886841c02010fefdccfe5cfe5c00000000000e08df7ee67f067f06000000000002043fbf7340234023000000000000821fdfb99fee9fee800000000000c11befdccfe0cfe0c000000000008087f7ee67f2e7f2e000000000005043fbf733fdd3fdd000000000003021fdfb9a011a0118000000000";
        ASSERT_EQ(encoded_spat_str, encodedSpat.get_payload_str());
        ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SPAT, spat_ptr.get());
    }

    /**
     * @brief Unit test to ensure encodeSpat will throw and exception attempting to encode timing
     * data that exceeds max value of 36111 (see 2020 ASN1 documentation for TimeMark).
     * 
     */
    TEST_F(test_JsonToJ2735SpatConverter, time_mark_over_max_value) {
        std::string spat_json_string = "{\"time_stamp\":387178,\"name\":\"\",\"intersections\":[{\"name\":\"East Intersection\",\"id\":9945,\"revision\":1,\"status\":0,\"moy\":387178,\"time_stamp\":32248,\"states\":["
            "{\"movement_name\":\"\",\"signal_group\":8,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":34975,\"min_end_time\":35405,\"max_end_time\":35405,\"confidence\":0}},{\"event_state\":6,\"timing\":{\"start_time\":35405,\"min_end_time\":35505,\"max_end_time\":35505,\"confidence\":0}},{\"event_state\":8,\"timing\":{\"start_time\":35505,\"min_end_time\":35535,\"max_end_time\":35535,\"confidence\":0}},{\"event_state\":3,\"timing\":{\"start_time\":35535,\"min_end_time\":35965,\"max_end_time\":35965,\"confidence\":0}}]},"
            "{\"movement_name\":\"\",\"signal_group\":2,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":34695,\"min_end_time\":35125,\"max_end_time\":35125,\"confidence\":0}},{\"event_state\":6,\"timing\":{\"start_time\":35125,\"min_end_time\":35225,\"max_end_time\":35225,\"confidence\":0}},{\"event_state\":8,\"timing\":{\"start_time\":35225,\"min_end_time\":35255,\"max_end_time\":35255,\"confidence\":0}},{\"event_state\":3,\"timing\":{\"start_time\":35255,\"min_end_time\":35685,\"max_end_time\":35685,\"confidence\":0}}]},"
            "{\"movement_name\":\"\",\"signal_group\":1,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":35115,\"min_end_time\":35545,\"max_end_time\":35545,\"confidence\":0}},{\"event_state\":6,\"timing\":{\"start_time\":35545,\"min_end_time\":35645,\"max_end_time\":35645,\"confidence\":0}},{\"event_state\":8,\"timing\":{\"start_time\":35645,\"min_end_time\":35675,\"max_end_time\":35675,\"confidence\":0}},{\"event_state\":3,\"timing\":{\"start_time\":35675,\"min_end_time\":36205,\"max_end_time\":36205,\"confidence\":0}}]},"
            "{\"movement_name\":\"\",\"signal_group\":3,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":34835,\"min_end_time\":35265,\"max_end_time\":35265,\"confidence\":0}},{\"event_state\":6,\"timing\":{\"start_time\":35265,\"min_end_time\":35365,\"max_end_time\":35365,\"confidence\":0}},{\"event_state\":8,\"timing\":{\"start_time\":35365,\"min_end_time\":35395,\"max_end_time\":35395,\"confidence\":0}},{\"event_state\":3,\"timing\":{\"start_time\":35395,\"min_end_time\":35825,\"max_end_time\":35825,\"confidence\":0}}]},"
            "{\"movement_name\":\"\",\"signal_group\":4,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":34975,\"min_end_time\":35405,\"max_end_time\":35405,\"confidence\":0}},{\"event_state\":6,\"timing\":{\"start_time\":35405,\"min_end_time\":35505,\"max_end_time\":35505,\"confidence\":0}},{\"event_state\":8,\"timing\":{\"start_time\":35505,\"min_end_time\":35535,\"max_end_time\":35535,\"confidence\":0}},{\"event_state\":3,\"timing\":{\"start_time\":35535,\"min_end_time\":35965,\"max_end_time\":35965,\"confidence\":0}}]},"
            "{\"movement_name\":\"\",\"signal_group\":5,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":35115,\"min_end_time\":35545,\"max_end_time\":35545,\"confidence\":0}},{\"event_state\":6,\"timing\":{\"start_time\":35545,\"min_end_time\":35645,\"max_end_time\":35645,\"confidence\":0}},{\"event_state\":8,\"timing\":{\"start_time\":35645,\"min_end_time\":35675,\"max_end_time\":35675,\"confidence\":0}},{\"event_state\":3,\"timing\":{\"start_time\":35675,\"min_end_time\":36205,\"max_end_time\":36205,\"confidence\":0}}]},"
            "{\"movement_name\":\"\",\"signal_group\":6,\"state_time_speed\":[{\"event_state\":3,\"timing\":{\"start_time\":34695,\"min_end_time\":35125,\"max_end_time\":35125,\"confidence\":0}},{\"event_state\":6,\"timing\":{\"start_time\":35125,\"min_end_time\":35225,\"max_end_time\":35225,\"confidence\":0}},{\"event_state\":8,\"timing\":{\"start_time\":35225,\"min_end_time\":35255,\"max_end_time\":35255,\"confidence\":0}},{\"event_state\":3,\"timing\":{\"start_time\":35255,\"min_end_time\":35685,\"max_end_time\":35685,\"confidence\":0}}]}]}]}";
        Json::Reader reader;
        auto spat_ptr = std::make_shared<SPAT>();
        auto parse_success = reader.parse(spat_json_string, spat_json, true);
        JsonToJ2735SpatConverter converter;
        converter.convertJson2Spat(spat_json, spat_ptr.get());
        tmx::messages::SpatEncodedMessage encodedSpat;
       
        ASSERT_THROW( converter.encodeSpat(spat_ptr, encodedSpat), tmx::TmxException );
        
        

    }

}