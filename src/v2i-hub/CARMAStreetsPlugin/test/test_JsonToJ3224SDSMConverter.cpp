#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "jsoncpp/json/json.h"
#include "JsonToJ3224SDSMConverter.h"

namespace CARMAStreetsPlugin
{
    class test_JsonToJ3224SDSMConverter : public ::testing::Test
    {
    };

    // Test for SDSM string json parsing
    TEST_F(test_JsonToJ3224SDSMConverter, parseJsonString)
    {
        // TODO: consider adding functionality for converting to input strings from existing json files
        JsonToJ3224SDSMConverter converter;
        std::string valid_sdsm_json = "{\"equipment_type\":1,\"msg_cnt\":1,\"ref_pos\":{\"Long\":600000000,\"elevation\":30,\"lat\":400000000},\"ref_pos_el_conf\":10,\"ref_pos_xy_conf\":{\"orientation\":25000,\"semi_major\":235,\"semi_minor\":200},\"sdsm_time_stamp\":{\"day\":4,\"hour\":19,\"minute\":15,\"month\":7,\"offset\":400,\"second\":5000,\"year\":2007},\"source_id\":\"01020304\",\"objects\":[{\"detected_object_data\":{\"detected_object_common_data\":{\"acc_cfd_x\":4,\"acc_cfd_y\":5,\"acc_cfd_yaw\":3,\"acc_cfd_z\":6,\"accel_4_way\":{\"lat\":-500,\"Long\":200,\"vert\":1,\"yaw\":400},\"heading\":16000,\"heading_conf\":4,\"measurement_time\":-1100,\"object_id\":12200,\"obj_type\":1,\"obj_type_cfd\":65,\"pos\":{\"offset_x\":4000,\"offset_y\":-720,\"offset_z\":20},\"pos_confidence\":{\"elevation\":5,\"pos\":2},\"speed\":2100,\"speed_confidence\":3,\"speed_confidence_z\":4,\"speed_z\":1000,\"time_confidence\":2}}}]}";
        Json::Value root;
        bool result = converter.parseJsonString(valid_sdsm_json, root);
        ASSERT_TRUE(result);
        std::string invalid_json = "invalid";
        result = converter.parseJsonString(invalid_json, root);
        ASSERT_FALSE(result);

    }

    // Test for SDSM header data
    TEST_F(test_JsonToJ3224SDSMConverter, convertJsonToSDSM_header)
    {
        JsonToJ3224SDSMConverter converter;
        std::string valid_json_str = "{\"equipment_type\":1,\"msg_cnt\":1,\"ref_pos\":{\"Long\":600000000,\"elevation\":30,\"lat\":400000000},\"ref_pos_el_conf\":10,\"ref_pos_xy_conf\":{\"orientation\":25000,\"semi_major\":235,\"semi_minor\":200},\"sdsm_time_stamp\":{\"day\":4,\"hour\":19,\"minute\":15,\"month\":7,\"offset\":400,\"second\":5000,\"year\":2007},\"source_id\":\"01020304\",\"objects\":[{\"detected_object_data\":{\"detected_object_common_data\":{\"acc_cfd_x\":4,\"acc_cfd_y\":5,\"acc_cfd_yaw\":3,\"acc_cfd_z\":6,\"accel_4_way\":{\"lat\":-500,\"Long\":200,\"vert\":1,\"yaw\":400},\"heading\":16000,\"heading_conf\":4,\"measurement_time\":-1100,\"object_id\":12200,\"obj_type\":1,\"obj_type_cfd\":65,\"pos\":{\"offset_x\":4000,\"offset_y\":-720,\"offset_z\":20},\"pos_confidence\":{\"elevation\":5,\"pos\":2},\"speed\":2100,\"speed_confidence\":3,\"speed_confidence_z\":4,\"speed_z\":1000,\"time_confidence\":2}}}]}";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        ASSERT_TRUE(result);
        auto sdsmPtr = std::make_shared<SensorDataSharingMessage>();
        converter.convertJsonToSDSM(root, sdsmPtr);
        ASSERT_EQ(1, sdsmPtr->msgCnt);
        
        // ASSERT_EQ("01020304", sdsmPtr->sourceID);

        ASSERT_EQ(1, sdsmPtr->equipmentType);
        ASSERT_EQ(2007, *sdsmPtr->sDSMTimeStamp.year);
        ASSERT_EQ(7, *sdsmPtr->sDSMTimeStamp.month);
        ASSERT_EQ(4, *sdsmPtr->sDSMTimeStamp.day);
        ASSERT_EQ(19, *sdsmPtr->sDSMTimeStamp.hour);
        ASSERT_EQ(15, *sdsmPtr->sDSMTimeStamp.minute);
        ASSERT_EQ(5000, *sdsmPtr->sDSMTimeStamp.second);
        ASSERT_EQ(400, *sdsmPtr->sDSMTimeStamp.offset);

        ASSERT_EQ(400000000, sdsmPtr->refPos.lat);
        ASSERT_EQ(600000000, sdsmPtr->refPos.Long);
        ASSERT_EQ(30, *sdsmPtr->refPos.elevation);

        ASSERT_EQ(235, sdsmPtr->refPosXYConf.semiMajor);
        ASSERT_EQ(200, sdsmPtr->refPosXYConf.semiMinor);
        ASSERT_EQ(25000, sdsmPtr->refPosXYConf.orientation);

        ASSERT_EQ(10, *sdsmPtr->refPosElConf);

    }

