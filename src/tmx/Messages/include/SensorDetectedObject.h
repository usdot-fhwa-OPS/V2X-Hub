#pragma once

#include <tmx/messages/message.hpp>
#include <MessageTypes.h>
#include "Position.h"
#include "Covariance.h"
#include "Velocity.h"
#include "Size.h"

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
            //Flag to indicate whether sensor detected object is simulated.
            std_attribute(this->msg, bool, isSimulated, false,);
            // Classification of detected object.
            std_attribute(this->msg, std::string, type, "",);
            // Confidence of type classification
            std_attribute(this->msg, double, confidence, 0.0,);
            // Unique indentifier of sensor reporting detection.
            std_attribute(this->msg, std::string, sensorId, "", );
            // String describing projection used to convert cartesian data to WGS84 data.
            std_attribute(this->msg, std::string, projString, "", );
            // Unique identifier of detected object.
            std_attribute(this->msg, int, objectId, 0, );

                 
            object_attribute(Position, position);    
            two_dimension_array_attribute(Covariance, positionCovariance);       
            //Linear velocity in meter per second
            object_attribute(Velocity, velocity);
            //Covariance associated with linear velocity.
            two_dimension_array_attribute(Covariance, velocityCovariance);
            //Angular velocity in radians per second.
            object_attribute(Velocity, angularVelocity);
            //Covariance associated with angular velocity.
            two_dimension_array_attribute(Covariance, angularVelocityCovariance);

            // Epoch time in milliseconds.
            // long timestamp = 0;
            std_attribute(this->msg, long, timestamp, 0, );            
            object_attribute(Size, size);
           
        };

    }

}; // namespace tmx
