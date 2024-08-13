#pragma once
#include <tmx/messages/message.hpp>
namespace tmx::messages
{
    // Cartesian positiion of object. Assumed to be ENU coordinate frame.
    struct Position{
        double x;
        double y;
        double z;
        Position()=default;
        explicit Position(double x, double y, double z):x(x),y(y),z(z){};
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
    };       
}