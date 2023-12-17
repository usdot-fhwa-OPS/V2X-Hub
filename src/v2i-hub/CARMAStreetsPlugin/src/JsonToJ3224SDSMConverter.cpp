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


}