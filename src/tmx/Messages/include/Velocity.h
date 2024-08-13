#pragma once
#include <tmx/messages/message.hpp>
namespace tmx
{
    namespace messages
    {
       // Cartesian velocity vector of object. Assumed to be ENU coordinate frame.
        typedef struct Velocity{
            double x;
            double y;
            double z;
            Velocity()=default;
            explicit Velocity(double x, double y, double z):x(x),y(y),z(z){};
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
    }
}