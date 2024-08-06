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

            // Cartesian positiion of object. Assumed to be ENU coordinate frame.
            typedef struct Position{
                double x,  y, z;
                Position(){};
                Position(double x, double y, double z):x(x),y(y),z(z){};
                static message_tree_type to_tree(const Position& pos){
                    message_tree_type tree;
                    tree.put("x", pos.x);
                    tree.put("y", pos.y);
                    tree.put("z", pos.z);
                    return tree;
                }
                static Position from_tree(const message_tree_type& tree){
                    Position pos;
                    pos.x = tree.get<double>("x");
                    pos.y = tree.get<double>("y");
                    pos.z = tree.get<double>("z");
                    return pos;
                }
            } Position;            
            object_attribute(Position, Position);

            typedef struct Covariance{
                std::string value;                
                Covariance(){};
                Covariance( std::string value):value(value){};
                static message_tree_type to_tree(const Covariance& cov){
                    message_tree_type tree;
                    tree.put("", cov.value);
                    return tree;
                }
                static Covariance from_tree(const message_tree_type& tree){
                    Covariance cov;
                    cov.value = tree.get<std::string>("");
                    return cov;
                }
            } Covariance;
            array_attribute(Covariance, PositionCovariance);
            
            // Cartesian velocity vector of object. Assumed to be ENU coordinate frame.
            typedef struct Velocity{
                double x, y, z;
                Velocity(){};
                Velocity(double x, double y, double z):x(x),y(y),z(z){};
                static message_tree_type to_tree(const Velocity& velocity){
                    message_tree_type tree;
                    tree.put("x", velocity.x);
                    tree.put("y", velocity.y);
                    tree.put("z", velocity.z);
                    return tree;
                }
                static Velocity from_tree(const message_tree_type& tree){
                    Velocity velocity;
                    velocity.x = tree.get<double>("x");
                    velocity.y = tree.get<double>("y");
                    velocity.z = tree.get<double>("z");
                    return velocity;
                }
            } Velocity;
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

            //Length, width and height of object in meter.
            typedef struct Size{
                double length;
                double width;
                double height;
                Size(){};
                Size(double length, double width, double height): length(length), width(width), height(height){};
                static message_tree_type to_tree(const Size& size){
                    message_tree_type tree;
                    tree.put("length", size.length);
                    tree.put("width", size.width);
                    tree.put("height", size.height);
                    return tree;
                }
                static Size from_tree(const message_tree_type & tree){
                    Size size;
                    size.length = tree.get<double>("length");
                    size.width = tree.get<double>("width");
                    size.height = tree.get<double>("height");
                    return size;
                }
            } Size;
            object_attribute(Size, Size);
           
        };

    }

}; // namespace tmx
#endif
