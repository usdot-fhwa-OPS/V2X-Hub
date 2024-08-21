#pragma once
#include <string>
#include <unordered_map>
#include <stdexcept>      // std::out_of_range
#include <SensorDetectedObject.h>
#include <PluginLog.h>
#include <Vector3d.h>
#include <algorithm>
#include <tmx/TmxException.hpp>


namespace MUSTSensorDriverPlugin {
    /**
     * @brief Enumeration for Detection Classifications
     */
    enum class DetectionClassification {
        SEDAN,
        TRUCK,
        BUS,
        PICKUP_TRUCK,
        PEDESTRIAN,
        VAN,
        NA
    };
    
    /**
     * @brief Enumeration for Detection Sizes
     */
    enum class DetectionSize
    {
        SMALL,
        MEDIUM,
        LARGE,
        NA
    };

    /**
     * @brief Static map used to convert string detection size information to enumeration.
     */
    const static std::unordered_map<std::string, DetectionSize> stringToDetectionSizeMap = {
        { "small", DetectionSize::SMALL},
        { "medium", DetectionSize::MEDIUM},
        { "large", DetectionSize::LARGE}
    };
    
    /**
     * @brief Static map used to convert string detection classification to enumeration.
     */
    const static std::unordered_map<std::string, DetectionClassification> stringToDetectionClassificationMap = {
        {"sedan", DetectionClassification::SEDAN},
        {"truck", DetectionClassification::TRUCK},
        {"van", DetectionClassification::VAN},
        {"bus", DetectionClassification::BUS},
        {"pickup truck", DetectionClassification::PICKUP_TRUCK},
        {"pedestrian", DetectionClassification::PEDESTRIAN}
    };

    /**
     * @brief Function to convert string detection size information to enumeration.
     * @param str detection size
     * @return DetectionSize enumeration if found in map or DetectionSize::NA if not found.
     */
    DetectionSize fromStringToDetectionSize(const std::string &str) noexcept;

    /**
     * @brief Function to convert string detection classification information to enumeration.
     * @param str detection classification
     * @return DetectionClassification enumeration if found in map or DetectionClassification::NA if not found.
     */
    DetectionClassification fromStringToDetectionClassification(const std::string &str) noexcept;

    /**
     * @brief Converts DetectionClassification enumeration to string type for SensorDetectedObject. All types are 
     * assumed to be capitalize versions of the DetectionClassifications.
     * @param classifcation DetectionClassification
     * @return std::string type for SensorDetectedObject
     * @throws tmx::TmxException if DetectionClassification is not included in map.
     */
    std::string detectionClassificationToSensorDetectedObjectType(const DetectionClassification &classifcation);

    /**
     * @brief Struct for storing MUST Sensor Detection information
     */
    struct MUSTSensorDetection {
        DetectionClassification cl = DetectionClassification::NA;
        // Meters
        double position_x = 0;
        // Meters
        double position_y = 0;
        // Degrees
        double heading = 0;
        // Meters/Second
        double speed = 0;
        DetectionSize size = DetectionSize::NA;
        // Confidence in type
        double confidence = 0;
        // Unique ID
        unsigned int trackID = 0;
        // Timestamp in seconds
        double timestamp = 0; 

    };

    /**
     * @brief Function to convert CSV string to MUSTSensorDetection struct
     * @param csv std::string
     * @return MUSTSensorDetection 
     * @throws tmx::TmxException if string is misformatted.
     */
    MUSTSensorDetection csvToDetection(const std::string &csv );

    /**
     * @brief Function to convert MUSTSensorDetections to SensorDetectedObject
     * @param detection MUSTSensorDetection
     * @param sensorId std::string unique indentifier of MUSTSensor
     * @param projString std::string describing reference point and WGS84 projection of detection information
     * @return tmx::messages::SensorDetectedObject
     */
    tmx::messages::SensorDetectedObject mustDetectionToSensorDetectedObject(const MUSTSensorDetection &detection, std::string_view sensorId, std::string_view projString);
    
    /**
     * @brief Function to convert MUSTSensor provided heading and speed to a velocity vector 
     * @param heading double heading in degrees
     * @param speed double speed in m/s
     * @return tmx::utils::Vector3d velocity.
     */
    tmx::utils::Vector3d headingSpeedToVelocity(double heading, double speed);
}