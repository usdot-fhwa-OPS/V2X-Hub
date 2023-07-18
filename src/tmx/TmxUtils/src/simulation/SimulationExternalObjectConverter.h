#pragma once

#include <jsoncpp/json/json.h>
#include <iostream>
#include <simulation/ExternalObject.h>

using namespace tmx::messages;
using namespace std;
namespace tmx::utils::sim
{

    class SimulationExternalObjectConverter
    {
    private:
        static void populateSimExternalObjectMetadata(const Json::Value &metadataValue, simulation::ExternalObject &simExternalObj);
        static void populateSimExternalObjectHeader(const Json::Value &headerValue, simulation::ExternalObject &simExternalObj);
        static void populateSimExternalObjectPose(const Json::Value &poseValue, simulation::ExternalObject &simExternalObj);
        static void populateSimExternalObjectVelocity(const Json::Value &velocityValue, simulation::ExternalObject &simExternalObj);
        static void populateSimExternalObjectSize(const Json::Value &sizeValue, simulation::ExternalObject &simExternalObj);
        static void populateSimCovarianceArray(const  Json::Value &covarianceArrayValue, std::vector<simulation::Covariance> &covarianceV);
        static simulation::OBJECT_TYPES objectTypeStringToEnum(const string& object_type_str);
        static simulation::PRESENCE_VECTOR_TYPES presenceVectorIntToEnum(uint16_t presence_vector);

    public:
        SimulationExternalObjectConverter() = default;
        ~SimulationExternalObjectConverter() = default;
        static string simExternalObjToJsonStr(simulation::ExternalObject &simExternalObj);
        static void jsonToSimExternalObj(const string &jsonStr, simulation::ExternalObject &simExternalObj);
    };
}
