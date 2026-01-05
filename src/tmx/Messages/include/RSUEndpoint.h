#pragma once
#include <tmx/messages/message.hpp>


namespace tmx::messages{

    struct RSUEndpoint{
        RSUEndpoint(): ip(""), port(8080) {}
        RSUEndpoint(std::string rsuIp, int rsuPort): ip(rsuIp), port(rsuPort) {}

        static message_tree_type to_tree(const RSUEndpoint& rsu){
            message_tree_type tree;
            tree.put("ip", rsu.ip);
            tree.put("port", rsu.port);

            return tree;
        }

        static RSUEndpoint from_tree(const message_tree_type& tree){
            RSUEndpoint rsu("127.0.0.1", 8080);
            rsu.ip = tree.get<std::string>("ip");
            rsu.port = tree.get<int>("port");
            return rsu;
        }

        std::string ip;
        int port;
    };
}