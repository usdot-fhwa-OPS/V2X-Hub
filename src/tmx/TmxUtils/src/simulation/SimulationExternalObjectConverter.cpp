#include <simulation/SimulationExternalObjectConverter.h>

namespace tmx::utils::sim
{
    std::string SimulationExternalObjectConverter::simExternalObjToJsonStr(tmx::messages::simulation::ExternalObject &simExternalObj)
    {
        Json::Value root;
        Json::Value metadata;
        metadata["is_simulation"] = simExternalObj.get_MetadataIsSimulation();
        metadata["datum"] = simExternalObj.get_MetadataDatum();
        metadata["proj_string"] = simExternalObj.get_MetadataProjString();
        metadata["sensor_x"] = simExternalObj.get_MetadataSensorX();
        metadata["sensor_y"] = simExternalObj.get_MetadataSensorY();
        metadata["sensor_z"] = simExternalObj.get_MetadataSensorZ();
        metadata["infrastructure_id"] = simExternalObj.get_MetadataInfrastructureId();
        metadata["sensor_id"] = simExternalObj.get_MetadataSensorId();
        root["metadata"] = metadata;

        Json::Value header;
        header["seq"] = simExternalObj.get_HeaderSeq();
        header["stamp"]["secs"] = simExternalObj.get_HeaderStampSecs();
        header["stamp"]["nsecs"] = simExternalObj.get_HeaderStampNSecs();
        root["header"] = header;

        root["id"] = simExternalObj.get_Id();
        if (simExternalObj.get_PresenceVector() != tmx::messages::simulation::PRESENCE_VECTOR_TYPES::UNAVAILABLE)
        {
            root["presence_vector"] = simExternalObj.get_PresenceVector();
        }

        Json::Value pose;
        pose["pose"]["position"]["x"] = simExternalObj.get_PosePosePositionX();
        pose["pose"]["position"]["y"] = simExternalObj.get_PosePosePositionY();
        pose["pose"]["position"]["z"] = simExternalObj.get_PosePosePositionZ();
        pose["pose"]["orientation"]["x"] = simExternalObj.get_PosePoseOrientationX();
        pose["pose"]["orientation"]["y"] = simExternalObj.get_PosePoseOrientationY();
        pose["pose"]["orientation"]["z"] = simExternalObj.get_PosePoseOrientationZ();
        pose["pose"]["orientation"]["w"] = simExternalObj.get_PosePoseOrientationW();
        auto pCovarianceV = simExternalObj.get_PoseCovariance();
        std::for_each(pCovarianceV.begin(), pCovarianceV.end(), [&pose](const auto &item)
                      { pose["covariance"].append(Json::Value(item.covariance)); });
        root["pose"] = pose;

        Json::Value velocity;
        velocity["twist"]["linear"]["x"] = simExternalObj.get_VelocityTwistLinearX();
        velocity["twist"]["linear"]["y"] = simExternalObj.get_VelocityTwistLinearY();
        velocity["twist"]["linear"]["z"] = simExternalObj.get_VelocityTwistLinearZ();
        velocity["twist"]["angular"]["x"] = simExternalObj.get_VelocityTwistAngularX();
        velocity["twist"]["angular"]["y"] = simExternalObj.get_VelocityTwistAngularY();
        velocity["twist"]["angular"]["z"] = simExternalObj.get_VelocityTwistAngularZ();
        auto vCovarianceV = simExternalObj.get_VelocityCovariance();
        std::for_each(vCovarianceV.begin(), vCovarianceV.end(), [&velocity](const auto &item)
                      { velocity["covariance"].append(Json::Value(item.covariance)); });
        root["velocity"] = velocity;

        Json::Value size;
        size["x"] = simExternalObj.get_SizeX();
        size["y"] = simExternalObj.get_SizeY();
        size["z"] = simExternalObj.get_SizeZ();
        root["size"] = size;

        root["confidence"] = simExternalObj.get_Confidence();
        root["object_type"] = std::to_string(simExternalObj.get_ObjectType());
        root["dynamic_obj"] = simExternalObj.get_DynamticObj();
        Json::StreamWriterBuilder builder;
        const std::string json_str = Json::writeString(builder, root);
        return json_str;
    }

