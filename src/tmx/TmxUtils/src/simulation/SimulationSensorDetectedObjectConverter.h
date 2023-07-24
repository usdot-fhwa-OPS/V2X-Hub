#pragma once

#include <jsoncpp/json/json.h>
#include <iostream>
#include <simulation/SensorDetectedObject.h>

namespace tmx::utils::sim
{

    class SimulationSensorDetectedObjectConverter
    {
    private:
        /***
         * @brief Populate simulation detected object metadata with metadata information from JSON string.
         * @param SensorDetectedObject V2xHub customized simulation detected object to populate.
         * @param metadataValue Json value that contains the metadata information.
        */
        static void populateSimSensorDetectedObjectMetadata(const Json::Value &metadataValue, tmx::messages::simulation::SensorDetectedObject &simExternalObj);    
         /***
         * @brief Populate simulation detected object sensor with sensor information from JSON string.
         * @param SensorDetectedObject V2xHub customized simulation detected object to populate.
         * @param sensorValue Json value that contains the sensor information.
        */
        static void populateSimSensorDetectedObjectSensor(const Json::Value &sensorValue, tmx::messages::simulation::SensorDetectedObject &simExternalObj);   
        /***
         * @brief Populate simulation detected object position with position information from JSON string.
         * @param SensorDetectedObject V2xHub customized simulation detected object to populate.
         * @param positionValue Json value that contains the position information.
        */
        static void populateSimSensorDetectedObjectPosition(const Json::Value &positionValue, tmx::messages::simulation::SensorDetectedObject &simExternalObj);
        /***
         * @brief Populate simulation detected object velocity with velocity information from JSON string.
         * @param SensorDetectedObject V2xHub customized simulation detected object to populate.
         * @param velocityValue Json value that contains the velocity information.
        */
        static void populateSimSensorDetectedObjectVelocity(const Json::Value &velocityValue, tmx::messages::simulation::SensorDetectedObject &simExternalObj);
        /***
         * @brief Populate simulation detected object size with size information from JSON string.
         * @param SensorDetectedObject V2xHub customized simulation detected object to populate.
         * @param sizeValue Json value that contains the size information.
        */
        static void populateSimSensorDetectedObjectSize(const Json::Value &sizeValue, tmx::messages::simulation::SensorDetectedObject &simExternalObj);
        /***
         * @brief Populate simulation detected object covariance array with covariance array information from JSON string.
         * @param SensorDetectedObject V2xHub customized simulation detected object to populate.
         * @param covarianceArrayValue Json value that contains the covariance array information.
        */
        static void populateSimCovarianceArray(const  Json::Value &covarianceArrayValue, std::vector<tmx::messages::simulation::Covariance> &covarianceV);

    public:
        SimulationSensorDetectedObjectConverter() = delete;
        ~SimulationSensorDetectedObjectConverter() = delete;
        /***
         * @brief Convert simulation detected object into JSON string defined by MOSAIC and CARMAStreets integration.
         * @param SensorDetectedObject V2xHub customized simulation detected object.
         * @return EcternalObject string in JSON format defined by MOSAIC and CARMAStreets integration.
        */
        static std::string simExternalObjToJsonStr(tmx::messages::simulation::SensorDetectedObject &simExternalObj);
        /***
         * @brief Populate simulation detected object with JSON string defined by MOSAIC and CARMAStreets integration.
         * @param SensorDetectedObject V2xHub customized simulation detected object to populate.
         * @param jsonStr EcternalObject in JSON format defined by MOSAIC and CARMAStreets integration.
        */
        static void jsonToSimExternalObj(const std::string &jsonStr, tmx::messages::simulation::SensorDetectedObject &simExternalObj);
    };
}
