#pragma once
#include <string>
#include <unordered_map>
#include <stdexcept>      // std::out_of_range
#include <SensorDetectedObject.h>
#include <PluginLog.h>
#include <Vector3d.h>

namespace MUSTSensorDriverPlugin {

    enum class DetectionClassification {
        SEDAN,
        TRUCK,
        VAN,
        NA
    };

    enum class DetectionSize
    {
        SMALL,
        MEDIUM,
        LARGE,
        NA
    };

    const static std::unordered_map<std::string, DetectionSize> stringToDetectionSizeMap = {
        { "small", DetectionSize::SMALL},
        { "medium", DetectionSize::MEDIUM},
        { "large", DetectionSize::LARGE}
    };
    
    const static std::unordered_map<std::string, DetectionClassification> stringToDetectionClassificationMap = {
        {"sedan", DetectionClassification::SEDAN},
        {"truck", DetectionClassification::TRUCK},
        {"van", DetectionClassification::VAN}
    };

    const DetectionSize fromStringToDetectionSize(const std::string &str) noexcept;

    const DetectionClassification fromStringToDetectionClassification(const std::string &str) noexcept;

    struct MUSTSensorDetection {
        DetectionClassification cl = DetectionClassification::NA;
        double position_x = 0;
        double position_y = 0;
        double heading = 0;
        double speed = 0;
        DetectionSize size = DetectionSize::NA;
        double confidence = 0;
        unsigned trackID = 0;
        unsigned long timestamp = 0; 

    };


    MUSTSensorDetection csvToDectection(const std::string &csv );

    tmx::messages::SensorDetectedObject mustDectionToSensorDetectedObject(const MUSTSensorDetection &detection);

    tmx::utils::Vector3d headingSpeedToVelocity(double heading, double speed);
}