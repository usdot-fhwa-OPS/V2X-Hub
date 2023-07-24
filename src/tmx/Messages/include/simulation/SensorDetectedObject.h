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

                // Message sub type for routing this message through TMX core
                static constexpr const char *MessageSubType = MSGSUBTYPE_SENSOR_DETECTED_OBJECT_STRING;
                /**
                 * Metadata to describe the external object
                */
                std_attribute(this->msg, bool, MetadataIsSimulation, false, );
                std_attribute(this->msg, uint64_t, MetadataTimestamp, 0, );
                std_attribute(this->msg, std::string, SensorProjString, "", );
                std_attribute(this->msg, std::string, SensorType, "", );
                std_attribute(this->msg, std::string, SensorId, "", );
                std_attribute(this->msg, double, SensorLocationX, 0, );
                std_attribute(this->msg, double, SensorLocationY, 0, );
                std_attribute(this->msg, double, SensorLocationZ, 0, );
                std_attribute(this->msg, std::string, MetadataInfrastructureId, "", );
                std_attribute(this->msg, std::string, MetadataSensorId, "", );                

                // Object id. Matching ids on a topic should refer to the same object within some time period, expanded
                std_attribute(this->msg, std::string, Id, "", );

                // Pose of the object within the frame specified in header
                // This represents a pose in free space with uncertainty.
                std_attribute(this->msg, double, PositionX, 0, );
                std_attribute(this->msg, double, PositionY, 0, );
                std_attribute(this->msg, double, PositionZ, 0, );
                // This represents an orientation in free space in quaternion form.
                std_attribute(this->msg, double, OrientationX, 0, );
                std_attribute(this->msg, double, OrientationY, 0, );
                std_attribute(this->msg, double, OrientationZ, 0, );
                std_attribute(this->msg, double, OrientationW, 0, );
                array_attribute( Covariance, PositionCovariance);

                // #Average velocity of the object within the frame specified in header
                std_attribute(this->msg, double, VelocityTwistLinearX, 0, );
                std_attribute(this->msg, double, VelocityTwistLinearY, 0, );
                std_attribute(this->msg, double, VelocityTwistLinearZ, 0, );
                std_attribute(this->msg, double, VelocityTwistAngularX, 0, );
                std_attribute(this->msg, double, VelocityTwistAngularY, 0, );
                std_attribute(this->msg, double, VelocityTwistAngularZ, 0, );
                array_attribute( Covariance, VelocityCovariance);

                // #Instantaneous velocity of an object within the frame specified in header
                std_attribute(this->msg, double, VelocityInstTwistLinearX, 0, );
                std_attribute(this->msg, double, VelocityInstTwistLinearY, 0, );
                std_attribute(this->msg, double, VelocityInstTwistLinearZ, 0, );
                std_attribute(this->msg, double, VelocityInstTwistAngularX, 0, );
                std_attribute(this->msg, double, VelocityInstTwistAngularY, 0, );
                std_attribute(this->msg, double, VelocityInstTwistAngularZ, 0, );
                array_attribute( Covariance, VelocityInstCovariance);

                // #The size of the object aligned along the axis of the object described by the orientation in pose
                // #Dimensions are specified in meters
                /**
                 * # This represents a vector in free space.
                    # It is only meant to represent a direction. Therefore, it does not
                    # make sense to apply a translation to it (e.g., when applying a
                    # generic rigid transformation to a Vector3, tf2 will only apply the
                    # rotation).
                */
                std_attribute(this->msg, double, SizeLength, 0, );
                std_attribute(this->msg, double, SizeWidth, 0, );
                std_attribute(this->msg, double, SizeHeight, 0, );

                // #Confidence
                std_attribute(this->msg, double, Confidence, 0, );

                // #describes a general object type as defined in this message
                std_attribute(this->msg, std::string, ObjectType, "", );
            };

        }
    }

}; // namespace tmx
#endif
