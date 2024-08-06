#include "MUSTSensorDetection.h"

namespace MUSTSensorDriverPlugin {

    
    MUSTSensorDetection csvToDetection(const std::string &csv ) {
        MUSTSensorDetection detection;
        std::vector<std::string> csv_values;
        std::stringstream ss(csv);
        while (ss.good()) {
            std::string substr;
            std::getline(ss, substr, ',');
            csv_values.push_back(substr);
        }
        if (csv_values.size() != 9 ){
            FILE_LOG(tmx::utils::logERROR) << "Data " << csv << " does not match expected csv data format : \'class,x,y,heading,speed,size,confidence,trackId,timestamp\'" << std::endl;
            throw tmx::TmxException("Failed to parse CSV MUST Detection data");
        }
        // Read out CSV information
        detection.cl = fromStringToDetectionClassification(csv_values.at(0));
        detection.position_x = std::stod(csv_values.at(1));
        detection.position_y = std::stod(csv_values.at(2));
        detection.heading = std::stod(csv_values.at(3));
        detection.speed = std::stod(csv_values.at(4));
        detection.size = fromStringToDetectionSize(csv_values.at(5));
        detection.confidence = std::stod(csv_values.at(6));
        detection.trackID = std::stoi(csv_values.at(7));
        detection.timestamp = std::stod(csv_values.at(8));
        return detection;
    }

    tmx::messages::SensorDetectedObject mustDetectionToSensorDetectedObject(const MUSTSensorDetection &detection, std::string_view sensorId, std::string_view projString) {
        tmx::messages::SensorDetectedObject detectedObject;
        detectedObject.set_ObjectId(detection.trackID);
        tmx::messages::SensorDetectedObject::Position pos(detection.position_x, detection.position_y, 0);
        detectedObject.set_Position(pos);
        detectedObject.set_Confidence(detection.confidence);
        detectedObject.set_Timestamp(static_cast<long>(detection.timestamp*1000)); // convert decimal seconds to int milliseconds.
        detectedObject.set_Velocity(headingSpeedToVelocity(detection.heading, detection.speed));
        detectedObject.set_Type(detectionClassificationToSensorDetectedObjectType(detection.cl));
        detectedObject.set_SensorId(std::string(sensorId));
        detectedObject.set_ProjString(std::string(projString));
        return detectedObject;
    }
    DetectionClassification fromStringToDetectionClassification(const std::string &str) noexcept {
        try {
            
            return stringToDetectionClassificationMap.at(str);
        }
        catch( const std::out_of_range &e ) {
            FILE_LOG(tmx::utils::logWARNING) << e.what() << "No registered Detection Classification for " << str << " in stringToDetectionClassificationMap! Setting classification as NA." << std::endl;
            return DetectionClassification::NA;
        }
    }

    std::string detectionClassificationToSensorDetectedObjectType(const DetectionClassification &classification) {
       for (auto const &[name, cl] : stringToDetectionClassificationMap){
            if (classification == cl) {
                std::string rtn = name;
                std::transform(rtn.begin(), rtn.end(), rtn.begin(), ::toupper);
                return rtn;
            }
       }
       throw tmx::TmxException("DetectionClassification type is not registered in stringToDetectionClassificationMap!");
    }



    DetectionSize fromStringToDetectionSize(const std::string &str) noexcept {
        try {
            
            return stringToDetectionSizeMap.at(str);
        }
        catch( const std::out_of_range &e) {
            FILE_LOG(tmx::utils::logWARNING) << e.what() << "No registered Detection Size for " << str << " in stringToDetectionSizeMap! Setting classification as NA." << std::endl;
            return DetectionSize::NA;
        }
    };

    tmx::messages::SensorDetectedObject::Velocity headingSpeedToVelocity(double heading, double speed) {
        // Convert North East heading to Angle with 0 at (1, 0) (See README Unit Circle)
        heading = heading - 270;
        // factor for converting heading from degrees to radians
        auto headingInRad = M_PI / 180;
        tmx::messages::SensorDetectedObject::Velocity velocity;
        velocity.x = std::cos(headingInRad * heading) * speed;
        velocity.y = std::sin(headingInRad * heading) * speed;
        velocity.z = 0;
        return velocity;
    };
}
