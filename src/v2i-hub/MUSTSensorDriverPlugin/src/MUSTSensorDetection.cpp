#include "MUSTSensorDetection.h"

namespace MUSTSensorDriverPlugin {

    
    MUSTSensorDetection csvToDectection(const std::string &csv ) {
        MUSTSensorDetection detection;
        std::vector<std::string> csv_values;
        std::stringstream ss(csv);
        while (ss.good()) {
            std::string substr;
            std::getline(ss, substr, ',');
            csv_values.push_back(substr);
        }
        if (csv.size() != 9 ){
            throw std::runtime_error("Failed to parse csv MUST Detection data");
        }
        // Read out CSV information
        detection.cl = fromStringToDetectionClassification(&csv.at(0));
        detection.position_x = std::stod(&csv.at(1));
        detection.position_y = std::stod(&csv.at(2));
        detection.heading = std::stod(&csv.at(3));
        detection.speed = std::stod(&csv.at(4));
        detection.size = fromStringToDetectionSize(&csv.at(5));
        detection.confidence = std::stod(&csv.at(6));
        detection.trackID = std::stoi(&csv.at(7));
        detection.timestamp = std::stoll(&csv.at(8));
        return detection;
    }

    tmx::messages::simulation::SensorDetectedObject mustDectionToSensorDetectedObject(const MUSTSensorDetection &detection) {
        tmx::messages::simulation::SensorDetectedObject detectedObject;
        detectedObject.objectId = detection.trackID;
        detectedObject.position.X = detection.position_x;
        detectedObject.position.Y = detection.position_y;
        detectedObject.confidence = detection.confidence;
        detectedObject.timestamp = detection.timestamp;
        return detectedObject;
    }
    const DetectionClassification fromStringToDetectionClassification(const std::string &str) noexcept {
        try {
            
            return stringToDetectionClassificationMap.at(str);
        }
        catch( const std::out_of_range &e) {
            return DetectionClassification::NA;
        }
    }

    const DetectionSize fromStringToDetectionSize(const std::string &str) noexcept {
        try {
            
            return stringToDetectionSizeMap.at(str);
        }
        catch( const std::out_of_range &e) {
            return DetectionSize::NA;
        }
    };
}