    // // Test for SDSM common data
    TEST_F(test_JsonToJ3224SDSMConverter, convertJsonToSDSM_common)
    {
        JsonToJ3224SDSMConverter converter;
        std::string valid_json_str = "{\"equipment_type\":1,\"msg_cnt\":1,\"ref_pos\":{\"Long\":600000000,\"elevation\":30,\"lat\":400000000},\"ref_pos_el_conf\":10,\"ref_pos_xy_conf\":{\"orientation\":25000,\"semi_major\":235,\"semi_minor\":200},\"sdsm_time_stamp\":{\"day\":4,\"hour\":19,\"minute\":15,\"month\":7,\"offset\":400,\"second\":5000,\"year\":2007},\"source_id\":\"01020304\",\"objects\":[{\"detected_object_data\":{\"detected_object_common_data\":{\"acc_cfd_x\":4,\"acc_cfd_y\":5,\"acc_cfd_yaw\":3,\"acc_cfd_z\":6,\"accel_4_way\":{\"lat\":-500,\"Long\":200,\"vert\":1,\"yaw\":400},\"heading\":16000,\"heading_conf\":4,\"measurement_time\":-1100,\"object_id\":12200,\"obj_type\":1,\"obj_type_cfd\":65,\"pos\":{\"offset_x\":4000,\"offset_y\":-720,\"offset_z\":20},\"pos_confidence\":{\"elevation\":5,\"pos\":2},\"speed\":2100,\"speed_confidence\":3,\"speed_confidence_z\":4,\"speed_z\":1000,\"time_confidence\":2}}}]}";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        ASSERT_TRUE(result);
        auto sdsmPtr = std::make_shared<SensorDataSharingMessage>();
        converter.convertJsonToSDSM(root, sdsmPtr);

        ASSERT_EQ(1, sdsmPtr->objects.list.array[0]->detObjCommon.objType);
        ASSERT_EQ(65, sdsmPtr->objects.list.array[0]->detObjCommon.objTypeCfd);
        ASSERT_EQ(12200, sdsmPtr->objects.list.array[0]->detObjCommon.objectID);
        ASSERT_EQ(-1100, sdsmPtr->objects.list.array[0]->detObjCommon.measurementTime);
        ASSERT_EQ(2, sdsmPtr->objects.list.array[0]->detObjCommon.timeConfidence);

        ASSERT_EQ(4000, sdsmPtr->objects.list.array[0]->detObjCommon.pos.offsetX);
        ASSERT_EQ(-720, sdsmPtr->objects.list.array[0]->detObjCommon.pos.offsetY);
        ASSERT_EQ(20, *sdsmPtr->objects.list.array[0]->detObjCommon.pos.offsetZ);

        ASSERT_EQ(2, sdsmPtr->objects.list.array[0]->detObjCommon.posConfidence.pos);
        ASSERT_EQ(5, sdsmPtr->objects.list.array[0]->detObjCommon.posConfidence.elevation);

        ASSERT_EQ(2100, sdsmPtr->objects.list.array[0]->detObjCommon.speed);
        ASSERT_EQ(3, sdsmPtr->objects.list.array[0]->detObjCommon.speedConfidence);
        ASSERT_EQ(1000, *sdsmPtr->objects.list.array[0]->detObjCommon.speedZ);
        ASSERT_EQ(4, *sdsmPtr->objects.list.array[0]->detObjCommon.speedConfidenceZ);

        ASSERT_EQ(16000, sdsmPtr->objects.list.array[0]->detObjCommon.heading);
        ASSERT_EQ(4, sdsmPtr->objects.list.array[0]->detObjCommon.headingConf);

        ASSERT_EQ(200, sdsmPtr->objects.list.array[0]->detObjCommon.accel4way->Long);
        ASSERT_EQ(-500, sdsmPtr->objects.list.array[0]->detObjCommon.accel4way->lat);
        ASSERT_EQ(1, sdsmPtr->objects.list.array[0]->detObjCommon.accel4way->vert);
        ASSERT_EQ(400, sdsmPtr->objects.list.array[0]->detObjCommon.accel4way->yaw);

    }

