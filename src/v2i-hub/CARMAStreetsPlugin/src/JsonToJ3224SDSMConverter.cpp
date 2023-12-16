#include "JsonToJ3224SDSMConverter.h"

namespace CARMAStreetsPlugin
{
    // TODO: Move these template functions to a more central location such as in a utility file
    // Template to use when created shared pointer objects for optional data
    template <typename T>
    T *create_store_shared(std::vector<std::shared_ptr<void>> &shared_pointers)
    {
        auto obj_shared = std::make_shared<T>();
        shared_pointers.push_back(obj_shared);
        return obj_shared.get();
    }

    // Template for shared pointers with array elements
    template <typename T>
    T *create_store_shared_array(std::vector<std::shared_ptr<void>> &shared_pointers, int size)
    {
        std::shared_ptr<T[]> array_shared(new T[size]{0});
        shared_pointers.push_back(array_shared);
        return array_shared.get();
    }

    // TODO: Consolidate parseJsonString into a more central location since it is being used verbatim in other converters
    bool JsonToJ3224SDSMConverter::parseJsonString(const std::string &consumedMsg, Json::Value &sdsmJsonOut) const
    {
        const auto jsonLen = static_cast<int>(consumedMsg.length());
        Json::CharReaderBuilder builder;
        JSONCPP_STRING err;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        bool parseResult = reader->parse(consumedMsg.c_str(), consumedMsg.c_str() + jsonLen, &sdsmJsonOut, &err);
        if (!parseResult)
        {
            PLOG(logERROR) << "Parse error: " << err << std::endl;
        }
        return parseResult;
    }
    void JsonToJ3224SDSMConverter::convertJson2SDSM(const Json::Value &sdsm_json, std::shared_ptr<SensorDataSharingMessage_t> sdsm) const {
        ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, sdsm.get());
        // Message Count
        sdsm->msgCnt = sdsm_json["msg_cnt"].asInt64();
        // Source ID
        std::string id_data = sdsm_json["source_id"].asString();
        TemporaryID_t tempID;
        std::vector<uint8_t> id_vector(id_data.begin(), id_data.end());
        uint8_t *id_ptr = &id_vector[0];
        tempID.buf = id_ptr;
        tempID.size = sizeof(4);
        sdsm->sourceID = tempID;
        // Equipment Type
        sdsm->equipmentType = sdsm_json["equipment_type"].asInt64();

