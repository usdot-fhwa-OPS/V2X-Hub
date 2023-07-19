#pragma once

#include <jsoncpp/json/json.h>
#include <iostream>
#include <simulation/ExternalObject.h>

namespace tmx::utils::sim
{

    class SimulationExternalObjectConverter
    {
    private:
        /***
         * @brief Populate simulation external object metadata with metadata information from JSON string.
         * @param ExternalObject V2xHub customized simulation external object to populate.
         * @param metadataValue Json value that contains the metadata information.
        */
        static void populateSimExternalObjectMetadata(const Json::Value &metadataValue, tmx::messages::simulation::ExternalObject &simExternalObj);
        /***
         * @brief Populate simulation external object header with header information from JSON string.
         * @param ExternalObject V2xHub customized simulation external object to populate.
         * @param headerValue Json value that contains the header information.
        */
        static void populateSimExternalObjectHeader(const Json::Value &headerValue, tmx::messages::simulation::ExternalObject &simExternalObj);
        /***
         * @brief Populate simulation external object pose with pose information from JSON string.
         * @param ExternalObject V2xHub customized simulation external object to populate.
         * @param poseValue Json value that contains the pose information.
        */
        static void populateSimExternalObjectPose(const Json::Value &poseValue, tmx::messages::simulation::ExternalObject &simExternalObj);
        /***
         * @brief Populate simulation external object velocity with velocity information from JSON string.
         * @param ExternalObject V2xHub customized simulation external object to populate.
         * @param velocityValue Json value that contains the velocity information.
        */
        static void populateSimExternalObjectVelocity(const Json::Value &velocityValue, tmx::messages::simulation::ExternalObject &simExternalObj);
        /***
         * @brief Populate simulation external object size with size information from JSON string.
         * @param ExternalObject V2xHub customized simulation external object to populate.
         * @param sizeValue Json value that contains the size information.
        */
        static void populateSimExternalObjectSize(const Json::Value &sizeValue, tmx::messages::simulation::ExternalObject &simExternalObj);
        /***
         * @brief Populate simulation external object covariance array with covariance array information from JSON string.
         * @param ExternalObject V2xHub customized simulation external object to populate.
         * @param covarianceArrayValue Json value that contains the covariance array information.
        */
        static void populateSimCovarianceArray(const  Json::Value &covarianceArrayValue, std::vector<tmx::messages::simulation::Covariance> &covarianceV);
        /***
         * @brief Convert the object type in string format to enum.
         * @param object_type_str object type in string format.
         * @return tmx::messages::simulation::OBJECT_TYPES V2xHub customized simulation object types.
        */
        static tmx::messages::simulation::OBJECT_TYPES objectTypeStringToEnum(const std::string& object_type_str);
         /***
         * @brief Convert the presence vector integer to enum.
         * @param presence_vector presence vetor integer.
         * @return tmx::messages::simulation::PRESENCE_VECTOR_TYPES V2xHub customized simulation presence vetor types.
        */
        static tmx::messages::simulation::PRESENCE_VECTOR_TYPES presenceVectorIntToEnum(uint16_t presence_vector);

    public:
        SimulationExternalObjectConverter() = delete;
        ~SimulationExternalObjectConverter() = delete;
        /***
         * @brief Convert simulation external object into JSON string defined by MOSAIC and CARMAStreets integration.
         * @param ExternalObject V2xHub customized simulation external object.
         * @return EcternalObject string in JSON format defined by MOSAIC and CARMAStreets integration.
        */
        static std::string simExternalObjToJsonStr(tmx::messages::simulation::ExternalObject &simExternalObj);
        /***
         * @brief Populate simulation external object with JSON string defined by MOSAIC and CARMAStreets integration.
         * @param ExternalObject V2xHub customized simulation external object to populate.
         * @param jsonStr EcternalObject in JSON format defined by MOSAIC and CARMAStreets integration.
        */
        static void jsonToSimExternalObj(const std::string &jsonStr, tmx::messages::simulation::ExternalObject &simExternalObj);
    };
}