    void SimulationExternalObjectConverter::jsonToSimExternalObj(const std::string &jsonStr, tmx::messages::simulation::ExternalObject &simExternalObj)
    {
        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        Json::Value root;
        JSONCPP_STRING err;
        if (!reader->parse(jsonStr.c_str(), jsonStr.c_str() + static_cast<int>(jsonStr.length()), &root, &err))
        {
            throw std::runtime_error("Error parsing external object JSON string.");
        }

        /**
         * Populate simulation external object metadata
         */
        if (root.isMember("metadata") && root["metadata"].isObject())
        {
            populateSimExternalObjectMetadata(root["metadata"], simExternalObj);
        }

        /**
         * Populate simulation external object header
         */
        if (root.isMember("header") && root["header"].isObject())
        {
            populateSimExternalObjectHeader(root["header"], simExternalObj);
        }

        if (root.isMember("presence_vector") && root["presence_vector"].isUInt())
        {
            auto presence_vetor = presenceVectorIntToEnum(root["presence_vector"].asUInt());
            simExternalObj.set_PresenceVector(presence_vetor);
        }

        // Populate simulation external object id
        if (root.isMember("id") && root["id"].isUInt())
        {
            simExternalObj.set_Id(root["id"].asUInt());
        }

        /***
         * Populate simulation external object pose
         */
        if (root.isMember("pose") && root["pose"].isObject())
        {
            populateSimExternalObjectPose(root["pose"], simExternalObj);
        }

        /***
         *Populate simulation external object velocity
         */
        if (root.isMember("velocity") && root["velocity"].isObject())
        {
            populateSimExternalObjectVelocity(root["velocity"], simExternalObj);
        }

        /***
         * Populate simulation external object size
         */
        if (root.isMember("size") && root["size"].isObject())
        {
            populateSimExternalObjectSize(root["size"], simExternalObj);
        }

        // Populate simulation external object confidence
        if (root.isMember("confidence") && root["confidence"].isDouble())
        {
            simExternalObj.set_Confidence(root["confidence"].asDouble());
        }

        // Populate simulation external object object_type
        if (root.isMember("object_type") && root["object_type"].isString())
        {
            auto object_type = objectTypeStringToEnum(root["object_type"].asString());
            simExternalObj.set_ObjectType(object_type);
        }

        // Populate simulation external object dynamic_obj
        if (root.isMember("dynamic_obj") && root["dynamic_obj"].isBool())
        {
            simExternalObj.set_DynamticObj(root["dynamic_obj"].asBool());
        }
    }

    void SimulationExternalObjectConverter::populateSimExternalObjectMetadata(const Json::Value &metadataValue, tmx::messages::simulation::ExternalObject &simExternalObj)
    {
        if (metadataValue.isMember("is_simulation") && metadataValue["is_simulation"].isBool())
        {
            simExternalObj.set_MetadataIsSimulation(metadataValue["is_simulation"].asBool());
        }

        if (metadataValue.isMember("datum") && metadataValue["datum"].isString())
        {
            simExternalObj.set_MetadataDatum(metadataValue["datum"].asString());
        }

        if (metadataValue.isMember("proj_string") && metadataValue["proj_string"].isString())
        {
            simExternalObj.set_MetadataProjString(metadataValue["proj_string"].asString());
        }

        if (metadataValue.isMember("sensor_x") && metadataValue["sensor_x"].isDouble())
        {
            simExternalObj.set_MetadataSensorX(metadataValue["sensor_x"].asDouble());
        }

        if (metadataValue.isMember("sensor_y") && metadataValue["sensor_y"].isDouble())
        {
            simExternalObj.set_MetadataSensorY(metadataValue["sensor_y"].asDouble());
        }

        if (metadataValue.isMember("sensor_z") && metadataValue["sensor_z"].isDouble())
        {
            simExternalObj.set_MetadataSensorZ(metadataValue["sensor_z"].asDouble());
        }

        if (metadataValue.isMember("infrastructure_id") && metadataValue["infrastructure_id"].isString())
        {
            simExternalObj.set_MetadataInfrastructureId(metadataValue["infrastructure_id"].asString());
        }

        if (metadataValue.isMember("sensor_id") && metadataValue["sensor_id"].isString())
        {
            simExternalObj.set_MetadataSensorId(metadataValue["sensor_id"].asString());
        }
    }

