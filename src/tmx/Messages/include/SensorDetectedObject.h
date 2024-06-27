#ifndef INCLUDE_SIMULATED_SensorDetectedObject_H_
#define INCLUDE_SIMULATED_SensorDetectedObject_H_

#include <tmx/messages/message.hpp>
#include <MessageTypes.h>
#include <Vector3d.h>
#include <Point.h>

namespace tmx
{
    namespace messages
    {
       
        /**
         * This SensorDetectedObject is used to communicate the sensor detected object information with various applications.
         * This message is the generic representation of a sensor detection.
         */
        class SensorDetectedObject : public tmx::message
        {
        public:
            SensorDetectedObject(){};
            SensorDetectedObject(const tmx::message_container_type &contents) : tmx::message(contents) {};
            ~SensorDetectedObject(){};
            // Message type for routing this message through TMX core
            static constexpr const char *MessageType = MSGTYPE_APPLICATION_STRING;

            // // Message sub type for routing this message through TMX core
            static constexpr const char *MessageSubType = MSGSUBTYPE_SENSOR_DETECTED_OBJECT_STRING;

            // TODO: Convert this member variable to std::attributes and handle nested object and arrays. (see [CloudHeartbeatMessage.h](./CloudHearbeatMessage.h) array_attribute )
            
            // Classification of detected object
            std::string type = "";
            // Confidence of type classification
            double confidence = 0.0;
            // Unique indentifier of sensor reporting detection
            std::string sensorId = "";
            // String describing projection used to convert cartesian data to WGS84 data 
            std::string projString = "";
            // Unique identifier of detected object
            int objectId = 0;
            // Cartesian positiion of object. Assumed to be ENU coordinate frame.
            tmx::utils::Point position = tmx::utils::Point();
            // Cartesian velocity vector of object. Assumed to be ENU coordinate frame.
            tmx::utils::Vector3d velocity = tmx::utils::Vector3d();
            // Epoch time in milliseconds
            long timestamp = 0;
           
        };

    }

}; // namespace tmx
#endif
