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
        std::string valid_sdsm_json = "{\"equipment_type\":1,\"msg_cnt\":1,\"ref_pos\":{\"long\":600000000,\"elevation\":30,\"lat\":400000000},\"ref_pos_el_conf\":10,\"ref_pos_xy_conf\":{\"orientation\":25000,\"semi_major\":235,\"semi_minor\":200},\"sdsm_time_stamp\":{\"day\":4,\"hour\":19,\"minute\":15,\"month\":7,\"offset\":400,\"second\":5000,\"year\":2007},\"source_id\":\"01020304\",\"objects\":[{\"detected_object_data\":{\"detected_object_common_data\":{\"acc_cfd_x\":4,\"acc_cfd_y\":5,\"acc_cfd_yaw\":3,\"acc_cfd_z\":6,\"accel_4_way\":{\"lat\":-500,\"long\":200,\"vert\":1,\"yaw\":400},\"heading\":16000,\"heading_conf\":4,\"measurement_time\":-1100,\"object_id\":12200,\"obj_type\":1,\"obj_type_cfd\":65,\"pos\":{\"offset_x\":4000,\"offset_y\":-720,\"offset_z\":20},\"pos_confidence\":{\"elevation\":5,\"pos\":2},\"speed\":2100,\"speed_confidence\":3,\"speed_confidence_z\":4,\"speed_z\":1000,\"time_confidence\":2}}}]}";
        Json::Value root;
        bool result = converter.parseJsonString(valid_sdsm_json, root);
        EXPECT_TRUE(result);
        std::string invalid_json = "invalid";
        result = converter.parseJsonString(invalid_json, root);
        ASSERT_FALSE(result);

    }

 // // Test for SDSM common data
    TEST_F(test_JsonToJ3224SDSMConverter, convertToSdsm)
    {
        JsonToJ3224SDSMConverter converter;
        std::string valid_json_str = R"(
            {
                "equipment_type":1,
                "msg_cnt":1,
                "ref_pos":{
                    "long":600000000,
                    "elevation":30,
                    "lat":400000000
                },
                "ref_pos_el_conf":10,
                "ref_pos_xy_conf":{
                    "orientation":25000,
                    "semi_major":235,
                    "semi_minor":200
                },
                "sdsm_time_stamp":{
                    "day":4,
                    "hour":19,
                    "minute":15,
                    "month":7,
                    "offset":400,
                    "second":5000,
                    "year":2007
                },
                "source_id":"rsu_1234",
                "objects":[
                    {
                        "detected_object_data":{
                            "detected_object_common_data":{
                                "acc_cfd_x":4,
                                "acc_cfd_y":5,
                                "acc_cfd_yaw":3,
                                "acc_cfd_z":6,
                                "accel_4_way":{
                                    "lat":-500,
                                    "long":200,
                                    "vert":1,
                                    "yaw":400
                                },
                                "heading":16000,
                                "heading_conf":4,
                                "measurement_time":-1100,
                                "object_id":12200,
                                "obj_type":1,
                                "obj_type_cfd":65,
                                "pos":{
                                    "offset_x":4000,
                                    "offset_y":-720,
                                    "offset_z":20
                                },
                                "pos_confidence":{
                                    "elevation":5,
                                    "pos":2
                                },
                                "speed":2100,
                                "speed_confidence":3,
                                "speed_confidence_z":4,
                                "speed_z":1000,
                                "time_confidence":2
                            }
                        }
                    }
                ]
            })";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        EXPECT_TRUE(result);
        auto sdsmPtr = std::make_shared<SensorDataSharingMessage_t>();
        converter.convertJsonToSDSM(root, sdsmPtr);

        EXPECT_EQ(1, sdsmPtr->objects.list.array[0]->detObjCommon.objType);
        EXPECT_EQ(65, sdsmPtr->objects.list.array[0]->detObjCommon.objTypeCfd);
        EXPECT_EQ(12200, sdsmPtr->objects.list.array[0]->detObjCommon.objectID);
        EXPECT_EQ(-1100, sdsmPtr->objects.list.array[0]->detObjCommon.measurementTime);
        EXPECT_EQ(2, sdsmPtr->objects.list.array[0]->detObjCommon.timeConfidence);

        EXPECT_EQ(4000, sdsmPtr->objects.list.array[0]->detObjCommon.pos.offsetX);
        EXPECT_EQ(-720, sdsmPtr->objects.list.array[0]->detObjCommon.pos.offsetY);
        EXPECT_EQ(20, *sdsmPtr->objects.list.array[0]->detObjCommon.pos.offsetZ);

        EXPECT_EQ(2, sdsmPtr->objects.list.array[0]->detObjCommon.posConfidence.pos);
        EXPECT_EQ(5, sdsmPtr->objects.list.array[0]->detObjCommon.posConfidence.elevation);

        EXPECT_EQ(2100, sdsmPtr->objects.list.array[0]->detObjCommon.speed);
        EXPECT_EQ(3, sdsmPtr->objects.list.array[0]->detObjCommon.speedConfidence);
        EXPECT_EQ(1000, *sdsmPtr->objects.list.array[0]->detObjCommon.speedZ);
        EXPECT_EQ(4, *sdsmPtr->objects.list.array[0]->detObjCommon.speedConfidenceZ);

        EXPECT_EQ(16000, sdsmPtr->objects.list.array[0]->detObjCommon.heading);
        EXPECT_EQ(4, sdsmPtr->objects.list.array[0]->detObjCommon.headingConf);

        EXPECT_EQ(200, sdsmPtr->objects.list.array[0]->detObjCommon.accel4way->Long);
        EXPECT_EQ(-500, sdsmPtr->objects.list.array[0]->detObjCommon.accel4way->lat);
        EXPECT_EQ(1, sdsmPtr->objects.list.array[0]->detObjCommon.accel4way->vert);
        EXPECT_EQ(400, sdsmPtr->objects.list.array[0]->detObjCommon.accel4way->yaw);
        EXPECT_EQ(4, *sdsmPtr->objects.list.array[0]->detObjCommon.accCfdX);
        EXPECT_EQ(5, *sdsmPtr->objects.list.array[0]->detObjCommon.accCfdY);
        EXPECT_EQ(6, *sdsmPtr->objects.list.array[0]->detObjCommon.accCfdZ);
        EXPECT_EQ(3, *sdsmPtr->objects.list.array[0]->detObjCommon.accCfdYaw);

        tmx::messages::SdsmEncodedMessage encodedSdsm;
        converter.encodeSDSM(sdsmPtr, encodedSdsm);
        EXPECT_EQ(41,  encodedSdsm.get_msgId());
	
        std::string expectedSDSMEncHex = "00293981313233343fdf5dc933c4e226c29af8da011e1a2ffe203dd790c3514007f304bea06402c7cfbe97c00992a0d18fa23e809130bb901031f2e6";
        EXPECT_EQ(expectedSDSMEncHex, encodedSdsm.get_payload_str());	
    }


    // Test for SDSM optional data - vehicle data
    TEST_F(test_JsonToJ3224SDSMConverter, convertJsonToSDSM_veh)
    {
        JsonToJ3224SDSMConverter converter;
        std::string valid_json_str = R"(
            {
                "equipment_type":1,
                "msg_cnt":1,
                "objects":[
                    {
                        "detected_object_data":{
                            "detected_object_common_data":{
                                "acc_cfd_x":4,
                                "acc_cfd_y":5,
                                "acc_cfd_yaw":3,
                                "acc_cfd_z":6,
                                "accel_4_way":{
                                    "lat":-500,"
                                    long":200,
                                    "vert":1,"
                                    yaw":400
                                },
                                "heading":16000,
                                "heading_conf":4,
                                "measurement_time":-1100,
                                "object_id":12200,
                                "obj_type":1,
                                "obj_type_cfd":65,
                                "pos":{"offset_x":4000,"offset_y":-720,"offset_z":20},
                                "pos_confidence":{"elevation":5,"pos":2},
                                "speed":2100,
                                "speed_confidence":3,
                                "speed_confidence_z":4,
                                "speed_z":1000,
                                "time_confidence":2
                            },
                            "detected_object_optional_data":{
                                "detected_vehicle_data":{
                                    "height":70,
                                    "lights":8,
                                    "size":{
                                        "length":700,
                                        "width":300
                                    },
                                    "veh_ang_vel":{
                                        "pitch_rate":600,
                                        "roll_rate":-800
                                    },
                                    "veh_ang_vel_confidence":{
                                        "pitch_rate_confidence":3,
                                        "roll_rate_confidence":4
                                    },
                                    "veh_attitude":{
                                        "pitch":2400,
                                        "roll":-12000,
                                        "yaw":400
                                    },
                                    "veh_attitude_confidence":{
                                        "pitch_confidence":2,
                                        "roll_confidence":3,
                                        "yaw_confidence":4
                                    },
                                    "vehicle_class":11,
                                    "vehicle_class_conf":75,
                                    "vehicle_size_confidence":{
                                        "vehicle_height_confidence":5,
                                        "vehicle_length_confidence":6,
                                        "vehicle_width_confidence":7
                                    }
                                }
                            }
                        }
                    }
                ],
                "ref_pos":{
                    "long":600000000,
                    "elevation":30,
                    "lat":400000000
                },
                "ref_pos_el_conf":10,
                "ref_pos_xy_conf":{"orientation":25000,"semi_major":235,"semi_minor":200},
                "sdsm_time_stamp":{"day":4,"hour":19,"minute":15,"month":7,"offset":400,"second":5000,"year":2007},
                "source_id":"01020304"
            })";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        EXPECT_TRUE(result);
        auto sdsmPtr = std::make_shared<SensorDataSharingMessage>();
        converter.convertJsonToSDSM(root, sdsmPtr);


        EXPECT_EQ(2400, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAttitude->pitch);
        EXPECT_EQ(-12000, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAttitude->roll);
        EXPECT_EQ(400, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAttitude->yaw);

        EXPECT_EQ(2, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAttitudeConfidence->pitchConfidence);
        EXPECT_EQ(3, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAttitudeConfidence->rollConfidence);
        EXPECT_EQ(4, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAttitudeConfidence->yawConfidence);

        EXPECT_EQ(600, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAngVel->pitchRate);
        EXPECT_EQ(-800, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAngVel->rollRate);

        EXPECT_EQ(3, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAngVelConfidence->pitchRateConfidence);
        EXPECT_EQ(4, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehAngVelConfidence->rollRateConfidence);

        EXPECT_EQ(300, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.size->width);
        EXPECT_EQ(700, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.size->length);
        EXPECT_EQ(70, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.height);

        EXPECT_EQ(7, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehicleSizeConfidence->vehicleWidthConfidence);
        EXPECT_EQ(6, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehicleSizeConfidence->vehicleLengthConfidence);
        EXPECT_EQ(5, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehicleSizeConfidence->vehicleHeightConfidence);

        EXPECT_EQ(11, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.vehicleClass);
        EXPECT_EQ(75, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVeh.classConf);
        tmx::messages::SdsmEncodedMessage encodedSdsm;
        converter.encodeSDSM(sdsmPtr, encodedSdsm);
        EXPECT_EQ(41,  encodedSdsm.get_msgId());
	
        std::string expectedSDSMEncHex = "00294e81303330343fdf5dc933c4e226c29af8da011e1a2ffe203dd790c3514017f304bea06402c7cfbe97c00992a0d18fa23e808fa0bb900ffff2e61ff96004b039d04e412bbe6fee2585791aeca172c0";
        EXPECT_EQ(expectedSDSMEncHex, encodedSdsm.get_payload_str());	
    }

    // Test for SDSM optional data - VRU data
    TEST_F(test_JsonToJ3224SDSMConverter, convertJsonToSDSM_vru)
    {
        JsonToJ3224SDSMConverter converter;
        std::string valid_json_str = R"(
        {
            "equipment_type": 1,
            "msg_cnt": 1,
            "objects": [
                {
                "detected_object_data": {
                    "detected_object_common_data": {
                    "acc_cfd_x": 4,
                    "acc_cfd_y": 5,
                    "acc_cfd_yaw": 3,
                    "acc_cfd_z": 6,
                    "accel_4_way": {
                        "lat": -500,
                        "long": 200,
                        "vert": 1,
                        "yaw": 400
                    },
                    "heading": 16000,
                    "heading_conf": 4,
                    "measurement_time": -1100,
                    "object_id": 12200,
                    "obj_type": 1,
                    "obj_type_cfd": 65,
                    "pos": {
                        "offset_x": 4000,
                        "offset_y": -720,
                        "offset_z": 20
                    },
                    "pos_confidence": {
                        "elevation": 5,
                        "pos": 2
                    },
                    "speed": 2100,
                    "speed_confidence": 3,
                    "speed_confidence_z": 4,
                    "speed_z": 1000,
                    "time_confidence": 2
                    },
                    "detected_object_optional_data": {
                    "detected_vru_data": {
                        "attachment": 3,
                        "basic_type": 1,
                        "propulsion": {
                        "human": 2
                        },
                        "radius": 30
                    }
                    }
                }
                }
            ],
            "ref_pos": {
                "long": 600000000,
                "elevation": 30,
                "lat": 400000000
            },
            "ref_pos_el_conf": 10,
            "ref_pos_xy_conf": {
                "orientation": 25000,
                "semi_major": 235,
                "semi_minor": 200
            },
            "sdsm_time_stamp": {
                "day": 4,
                "hour": 19,
                "minute": 15,
                "month": 7,
                "offset": 400,
                "second": 5000,
                "year": 2007
            },
            "source_id": "01020304"
        })";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        EXPECT_TRUE(result);
        auto sdsmPtr = std::make_shared<SensorDataSharingMessage>();
        converter.convertJsonToSDSM(root, sdsmPtr);

        EXPECT_EQ(1, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVRU.basicType);
        EXPECT_EQ(2, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVRU.propulsion->choice.human);
        EXPECT_EQ(3, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVRU.attachment);
        EXPECT_EQ(30, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detVRU.radius);
        tmx::messages::SdsmEncodedMessage encodedSdsm;
        converter.encodeSDSM(sdsmPtr, encodedSdsm);
        EXPECT_EQ(41,  encodedSdsm.get_msgId());
	
        std::string expectedSDSMEncHex = "00293d81303330343fdf5dc933c4e226c29af8da011e1a2ffe203dd790c3514017f304bea06402c7cfbe97c00992a0d18fa23e809130bb901031f2e6f88231e0";
        EXPECT_EQ(expectedSDSMEncHex, encodedSdsm.get_payload_str());

    }

    // Test for SDSM optional data - obstacle data
    TEST_F(test_JsonToJ3224SDSMConverter, convertJsonToSDSM_obst)
    {
        JsonToJ3224SDSMConverter converter;
        std::string valid_json_str = R"(
            {
                "equipment_type": 1,
                "msg_cnt": 1,
                "objects": [
                    {
                    "detected_object_data": {
                        "detected_object_common_data": {
                        "acc_cfd_x": 4,
                        "acc_cfd_y": 5,
                        "acc_cfd_yaw": 3,
                        "acc_cfd_z": 6,
                        "accel_4_way": {
                            "lat": -500,
                            "long": 200,
                            "vert": 1,
                            "yaw": 400
                        },
                        "heading": 16000,
                        "heading_conf": 4,
                        "measurement_time": -1100,
                        "object_id": 12200,
                        "obj_type": 1,
                        "obj_type_cfd": 65,
                        "pos": {
                            "offset_x": 4000,
                            "offset_y": -720,
                            "offset_z": 20
                        },
                        "pos_confidence": {
                            "elevation": 5,
                            "pos": 2
                        },
                        "speed": 2100,
                        "speed_confidence": 3,
                        "speed_confidence_z": 4,
                        "speed_z": 1000,
                        "time_confidence": 2
                        },
                        "detected_object_optional_data": {
                        "detected_obstacle_data": {
                            "obst_size": {
                            "height": 100,
                            "length": 300,
                            "width": 400
                            },
                            "obst_size_confidence": {
                            "height_confidence": 8,
                            "length_confidence": 7,
                            "width_confidence": 6
                            }
                        }
                        }
                    }
                    }
                ],
                "ref_pos": {
                    "long": 600000000,
                    "elevation": 30,
                    "lat": 400000000
                },
                "ref_pos_el_conf": 10,
                "ref_pos_xy_conf": {
                    "orientation": 25000,
                    "semi_major": 235,
                    "semi_minor": 200
                },
                "sdsm_time_stamp": {
                    "day": 4,
                    "hour": 19,
                    "minute": 15,
                    "month": 7,
                    "offset": 400,
                    "second": 5000,
                    "year": 2007
                },
                "source_id": "01020304"
                }
        )";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        EXPECT_TRUE(result);
        auto sdsmPtr = std::make_shared<SensorDataSharingMessage>();
        converter.convertJsonToSDSM(root, sdsmPtr);

        EXPECT_EQ(400, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detObst.obstSize.width);
        EXPECT_EQ(300, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detObst.obstSize.length);
        EXPECT_EQ(100, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detObst.obstSize.height);

        EXPECT_EQ(6, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detObst.obstSizeConfidence.widthConfidence);
        EXPECT_EQ(7, sdsmPtr->objects.list.array[0]->detObjOptData->choice.detObst.obstSizeConfidence.lengthConfidence);
        EXPECT_EQ(8, *sdsmPtr->objects.list.array[0]->detObjOptData->choice.detObst.obstSizeConfidence.heightConfidence);
        tmx::messages::SdsmEncodedMessage encodedSdsm;
        converter.encodeSDSM(sdsmPtr, encodedSdsm);
        EXPECT_EQ(41,  encodedSdsm.get_msgId());
	
        std::string expectedSDSMEncHex = "00293f81303330343fdf5dc933c4e226c29af8da011e1a2ffe203dd790c3514017f304bea06402c7cfbe97c00992a0d18fa23e809130bb901031f2e75904b064b3c0";
        EXPECT_EQ(expectedSDSMEncHex, encodedSdsm.get_payload_str());
    }

    // Test for SDSM optional data - obstacle data
    TEST_F(test_JsonToJ3224SDSMConverter, convertJsonToSDSM_sdsm_service_generated)
    {
        JsonToJ3224SDSMConverter converter;
        std::string valid_json_str = R"(
            {
                "msg_cnt": 0,
                "source_id": "rsu_1234",
                "equipment_type": 1,
                "sdsm_time_stamp": {
                    "second": 30275,
                    "minute": 3,
                    "hour": 19,
                    "day": 17,
                    "month": 12,
                    "year": 2023,
                    "offset": 0
                },
                "ref_pos": {
                    "long": 0,
                    "lat": 0
                },
                "ref_pos_xy_conf": {
                    "semi_major": 0,
                    "semi_minor": 0,
                    "orientation": 0
                },
                "objects": [
                    {
                    "detected_object_data": {
                        "detected_object_common_data": {
                        "obj_type": 0,
                        "object_id": 1,
                        "obj_type_cfd": 70,
                        "measurement_time": 0,
                        "time_confidence": 0,
                        "pos": {
                            "offset_x": -11,
                            "offset_y": -20,
                            "offset_z": -32
                        },
                        "pos_confidence": {
                            "pos": 0,
                            "elevation": 0
                        },
                        "speed": 70,
                        "speed_confidence": 0,
                        "speed_z": 50,
                        "heading": 0,
                        "heading_conf": 0
                        },
                        "detected_object_optional_data": {
                        "detected_obstacle_data": {
                            "obst_size": {
                            "width": 5,
                            "length": 20,
                            "height": 10
                            },
                            "obst_size_confidence": {
                            "width_confidence": 0,
                            "length_confidence": 0
                            }
                        }
                        }
                    }
                    }
                ]
                }
        )";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        EXPECT_TRUE(result);
        auto sdsmPtr = std::make_shared<SensorDataSharingMessage>();
        converter.convertJsonToSDSM(root, sdsmPtr);

        tmx::messages::SdsmEncodedMessage encodedSdsm;
        converter.encodeSDSM(sdsmPtr, encodedSdsm);
        EXPECT_EQ(41,  encodedSdsm.get_msgId());
	
        std::string expectedSDSMEncHex = "00293400313233343fdf9f2330dd90da406b49d200d693a3fe00000000014011800057700bffa3ff5bfef80011800c80000a0282805000";
        EXPECT_EQ(expectedSDSMEncHex, encodedSdsm.get_payload_str());
    }