    void SimulationExternalObjectConverter::populateSimExternalObjectHeader(const Json::Value &headerValue, tmx::messages::simulation::ExternalObject &simExternalObj)
    {
        if (headerValue.isMember("seq") && headerValue["seq"].isUInt())
        {
            simExternalObj.set_HeaderSeq(headerValue["seq"].asUInt());
        }

        if (headerValue.isMember("stamp") && headerValue["stamp"].isMember("secs") && headerValue["stamp"]["secs"].isUInt())
        {
            simExternalObj.set_HeaderStampSecs(headerValue["stamp"]["secs"].asUInt());
        }

        if (headerValue.isMember("stamp") && headerValue["stamp"].isMember("nsecs") && headerValue["stamp"]["nsecs"].isUInt())
        {
            simExternalObj.set_HeaderStampNSecs(headerValue["stamp"]["nsecs"].asUInt());
        }
    }

    void SimulationExternalObjectConverter::populateSimExternalObjectPose(const Json::Value &poseValue, tmx::messages::simulation::ExternalObject &simExternalObj)
    {
        if (poseValue.isMember("pose") && poseValue["pose"].isMember("position") && poseValue["pose"]["position"].isObject())
        {
            Json::Value positionValue = poseValue["pose"]["position"];
            if (positionValue.isMember("x"))
            {
                simExternalObj.set_PosePosePositionX(positionValue["x"].asDouble());
            }
            if (positionValue.isMember("y"))
            {
                simExternalObj.set_PosePosePositionY(positionValue["y"].asDouble());
            }
            if (positionValue.isMember("z"))
            {
                simExternalObj.set_PosePosePositionZ(positionValue["z"].asDouble());
            }
        }

        if (poseValue.isMember("pose") && poseValue["pose"].isMember("position") && poseValue["pose"]["orientation"].isObject())
        {
            Json::Value orientationValue = poseValue["pose"]["orientation"];
            if (orientationValue.isMember("x"))
            {
                simExternalObj.set_PosePoseOrientationX(orientationValue["x"].asDouble());
            }
            if (orientationValue.isMember("y"))
            {
                simExternalObj.set_PosePoseOrientationY(orientationValue["y"].asDouble());
            }
            if (orientationValue.isMember("z"))
            {
                simExternalObj.set_PosePoseOrientationZ(orientationValue["z"].asDouble());
            }
            if (orientationValue.isMember("w"))
            {
                simExternalObj.set_PosePoseOrientationW(orientationValue["w"].asDouble());
            }
        }

        if (poseValue.isMember("covariance") && poseValue["covariance"].isArray())
        {
            Json::Value covarianceArrayValue = poseValue["covariance"];
            std::vector<tmx::messages::simulation::Covariance> covarianceV;
            populateSimCovarianceArray(covarianceArrayValue, covarianceV);
            simExternalObj.set_PoseCovariance(covarianceV);
        }
    }

