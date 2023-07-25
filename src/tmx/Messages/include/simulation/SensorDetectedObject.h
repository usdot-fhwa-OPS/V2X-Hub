#ifndef INCLUDE_SIMULATED_SensorDetectedObject_H_
#define INCLUDE_SIMULATED_SensorDetectedObject_H_

#include <tmx/messages/message.hpp>
#include <MessageTypes.h>
#include <simulation/Covariance.h>

namespace tmx
{
    namespace messages
    {
        namespace simulation
        {
            /**
             * This SensorDetectedObject is used to communicate the sensor detected object information with various applications
             * including internal infrastructure applications and external road user applications through simulated environment.
             * It defines the message type and sub type and all data members.
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
            };

        }
    }

}; // namespace tmx
#endif
