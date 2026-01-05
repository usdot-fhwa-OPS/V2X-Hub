#pragma once
#include <tmx/messages/message.hpp>
#include <RSUEndpoint.h>
#include <RSUSnmpConfig.h>

namespace tmx::messages{
    struct RSUConfig
    {
        RSUConfig(): action(""), event(""), rsu(RSUEndpoint("127.0.0.1","161")), snmp(RSUSnmpConfig()) {}

        RSUConfig(const std::string& action,const std::string& event,const RSUEndpoint& rsuEndpoint,const RSUSnmpConfig& snmpConfig):
                action(action), event(event), rsu(rsuEndpoint), snmp(snmpConfig) {}

        static message_tree_type to_tree(const RSUConfig& rsuConfig){
            message_tree_type tree;
            tree.put("action",rsuConfig.action);
            tree.put("event", rsuConfig.event);
            tree.add_child("rsu", RSUEndpoint::to_tree(rsuConfig.rsu));
            tree.add_child("snmp", RSUSnmpConfig::to_tree(rsuConfig.snmp));

            return tree;
        }

        static RSUConfig from_tree(const message_tree_type& tree)
        {
            RSUConfig rsuConfig;
            rsuConfig.action = tree.get<std::string>("action");
            rsuConfig.event = tree.get<std::string>("event");
            rsuConfig.rsu = RSUEndpoint::from_tree(tree.get_child("rsu"));
            rsuConfig.snmp = RSUSnmpConfig::from_tree(tree.get_child("snmp"));

            return rsuConfig;
        }


        std::string action;
        std::string  event;
        RSUEndpoint rsu;
        RSUSnmpConfig snmp;

    };
}