    void SimulationExternalObjectConverter::populateSimExternalObjectVelocity(const Json::Value &velocityValue, tmx::messages::simulation::ExternalObject &simExternalObj)
    {
        if (velocityValue.isMember("twist") && velocityValue["twist"].isMember("linear") && velocityValue["twist"]["linear"].isObject())
        {
            Json::Value linearValue = velocityValue["twist"]["linear"];
            if (linearValue.isMember("x"))
            {
                simExternalObj.set_VelocityTwistLinearX(linearValue["x"].asDouble());
            }
            if (linearValue.isMember("y"))
            {
                simExternalObj.set_VelocityTwistLinearY(linearValue["y"].asDouble());
            }
            if (linearValue.isMember("z"))
            {
                simExternalObj.set_VelocityTwistLinearZ(linearValue["z"].asDouble());
            }
        }

        if (velocityValue.isMember("twist") && velocityValue["twist"].isMember("angular") && velocityValue["twist"]["angular"].isObject())
        {
            Json::Value angularValue = velocityValue["twist"]["angular"];
            if (angularValue.isMember("x"))
            {
                simExternalObj.set_VelocityTwistAngularX(angularValue["x"].asDouble());
            }
            if (angularValue.isMember("y"))
            {
                simExternalObj.set_VelocityTwistAngularY(angularValue["y"].asDouble());
            }
            if (angularValue.isMember("z"))
            {
                simExternalObj.set_VelocityTwistAngularZ(angularValue["z"].asDouble());
            }
            if (angularValue.isMember("w"))
            {
                simExternalObj.set_PosePoseOrientationW(angularValue["w"].asDouble());
            }
        }
        if (velocityValue.isMember("covariance") && velocityValue["covariance"].isArray())
        {
            Json::Value covarianceArrayValue = velocityValue["covariance"];
            std::vector<tmx::messages::simulation::Covariance> covarianceV;
            populateSimCovarianceArray(covarianceArrayValue, covarianceV);
            simExternalObj.set_VelocityCovariance(covarianceV);
        }
    }

    void SimulationExternalObjectConverter::populateSimExternalObjectSize(const Json::Value &sizeValue, tmx::messages::simulation::ExternalObject &simExternalObj)
    {
        if (sizeValue.isMember("x"))
        {
            simExternalObj.set_SizeX(sizeValue["x"].asDouble());
        }
        if (sizeValue.isMember("y"))
        {
            simExternalObj.set_SizeY(sizeValue["y"].asDouble());
        }
        if (sizeValue.isMember("z"))
        {
            simExternalObj.set_SizeZ(sizeValue["z"].asDouble());
        }
    }

    void SimulationExternalObjectConverter::populateSimCovarianceArray(const Json::Value &covarianceArrayValue, std::vector<tmx::messages::simulation::Covariance> &covarianceV)
    {
        std::for_each(covarianceArrayValue.begin(), covarianceArrayValue.end(), [&covarianceV](const auto &item)
                      { tmx::messages::simulation::Covariance covariance(item.asDouble()); 
                      covarianceV.push_back(covariance); });
    }

    tmx::messages::simulation::OBJECT_TYPES SimulationExternalObjectConverter::objectTypeStringToEnum(const std::string &object_type_str)
    {
        tmx::messages::simulation::OBJECT_TYPES object_type = tmx::messages::simulation::OBJECT_TYPES::UNKNOWN;
        try
        {
            int object_type_int = stoi(object_type_str);
            switch (object_type_int)
            {
            case tmx::messages::simulation::OBJECT_TYPES::LARGE_VEHICLE:
                object_type = tmx::messages::simulation::OBJECT_TYPES::LARGE_VEHICLE;
                break;

            case tmx::messages::simulation::OBJECT_TYPES::MOTORCYCLE:
                object_type = tmx::messages::simulation::OBJECT_TYPES::MOTORCYCLE;
                break;

            case tmx::messages::simulation::OBJECT_TYPES::PEDESTRIAN:
                object_type = tmx::messages::simulation::OBJECT_TYPES::PEDESTRIAN;
                break;

            case tmx::messages::simulation::OBJECT_TYPES::SMALL_VEHICLE:
                object_type = tmx::messages::simulation::OBJECT_TYPES::SMALL_VEHICLE;
                break;

            default:
                object_type = tmx::messages::simulation::OBJECT_TYPES::UNKNOWN;
                break;
            }
        }
        catch (std::exception &err)
        {
            object_type = tmx::messages::simulation::OBJECT_TYPES::UNKNOWN;
        }
        return object_type;
    }

