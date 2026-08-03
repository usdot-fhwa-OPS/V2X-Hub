#include <gtest/gtest.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include "jsoncpp/json/json.h"
#include "J2735MapToJsonConverter.h"

class test_J2735MapToJsonConverter : public testing::Test
{
public:
    test_J2735MapToJsonConverter() = default;
    ~test_J2735MapToJsonConverter() = default;
};

namespace unit_test
{
    template <typename T>
    T *AllocAsn() {
        return static_cast<T *>(calloc(1, sizeof(T)));
    }
    TEST_F(test_J2735MapToJsonConverter, convertJ2735MAPToMapJSON)
    {
        CARMAStreetsPlugin::J2735MapToJsonConverter converter;
        std::shared_ptr<MapData> mapData = tmx::messages::j2735::j2735_create<tmx::messages::MapDataTraits>();
        LayerID_t layer_id = 1;
        #if SAEJ2735_SPEC < 2020
        DSRC_MsgCount_t msgIssueRevision = 2;
        #else
        Common_MsgCount_t msgIssueRevision = 2;
        #endif
        mapData->layerID = &layer_id;
        mapData->msgIssueRevision = msgIssueRevision;
        
        mapData->layerType = AllocAsn<LayerType_t>();
        *mapData->layerType = LayerType_intersectionData;
        Json::Value mapJson;
        converter.convertJ2735MAPToMapJSON(mapData, mapJson);
        ASSERT_EQ("1", mapJson["map_data"]["layer_id"].asString());
        ASSERT_EQ("2", mapJson["map_data"]["msg_issue_revision"].asString());
        ASSERT_EQ("1", mapJson["map_data"]["layer_id"].asString());
        ASSERT_EQ(true, mapJson["map_data"]["intersections"].empty());

        GenericLane *lane_ptr = AllocAsn<GenericLane>();
        lane_ptr->laneID = 3;
        GenericLane *lane_list_ptr = AllocAsn<GenericLane>();
        IntersectionGeometry *intersection = AllocAsn<IntersectionGeometry>();
        intersection->id.id = 1002;
        intersection->refPoint.lat = 111;
        intersection->refPoint.Long = 222;
        LaneWidth_t lanewith = 12;
        intersection->laneWidth = &lanewith;
        asn_sequence_add(&intersection->laneSet, lane_ptr);
        IntersectionGeometryList *intersection_list = AllocAsn<IntersectionGeometryList>();
        asn_sequence_add(&intersection_list->list, intersection);
        mapData->intersections = intersection_list;
        converter.convertJ2735MAPToMapJSON(mapData, mapJson);
        ASSERT_EQ(false, mapJson["map_data"]["intersections"]["intersection_geometry"]["lane_set"].empty());
        ASSERT_EQ("3", mapJson["map_data"]["intersections"]["intersection_geometry"]["lane_set"][0]["lane_id"].asString());
        ASSERT_EQ("111", mapJson["map_data"]["intersections"]["intersection_geometry"]["ref_point"]["lat"].asString());
        ASSERT_EQ("222", mapJson["map_data"]["intersections"]["intersection_geometry"]["ref_point"]["long"].asString());
        ASSERT_EQ("12", mapJson["map_data"]["intersections"]["intersection_geometry"]["lane_width"].asString());
    }
}