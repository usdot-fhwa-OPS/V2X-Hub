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
    void JsonToJ3224SDSMConverter::convertJsonToSDSM(const Json::Value &sdsm_json, const std::shared_ptr<SensorDataSharingMessage_t> &sdsm) const {
        ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SensorDataSharingMessage, sdsm.get());
        // Message Count
        sdsm->msgCnt = sdsm_json["msg_cnt"].asInt64();
        // Source ID (Expecting format "rsu_<4-digit-number>")
        std::string id_data = sdsm_json["source_id"].asString().substr(4);
	    auto *tempID = (TemporaryID_t *)calloc(1, sizeof(TemporaryID_t));
        OCTET_STRING_fromString(tempID, id_data.c_str());
        sdsm->sourceID = *tempID;
        free(tempID);

        // Equipment Type
        sdsm->equipmentType = sdsm_json["equipment_type"].asInt64();

        // SDSM DateTime timestamp
        DDateTime_t sDSMTimeStamp;
        // Optional Year
        if ( sdsm_json["sdsm_time_stamp"].isMember("year") ) {
            auto year = (DYear_t*) calloc(1, sizeof(DYear_t));
            *year = sdsm_json["sdsm_time_stamp"]["year"].asInt64();
            sDSMTimeStamp.year = year;
        }
        // Optional Month
        if ( sdsm_json["sdsm_time_stamp"].isMember("month") ) {
            auto month = (DMonth_t*) calloc(1, sizeof(DMonth_t));
            *month = sdsm_json["sdsm_time_stamp"]["month"].asInt64();
            sDSMTimeStamp.month = month;
        }
        // Optional Day
        if ( sdsm_json["sdsm_time_stamp"].isMember("day") ) {
            auto day = (DDay_t*) calloc(1, sizeof(DDay_t));
            *day = sdsm_json["sdsm_time_stamp"]["day"].asInt64();
            sDSMTimeStamp.day = day;
        }
        // Optional Hour
        if ( sdsm_json["sdsm_time_stamp"].isMember("hour") ) {
            auto hour = (DHour_t*) calloc(1, sizeof(DHour_t));
            *hour = sdsm_json["sdsm_time_stamp"]["hour"].asInt64();
            sDSMTimeStamp.hour = hour;
        }
        // Optional Minute
        if ( sdsm_json["sdsm_time_stamp"].isMember("minute") ) {
            auto minute = (DMinute_t*) calloc(1, sizeof(DMinute_t));
            *minute = sdsm_json["sdsm_time_stamp"]["minute"].asInt64();
            sDSMTimeStamp.minute = minute;
        }
        // Optional Second
        if ( sdsm_json["sdsm_time_stamp"].isMember("second") ) {
            auto second = (DSecond_t*) calloc(1, sizeof(DSecond_t));
            *second = sdsm_json["sdsm_time_stamp"]["second"].asInt64();
            sDSMTimeStamp.second = second;
        }
        // Optional Offset
        if ( sdsm_json["sdsm_time_stamp"].isMember("offset") ) {
            auto offset = (DOffset_t*) calloc( 1, sizeof(DOffset_t));
            *offset = sdsm_json["sdsm_time_stamp"]["offset"].asInt64();
            sDSMTimeStamp.offset = offset;
        }
        sdsm->sDSMTimeStamp = sDSMTimeStamp;
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
                // Optional acceleration confidence X 
                if( (*itr)["detected_object_data"]["detected_object_common_data"].isMember("acc_cfd_x") ) {
                    auto acc_cfd_x = (AccelerationConfidence_t*)calloc(1, sizeof(AccelerationConfidence_t));
                    *acc_cfd_x = (*itr)["detected_object_data"]["detected_object_common_data"]["acc_cfd_x"].asInt64();
                    objectData->detObjCommon.accCfdX = acc_cfd_x;
                }
                // Optional acceleration confidence Y
                if( (*itr)["detected_object_data"]["detected_object_common_data"].isMember("acc_cfd_y") ) {
                    auto acc_cfd_y = (AccelerationConfidence_t*)calloc(1, sizeof(AccelerationConfidence_t));
                    *acc_cfd_y = (*itr)["detected_object_data"]["detected_object_common_data"]["acc_cfd_y"].asInt64();
                    objectData->detObjCommon.accCfdY = acc_cfd_y;
                }
                // Optional acceleration confidence Z 
                if( (*itr)["detected_object_data"]["detected_object_common_data"].isMember("acc_cfd_z") ) {
                    auto acc_cfd_z = (AccelerationConfidence_t*)calloc(1, sizeof(AccelerationConfidence_t));
                    *acc_cfd_z = (*itr)["detected_object_data"]["detected_object_common_data"]["acc_cfd_z"].asInt64();
                    objectData->detObjCommon.accCfdZ = acc_cfd_z;
                }
                // Optional acceleration confidence Yaw 
                if( (*itr)["detected_object_data"]["detected_object_common_data"].isMember("acc_cfd_yaw") ) {
                    auto acc_cfd_yaw = (AccelerationConfidence_t*)calloc(1, sizeof(YawRateConfidence_t));
                    *acc_cfd_yaw = (*itr)["detected_object_data"]["detected_object_common_data"]["acc_cfd_yaw"].asInt64();
                    objectData->detObjCommon.accCfdYaw = acc_cfd_yaw;
                }
                // Object Optional Data
                if ((*itr)["detected_object_data"].isMember("detected_object_optional_data") ){
                    auto optional_data = (DetectedObjectOptionalData_t*)calloc(1, sizeof(DetectedObjectOptionalData_t));
                    populateOptionalData((*itr)["detected_object_data"]["detected_object_optional_data"], optional_data);
                    objectData->detObjOptData = optional_data;
                }
	            ASN_SEQUENCE_ADD(&objects->list.array, objectData);
            }
            sdsm->objects = *objects;
            free(objects);

        }
        asn_fprint(stdout, &asn_DEF_SensorDataSharingMessage, sdsm.get());
    }


    void JsonToJ3224SDSMConverter::encodeSDSM(const std::shared_ptr<SensorDataSharingMessage_t> &sdsmPtr, tmx::messages::SdsmEncodedMessage &encodedSDSM) const
    {
        auto _sdsmMessage = new tmx::messages::SdsmMessage(sdsmPtr);
        tmx::messages::MessageFrameMessage frame(_sdsmMessage->get_j2735_data());
        encodedSDSM.set_data(tmx::messages::TmxJ2735EncodedMessage<SensorDataSharingMessage>::encode_j2735_message<tmx::messages::codec::uper<tmx::messages::MessageFrameMessage>>(frame));
        asn_fprint(stdout, &asn_DEF_MessageFrame, frame.get_j2735_data().get());
        free(frame.get_j2735_data().get());
        delete(_sdsmMessage);
    }

    void JsonToJ3224SDSMConverter::populateOptionalData(const Json::Value &optional_data_json, DetectedObjectOptionalData_t *optional_data) const {
        if ( optional_data_json.isMember("detected_vehicle_data") ) {
            optional_data->present = DetectedObjectOptionalData_PR_detVeh;
            // Optional Vehicle Attitude 
            if (optional_data_json["detected_vehicle_data"].isMember("veh_attitude")) {
                auto attitude = (Attitude_t*) calloc( 1, sizeof(Attitude_t));
                attitude->pitch = optional_data_json["detected_vehicle_data"]["veh_attitude"]["pitch"].asInt64();
                attitude->roll = optional_data_json["detected_vehicle_data"]["veh_attitude"]["roll"].asInt64();
                attitude->yaw = optional_data_json["detected_vehicle_data"]["veh_attitude"]["yaw"].asInt64();
                optional_data->choice.detVeh.vehAttitude = attitude;
            }
            // Optional Vehicle Attitude Confidence 
            if (optional_data_json["detected_vehicle_data"].isMember("veh_attitude_confidence")) {
                auto attitude_confidence = (AttitudeConfidence_t*) calloc( 1, sizeof(AttitudeConfidence_t));
                attitude_confidence->pitchConfidence = optional_data_json["detected_vehicle_data"]["veh_attitude_confidence"]["pitch_confidence"].asInt64();
                attitude_confidence->rollConfidence = optional_data_json["detected_vehicle_data"]["veh_attitude_confidence"]["roll_confidence"].asInt64();
                attitude_confidence->yawConfidence = optional_data_json["detected_vehicle_data"]["veh_attitude_confidence"]["yaw_confidence"].asInt64();
                optional_data->choice.detVeh.vehAttitudeConfidence = attitude_confidence;
            }
            // Optional Vehicle Angular Velocity
            if (optional_data_json["detected_vehicle_data"].isMember("veh_ang_vel")) {
                auto angular_velocity = (AngularVelocity_t*) calloc( 1, sizeof(AngularVelocity_t));
                angular_velocity->pitchRate = optional_data_json["detected_vehicle_data"]["veh_ang_vel"]["pitch_rate"].asInt64();
                angular_velocity->rollRate = optional_data_json["detected_vehicle_data"]["veh_ang_vel"]["roll_rate"].asInt64();
                optional_data->choice.detVeh.vehAngVel = angular_velocity;
            }
            // Optional Vehicle Angular Velocity
            if (optional_data_json["detected_vehicle_data"].isMember("veh_ang_vel_confidence")) {
                auto angular_velocity_confidence = (AngularVelocityConfidence_t*) calloc( 1, sizeof(AngularVelocityConfidence_t));
                if (optional_data_json["detected_vehicle_data"]["veh_ang_vel_confidence"].isMember("pitch_rate_confidence")) {
                    auto pitch_rate_confidence = (PitchRateConfidence_t*) calloc(1, sizeof(PitchRateConfidence_t));
                    *pitch_rate_confidence = optional_data_json["detected_vehicle_data"]["veh_ang_vel_confidence"]["pitch_rate_confidence"].asInt64();
                    angular_velocity_confidence->pitchRateConfidence = pitch_rate_confidence;
                }
                if (optional_data_json["detected_vehicle_data"]["veh_ang_vel_confidence"].isMember("roll_rate_confidence")) {
                    auto roll_rate_confidence = (RollRateConfidence_t*) calloc(1, sizeof(RollRateConfidence_t));
                    *roll_rate_confidence = optional_data_json["detected_vehicle_data"]["veh_ang_vel_confidence"]["roll_rate_confidence"].asInt64();
                    angular_velocity_confidence->rollRateConfidence = roll_rate_confidence;
                }
                optional_data->choice.detVeh.vehAngVelConfidence = angular_velocity_confidence;
            }
            // Optional Vehicle Size
            if (optional_data_json["detected_vehicle_data"].isMember("size")) {
                auto veh_size = (VehicleSize_t*) calloc( 1, sizeof(VehicleSize_t));
                veh_size->length = optional_data_json["detected_vehicle_data"]["size"]["length"].asInt64();
                veh_size->width = optional_data_json["detected_vehicle_data"]["size"]["width"].asInt64();
                optional_data->choice.detVeh.size = veh_size;
            }
            // Optional Vehicle Height
            if (optional_data_json["detected_vehicle_data"].isMember("height")) {
                auto veh_height = (VehicleHeight_t*)calloc(1, sizeof(VehicleHeight_t));
                *veh_height = optional_data_json["detected_vehicle_data"]["height"].asInt64();
                optional_data->choice.detVeh.height = veh_height;
            }
            // Optional Vehicle Size Confidence
            if (optional_data_json["detected_vehicle_data"].isMember("vehicle_size_confidence"))  {
                auto veh_size_confidence = (VehicleSizeConfidence_t*)calloc(1, sizeof(VehicleSizeConfidence_t));
                veh_size_confidence->vehicleLengthConfidence = optional_data_json["detected_vehicle_data"]["vehicle_size_confidence"]["vehicle_length_confidence"].asInt64();
                veh_size_confidence->vehicleWidthConfidence = optional_data_json["detected_vehicle_data"]["vehicle_size_confidence"]["vehicle_width_confidence"].asInt64();
                if (optional_data_json["detected_vehicle_data"]["vehicle_size_confidence"].isMember("vehicle_height_confidence")) {
                    auto veh_height_confidence = (SizeValueConfidence_t*)calloc(1, sizeof(SizeValueConfidence_t));
                    *veh_height_confidence = optional_data_json["detected_vehicle_data"]["vehicle_size_confidence"]["vehicle_height_confidence"].asInt64();
                    veh_size_confidence->vehicleHeightConfidence = veh_height_confidence;
                }
                optional_data->choice.detVeh.vehicleSizeConfidence = veh_size_confidence;
            }
            // Optional Vehicle Class
            if (optional_data_json["detected_vehicle_data"].isMember("vehicle_class"))  {
                auto vehicle_class = (BasicVehicleClass_t*)calloc(1, sizeof(BasicVehicleClass_t));
                *vehicle_class = optional_data_json["detected_vehicle_data"]["vehicle_class"].asInt64();
                optional_data->choice.detVeh.vehicleClass = vehicle_class;
            }
            if (optional_data_json["detected_vehicle_data"].isMember("vehicle_class_conf"))  {
                auto vehicle_class_conf = (ClassificationConfidence_t*)calloc(1, sizeof(ClassificationConfidence_t));
                *vehicle_class_conf = optional_data_json["detected_vehicle_data"]["vehicle_class_conf"].asInt64();
                optional_data->choice.detVeh.classConf = vehicle_class_conf;
            }

        }
        else if (optional_data_json.isMember("detected_vru_data") ) {
            optional_data->present = DetectedObjectOptionalData_PR_detVRU;
            // Optional VRU Basic Type
            if ( optional_data_json["detected_vru_data"].isMember("basic_type")) {
                auto basic_type = (PersonalDeviceUserType_t*) calloc(1, sizeof(PersonalDeviceUserType_t));
                *basic_type = optional_data_json["detected_vru_data"]["basic_type"].asInt64();
                optional_data->choice.detVRU.basicType = basic_type;
            }
            // Optional propulsion information
            if (optional_data_json["detected_vru_data"].isMember("propulsion")) {
                auto propelled_info = (PropelledInformation_t*) calloc(1, sizeof(PropelledInformation_t));
                if (optional_data_json["detected_vru_data"]["propulsion"].isMember("human") ) {
                    propelled_info->present = PropelledInformation_PR_human;
                    propelled_info->choice.human =  optional_data_json["detected_vru_data"]["propulsion"]["human"].asInt64();
                }
                else if (optional_data_json["detected_vru_data"]["propulsion"].isMember("animal") ) {
                    propelled_info->present = PropelledInformation_PR_animal;
                    propelled_info->choice.animal =  optional_data_json["detected_vru_data"]["propulsion"]["animal"].asInt64();
                }
                else if (optional_data_json["detected_vru_data"]["propulsion"].isMember("motor") ) {
                    propelled_info->present = PropelledInformation_PR_motor;
                    propelled_info->choice.motor =  optional_data_json["detected_vru_data"]["propulsion"]["motor"].asInt64();
                }
                optional_data->choice.detVRU.propulsion = propelled_info;
            }
            // Optional VRU Attachment
            if ( optional_data_json["detected_vru_data"].isMember("attachment")) {
                auto attachment = (Attachment_t*) calloc(1, sizeof(Attachment_t));
                *attachment = optional_data_json["detected_vru_data"]["attachment"].asInt64();
                optional_data->choice.detVRU.attachment = attachment;
            }
            // Optional VRU Attachment Radius
            if ( optional_data_json["detected_vru_data"].isMember("radius")) {
                auto radius = (AttachmentRadius_t*) calloc(1, sizeof(AttachmentRadius_t));
                *radius = optional_data_json["detected_vru_data"]["radius"].asInt64();
                optional_data->choice.detVRU.radius = radius;
            }


        }
    }

}