#pragma once

#include <jsoncpp/json/json.h>
#include <iostream>
#include <simulation/ExternalObject.h>

namespace tmx::utils::sim
{

    class SimulationExternalObjectConverter
    {
    private:
        static void populateSimExternalObjectMetadata(const Json::Value &metadataValue, tmx::messages::simulation::ExternalObject &simExternalObj);
        static void populateSimExternalObjectHeader(const Json::Value &headerValue, tmx::messages::simulation::ExternalObject &simExternalObj);
        static void populateSimExternalObjectPose(const Json::Value &poseValue, tmx::messages::simulation::ExternalObject &simExternalObj);
        static void populateSimExternalObjectVelocity(const Json::Value &velocityValue, tmx::messages::simulation::ExternalObject &simExternalObj);
        static void populateSimExternalObjectSize(const Json::Value &sizeValue, tmx::messages::simulation::ExternalObject &simExternalObj);
        static void populateSimCovarianceArray(const  Json::Value &covarianceArrayValue, std::vector<tmx::messages::simulation::Covariance> &covarianceV);
        static tmx::messages::simulation::OBJECT_TYPES objectTypeStringToEnum(const std::string& object_type_str);
        static tmx::messages::simulation::PRESENCE_VECTOR_TYPES presenceVectorIntToEnum(uint16_t presence_vector);

    public:
        SimulationExternalObjectConverter() = default;
        ~SimulationExternalObjectConverter() = default;
        static std::string simExternalObjToJsonStr(tmx::messages::simulation::ExternalObject &simExternalObj);
        static void jsonToSimExternalObj(const std::string &jsonStr, tmx::messages::simulation::ExternalObject &simExternalObj);
    };
}
