#ifndef INCLUDE_SIMULATED_SensorDetectedObject_H_
#define INCLUDE_SIMULATED_SensorDetectedObject_H_

#include <tmx/messages/message.hpp>
#include <MessageTypes.h>
#include <Vector3d.h>
#include <Point.h>
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
            std_attribute(this->msg, bool, ISSimulated, false,);
            // Classification of detected object.
            std_attribute(this->msg, std::string, Type, "",);
            // Confidence of type classification
            std_attribute(this->msg, double, Confidence, 0.0,);
            // Unique indentifier of sensor reporting detection.
            std_attribute(this->msg, std::string, SensorId, "", );
            // String describing projection used to convert cartesian data to WGS84 data.
            std_attribute(this->msg, std::string, ProjString, "", );
            // Unique identifier of detected object.
            std_attribute(this->msg, int, ObjectId, 0, );

                 
            object_attribute(Position, Position);           
            array_attribute(Covariance, PositionCovariance);
            //Linear velocity in meter per second
            object_attribute(Velocity, Velocity);
            //Covariance associated with linear velocity.
            array_attribute(Covariance, VelocityCovariance);
            //Angular velocity in radians per second.
            object_attribute(Velocity, AngularVelocity);
            //Covariance associated with angular velocity.
            array_attribute(Covariance, AngularVelocityCovariance);

            // Epoch time in milliseconds.
            // long timestamp = 0;
            std_attribute(this->msg, long, Timestamp, 0, );            
            object_attribute(Size, Size);
           
        };

    }

}; // namespace tmx
#endif
