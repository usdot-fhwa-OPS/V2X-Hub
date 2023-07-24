#include <simulation/SimulationSensorDetectedObjectConverter.h>

namespace tmx::utils::sim
{
    std::string SimulationSensorDetectedObjectConverter::simExternalObjToJsonStr(tmx::messages::simulation::SensorDetectedObject &simExternalObj)
    {
        Json::Value root;
        Json::Value rootContent;
        Json::Value metadata;
        Json::Value sensor;
        root["type"] = simExternalObj.MessageType;
        root["subtype"] = simExternalObj.MessageSubType;
        rootContent["isSimulated"] = simExternalObj.get_MetadataIsSimulation();
        rootContent["timestamp"] = simExternalObj.get_MetadataTimestamp();
        sensor["proj_string"] = simExternalObj.get_SensorProjString();
        sensor["location"]["x"] = simExternalObj.get_SensorLocationX();
        sensor["location"]["y"] = simExternalObj.get_SensorLocationY();
        sensor["location"]["z"] = simExternalObj.get_SensorLocationZ();
        sensor["id"] = simExternalObj.get_MetadataSensorId();
        sensor["type"] = simExternalObj.get_SensorType();
        rootContent["sensor"] = sensor;

        rootContent["objectId"] = simExternalObj.get_Id();

        Json::Value pose;
        pose["x"] = simExternalObj.get_PositionX();
        pose["y"] = simExternalObj.get_PositionY();
        pose["z"] = simExternalObj.get_PositionZ();
        rootContent["position"] = pose;
        auto pCovarianceV = simExternalObj.get_PositionCovariance();
        std::for_each(pCovarianceV.begin(), pCovarianceV.end(), [&rootContent](const auto &item)
                      { rootContent["positionCovariance"].append(Json::Value(item.covariance)); });

        Json::Value velocity;
        velocity["x"] = simExternalObj.get_VelocityTwistLinearX();
        velocity["y"] = simExternalObj.get_VelocityTwistLinearY();
        velocity["z"] = simExternalObj.get_VelocityTwistLinearZ();
        auto vCovarianceV = simExternalObj.get_VelocityCovariance();
        std::for_each(vCovarianceV.begin(), vCovarianceV.end(), [&rootContent](const auto &item)
                      { rootContent["velocityCovariance"].append(Json::Value(item.covariance)); });
        rootContent["velocity"] = velocity;

        Json::Value size;
        size["length"] = simExternalObj.get_SizeLength();
        size["width"] = simExternalObj.get_SizeWidth();
        size["height"] = simExternalObj.get_SizeHeight();
        rootContent["size"] = size;

        rootContent["confidence"] = simExternalObj.get_Confidence();
        rootContent["type"] = simExternalObj.get_ObjectType();
        root["payload"] = rootContent;
        Json::StreamWriterBuilder builder;
        const std::string json_str = Json::writeString(builder, root);
        return json_str;
    }

    void SimulationSensorDetectedObjectConverter::jsonToSimExternalObj(const std::string &jsonStr, tmx::messages::simulation::SensorDetectedObject &simExternalObj)
    {
        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        Json::Value root;
        Json::Value rootContent;
        JSONCPP_STRING err;
        if (!reader->parse(jsonStr.c_str(), jsonStr.c_str() + static_cast<int>(jsonStr.length()), &root, &err))
        {
            throw std::runtime_error("Error parsing external object JSON string.");
        }

        if (!root.isMember("content"))
        {
            throw std::runtime_error("No content from JSON.");
        }
        rootContent = root["content"];
        /**
         * Populate simulation external object metadata
         */
        populateSimSensorDetectedObjectMetadata(rootContent, simExternalObj);

        // Populate simulation external object id
        if (rootContent.isMember("objectId") && rootContent["objectId"].isString())
        {
            simExternalObj.set_Id(rootContent["objectId"].asString());
        }

        if (rootContent.isMember("sensor") && rootContent["sensor"].isObject())
        {
            populateSimSensorDetectedObjectSensor(rootContent["sensor"], simExternalObj);
        }

        /***
         * Populate simulation external object position
         */
        populateSimSensorDetectedObjectPosition(rootContent, simExternalObj);

        /***
         *Populate simulation external object velocity
         */
        populateSimSensorDetectedObjectVelocity(rootContent, simExternalObj);

        /***
         * Populate simulation external object size
         */
        if (rootContent.isMember("size") && rootContent["size"].isObject())
        {
            populateSimSensorDetectedObjectSize(rootContent["size"], simExternalObj);
        }

        // Populate simulation external object confidence
        if (rootContent.isMember("confidence") && rootContent["confidence"].isDouble())
        {
            simExternalObj.set_Confidence(rootContent["confidence"].asDouble());
        }

        // Populate simulation external object type
        if (rootContent.isMember("type") && rootContent["type"].isString())
        {
            auto object_type = rootContent["type"].asString();
            simExternalObj.set_ObjectType(object_type);
        }
    }