    // Test for SDSM optional data - vehicle data
    TEST_F(test_JsonToJ3224SDSMConverter, convertJsonToSDSM_veh)
    {
        JsonToJ3224SDSMConverter converter;
        std::string valid_json_str = "{\"equipment_type\":1,\"msg_cnt\":1,\"objects\":[{\"detected_object_data\":{\"detected_object_common_data\":{\"acc_cfd_x\":4,\"acc_cfd_y\":5,\"acc_cfd_yaw\":3,\"acc_cfd_z\":6,\"accel_4_way\":{\"lat\":-500,\"long\":200,\"vert\":1,\"yaw\":400},\"heading\":16000,\"heading_conf\":4,\"measurement_time\":-1100,\"object_id\":12200,\"obj_type\":1,\"obj_type_cfd\":65,\"pos\":{\"offset_x\":4000,\"offset_y\":-720,\"offset_z\":20},\"pos_confidence\":{\"elevation\":5,\"pos\":2},\"speed\":2100,\"speed_confidence\":3,\"speed_confidence_z\":4,\"speed_z\":1000,\"time_confidence\":2},\"detected_object_optional_data\":{\"detected_vehicle_data\":{\"height\":70,\"lights\":\"8\",\"size\":{\"length\":700,\"width\":300},\"veh_ang_vel\":{\"pitch_rate\":600,\"roll_rate\":-800},\"veh_ang_vel_confidence\":{\"pitch_rate_confidence\":3,\"roll_rate_confidence\":4},\"veh_attitude\":{\"pitch\":2400,\"roll\":-12000,\"yaw\":400},\"veh_attitude_confidence\":{\"pitch_confidence\":2,\"roll_confidence\":3,\"yaw_confidence\":4},\"vehicle_class\":11,\"vehicle_class_conf\":75,\"vehicle_size_confidence\":{\"vehicle_height_confidence\":5,\"vehicle_length_confidence\":6,\"vehicle_width_confidence\":7}}}}}],\"ref_pos\":{\"Long\":600000000,\"elevation\":30,\"lat\":400000000},\"ref_pos_el_conf\":10,\"ref_pos_xy_conf\":{\"orientation\":25000,\"semi_major\":235,\"semi_minor\":200},\"sdsm_time_stamp\":{\"day\":4,\"hour\":19,\"minute\":15,\"month\":7,\"offset\":400,\"second\":5000,\"year\":2007},\"source_id\":\"01020304\"}";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        ASSERT_TRUE(result);
        auto sdsmPtr = std::make_shared<SensorDataSharingMessage>();
        converter.convertJsonToSDSM(root, sdsmPtr);

        // ASSERT_EQ("0106", sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.lights);
        ASSERT_EQ(2400, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAttitude->pitch);
        ASSERT_EQ(-12000, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAttitude->roll);
        ASSERT_EQ(400, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAttitude->yaw);

        ASSERT_EQ(2, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAttitudeConfidence->pitchConfidence);
        ASSERT_EQ(3, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAttitudeConfidence->rollConfidence);
        ASSERT_EQ(4, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAttitudeConfidence->yawConfidence);

        ASSERT_EQ(600, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAngVel->pitchRate);
        ASSERT_EQ(-800, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAngVel->rollRate);

        ASSERT_EQ(3, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAngVelConfidence->pitchRateConfidence);
        ASSERT_EQ(4, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAngVelConfidence->rollRateConfidence);

        ASSERT_EQ(300, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.size->width);
        ASSERT_EQ(700, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.size->length);
        ASSERT_EQ(70, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.height);

        ASSERT_EQ(7, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehicleSizeConfidence->vehicleWidthConfidence);
        ASSERT_EQ(6, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehicleSizeConfidence->vehicleLengthConfidence);
        ASSERT_EQ(5, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehicleSizeConfidence->vehicleHeightConfidence);

        ASSERT_EQ(11, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehicleClass);
        ASSERT_EQ(75, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.classConf);

    }

