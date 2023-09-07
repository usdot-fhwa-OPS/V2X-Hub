#include "J3224ToSDSMJsonConverter.h"

namespace CARMAStreetsPlugin
{

    void J3224ToSDSMJsonConverter::convertJ3224ToSDSMJSON(const std::shared_ptr<SensorDataSharingMessage> sdsmMsgPtr, Json::Value &SDSMDataJson) const
    {
        // Define shared pointers for optional data
        std::vector<std::shared_ptr<void>> shared_ptrs;

        // SDSM header data
        SDSMDataJson["msg_cnt"] = sdsmMsgPtr->msgCnt;

		auto id_len = sdsmMsgPtr->sourceID.size;
		unsigned long id_num = 0;
		for(auto i = 0; i < id_len; i++)
		{			
			 id_num = (id_num << 8) | sdsmMsgPtr->sourceID.buf[i];
		}
		std::stringstream id_fill_ss;
		id_fill_ss << std::setfill('0') << std::setw(8) <<std::hex << id_num;
		SDSMDataJson["source_id"]  = id_fill_ss.str();

        SDSMDataJson["equipment_type"] = sdsmMsgPtr->equipmentType;

        Json::Value sDSMTimeStamp;
        sDSMTimeStamp["year"] = *sdsmMsgPtr->sDSMTimeStamp.year;
        sDSMTimeStamp["month"] = *sdsmMsgPtr->sDSMTimeStamp.month;
        sDSMTimeStamp["day"] = *sdsmMsgPtr->sDSMTimeStamp.day;
        sDSMTimeStamp["hour"] = *sdsmMsgPtr->sDSMTimeStamp.hour;
        sDSMTimeStamp["minute"] = *sdsmMsgPtr->sDSMTimeStamp.minute;
        sDSMTimeStamp["second"] = *sdsmMsgPtr->sDSMTimeStamp.second;
        sDSMTimeStamp["offset"] = *sdsmMsgPtr->sDSMTimeStamp.offset;
        SDSMDataJson["sdsm_time_stamp"] = sDSMTimeStamp;

        Json::Value refPos;
        refPos["lat"] = sdsmMsgPtr->refPos.lat;
        refPos["Long"] = sdsmMsgPtr->refPos.Long;
        refPos["elevation"] = *sdsmMsgPtr->refPos.elevation;
        SDSMDataJson["ref_pos"] = refPos;

        Json::Value refPosXYConf;
		refPosXYConf["semi_major"] = sdsmMsgPtr->refPosXYConf.semiMajor;
		refPosXYConf["semi_minor"] = sdsmMsgPtr->refPosXYConf.semiMinor;
		refPosXYConf["orientation"] = sdsmMsgPtr->refPosXYConf.orientation;
        SDSMDataJson["ref_pos_xy_conf"] = refPosXYConf;

        SDSMDataJson["ref_pos_el_conf"] = *sdsmMsgPtr->refPosElConf;
        
       
        // Create list to append detected objects to
        const DetectedObjectList det_obj_list = sdsmMsgPtr->objects;

        if(det_obj_list.list.array != nullptr){

            Json::Value objectsJson;

            for(size_t i = 0; i < det_obj_list.list.count; i++){

                Json::Value detectedObjectJson;
                auto det_object = det_obj_list.list.array[i];

                // Detected object common data
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["obj_type"] = det_object->detObjCommon.objType;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["obj_type_cfd"] = det_object->detObjCommon.objTypeCfd;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["object_id"] = det_object->detObjCommon.objectID;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["measurement_time"] = det_object->detObjCommon.measurementTime;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["time_confidence"] = det_object->detObjCommon.timeConfidence;

                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["pos"]["offset_x"] = det_object->detObjCommon.pos.offsetX;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["pos"]["offset_y"] = det_object->detObjCommon.pos.offsetY;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["pos"]["offset_z"] = *det_object->detObjCommon.pos.offsetZ;
                
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["pos_confidence"]["pos"] = det_object->detObjCommon.posConfidence.pos;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["pos_confidence"]["elevation"] = det_object->detObjCommon.posConfidence.elevation;

                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["speed"] = det_object->detObjCommon.speed;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["speed_confidence"] = det_object->detObjCommon.speedConfidence;

                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["heading"] = det_object->detObjCommon.heading;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["heading_conf"] = det_object->detObjCommon.headingConf;       

                // Optional common data
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["speed_z"] = *det_object->detObjCommon.speedZ;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["speed_confidence_z"] = *det_object->detObjCommon.speedConfidenceZ;

                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["Long"] = det_object->detObjCommon.accel4way->Long;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["lat"] = det_object->detObjCommon.accel4way->lat;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["vert"] = det_object->detObjCommon.accel4way->vert;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["accel_4_way"]["yaw"] = det_object->detObjCommon.accel4way->yaw;

                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["acc_cfd_x"] = *det_object->detObjCommon.accCfdX;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["acc_cfd_y"] = *det_object->detObjCommon.accCfdY;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["acc_cfd_z"] = *det_object->detObjCommon.accCfdZ;
                detectedObjectJson["detected_object_data"]["detected_object_common_data"]["acc_cfd_yaw"] = *det_object->detObjCommon.accCfdYaw;

                // Detected object optional data for vehicles, VRUs, obstacles
                if(det_object->detObjOptData != nullptr){
                    Json::Value optionalDataJson;
                    
                    switch(det_object->detObjOptData->present)
                    {
                        case DetectedObjectOptionalData_PR_NOTHING:
                            break;

                        case DetectedObjectOptionalData_PR_detVeh:

                            // TODO: find a better way to convert bit string data (JsonTo2735SpatConverter)
                            optionalDataJson["detected_vehicle_data"]["lights"] = std::to_string(*det_object->detObjOptData->choice.detVeh.lights->buf);
                            optionalDataJson["detected_vehicle_data"]["veh_attitude"]["pitch"] = det_object->detObjOptData->choice.detVeh.vehAttitude->pitch;
                            optionalDataJson["detected_vehicle_data"]["veh_attitude"]["roll"] = det_object->detObjOptData->choice.detVeh.vehAttitude->roll;
                            optionalDataJson["detected_vehicle_data"]["veh_attitude"]["yaw"] = det_object->detObjOptData->choice.detVeh.vehAttitude->yaw;

                            optionalDataJson["detected_vehicle_data"]["veh_attitude_confidence"]["pitch_confidence"] = det_object->detObjOptData->choice.detVeh.vehAttitudeConfidence->pitchConfidence;
                            optionalDataJson["detected_vehicle_data"]["veh_attitude_confidence"]["roll_confidence"] = det_object->detObjOptData->choice.detVeh.vehAttitudeConfidence->rollConfidence;
                            optionalDataJson["detected_vehicle_data"]["veh_attitude_confidence"]["yaw_confidence"] = det_object->detObjOptData->choice.detVeh.vehAttitudeConfidence->yawConfidence;

                            optionalDataJson["detected_vehicle_data"]["veh_ang_vel"]["pitch_rate"] = det_object->detObjOptData->choice.detVeh.vehAngVel->pitchRate;
                            optionalDataJson["detected_vehicle_data"]["veh_ang_vel"]["roll_rate"] = det_object->detObjOptData->choice.detVeh.vehAngVel->rollRate;

                            optionalDataJson["detected_vehicle_data"]["veh_ang_vel_confidence"]["pitch_rate_confidence"] = *det_object->detObjOptData->choice.detVeh.vehAngVelConfidence->pitchRateConfidence;
                            optionalDataJson["detected_vehicle_data"]["veh_ang_vel_confidence"]["roll_rate_confidence"] = *det_object->detObjOptData->choice.detVeh.vehAngVelConfidence->rollRateConfidence;

                            optionalDataJson["detected_vehicle_data"]["size"]["width"] = det_object->detObjOptData->choice.detVeh.size->width;
                            optionalDataJson["detected_vehicle_data"]["size"]["length"] = det_object->detObjOptData->choice.detVeh.size->length;
                            optionalDataJson["detected_vehicle_data"]["height"] = *det_object->detObjOptData->choice.detVeh.height;

                            optionalDataJson["detected_vehicle_data"]["vehicle_size_confidence"]["vehicle_width_confidence"] = det_object->detObjOptData->choice.detVeh.vehicleSizeConfidence->vehicleWidthConfidence;
                            optionalDataJson["detected_vehicle_data"]["vehicle_size_confidence"]["vehicle_length_confidence"] = det_object->detObjOptData->choice.detVeh.vehicleSizeConfidence->vehicleLengthConfidence;
                            optionalDataJson["detected_vehicle_data"]["vehicle_size_confidence"]["vehicle_height_confidence"] = *det_object->detObjOptData->choice.detVeh.vehicleSizeConfidence->vehicleHeightConfidence;

                            optionalDataJson["detected_vehicle_data"]["vehicle_class"] = *det_object->detObjOptData->choice.detVeh.vehicleClass;
                            optionalDataJson["detected_vehicle_data"]["vehicle_class_conf"] = *det_object->detObjOptData->choice.detVeh.classConf;
                            break;

                        case DetectedObjectOptionalData_PR_detVRU:
                            optionalDataJson["detected_vru_data"]["basic_type"] = *det_object->detObjOptData->choice.detVRU.basicType;

                            switch(det_object->detObjOptData->choice.detVRU.propulsion->present)
                            {
                                case PropelledInformation_PR_NOTHING:
                                    break;
                                case PropelledInformation_PR_human:
                                    optionalDataJson["detected_vru_data"]["propulsion"]["human"] = det_object->detObjOptData->choice.detVRU.propulsion->choice.human;
                                    break;
                                case PropelledInformation_PR_animal:
                                    optionalDataJson["detected_vru_data"]["propulsion"]["human"] = det_object->detObjOptData->choice.detVRU.propulsion->choice.animal;
                                    break;
                                case PropelledInformation_PR_motor:
                                    optionalDataJson["detected_vru_data"]["propulsion"]["human"] = det_object->detObjOptData->choice.detVRU.propulsion->choice.motor;
                                    break;
                            }

                            optionalDataJson["detected_vru_data"]["attachment"] = *det_object->detObjOptData->choice.detVRU.attachment;
                            optionalDataJson["detected_vru_data"]["radius"] = *det_object->detObjOptData->choice.detVRU.radius;
                            break;

                        case DetectedObjectOptionalData_PR_detObst:
                            optionalDataJson["detected_obstacle_data"]["obst_size"]["width"] = det_object->detObjOptData->choice.detObst.obstSize.width;
                            optionalDataJson["detected_obstacle_data"]["obst_size"]["length"] = det_object->detObjOptData->choice.detObst.obstSize.length;
                            optionalDataJson["detected_obstacle_data"]["obst_size"]["height"] = *det_object->detObjOptData->choice.detObst.obstSize.height; // optional

                            optionalDataJson["detected_obstacle_data"]["obst_size_confidence"]["width_confidence"] = det_object->detObjOptData->choice.detObst.obstSizeConfidence.widthConfidence;
                            optionalDataJson["detected_obstacle_data"]["obst_size_confidence"]["length_confidence"] = det_object->detObjOptData->choice.detObst.obstSizeConfidence.lengthConfidence;
                            optionalDataJson["detected_obstacle_data"]["obst_size_confidence"]["height_confidence"] = *det_object->detObjOptData->choice.detObst.obstSizeConfidence.heightConfidence;
                            break;
                    }
                    detectedObjectJson["detected_object_data"]["detected_object_optional_data"] = optionalDataJson;
                }

                // Add the generated object jsons to the detected object list
                SDSMDataJson["objects"].append(detectedObjectJson);

            }
        }

        //std::cout << SDSMDataJson.toStyledString() << std::endl;
    }
}