    tmx::messages::simulation::PRESENCE_VECTOR_TYPES SimulationExternalObjectConverter::presenceVectorIntToEnum(uint16_t presence_vector)
    {
        tmx::messages::simulation::PRESENCE_VECTOR_TYPES presence_vector_enum = tmx::messages::simulation::PRESENCE_VECTOR_TYPES::UNAVAILABLE;
        switch (presence_vector)
        {
        case tmx::messages::simulation::PRESENCE_VECTOR_TYPES::BSM_ID_PRESENCE_VECTOR:
            presence_vector_enum = tmx::messages::simulation::PRESENCE_VECTOR_TYPES::BSM_ID_PRESENCE_VECTOR;
            break;

        case tmx::messages::simulation::PRESENCE_VECTOR_TYPES::CONFIDENCE_PRESENCE_VECTOR:
            presence_vector_enum = tmx::messages::simulation::PRESENCE_VECTOR_TYPES::CONFIDENCE_PRESENCE_VECTOR;
            break;

        case tmx::messages::simulation::PRESENCE_VECTOR_TYPES::DYNAMIC_OBJ_PRESENCE:
            presence_vector_enum = tmx::messages::simulation::PRESENCE_VECTOR_TYPES::DYNAMIC_OBJ_PRESENCE;
            break;

        case tmx::messages::simulation::PRESENCE_VECTOR_TYPES::OBJECT_TYPE_PRESENCE_VECTOR:
            presence_vector_enum = tmx::messages::simulation::PRESENCE_VECTOR_TYPES::OBJECT_TYPE_PRESENCE_VECTOR;
            break;

        case tmx::messages::simulation::PRESENCE_VECTOR_TYPES::POSE_PRESENCE_VECTOR:
            presence_vector_enum = tmx::messages::simulation::PRESENCE_VECTOR_TYPES::POSE_PRESENCE_VECTOR;
            break;

        case tmx::messages::simulation::PRESENCE_VECTOR_TYPES::PREDICTION_PRESENCE_VECTOR:
            presence_vector_enum = tmx::messages::simulation::PRESENCE_VECTOR_TYPES::PREDICTION_PRESENCE_VECTOR;
            break;

        case tmx::messages::simulation::PRESENCE_VECTOR_TYPES::SIZE_PRESENCE_VECTOR:
            presence_vector_enum = tmx::messages::simulation::PRESENCE_VECTOR_TYPES::SIZE_PRESENCE_VECTOR;
            break;
            
        case tmx::messages::simulation::PRESENCE_VECTOR_TYPES::VELOCITY_INST_PRESENCE_VECTOR:
            presence_vector_enum = tmx::messages::simulation::PRESENCE_VECTOR_TYPES::VELOCITY_INST_PRESENCE_VECTOR;
            break;

        case tmx::messages::simulation::PRESENCE_VECTOR_TYPES::VELOCITY_PRESENCE_VECTOR:
            presence_vector_enum = tmx::messages::simulation::PRESENCE_VECTOR_TYPES::VELOCITY_PRESENCE_VECTOR;
            break;

        case tmx::messages::simulation::PRESENCE_VECTOR_TYPES::ID_PRESENCE_VECTOR:
            presence_vector_enum = tmx::messages::simulation::PRESENCE_VECTOR_TYPES::ID_PRESENCE_VECTOR;
            break;

        default:
            presence_vector_enum = tmx::messages::simulation::PRESENCE_VECTOR_TYPES::UNAVAILABLE;
            break;
        }
        return presence_vector_enum;
    }
}