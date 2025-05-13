#pragma once
#include <tmx/messages/message.hpp>
namespace tmx::messages
{
    // Cartesian Orientation vector of object. Assumed to be ENU coordinate frame.
    struct Orientation{
        double x;
        double y;
        double z;
        Orientation()=default;
        explicit Orientation(double x, double y, double z):x(x),y(y),z(z){};
        static message_tree_type to_tree(const Orientation& Orientation){
            message_tree_type tree;
            tree.put("x", Orientation.x);
            tree.put("y", Orientation.y);
            tree.put("z", Orientation.z);
            return tree;
        }
        static Orientation from_tree(const message_tree_type& tree){
            Orientation Orientation;
            Orientation.x = tree.get<double>("x");
            Orientation.y = tree.get<double>("y");
            Orientation.z = tree.get<double>("z");
            return Orientation;
        }
    }; 
}