    void SimulationSensorDetectedObjectConverter::populateSimSensorDetectedObjectMetadata(const Json::Value &metadataValue, tmx::messages::simulation::SensorDetectedObject &simExternalObj)
    {
        if (metadataValue.isMember("isSimulated") && metadataValue["isSimulated"].isBool())
        {
            simExternalObj.set_MetadataIsSimulation(metadataValue["isSimulated"].asBool());
        }

        if (metadataValue.isMember("timestamp") && metadataValue["timestamp"].isUInt64())
        {
            simExternalObj.set_MetadataTimestamp(metadataValue["timestamp"].asUInt64());
        }
    }

    void SimulationSensorDetectedObjectConverter::populateSimSensorDetectedObjectSensor(const Json::Value &sensorValue, tmx::messages::simulation::SensorDetectedObject &simExternalObj)
    {
        if (sensorValue.isMember("id") && sensorValue["id"].isString())
        {
            simExternalObj.set_SensorId(sensorValue["id"].asString());
        }

        if (sensorValue.isMember("type") && sensorValue["type"].isString())
        {
            simExternalObj.set_SensorType(sensorValue["type"].asString());
        }

        if (sensorValue.isMember("location") && sensorValue["location"]["x"].isDouble())
        {
            simExternalObj.set_SensorLocationX(sensorValue["location"]["x"].asDouble());
        }

        if (sensorValue.isMember("location") && sensorValue["location"]["y"].isDouble())
        {
            simExternalObj.set_SensorLocationY(sensorValue["location"]["y"].asDouble());
        }

        if (sensorValue.isMember("location") && sensorValue["location"]["z"].isDouble())
        {
            simExternalObj.set_SensorLocationZ(sensorValue["location"]["z"].asDouble());
        }

        if (sensorValue.isMember("proj_string") && sensorValue["proj_string"].isString())
        {
            simExternalObj.set_SensorProjString(sensorValue["proj_string"].asString());
        }
    }

    void SimulationSensorDetectedObjectConverter::populateSimSensorDetectedObjectPosition(const Json::Value &contentValue, tmx::messages::simulation::SensorDetectedObject &simExternalObj)
    {
        if (contentValue.isMember("position") && contentValue["position"].isObject())
        {
            Json::Value positionValue = contentValue["position"];
            if (positionValue.isMember("x"))
            {
                simExternalObj.set_PositionX(positionValue["x"].asDouble());
            }
            if (positionValue.isMember("y"))
            {
                simExternalObj.set_PositionY(positionValue["y"].asDouble());
            }
            if (positionValue.isMember("z"))
            {
                simExternalObj.set_PositionZ(positionValue["z"].asDouble());
            }
        }

        if (contentValue.isMember("positionCovariance") && contentValue["positionCovariance"].isArray())
        {
            Json::Value covarianceArrayValue = contentValue["positionCovariance"];
            std::vector<tmx::messages::simulation::Covariance> covarianceV;
            populateSimCovarianceArray(covarianceArrayValue, covarianceV);
            simExternalObj.set_PositionCovariance(covarianceV);
        }
    }

    void SimulationSensorDetectedObjectConverter::populateSimSensorDetectedObjectVelocity(const Json::Value &contentValue, tmx::messages::simulation::SensorDetectedObject &simExternalObj)
    {
        if (contentValue.isMember("velocity") && contentValue["velocity"].isObject())
        {
            Json::Value velocityValue = contentValue["velocity"];
            if (velocityValue.isMember("x"))
            {
                simExternalObj.set_VelocityTwistLinearX(velocityValue["x"].asDouble());
            }
            if (velocityValue.isMember("y"))
            {
                simExternalObj.set_VelocityTwistLinearY(velocityValue["y"].asDouble());
            }
            if (velocityValue.isMember("z"))
            {
                simExternalObj.set_VelocityTwistLinearZ(velocityValue["z"].asDouble());
            }
        }

        if (contentValue.isMember("velocityCovariance") && contentValue["velocityCovariance"].isArray())
        {
            Json::Value covarianceArrayValue = contentValue["velocityCovariance"];
            std::vector<tmx::messages::simulation::Covariance> covarianceV;
            populateSimCovarianceArray(covarianceArrayValue, covarianceV);
            simExternalObj.set_VelocityCovariance(covarianceV);
        }
    }

    void SimulationSensorDetectedObjectConverter::populateSimSensorDetectedObjectSize(const Json::Value &sizeValue, tmx::messages::simulation::SensorDetectedObject &simExternalObj)
    {
        if (sizeValue.isMember("length"))
        {
            simExternalObj.set_SizeLength(sizeValue["length"].asDouble());
        }
        if (sizeValue.isMember("width"))
        {
            simExternalObj.set_SizeWidth(sizeValue["width"].asDouble());
        }
        if (sizeValue.isMember("height"))
        {
            simExternalObj.set_SizeHeight(sizeValue["height"].asDouble());
        }
    }

    void SimulationSensorDetectedObjectConverter::populateSimCovarianceArray(const Json::Value &covarianceArrayValue, std::vector<tmx::messages::simulation::Covariance> &covarianceV)
    {
        std::for_each(covarianceArrayValue.begin(), covarianceArrayValue.end(), [&covarianceV](const auto &item)
                      { tmx::messages::simulation::Covariance covariance(item.asDouble()); 
                      covarianceV.push_back(covariance); });
    }

}