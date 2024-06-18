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

            std::string type = "";
            double confidence = 0.0;
            std::string sensorId = "";
            std::string projString = "";
            int objectId = 0;
            tmx::utils::Point position = tmx::utils::Point();
            tmx::utils::Vector3d velocity = tmx::utils::Vector3d();
            long timestamp = 0;
        };

    }

}; // namespace tmx
#endif
