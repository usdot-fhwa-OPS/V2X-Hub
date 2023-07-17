#ifndef INCLUDE_SIMULATED_EXTERNALOBJECT_H_
#define INCLUDE_SIMULATED_EXTERNALOBJECT_H_

#include <tmx/messages/message.hpp>
#include <MessageTypes.h>
// #include <simulation/Header.h>
#include <simulation/PresenceVectorEnumTypes.h>
#include <simulation/ObjectEnumTypes.h>

namespace tmx
{
    namespace messages
    {
        namespace simulation
        {
            /**
             * This ExternalObject is used to communicate the sensor detected object information with various applications
             * including internal infrastructure applications and external road user applications through simulated environment.
             * It defines the message type and sub type and all data members.
             */
            class ExternalObject : public tmx::message
            {
            public:
                ExternalObject(){};
                ExternalObject(const tmx::message_container_type &contents) : tmx::message(contents) {}
                ExternalObject(PRESENCE_VECTOR_TYPES presenceVector, uint32_t id,
                               double confidence, OBJECT_TYPES objectType, bool dynamticObj)
                {
                    set_PresenceVector(presenceVector);
                    set_Id(id);
                    set_Confidence(confidence);
                    set_ObjectType(objectType);
                    set_DynamticObj(dynamticObj);
                };
                ~ExternalObject(){};
                // Message type fpr routing this message through TMX core
                static constexpr const char *MessageType = MSGTYPE_DETECTED_STRING;

                // Message sub type for routing this message through TMX core
                static constexpr const char *MessageSubType = MSGSUBTYPE_EXTERNAL_OBJECT_STRING;

                /**
                 *Header contains the frame rest of the fields will use
                 */
                // sequence ID: consecutively increasing ID
                std_attribute(this->msg, uint32_t, HeaderSeq, 0, );
                // Two-integer timestamp that is expressed as:
                //  # * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
                //  # * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
                //  # time-handling sugar is provided by the client library
                // The seconds component, valid over all int32 values.
                std_attribute(this->msg, uint32_t, HeaderTimeSec, 0, );
                // # The nanoseconds component, valid in the range [0, 10e9).
                std_attribute(this->msg, uint32_t, HeaderTimeNanoSec, 0, );

                // A presence vector, this message is used to describe objects coming from potentially different
                // sources. The presence vector is used to determine what items are set by the producer.
                std_attribute(this->msg, PRESENCE_VECTOR_TYPES, PresenceVector, PRESENCE_VECTOR_TYPES::OBJECT_TYPE_PRESENCE_VECTOR, );

                // Object id. Matching ids on a topic should refer to the same object within some time period, expanded
                std_attribute(this->msg, uint32_t, Id, 0, );

                // bsm id is of form [0xff, 0xff, 0xff, 0xff]. It is not required.
                // uint8[] bsm_id
                struct BSMID
                {
                    uint8_t BsmId = 0;

                    BSMID() {}
                    BSMID(std::uint8_t bsmId) : BsmId(bsmId) {}

                    static message_tree_type to_tree(BSMID element)
                    {
                        message_tree_type treeElement;
                        treeElement.put("BsmId", element.BsmId);
                        return treeElement;
                    }

                    static BSMID from_tree(message_tree_type &treeElement)
                    {
                        BSMID element;
                        element.BsmId = treeElement.get<std::uint8_t>("BsmId");
                        return element;
                    }
                };
                array_attribute( BSMID, BsmId);

                // Pose of the object within the frame specified in header
                // geometry_msgs/PoseWithCovariance pose
                // This represents a pose in free space with uncertainty.
                std_attribute(this->msg, double, PosePointX, 0, );
                std_attribute(this->msg, double, PosePointY, 0, );
                std_attribute(this->msg, double, PosePointZ, 0, );

                // This represents an orientation in free space in quaternion form.
                std_attribute(this->msg, double, PoseQuaternionX, 0, );
                std_attribute(this->msg, double, PoseQuaternionY, 0, );
                std_attribute(this->msg, double, PoseQuaternionZ, 0, );
                std_attribute(this->msg, double, PoseQuaternionW, 0, );

                // Row-major representation of the 6x6 covariance matrix
                //  # The orientation parameters use a fixed-axis representation.
                //  # In order, the parameters are:
                //  # (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)
                std_attribute(this->msg, double, Covariance, 0, );

                // #Average velocity of the object within the frame specified in header
                // geometry_msgs/TwistWithCovariance velocity

                // #Instantaneous velocity of an object within the frame specified in header
                // geometry_msgs/TwistWithCovariance velocity_inst

                // #The size of the object aligned along the axis of the object described by the orientation in pose
                // #Dimensions are specified in meters
                /**
                 * # This represents a vector in free space.
                    # It is only meant to represent a direction. Therefore, it does not
                    # make sense to apply a translation to it (e.g., when applying a
                    # generic rigid transformation to a Vector3, tf2 will only apply the
                    # rotation). If you want your data to be translatable too, use the
                    # geometry_msgs/Point message instead.
                */
                // geometry_msgs/Vector3 size
                std_attribute(this->msg, double, SizeX, 0, );
                std_attribute(this->msg, double, SizeY, 0, );
                std_attribute(this->msg, double, SizeZ, 0, );

                // #Confidence [0,1]
                std_attribute(this->msg, double, Confidence, 0, );

                // #describes a general object type as defined in this message
                std_attribute(this->msg, OBJECT_TYPES, ObjectType, OBJECT_TYPES::UNKNOWN, );

                // # Binary value to show if the object is static or dynamic (1: dynamic, 0: static)
                std_attribute(this->msg, bool, DynamticObj, false, );

                // Predictions for the object. It is not required.
                // carma_perception_msgs/PredictedState[] predictions
            };

        }
    }

}; // namespace tmx
#endif