// Test for SDSM optional data - obstacle data
    TEST_F(test_JsonToJ3224SDSMConverter, convertJsonToSDSM_multiple_objects)
    {
        JsonToJ3224SDSMConverter converter;
        std::string valid_json_str = R"(
            {
                "msg_cnt": 127,
                "source_id": "rsu_1234",
                "equipment_type": 1,
                "sdsm_time_stamp": {
                    "second": 1781,
                    "minute": 0,
                    "hour": 20,
                    "day": 17,
                    "month": 12,
                    "year": 2023,
                    "offset": 0
                },
                "ref_pos": {
                    "long": 0,
                    "lat": 0
                },
                "ref_pos_xy_conf": {
                    "semi_major": 0,
                    "semi_minor": 0,
                    "orientation": 0
                },
                "objects": [
                    {
                    "detected_object_data": {
                        "detected_object_common_data": {
                        "obj_type": 0,
                        "object_id": 1,
                        "obj_type_cfd": 70,
                        "measurement_time": 0,
                        "time_confidence": 0,
                        "pos": {
                            "offset_x": -11,
                            "offset_y": -20,
                            "offset_z": -32
                        },
                        "pos_confidence": {
                            "pos": 0,
                            "elevation": 0
                        },
                        "speed": 70,
                        "speed_confidence": 0,
                        "speed_z": 50,
                        "heading": 0,
                        "heading_conf": 0
                        },
                        "detected_object_optional_data": {
                        "detected_obstacle_data": {
                            "obst_size": {
                            "width": 5,
                            "length": 20,
                            "height": 10
                            },
                            "obst_size_confidence": {
                            "width_confidence": 0,
                            "length_confidence": 0
                            }
                        }
                        }
                    }
                    },
                    {
                    "detected_object_data": {
                        "detected_object_common_data": {
                        "obj_type": 2,
                        "object_id": 2,
                        "obj_type_cfd": 70,
                        "measurement_time": 0,
                        "time_confidence": 0,
                        "pos": {
                            "offset_x": -11,
                            "offset_y": -20,
                            "offset_z": -32
                        },
                        "pos_confidence": {
                            "pos": 0,
                            "elevation": 0
                        },
                        "speed": 26,
                        "speed_confidence": 0,
                        "speed_z": 0,
                        "heading": 0,
                        "heading_conf": 0
                        },
                        "detected_object_optional_data": {
                        "detected_vru_data": {}
                        }
                    }
                    },
                    {
                    "detected_object_data": {
                        "detected_object_common_data": {
                        "obj_type": 1,
                        "object_id": 3,
                        "obj_type_cfd": 70,
                        "measurement_time": 0,
                        "time_confidence": 0,
                        "pos": {
                            "offset_x": -11,
                            "offset_y": -20,
                            "offset_z": -32
                        },
                        "pos_confidence": {
                            "pos": 0,
                            "elevation": 0
                        },
                        "speed": 26,
                        "speed_confidence": 0,
                        "speed_z": 0,
                        "heading": 0,
                        "heading_conf": 0
                        },
                        "detected_object_optional_data": {
                        "detected_vehicle_data": {
                            "size": {
                            "width": 50,
                            "length": 50
                            },
                            "height": 40
                        }
                        }
                    }
                    }
                ]
                }
        )";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        EXPECT_TRUE(result);
        auto sdsmPtr = std::make_shared<SensorDataSharingMessage>();
        converter.convertJsonToSDSM(root, sdsmPtr);

        EXPECT_EQ(3, sdsmPtr->objects.list.count);
        // Encode message
        tmx::messages::SdsmEncodedMessage encodedSdsm;
        converter.encodeSDSM(sdsmPtr, encodedSdsm);
        EXPECT_EQ(41,  encodedSdsm.get_msgId());
	
        std::string expectedSDSMEncHex = "0029617f313233343fdf9f234001bd5a406b49d200d693a3fe00000000054011800057700bffa3ff5bfef80011800c80000a028280500280a300012ee017ff47feb7fdf0000d0000000008500c600035dc02ffe8ffd6ffbe0001a0000000000301901928";
        EXPECT_EQ(expectedSDSMEncHex, encodedSdsm.get_payload_str());
    }
    
}