    // Test for SDSM optional data - VRU data
    TEST_F(test_JsonToJ3224SDSMConverter, convertJsonToSDSM_vru)
    {
        JsonToJ3224SDSMConverter converter;
        std::string valid_json_str = "{\"equipment_type\":1,\"msg_cnt\":1,\"objects\":[{\"detected_object_data\":{\"detected_object_common_data\":{\"acc_cfd_x\":4,\"acc_cfd_y\":5,\"acc_cfd_yaw\":3,\"acc_cfd_z\":6,\"accel_4_way\":{\"lat\":-500,\"long\":200,\"vert\":1,\"yaw\":400},\"heading\":16000,\"heading_conf\":4,\"measurement_time\":-1100,\"object_id\":12200,\"obj_type\":1,\"obj_type_cfd\":65,\"pos\":{\"offset_x\":4000,\"offset_y\":-720,\"offset_z\":20},\"pos_confidence\":{\"elevation\":5,\"pos\":2},\"speed\":2100,\"speed_confidence\":3,\"speed_confidence_z\":4,\"speed_z\":1000,\"time_confidence\":2},\"detected_object_optional_data\":{\"detected_vru_data\":{\"attachment\":3,\"basic_type\":1,\"propulsion\":{\"human\":2},\"radius\":30}}}}],\"ref_pos\":{\"Long\":600000000,\"elevation\":30,\"lat\":400000000},\"ref_pos_el_conf\":10,\"ref_pos_xy_conf\":{\"orientation\":25000,\"semi_major\":235,\"semi_minor\":200},\"sdsm_time_stamp\":{\"day\":4,\"hour\":19,\"minute\":15,\"month\":7,\"offset\":400,\"second\":5000,\"year\":2007},\"source_id\":\"01020304\"}";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        ASSERT_TRUE(result);
        auto sdsmPtr = std::make_shared<SensorDataSharingMessage>();
        converter.convertJsonToSDSM(root, sdsmPtr);

        ASSERT_EQ(1, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVRU.basicType);
        ASSERT_EQ(2, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVRU.propulsion->choice.human);
        ASSERT_EQ(3, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVRU.attachment);
        ASSERT_EQ(30, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVRU.radius);

    }

    // Test for SDSM optional data - obstacle data
    TEST_F(test_JsonToJ3224SDSMConverter, convertJsonToSDSM_obst)
    {
        JsonToJ3224SDSMConverter converter;
        std::string valid_json_str = "{\"equipment_type\":1,\"msg_cnt\":1,\"objects\":[{\"detected_object_data\":{\"detected_object_common_data\":{\"acc_cfd_x\":4,\"acc_cfd_y\":5,\"acc_cfd_yaw\":3,\"acc_cfd_z\":6,\"accel_4_way\":{\"lat\":-500,\"long\":200,\"vert\":1,\"yaw\":400},\"heading\":16000,\"heading_conf\":4,\"measurement_time\":-1100,\"object_id\":12200,\"obj_type\":1,\"obj_type_cfd\":65,\"pos\":{\"offset_x\":4000,\"offset_y\":-720,\"offset_z\":20},\"pos_confidence\":{\"elevation\":5,\"pos\":2},\"speed\":2100,\"speed_confidence\":3,\"speed_confidence_z\":4,\"speed_z\":1000,\"time_confidence\":2},\"detected_object_optional_data\":{\"detected_obstacle_data\":{\"obst_size\":{\"height\":100,\"length\":300,\"width\":400},\"obst_size_confidence\":{\"height_confidence\":8,\"length_confidence\":7,\"width_confidence\":6}}}}}],\"ref_pos\":{\"Long\":600000000,\"elevation\":30,\"lat\":400000000},\"ref_pos_el_conf\":10,\"ref_pos_xy_conf\":{\"orientation\":25000,\"semi_major\":235,\"semi_minor\":200},\"sdsm_time_stamp\":{\"day\":4,\"hour\":19,\"minute\":15,\"month\":7,\"offset\":400,\"second\":5000,\"year\":2007},\"source_id\":\"01020304\"}";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        ASSERT_TRUE(result);
        auto sdsmPtr = std::make_shared<SensorDataSharingMessage>();
        converter.convertJsonToSDSM(root, sdsmPtr);

        ASSERT_EQ(400, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detObst.obstSize.width);
        ASSERT_EQ(300, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detObst.obstSize.length);
        ASSERT_EQ(100, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detObst.obstSize.height);

        ASSERT_EQ(6, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detObst.obstSizeConfidence.widthConfidence);
        ASSERT_EQ(7, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detObst.obstSizeConfidence.lengthConfidence);
        ASSERT_EQ(8, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detObst.obstSizeConfidence.heightConfidence);

    }

}