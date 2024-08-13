#pragma once
#include <tmx/messages/message.hpp>
namespace tmx::messages
{
    //Length, width and height of object in meter.
    typedef struct Size{
        double length;
        double width;
        double height;
        Size()=default;
        explicit Size(double length, double width, double height): length(length), width(width), height(height){};
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
}