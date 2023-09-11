#include <gtest/gtest.h>
#include "jsoncpp/json/json.h"
#include <J3224ToSDSMJsonConverter.h>
#include <cassert>

class test_J3224ToSDSMJsonConverter : public ::testing::Test
{
    public:
        test_J3224ToSDSMJsonConverter() = default;
        ~test_J3224ToSDSMJsonConverter() = default;
};

namespace unit_test
{

    TEST_F(test_J3224ToSDSMJsonConverter, convertJ3224ToSDSMJSON)
    {
        std::vector<std::shared_ptr<void>> shared_ptrs;
        CARMAStreetsPlugin::J3224ToSDSMJsonConverter sdsmConverter;

        SensorDataSharingMessage *message = (SensorDataSharingMessage_t *)calloc(1, sizeof(SensorDataSharingMessage_t));
        
        message->msgCnt = 15;

        uint8_t source_id_bytes[4] = {(uint8_t)1, (uint8_t)2, (uint8_t)3, (uint8_t)4};
        TemporaryID_t source_id;
        source_id.buf = source_id_bytes;
        source_id.size = sizeof(source_id_bytes);
        message->sourceID = source_id;

        message->equipmentType = EquipmentType_rsu;

        auto year_ptr = CARMAStreetsPlugin::create_store_shared<DYear_t>(shared_ptrs);
        *year_ptr = 2000;
        message->sDSMTimeStamp.year = year_ptr;
        auto month_ptr = CARMAStreetsPlugin::create_store_shared<DMonth_t>(shared_ptrs);
        *month_ptr = 7;
        message->sDSMTimeStamp.month = month_ptr;
        auto day_ptr = CARMAStreetsPlugin::create_store_shared<DDay_t>(shared_ptrs);
        *day_ptr = 4;
        message->sDSMTimeStamp.day = day_ptr;
        auto hour_ptr = CARMAStreetsPlugin::create_store_shared<DHour_t>(shared_ptrs);
        *hour_ptr = 21;
        message->sDSMTimeStamp.hour = hour_ptr;
        auto minute_ptr = CARMAStreetsPlugin::create_store_shared<DMinute_t>(shared_ptrs);
        *minute_ptr = 15;
        message->sDSMTimeStamp.minute = minute_ptr;
        auto second_ptr = CARMAStreetsPlugin::create_store_shared<DSecond_t>(shared_ptrs);
        *second_ptr = 10000;
        message->sDSMTimeStamp.second = second_ptr;
        auto offset_ptr = CARMAStreetsPlugin::create_store_shared<DOffset_t>(shared_ptrs);
        *offset_ptr = 2000;
        message->sDSMTimeStamp.offset = offset_ptr;

        message->refPos.lat = 400000000;
        message->refPos.Long = 800000000;
        auto pos_elevation_ptr = CARMAStreetsPlugin::create_store_shared<DSRC_Elevation_t>(shared_ptrs);
        *pos_elevation_ptr = 30;
        message->refPos.elevation = pos_elevation_ptr;

        message->refPosXYConf.semiMajor = 250;
        message->refPosXYConf.semiMinor = 200;
        message->refPosXYConf.orientation = 20000;

        auto elevation_conf_ptr = CARMAStreetsPlugin::create_store_shared<ElevationConfidence_t>(shared_ptrs);
        *elevation_conf_ptr = ElevationConfidence_elev_000_05;
        message->refPosElConf = elevation_conf_ptr;

        DetectedObjectList_t detected_obj_list;


        // Generate an object to test common data and detected vehicle data
        DetectedObjectData_t test_obj1;

        test_obj1.detObjCommon.objType = ObjectType_vehicle;
        test_obj1.detObjCommon.objTypeCfd = 65;
        test_obj1.detObjCommon.objectID = 1200;
        test_obj1.detObjCommon.measurementTime = 500;
        test_obj1.detObjCommon.timeConfidence = TimeConfidence_time_000_200;

        test_obj1.detObjCommon.pos.offsetX = 1000;
        test_obj1.detObjCommon.pos.offsetY = 750;
        auto offsetZ_ptr = CARMAStreetsPlugin::create_store_shared<ObjectDistance_t>(shared_ptrs);
        *offsetZ_ptr = 50;
        test_obj1.detObjCommon.pos.offsetZ = offsetZ_ptr;

        test_obj1.detObjCommon.posConfidence.pos = PositionConfidence_a200m;
        test_obj1.detObjCommon.posConfidence.elevation = ElevationConfidence_elev_100_00;

        test_obj1.detObjCommon.speed = 2100;
        test_obj1.detObjCommon.speedConfidence = SpeedConfidence_prec5ms;

        auto speedZ_ptr = CARMAStreetsPlugin::create_store_shared<Speed_t>(shared_ptrs);
        *speedZ_ptr = 1200;
        test_obj1.detObjCommon.speedZ = speedZ_ptr;
        auto speedZ_conf_ptr = CARMAStreetsPlugin::create_store_shared<SpeedConfidence_t>(shared_ptrs);
        *speedZ_conf_ptr = SpeedConfidence_prec0_1ms;
        test_obj1.detObjCommon.speedConfidenceZ = speedZ_conf_ptr;

        test_obj1.detObjCommon.heading = 15000;
        test_obj1.detObjCommon.headingConf = HeadingConfidence_prec0_05deg;

        auto accel_4_way_ptr = CARMAStreetsPlugin::create_store_shared<AccelerationSet4Way_t>(shared_ptrs);
        accel_4_way_ptr->Long = 200;
        accel_4_way_ptr->lat = -500;
        accel_4_way_ptr->vert = 1;
        accel_4_way_ptr->yaw = 400;
        test_obj1.detObjCommon.accel4way = accel_4_way_ptr;

        auto acc_cfd_x_ptr = CARMAStreetsPlugin::create_store_shared<AccelerationConfidence_t>(shared_ptrs);
        *acc_cfd_x_ptr = AccelerationConfidence_accl_100_00;
        test_obj1.detObjCommon.accCfdX = acc_cfd_x_ptr;
        auto acc_cfd_y_ptr = CARMAStreetsPlugin::create_store_shared<AccelerationConfidence_t>(shared_ptrs);
        *acc_cfd_y_ptr = AccelerationConfidence_accl_010_00;
        test_obj1.detObjCommon.accCfdY = acc_cfd_y_ptr;
        auto acc_cfd_z_ptr = CARMAStreetsPlugin::create_store_shared<AccelerationConfidence_t>(shared_ptrs);
        *acc_cfd_z_ptr = AccelerationConfidence_accl_005_00;
        test_obj1.detObjCommon.accCfdZ = acc_cfd_z_ptr;

        auto acc_cfd_yaw_ptr = CARMAStreetsPlugin::create_store_shared<YawRateConfidence_t>(shared_ptrs);
        *acc_cfd_yaw_ptr = YawRateConfidence_degSec_001_00;
        test_obj1.detObjCommon.accCfdYaw = acc_cfd_yaw_ptr;


        auto det_obj_opt_ptr = CARMAStreetsPlugin::create_store_shared<DetectedObjectOptionalData_t>(shared_ptrs);
        
        det_obj_opt_ptr->present = DetectedObjectOptionalData_PR_detVeh;

        auto lights_ptr = CARMAStreetsPlugin::create_store_shared<ExteriorLights_t>(shared_ptrs);
        // TODO: Make sure lights are being set and converted properly
        uint8_t lights_bytes[9] = {(uint8_t)8};
        lights_ptr->buf = lights_bytes;
        lights_ptr->size = sizeof(lights_bytes);
        det_obj_opt_ptr->choice.detVeh.lights = lights_ptr;

        auto veh_attitude_ptr = CARMAStreetsPlugin::create_store_shared<Attitude_t>(shared_ptrs);
        veh_attitude_ptr->pitch = 2400;
        veh_attitude_ptr->roll = -12000;
        veh_attitude_ptr->yaw = 600;
        det_obj_opt_ptr->choice.detVeh.vehAttitude = veh_attitude_ptr;

        auto veh_attitude_conf_ptr = CARMAStreetsPlugin::create_store_shared<AttitudeConfidence_t>(shared_ptrs);
        veh_attitude_conf_ptr->pitchConfidence = HeadingConfidence_prec0_05deg;
        veh_attitude_conf_ptr->rollConfidence = HeadingConfidence_prec0_01deg;
        veh_attitude_conf_ptr->yawConfidence = HeadingConfidence_prec0_0125deg;
        det_obj_opt_ptr->choice.detVeh.vehAttitudeConfidence = veh_attitude_conf_ptr;

        auto veh_ang_vel_ptr = CARMAStreetsPlugin::create_store_shared<AngularVelocity_t>(shared_ptrs);
        veh_ang_vel_ptr->pitchRate = 800;
        veh_ang_vel_ptr->rollRate = -600;
        det_obj_opt_ptr->choice.detVeh.vehAngVel = veh_ang_vel_ptr;

        auto veh_ang_vel_conf_ptr = CARMAStreetsPlugin::create_store_shared<AngularVelocityConfidence_t>(shared_ptrs);
        auto pitch_rate_conf_ptr = CARMAStreetsPlugin::create_store_shared<PitchRateConfidence_t>(shared_ptrs);
        *pitch_rate_conf_ptr = PitchRateConfidence_degSec_001_00;
        veh_ang_vel_conf_ptr->pitchRateConfidence = pitch_rate_conf_ptr;
        auto roll_rate_conf_ptr = CARMAStreetsPlugin::create_store_shared<RollRateConfidence_t>(shared_ptrs);
        *roll_rate_conf_ptr = RollRateConfidence_degSec_000_10;
        veh_ang_vel_conf_ptr->rollRateConfidence = roll_rate_conf_ptr;
        det_obj_opt_ptr->choice.detVeh.vehAngVelConfidence = veh_ang_vel_conf_ptr;

        auto veh_size_ptr = CARMAStreetsPlugin::create_store_shared<VehicleSize_t>(shared_ptrs);
        veh_size_ptr->width = 300;
        veh_size_ptr->length = 700;
        det_obj_opt_ptr->choice.detVeh.size = veh_size_ptr;

        auto veh_height_ptr = CARMAStreetsPlugin::create_store_shared<VehicleHeight_t>(shared_ptrs);
        *veh_height_ptr = 70;
        det_obj_opt_ptr->choice.detVeh.height= veh_height_ptr;

        auto veh_size_conf_ptr = CARMAStreetsPlugin::create_store_shared<VehicleSizeConfidence_t>(shared_ptrs);
        veh_size_conf_ptr->vehicleWidthConfidence = SizeValueConfidence_size_000_10;
        veh_size_conf_ptr->vehicleLengthConfidence = SizeValueConfidence_size_000_05;
        auto veh_height_conf_ptr = CARMAStreetsPlugin::create_store_shared<SizeValueConfidence_t>(shared_ptrs);
        *veh_height_conf_ptr = SizeValueConfidence_size_000_02;
        veh_size_conf_ptr->vehicleHeightConfidence = veh_height_conf_ptr;
        det_obj_opt_ptr->choice.detVeh.vehicleSizeConfidence = veh_size_conf_ptr;

        auto veh_class_ptr = CARMAStreetsPlugin::create_store_shared<BasicVehicleClass_t>(shared_ptrs);
        *veh_class_ptr = 45;
        det_obj_opt_ptr->choice.detVeh.vehicleClass = veh_class_ptr;

        auto veh_class_conf_ptr = CARMAStreetsPlugin::create_store_shared<ClassificationConfidence_t>(shared_ptrs);
        *veh_class_conf_ptr = 85;
        det_obj_opt_ptr->choice.detVeh.classConf = veh_class_conf_ptr;

        test_obj1.detObjOptData = det_obj_opt_ptr;

        asn_sequence_add(&message->objects.list, &test_obj1);


        // Generate a second object to test detected VRU data
        DetectedObjectData_t test_obj2;

        test_obj2.detObjCommon.objType = ObjectType_vehicle;
        test_obj2.detObjCommon.objTypeCfd = 65;
        test_obj2.detObjCommon.objectID = 110;
        test_obj2.detObjCommon.measurementTime = 500;
        test_obj2.detObjCommon.timeConfidence = TimeConfidence_time_000_200;

        test_obj2.detObjCommon.pos.offsetX = 1000;
        test_obj2.detObjCommon.pos.offsetY = 750;
        auto offsetZ_ptr2 = CARMAStreetsPlugin::create_store_shared<ObjectDistance_t>(shared_ptrs);
        *offsetZ_ptr2 = 50;
        test_obj2.detObjCommon.pos.offsetZ = offsetZ_ptr2;

        test_obj2.detObjCommon.posConfidence.pos = PositionConfidence_a200m;
        test_obj2.detObjCommon.posConfidence.elevation = ElevationConfidence_elev_100_00;

        test_obj2.detObjCommon.speed = 2100;
        test_obj2.detObjCommon.speedConfidence = SpeedConfidence_prec5ms;

        auto speedZ_ptr2 = CARMAStreetsPlugin::create_store_shared<Speed_t>(shared_ptrs);
        *speedZ_ptr2 = 1200;
        test_obj2.detObjCommon.speedZ = speedZ_ptr2;
        auto speedZ_conf_ptr2 = CARMAStreetsPlugin::create_store_shared<SpeedConfidence_t>(shared_ptrs);
        *speedZ_conf_ptr2 = SpeedConfidence_prec0_1ms;
        test_obj2.detObjCommon.speedConfidenceZ = speedZ_conf_ptr2;

        test_obj2.detObjCommon.heading = 15000;
        test_obj2.detObjCommon.headingConf = HeadingConfidence_prec0_05deg;

        auto accel_4_way_ptr2 = CARMAStreetsPlugin::create_store_shared<AccelerationSet4Way_t>(shared_ptrs);
        accel_4_way_ptr2->Long = 200;
        accel_4_way_ptr2->lat = -500;
        accel_4_way_ptr2->vert = 1;
        accel_4_way_ptr2->yaw = 400;
        test_obj2.detObjCommon.accel4way = accel_4_way_ptr2;

        auto acc_cfd_x_ptr2 = CARMAStreetsPlugin::create_store_shared<AccelerationConfidence_t>(shared_ptrs);
        *acc_cfd_x_ptr2 = AccelerationConfidence_accl_100_00;
        test_obj2.detObjCommon.accCfdX = acc_cfd_x_ptr2;
        auto acc_cfd_y_ptr2 = CARMAStreetsPlugin::create_store_shared<AccelerationConfidence_t>(shared_ptrs);
        *acc_cfd_y_ptr2 = AccelerationConfidence_accl_010_00;
        test_obj2.detObjCommon.accCfdY = acc_cfd_y_ptr2;
        auto acc_cfd_z_ptr2 = CARMAStreetsPlugin::create_store_shared<AccelerationConfidence_t>(shared_ptrs);
        *acc_cfd_z_ptr2 = AccelerationConfidence_accl_005_00;
        test_obj2.detObjCommon.accCfdZ = acc_cfd_z_ptr2;

        auto acc_cfd_yaw_ptr2 = CARMAStreetsPlugin::create_store_shared<YawRateConfidence_t>(shared_ptrs);
        *acc_cfd_yaw_ptr2 = YawRateConfidence_degSec_001_00;
        test_obj2.detObjCommon.accCfdYaw = acc_cfd_yaw_ptr2;

        auto det_obj_opt_ptr2 = CARMAStreetsPlugin::create_store_shared<DetectedObjectOptionalData_t>(shared_ptrs);
        det_obj_opt_ptr2->present = DetectedObjectOptionalData_PR_detVRU;

        auto basic_type = CARMAStreetsPlugin::create_store_shared<PersonalDeviceUserType_t>(shared_ptrs);
        *basic_type = PersonalDeviceUserType_aPEDESTRIAN;
        det_obj_opt_ptr2->choice.detVRU.basicType = basic_type;

        auto propulsion = CARMAStreetsPlugin::create_store_shared<PropelledInformation_t>(shared_ptrs);
        propulsion->present = PropelledInformation_PR_human;
        propulsion->choice.human = HumanPropelledType_onFoot;
        det_obj_opt_ptr2->choice.detVRU.propulsion = propulsion;

        auto attachment = CARMAStreetsPlugin::create_store_shared<Attachment_t>(shared_ptrs);
        *attachment = Attachment_wheelchair;
        det_obj_opt_ptr2->choice.detVRU.attachment = attachment;

        auto radius = CARMAStreetsPlugin::create_store_shared<AttachmentRadius_t>(shared_ptrs);
        *radius = 30;
        det_obj_opt_ptr2->choice.detVRU.radius = radius;

        test_obj2.detObjOptData = det_obj_opt_ptr2;
        asn_sequence_add(&message->objects.list, &test_obj2);



        // Generate a third object to test detected obstacle data
        DetectedObjectData_t test_obj3;

        test_obj3.detObjCommon.objType = ObjectType_vehicle;
        test_obj3.detObjCommon.objTypeCfd = 65;
        test_obj3.detObjCommon.objectID = 1000;
        test_obj3.detObjCommon.measurementTime = 500;
        test_obj3.detObjCommon.timeConfidence = TimeConfidence_time_000_200;

        test_obj3.detObjCommon.pos.offsetX = 1000;
        test_obj3.detObjCommon.pos.offsetY = 750;
        auto offsetZ_ptr3 = CARMAStreetsPlugin::create_store_shared<ObjectDistance_t>(shared_ptrs);
        *offsetZ_ptr3 = 50;
        test_obj3.detObjCommon.pos.offsetZ = offsetZ_ptr3;

        test_obj3.detObjCommon.posConfidence.pos = PositionConfidence_a200m;
        test_obj3.detObjCommon.posConfidence.elevation = ElevationConfidence_elev_100_00;

        test_obj3.detObjCommon.speed = 2100;
        test_obj3.detObjCommon.speedConfidence = SpeedConfidence_prec5ms;

        auto speedZ_ptr3 = CARMAStreetsPlugin::create_store_shared<Speed_t>(shared_ptrs);
        *speedZ_ptr3 = 1200;
        test_obj3.detObjCommon.speedZ = speedZ_ptr3;
        auto speedZ_conf_ptr3 = CARMAStreetsPlugin::create_store_shared<SpeedConfidence_t>(shared_ptrs);
        *speedZ_conf_ptr3 = SpeedConfidence_prec0_1ms;
        test_obj3.detObjCommon.speedConfidenceZ = speedZ_conf_ptr3;

        test_obj3.detObjCommon.heading = 15000;
        test_obj3.detObjCommon.headingConf = HeadingConfidence_prec0_05deg;

        auto accel_4_way_ptr3 = CARMAStreetsPlugin::create_store_shared<AccelerationSet4Way_t>(shared_ptrs);
        accel_4_way_ptr3->Long = 200;
        accel_4_way_ptr3->lat = -500;
        accel_4_way_ptr3->vert = 1;
        accel_4_way_ptr3->yaw = 400;
        test_obj3.detObjCommon.accel4way = accel_4_way_ptr3;

        auto acc_cfd_x_ptr3 = CARMAStreetsPlugin::create_store_shared<AccelerationConfidence_t>(shared_ptrs);
        *acc_cfd_x_ptr3 = AccelerationConfidence_accl_100_00;
        test_obj3.detObjCommon.accCfdX = acc_cfd_x_ptr3;
        auto acc_cfd_y_ptr3 = CARMAStreetsPlugin::create_store_shared<AccelerationConfidence_t>(shared_ptrs);
        *acc_cfd_y_ptr3 = AccelerationConfidence_accl_010_00;
        test_obj3.detObjCommon.accCfdY = acc_cfd_y_ptr3;
        auto acc_cfd_z_ptr3 = CARMAStreetsPlugin::create_store_shared<AccelerationConfidence_t>(shared_ptrs);
        *acc_cfd_z_ptr3 = AccelerationConfidence_accl_005_00;
        test_obj3.detObjCommon.accCfdZ = acc_cfd_z_ptr3;

        auto acc_cfd_yaw_ptr3 = CARMAStreetsPlugin::create_store_shared<YawRateConfidence_t>(shared_ptrs);
        *acc_cfd_yaw_ptr3 = YawRateConfidence_degSec_001_00;
        test_obj3.detObjCommon.accCfdYaw = acc_cfd_yaw_ptr3;



        auto det_obj_opt_ptr3 = CARMAStreetsPlugin::create_store_shared<DetectedObjectOptionalData_t>(shared_ptrs);
        det_obj_opt_ptr3->present = DetectedObjectOptionalData_PR_detObst;

        det_obj_opt_ptr3->choice.detObst.obstSize.width = 400;
        det_obj_opt_ptr3->choice.detObst.obstSize.length = 300;
        auto obj_height = CARMAStreetsPlugin::create_store_shared<SizeValue_t>(shared_ptrs);
        *obj_height = 100;
        det_obj_opt_ptr3->choice.detObst.obstSize.height = obj_height;

        det_obj_opt_ptr3->choice.detObst.obstSizeConfidence.widthConfidence = SizeValueConfidence_size_010_00;
        det_obj_opt_ptr3->choice.detObst.obstSizeConfidence.lengthConfidence = SizeValueConfidence_size_005_00;
        auto obj_height_conf = CARMAStreetsPlugin::create_store_shared<SizeValueConfidence_t>(shared_ptrs);
        *obj_height_conf = SizeValueConfidence_size_002_00;
        det_obj_opt_ptr3->choice.detObst.obstSizeConfidence.heightConfidence = obj_height_conf;

        test_obj3.detObjOptData = det_obj_opt_ptr3;

        asn_sequence_add(&message->objects.list, &test_obj3);

        // Create a SensorDataSharingMessage with the test data to load into a Json (sdsmJson)
        std::shared_ptr<SensorDataSharingMessage> sdsmMsgPtr(message);
        Json::Value sdsmJson;
        sdsmConverter.convertJ3224ToSDSMJSON(sdsmMsgPtr, sdsmJson);

        // Tests
        // Testing SDSM header data
        ASSERT_EQ(15, sdsmJson["msg_cnt"].asInt());
        ASSERT_EQ("01020304", sdsmJson["source_id"].asString());
        ASSERT_EQ(1, sdsmJson["equipment_type"].asInt());
        ASSERT_EQ(2000, sdsmJson["sdsm_time_stamp"]["year"].asInt());
        ASSERT_EQ(7, sdsmJson["sdsm_time_stamp"]["month"].asInt());
        ASSERT_EQ(4, sdsmJson["sdsm_time_stamp"]["day"].asInt());
        ASSERT_EQ(21, sdsmJson["sdsm_time_stamp"]["hour"].asInt());
        ASSERT_EQ(15, sdsmJson["sdsm_time_stamp"]["minute"].asInt());
        ASSERT_EQ(10000, sdsmJson["sdsm_time_stamp"]["second"].asInt());
        ASSERT_EQ(2000, sdsmJson["sdsm_time_stamp"]["offset"].asInt());
        ASSERT_EQ(400000000, sdsmJson["ref_pos"]["lat"].asInt());
        ASSERT_EQ(800000000, sdsmJson["ref_pos"]["long"].asInt());
        ASSERT_EQ(30, sdsmJson["ref_pos"]["elevation"].asInt());
        ASSERT_EQ(250, sdsmJson["ref_pos_xy_conf"]["semi_major"].asInt());
        ASSERT_EQ(200, sdsmJson["ref_pos_xy_conf"]["semi_minor"].asInt());
        ASSERT_EQ(20000, sdsmJson["ref_pos_xy_conf"]["orientation"].asInt());
        ASSERT_EQ(13, sdsmJson["ref_pos_el_conf"].asInt());

        // Object 1 detected object common data
        ASSERT_EQ(1, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["obj_type"].asInt());
        ASSERT_EQ(65, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["obj_type_cfd"].asInt());
        ASSERT_EQ(1200, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["object_id"].asInt());
        ASSERT_EQ(500, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["measurement_time"].asInt());
        ASSERT_EQ(8, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["time_confidence"].asInt());
        ASSERT_EQ(1000, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["pos"]["offset_x"].asInt());
        ASSERT_EQ(750, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["pos"]["offset_y"].asInt());
        ASSERT_EQ(50, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["pos"]["offset_z"].asInt());
        ASSERT_EQ(2, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["pos_confidence"]["pos"].asInt());
        ASSERT_EQ(3, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["pos_confidence"]["elevation"].asInt());
        ASSERT_EQ(2100, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["speed"].asInt());
        ASSERT_EQ(3, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["speed_confidence"].asInt());
        ASSERT_EQ(1200, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["speed_z"].asInt());
        ASSERT_EQ(5, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["speed_confidence_z"].asInt());
        ASSERT_EQ(15000, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["heading"].asInt());
        ASSERT_EQ(5, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["heading_conf"].asInt());
        ASSERT_EQ(200, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["long"].asInt());
        ASSERT_EQ(-500, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["lat"].asInt());
        ASSERT_EQ(1, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["vert"].asInt());
        ASSERT_EQ(400, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["yaw"].asInt());
        ASSERT_EQ(1, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["acc_cfd_x"].asInt());
        ASSERT_EQ(2, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["acc_cfd_y"].asInt());
        ASSERT_EQ(3, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["acc_cfd_z"].asInt());
        ASSERT_EQ(4, sdsmJson["objects"][0]["detected_object_data"]["detected_object_common_data"]["acc_cfd_yaw"].asInt());

        // Object 1 detected vehicle data (optional)
        // TODO: Better construction/test of ExteriorLights message and resulting bitstring
        // ASSERT_EQ(8, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["lights"].asInt());
        ASSERT_EQ(2400, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_attitude"]["pitch"].asInt());
        ASSERT_EQ(-12000, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_attitude"]["roll"].asInt());
        ASSERT_EQ(600, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_attitude"]["yaw"].asInt());
        ASSERT_EQ(2400, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_attitude"]["pitch"].asInt());
        ASSERT_EQ(5, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_attitude_confidence"]["pitch_confidence"].asInt());
        ASSERT_EQ(6, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_attitude_confidence"]["roll_confidence"].asInt());
        ASSERT_EQ(7, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_attitude_confidence"]["yaw_confidence"].asInt());
        ASSERT_EQ(800, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_ang_vel"]["pitch_rate"].asInt());
        ASSERT_EQ(-600, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_ang_vel"]["roll_rate"].asInt());
        ASSERT_EQ(4, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_ang_vel_confidence"]["pitch_rate_confidence"].asInt());
        ASSERT_EQ(5, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_ang_vel_confidence"]["roll_rate_confidence"].asInt());
        ASSERT_EQ(300, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["size"]["width"].asInt());
        ASSERT_EQ(700, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["size"]["length"].asInt());
        ASSERT_EQ(70, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["height"].asInt());
        ASSERT_EQ(10, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["vehicle_size_confidence"]["vehicle_width_confidence"].asInt());
        ASSERT_EQ(11, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["vehicle_size_confidence"]["vehicle_length_confidence"].asInt());
        ASSERT_EQ(12, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["vehicle_size_confidence"]["vehicle_height_confidence"].asInt());
        ASSERT_EQ(45, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["vehicle_class"].asInt());
        ASSERT_EQ(85, sdsmJson["objects"][0]["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["vehicle_class_conf"].asInt());

        // Only test the ID and optional VRU data for the 2nd object
        ASSERT_EQ(110, sdsmJson["objects"][1]["detected_object_data"]["detected_object_common_data"]["object_id"].asInt());
        ASSERT_EQ(1, sdsmJson["objects"][1]["detected_object_data"]["detected_object_optional_data"]["detected_vru_data"]["basic_type"].asInt());
        ASSERT_EQ(2, sdsmJson["objects"][1]["detected_object_data"]["detected_object_optional_data"]["detected_vru_data"]["propulsion"]["human"].asInt());
        ASSERT_EQ(4, sdsmJson["objects"][1]["detected_object_data"]["detected_object_optional_data"]["detected_vru_data"]["attachment"].asInt());
        ASSERT_EQ(30, sdsmJson["objects"][1]["detected_object_data"]["detected_object_optional_data"]["detected_vru_data"]["radius"].asInt());

        // Only test the ID and optional obstacle data for the 3rd object
        ASSERT_EQ(1000, sdsmJson["objects"][2]["detected_object_data"]["detected_object_common_data"]["object_id"].asInt());
        ASSERT_EQ(400, sdsmJson["objects"][2]["detected_object_data"]["detected_object_optional_data"]["detected_obstacle_data"]["obst_size"]["width"].asInt());
        ASSERT_EQ(300, sdsmJson["objects"][2]["detected_object_data"]["detected_object_optional_data"]["detected_obstacle_data"]["obst_size"]["length"].asInt());
        ASSERT_EQ(100, sdsmJson["objects"][2]["detected_object_data"]["detected_object_optional_data"]["detected_obstacle_data"]["obst_size"]["height"].asInt());
        ASSERT_EQ(4, sdsmJson["objects"][2]["detected_object_data"]["detected_object_optional_data"]["detected_obstacle_data"]["obst_size_confidence"]["width_confidence"].asInt());
        ASSERT_EQ(5, sdsmJson["objects"][2]["detected_object_data"]["detected_object_optional_data"]["detected_obstacle_data"]["obst_size_confidence"]["length_confidence"].asInt());
        ASSERT_EQ(6, sdsmJson["objects"][2]["detected_object_data"]["detected_object_optional_data"]["detected_obstacle_data"]["obst_size_confidence"]["height_confidence"].asInt());

    }

};