        // SDSM DateTime timestamp
        auto sDSMTimeStamp = (DDateTime_t*) calloc(1, sizeof(DDateTime_t));
        auto year = (DYear_t*) calloc(1, sizeof(DYear_t));
        *year = sdsm_json["sdsm_time_stamp"]["year"].asInt64();
        sDSMTimeStamp->year = year;
        auto month = (DMonth_t*) calloc(1, sizeof(DMonth_t));
        *month = sdsm_json["sdsm_time_stamp"]["month"].asInt64();
        sDSMTimeStamp->month = month;
        auto day = (DDay_t*) calloc(1, sizeof(DDay_t));
        *day = sdsm_json["sdsm_time_stamp"]["day"].asInt64();
        sDSMTimeStamp->day = day;
        auto minute = (DMinute_t*) calloc(1, sizeof(DMinute_t));
        *minute = sdsm_json["sdsm_time_stamp"]["minute"].asInt64();
        sDSMTimeStamp->minute = minute;
        auto second = (DSecond_t*) calloc(1, sizeof(DSecond_t));
        *second = sdsm_json["sdsm_time_stamp"]["second"].asInt64();
        sDSMTimeStamp->second = second;
        sdsm->sDSMTimeStamp = *sDSMTimeStamp;
        // Reference Position
        sdsm->refPos.lat = sdsm_json["ref_pos"]["lat"].asInt64();
        sdsm->refPos.Long = sdsm_json["ref_pos"]["long"].asInt64();
        // Optional elevation 
        if (sdsm_json["ref_pos"].isMember("elevation") ) {
            auto elevation = (DSRC_Elevation_t*) calloc(1, sizeof(DSRC_Elevation_t));
            *elevation = sdsm_json["ref_pos"]["elevation"].asInt64();
            sdsm->refPos.elevation = elevation;
        }
        // Positional accuracy
        sdsm->refPosXYConf.semiMajor = sdsm_json["ref_pos_xy_conf"]["semi_major"].asInt64();
        sdsm->refPosXYConf.semiMinor = sdsm_json["ref_pos_xy_conf"]["semi_minor"].asInt64();
        sdsm->refPosXYConf.orientation = sdsm_json["ref_pos_xy_conf"]["orientation"].asInt64();
        if (sdsm_json.isMember("ref_pos_el_conf")) {
            auto elevation_confidence = (ElevationConfidence_t*) calloc(1, sizeof(ElevationConfidence_t));
            *elevation_confidence = sdsm_json["ref_pos_el_conf"].asInt64();
            sdsm->refPosElConf = elevation_confidence;
        }
        if (sdsm_json.isMember("objects") && sdsm_json["objects"].isArray() ) {
            auto objects = (DetectedObjectList_t*) calloc(1, sizeof(DetectedObjectList_t));

            Json::Value objectsJsonArr = sdsm_json["objects"];
            for(auto itr = objectsJsonArr.begin(); itr != objectsJsonArr.end(); itr++){
                auto objectData = (DetectedObjectData_t*) calloc(1, sizeof(DetectedObjectData_t));
                // Object Common Required Properties
                // Object Type
                objectData->detObjCommon.objType = (*itr)["detected_object_data"]["detected_object_common_data"]["obj_type"].asInt64();
                // Object Type Classification confidence
                objectData->detObjCommon.objTypeCfd = (*itr)["detected_object_data"]["detected_object_common_data"]["obj_type_cfd"].asInt64();
                // Object ID
                objectData->detObjCommon.objectID = (*itr)["detected_object_data"]["detected_object_common_data"]["object_id"].asInt64();
                // Time offset from SDSM timestamp
                objectData->detObjCommon.measurementTime = (*itr)["detected_object_data"]["detected_object_common_data"]["measurement_time"].asInt64();
                // Time offset confidence
                objectData->detObjCommon.timeConfidence = (*itr)["detected_object_data"]["detected_object_common_data"]["time_confidence"].asInt64();
                // Position offset from reference position
                objectData->detObjCommon.pos.offsetX = (*itr)["detected_object_data"]["detected_object_common_data"]["pos"]["offset_x"].asInt64();
                objectData->detObjCommon.pos.offsetY =  (*itr)["detected_object_data"]["detected_object_common_data"]["pos"]["offset_y"].asInt64();
                // Optional Z offset
                if ( (*itr)["detected_object_data"]["detected_object_common_data"]["pos"].isMember("offset_z") ) {
                    auto offset_z = (ObjectDistance_t*) calloc(1, sizeof(ObjectDistance_t));
                    *offset_z = (*itr)["detected_object_data"]["detected_object_common_data"]["pos"]["offset_z"].asInt64();
                    objectData->detObjCommon.pos.offsetZ = offset_z;
                }
                // Position Confidence
                objectData->detObjCommon.posConfidence.pos = (*itr)["detected_object_data"]["detected_object_common_data"]["pos_confidence"]["pos"].asInt64();
                // Elevation Confidence
                objectData->detObjCommon.posConfidence.elevation = (*itr)["detected_object_data"]["detected_object_common_data"]["pos_confidence"]["elevation"].asInt64();
                // Horizontal Speed
                objectData->detObjCommon.speed = (*itr)["detected_object_data"]["detected_object_common_data"]["speed"].asInt64();
                // Horizontal Speed confidence
                objectData->detObjCommon.speedConfidence = (*itr)["detected_object_data"]["detected_object_common_data"]["speed_confidence"].asInt64();
                // Optional Vertical Speed
                if ( (*itr)["detected_object_data"]["detected_object_common_data"].isMember("speed_z") ) {
                    auto speed_z = (Speed_t*) calloc(1, sizeof(Speed_t));
                    *speed_z = (*itr)["detected_object_data"]["detected_object_common_data"]["speed_z"].asInt64();
                    objectData->detObjCommon.speedZ = speed_z;
                }
                // Optional Vertical Speed confidence
                if ( (*itr)["detected_object_data"]["detected_object_common_data"].isMember("speed_confidence_z")) {
                    auto speed_confidence_z = (SpeedConfidence_t*) calloc(1, sizeof(SpeedConfidence_t));
                    *speed_confidence_z = (*itr)["detected_object_data"]["detected_object_common_data"]["speed_confidence_z"].asInt64();
                    objectData->detObjCommon.speedConfidenceZ = speed_confidence_z;
                }
                // Heading
                objectData->detObjCommon.heading = (*itr)["detected_object_data"]["detected_object_common_data"]["heading"].asInt64();
                // Heading Confidence
                objectData->detObjCommon.headingConf = (*itr)["detected_object_data"]["detected_object_common_data"]["heading_conf"].asInt64();
                // Optional 4 way acceleration
                if ( (*itr)["detected_object_data"]["detected_object_common_data"].isMember("accel_4_way") ){
                    auto accel_4way = (AccelerationSet4Way_t*) calloc(1, sizeof(AccelerationSet4Way_t));
                    accel_4way->Long   = (*itr)["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["long"].asInt64();
                    accel_4way->lat    = (*itr)["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["lat"].asInt64();
                    accel_4way->vert   = (*itr)["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["vert"].asInt64();
                    accel_4way->yaw    = (*itr)["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["yaw"].asInt64();
                    objectData->detObjCommon.accel4way = accel_4way;
                }
                ASN_SEQUENCE_ADD(&objects->list.array, objectData);

            }
            sdsm->objects = *objects;
        }
        asn_fprint(stdout, &asn_DEF_SensorDataSharingMessage, sdsm.get());


    }


    void JsonToJ3224SDSMConverter::convertJsonToSDSM(const Json::Value &sdsm_json, std::shared_ptr<SensorDataSharingMessage> sdsm) const
    {
        std::vector<std::shared_ptr<void>> shared_ptrs;

        sdsm->msgCnt = sdsm_json["msg_cnt"].asInt64();

        // TODO: confirm input sourceID from JSON to C struct constructs octet appropriately
        // sourceID
        TemporaryID_t tempID;

        std::string id_data = sdsm_json["source_id"].asString();
        std::vector<uint8_t> id_vector(id_data.begin(), id_data.end());
        uint8_t *id_ptr = &id_vector[0];
        tempID.buf = id_ptr;
        tempID.size = sizeof(id_ptr);
        sdsm->sourceID = tempID;

        sdsm->equipmentType = sdsm_json["equipment_type"].asInt64();

        
        // sDSMTimeStamp
        auto sdsm_time_stamp_ptr = CARMAStreetsPlugin::create_store_shared<DDateTime_t>(shared_ptrs);

        auto year_ptr = CARMAStreetsPlugin::create_store_shared<DYear_t>(shared_ptrs);
        *year_ptr = sdsm_json["sdsm_time_stamp"]["year"].asInt64();
        sdsm_time_stamp_ptr->year = year_ptr;

        auto month_ptr = CARMAStreetsPlugin::create_store_shared<DMonth_t>(shared_ptrs);
        *month_ptr = sdsm_json["sdsm_time_stamp"]["month"].asInt64();
        sdsm_time_stamp_ptr->month = month_ptr;

        auto day_ptr = CARMAStreetsPlugin::create_store_shared<DDay_t>(shared_ptrs);
        *day_ptr = sdsm_json["sdsm_time_stamp"]["day"].asInt64();
        sdsm_time_stamp_ptr->day = day_ptr;

        auto hour_ptr = CARMAStreetsPlugin::create_store_shared<DHour_t>(shared_ptrs);
        *hour_ptr = sdsm_json["sdsm_time_stamp"]["hour"].asInt64();
        sdsm_time_stamp_ptr->hour = hour_ptr;

        auto minute_ptr = CARMAStreetsPlugin::create_store_shared<DMinute_t>(shared_ptrs);
        *minute_ptr = sdsm_json["sdsm_time_stamp"]["minute"].asInt64();
        sdsm_time_stamp_ptr->minute = minute_ptr;

        auto second_ptr = CARMAStreetsPlugin::create_store_shared<DSecond_t>(shared_ptrs);
        *second_ptr = sdsm_json["sdsm_time_stamp"]["second"].asInt64();
        sdsm_time_stamp_ptr->second = second_ptr;

        auto offset_ptr = CARMAStreetsPlugin::create_store_shared<DOffset_t>(shared_ptrs);
        *offset_ptr = sdsm_json["sdsm_time_stamp"]["offset"].asInt64();
        sdsm_time_stamp_ptr->offset = offset_ptr;

        sdsm->sDSMTimeStamp = *sdsm_time_stamp_ptr;

        // refPos
        auto ref_pos_ptr = CARMAStreetsPlugin::create_store_shared<Position3D_t>(shared_ptrs);
        ref_pos_ptr->lat = sdsm_json["ref_pos"]["lat"].asInt64();
        ref_pos_ptr->Long = sdsm_json["ref_pos"]["long"].asInt64();
        auto elevation_ptr = CARMAStreetsPlugin::create_store_shared<DSRC_Elevation_t>(shared_ptrs);
        *elevation_ptr = sdsm_json["ref_pos"]["elevation"].asInt64();
        ref_pos_ptr->elevation = elevation_ptr;

        sdsm->refPos = *ref_pos_ptr;

        // refPosXYConf
        PositionalAccuracy_t ref_pos_xy_conf;
        ref_pos_xy_conf.semiMajor = sdsm_json["ref_pos_xy_conf"]["semi_major"].asInt64();
        ref_pos_xy_conf.semiMinor = sdsm_json["ref_pos_xy_conf"]["semi_minor"].asInt64();
        ref_pos_xy_conf.orientation = sdsm_json["ref_pos_xy_conf"]["orientation"].asInt64();

        sdsm->refPosXYConf = ref_pos_xy_conf;

        // refPosElConf
        auto ref_pos_el_conf_ptr = CARMAStreetsPlugin::create_store_shared<ElevationConfidence_t>(shared_ptrs);
        *ref_pos_el_conf_ptr = sdsm_json["ref_pos_el_conf"].asInt64();
        sdsm->refPosElConf = ref_pos_el_conf_ptr;

        // Creat initial pointers for detected object list and contained objects
        auto object_list = CARMAStreetsPlugin::create_store_shared<DetectedObjectList_t>(shared_ptrs);
        auto object_data = CARMAStreetsPlugin::create_store_shared<DetectedObjectData_t>(shared_ptrs);


        if(sdsm_json.isMember("objects") && sdsm_json["objects"].isArray()){
            Json::Value objectsJsonArr = sdsm_json["objects"];
            for(auto itr = objectsJsonArr.begin(); itr != objectsJsonArr.end(); itr++){

                auto common_data = CARMAStreetsPlugin::create_store_shared<DetectedObjectCommonData_t>(shared_ptrs);
            
                // Propegate common data
                common_data->objType         = (*itr)["detected_object_data"]["detected_object_common_data"]["obj_type"].asInt64();;
                common_data->objTypeCfd      = (*itr)["detected_object_data"]["detected_object_common_data"]["obj_type_cfd"].asInt64();
                common_data->objectID        = (*itr)["detected_object_data"]["detected_object_common_data"]["object_id"].asInt64();
                common_data->measurementTime = (*itr)["detected_object_data"]["detected_object_common_data"]["measurement_time"].asInt64();
                common_data->timeConfidence  = (*itr)["detected_object_data"]["detected_object_common_data"]["time_confidence"].asInt64();
                
                // pos (posOffsetXYZ)
                common_data->pos.offsetX = (*itr)["detected_object_data"]["detected_object_common_data"]["pos"]["offset_x"].asInt64();
                common_data->pos.offsetY = (*itr)["detected_object_data"]["detected_object_common_data"]["pos"]["offset_y"].asInt64();
                auto offset_z_ptr = CARMAStreetsPlugin::create_store_shared<ObjectDistance_t>(shared_ptrs);
                *offset_z_ptr = (*itr)["detected_object_data"]["detected_object_common_data"]["pos"]["offset_z"].asInt64();
                common_data->pos.offsetZ = offset_z_ptr;

                // posConfidence
                common_data->posConfidence.pos       = (*itr)["detected_object_data"]["detected_object_common_data"]["pos_confidence"]["pos"].asInt64();
                common_data->posConfidence.elevation = (*itr)["detected_object_data"]["detected_object_common_data"]["pos_confidence"]["elevation"].asInt64();

                // speed/speedConfidence
                common_data->speed           = (*itr)["detected_object_data"]["detected_object_common_data"]["speed"].asInt64();
                common_data->speedConfidence = (*itr)["detected_object_data"]["detected_object_common_data"]["speed_confidence"].asInt64();

                // speedZ/speedConfidenceZ
                auto speed_z_ptr = CARMAStreetsPlugin::create_store_shared<Speed_t>(shared_ptrs);
                *speed_z_ptr        = (*itr)["detected_object_data"]["detected_object_common_data"]["speed_z"].asInt64();
                common_data->speedZ = speed_z_ptr;
                auto speed_conf_z_ptr = CARMAStreetsPlugin::create_store_shared<SpeedConfidence_t>(shared_ptrs);
                *speed_conf_z_ptr   = (*itr)["detected_object_data"]["detected_object_common_data"]["speed_confidence_z"].asInt64();
                common_data->speedConfidenceZ = speed_conf_z_ptr;

                // heading/headingConf
                common_data->heading     = (*itr)["detected_object_data"]["detected_object_common_data"]["heading"].asInt64();
                common_data->headingConf = (*itr)["detected_object_data"]["detected_object_common_data"]["heading_conf"].asInt64();

                // accel4way
                auto accel_4_way_ptr = CARMAStreetsPlugin::create_store_shared<AccelerationSet4Way_t>(shared_ptrs);
                accel_4_way_ptr->Long   = (*itr)["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["long"].asInt64();
                accel_4_way_ptr->lat    = (*itr)["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["lat"].asInt64();
                accel_4_way_ptr->vert   = (*itr)["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["vert"].asInt64();
                accel_4_way_ptr->yaw    = (*itr)["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["yaw"].asInt64();
                common_data->accel4way = accel_4_way_ptr;

                // accCfd(X/Y/Z/Yaw)
                auto acc_cfd_x_ptr = CARMAStreetsPlugin::create_store_shared<AccelerationConfidence_t>(shared_ptrs);
                *acc_cfd_x_ptr = (*itr)["detected_object_data"]["detected_object_common_data"]["acc_cfd_x"].asInt64();
                common_data->accCfdX = acc_cfd_x_ptr;

                auto acc_cfd_y_ptr = CARMAStreetsPlugin::create_store_shared<AccelerationConfidence_t>(shared_ptrs);
                *acc_cfd_y_ptr = (*itr)["detected_object_data"]["detected_object_common_data"]["acc_cfd_y"].asInt64();
                common_data->accCfdY = acc_cfd_y_ptr;

                auto acc_cfd_z_ptr = CARMAStreetsPlugin::create_store_shared<AccelerationConfidence_t>(shared_ptrs);
                *acc_cfd_z_ptr = (*itr)["detected_object_data"]["detected_object_common_data"]["acc_cfd_z"].asInt64();
                common_data->accCfdZ = acc_cfd_z_ptr;

                auto acc_cfd_yaw_ptr = CARMAStreetsPlugin::create_store_shared<YawRateConfidence_t>(shared_ptrs);
                *acc_cfd_yaw_ptr = (*itr)["detected_object_data"]["detected_object_common_data"]["acc_cfd_yaw"].asInt64();
                common_data->accCfdYaw = acc_cfd_yaw_ptr;


                // Add common data to object data
                object_data->detObjCommon = *common_data;



                // Propegate optional data fields

                // detectedObjectOptionalData
                auto optional_data_ptr = CARMAStreetsPlugin::create_store_shared<DetectedObjectOptionalData_t>(shared_ptrs);

                // determine which optional data choice is being used if any
                // detVeh
                if((*itr)["detected_object_data"]["detected_object_optional_data"].isMember("detected_vehicle_data")){

                    // set presence val to veh
                    optional_data_ptr->present = DetectedObjectOptionalData_PR_detVeh;

                    // TODO: find a better way to convert/test lights val without calloc
                    // // lights
                    // auto lights_ptr = CARMAStreetsPlugin::create_store_shared<ExteriorLights_t>(shared_ptrs);
                    // auto lights = static_cast<int16_t>((*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["lights"].asInt());
                    // lights_ptr->buf = (uint8_t *)calloc(2, sizeof(uint8_t)); // TODO: find calloc alternative if possible, causes a memory leak
                    // lights_ptr->size = 2 * sizeof(uint8_t);
                    // lights_ptr->bits_unused = 0;
                    // lights_ptr->buf[1] = static_cast<int8_t>(lights);
                    // lights_ptr->buf[0] = (lights >> 8);

                    // optional_data_ptr->choice.detVeh.lights = lights_ptr;


                    // vehAttitude
                    auto attitude_ptr = CARMAStreetsPlugin::create_store_shared<Attitude_t>(shared_ptrs);
                    attitude_ptr->pitch = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_attitude"]["pitch"].asInt64();
                    attitude_ptr->roll = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_attitude"]["roll"].asInt64();
                    attitude_ptr->yaw = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_attitude"]["yaw"].asInt64();
                    optional_data_ptr->choice.detVeh.vehAttitude = attitude_ptr;

                    // vehAttitudeConfidence
                    auto attitude_conf_ptr = CARMAStreetsPlugin::create_store_shared<AttitudeConfidence_t>(shared_ptrs);
                    attitude_conf_ptr->pitchConfidence = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_attitude_confidence"]["pitch_confidence"].asInt64();
                    attitude_conf_ptr->rollConfidence = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_attitude_confidence"]["roll_confidence"].asInt64();
                    attitude_conf_ptr->yawConfidence = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_attitude_confidence"]["yaw_confidence"].asInt64();
                    optional_data_ptr->choice.detVeh.vehAttitudeConfidence = attitude_conf_ptr;

                    // vehAngVel
                    auto ang_vel_ptr = CARMAStreetsPlugin::create_store_shared<AngularVelocity_t>(shared_ptrs);
                    ang_vel_ptr->pitchRate = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_ang_vel"]["pitch_rate"].asInt64();
                    ang_vel_ptr->rollRate = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_ang_vel"]["roll_rate"].asInt64();
                    optional_data_ptr->choice.detVeh.vehAngVel = ang_vel_ptr;

                    // vehAngVelConfidence
                    auto ang_vel_conf_ptr = CARMAStreetsPlugin::create_store_shared<AngularVelocityConfidence_t>(shared_ptrs);
                    auto pitch_conf_ptr = CARMAStreetsPlugin::create_store_shared<PitchRateConfidence_t>(shared_ptrs);
                    *pitch_conf_ptr = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_ang_vel_confidence"]["pitch_rate_confidence"].asInt64();
                    ang_vel_conf_ptr->pitchRateConfidence = pitch_conf_ptr;
                    auto roll_conf_ptr = CARMAStreetsPlugin::create_store_shared<RollRateConfidence_t>(shared_ptrs);
                    *roll_conf_ptr = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["veh_ang_vel_confidence"]["roll_rate_confidence"].asInt64();
                    ang_vel_conf_ptr->rollRateConfidence = roll_conf_ptr;
                    optional_data_ptr->choice.detVeh.vehAngVelConfidence = ang_vel_conf_ptr;

                    // size (VehicleSize)
                    auto size_ptr = CARMAStreetsPlugin::create_store_shared<VehicleSize_t>(shared_ptrs);
                    size_ptr->width = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["size"]["width"].asInt64();
                    size_ptr->length = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["size"]["length"].asInt64();
                    optional_data_ptr->choice.detVeh.size = size_ptr;

                    // height
                    auto height_ptr = CARMAStreetsPlugin::create_store_shared<VehicleHeight_t>(shared_ptrs);
                    *height_ptr = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["height"].asInt64();
                    optional_data_ptr->choice.detVeh.height = height_ptr;

                    // vehcleSizeConfidence
                    auto size_conf_ptr = CARMAStreetsPlugin::create_store_shared<VehicleSizeConfidence_t>(shared_ptrs);
                    size_conf_ptr->vehicleWidthConfidence = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["vehicle_size_confidence"]["vehicle_width_confidence"].asInt64();
                    size_conf_ptr->vehicleLengthConfidence = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["vehicle_size_confidence"]["vehicle_length_confidence"].asInt64();
                    auto height_conf_ptr = CARMAStreetsPlugin::create_store_shared<SizeValueConfidence_t>(shared_ptrs);
                    *height_conf_ptr = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["vehicle_size_confidence"]["vehicle_height_confidence"].asInt64();
                    size_conf_ptr->vehicleHeightConfidence = height_conf_ptr;
                    optional_data_ptr->choice.detVeh.vehicleSizeConfidence = size_conf_ptr;

                    // vehicleClass
                    auto veh_class_ptr = CARMAStreetsPlugin::create_store_shared<BasicVehicleClass_t>(shared_ptrs);
                    *veh_class_ptr = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["vehicle_class"].asInt64();
                    optional_data_ptr->choice.detVeh.vehicleClass = veh_class_ptr;

                    // vehClassConf
                    auto veh_class_conf_ptr = CARMAStreetsPlugin::create_store_shared<ClassificationConfidence_t>(shared_ptrs);
                    *veh_class_conf_ptr = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vehicle_data"]["vehicle_class_conf"].asInt64();
                    optional_data_ptr->choice.detVeh.classConf = veh_class_conf_ptr;
                }

                // detVRU
                else if((*itr)["detected_object_data"]["detected_object_optional_data"].isMember("detected_vru_data")){

                    optional_data_ptr->present = DetectedObjectOptionalData_PR_detVRU;

                    // basicType
                    auto basic_type_ptr = CARMAStreetsPlugin::create_store_shared<PersonalDeviceUserType_t>(shared_ptrs);
                    *basic_type_ptr = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vru_data"]["basic_type"].asInt64();
                    optional_data_ptr->choice.detVRU.basicType = basic_type_ptr;

                    // propulsion choice struct (check to see if propulsion exists)
                    auto propulsion_ptr = CARMAStreetsPlugin::create_store_shared<PropelledInformation_t>(shared_ptrs);

                    if((*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vru_data"]["propulsion"].isMember("human")){
                        propulsion_ptr->present = PropelledInformation_PR_human;
                        propulsion_ptr->choice.human = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vru_data"]["propulsion"]["human"].asInt64();
                    }
                    else if((*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vru_data"]["propulsion"].isMember("animal")){
                        propulsion_ptr->present = PropelledInformation_PR_animal;
                        propulsion_ptr->choice.animal = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vru_data"]["propulsion"]["animal"].asInt64();
                    }
                    else if((*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vru_data"]["propulsion"].isMember("motor")){
                        propulsion_ptr->present = PropelledInformation_PR_motor;
                        propulsion_ptr->choice.motor = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vru_data"]["propulsion"]["motor"].asInt64();
                    }
                    else{
                        propulsion_ptr->present = PropelledInformation_PR_NOTHING;
                        std::cout << "Value was nothing" << std::endl;
                    }
                    optional_data_ptr->choice.detVRU.propulsion = propulsion_ptr;

                    // attachement
                    auto attachment_ptr = CARMAStreetsPlugin::create_store_shared<Attachment_t>(shared_ptrs);
                    *attachment_ptr = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vru_data"]["attachment"].asInt64();
                    optional_data_ptr->choice.detVRU.attachment = attachment_ptr;

                    // radius
                    auto radius_ptr = CARMAStreetsPlugin::create_store_shared<AttachmentRadius_t>(shared_ptrs);
                    *radius_ptr = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_vru_data"]["radius"].asInt64();
                    optional_data_ptr->choice.detVRU.radius = radius_ptr;
                }


                // detObst
                else if((*itr)["detected_object_data"]["detected_object_optional_data"].isMember("detected_obstacle_data")){
                    optional_data_ptr->present = DetectedObjectOptionalData_PR_detObst;

                    ObstacleSize obst_size;
                    obst_size.width = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_obstacle_data"]["obst_size"]["width"].asInt64();
                    obst_size.length = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_obstacle_data"]["obst_size"]["length"].asInt64();
                    auto obst_height_ptr = CARMAStreetsPlugin::create_store_shared<SizeValue_t>(shared_ptrs);
                    *obst_height_ptr = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_obstacle_data"]["obst_size"]["height"].asInt64();
                    obst_size.height = obst_height_ptr;
                    optional_data_ptr->choice.detObst.obstSize = obst_size;

                    ObstacleSizeConfidence obst_size_conf;
                    obst_size_conf.widthConfidence = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_obstacle_data"]["obst_size_confidence"]["width_confidence"].asInt64();
                    obst_size_conf.lengthConfidence = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_obstacle_data"]["obst_size_confidence"]["length_confidence"].asInt64();
                    auto obst_height_conf_ptr = CARMAStreetsPlugin::create_store_shared<SizeValueConfidence_t>(shared_ptrs);
                    *obst_height_conf_ptr = (*itr)["detected_object_data"]["detected_object_optional_data"]["detected_obstacle_data"]["obst_size_confidence"]["height_confidence"].asInt64();
                    obst_size_conf.heightConfidence = obst_height_conf_ptr;
                    optional_data_ptr->choice.detObst.obstSizeConfidence = obst_size_conf;
                }


                // Add optional data to object data
                object_data->detObjOptData = optional_data_ptr;


            }
        }

        // Append the object to the detected objects list
        asn_sequence_add(&object_list->list.array, object_data);

        // Set the data to the ASN.1 C struct
        sdsm->objects = *object_list;


    }


    void JsonToJ3224SDSMConverter::encodeSDSM(const std::shared_ptr<SensorDataSharingMessage_t> &sdsmPtr, tmx::messages::SdsmEncodedMessage &encodedSDSM) const
    {
        auto _sdsmMessage = new tmx::messages::SdsmMessage(sdsmPtr);
        tmx::messages::MessageFrameMessage frame(_sdsmMessage->get_j2735_data());
        encodedSDSM.set_data(tmx::messages::TmxJ2735EncodedMessage<SensorDataSharingMessage>::encode_j2735_message<tmx::messages::codec::uper<tmx::messages::MessageFrameMessage>>(frame));
        asn_fprint(stdout, &asn_DEF_MessageFrame, frame.get_j2735_data().get());
        free(frame.get_j2735_data().get());
    }


}