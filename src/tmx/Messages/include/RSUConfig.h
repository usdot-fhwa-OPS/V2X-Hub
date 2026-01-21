#pragma once
#include <tmx/messages/message.hpp>
#include <RSUEndpoint.h>
#include <RSUSnmpConfig.h>

namespace tmx::messages{
    struct RSUConfig
    {
        RSUConfig(): action(""), event(""), status(""), rsu(RSUEndpoint("127.0.0.1",161)), snmp(RSUSnmpConfig()) {}

        RSUConfig(const std::string& action,const std::string& event,const std::string& status, const RSUEndpoint& rsuEndpoint,const RSUSnmpConfig& snmpConfig):
                action(action), event(event), status(status), rsu(rsuEndpoint), snmp(snmpConfig) {}

        static message_tree_type to_tree(const RSUConfig& rsuConfig){
            message_tree_type tree;
            tree.put("action",rsuConfig.action);
            tree.put("event", rsuConfig.event);
            tree.put("status", rsuConfig.status);
            tree.add_child("rsu", RSUEndpoint::to_tree(rsuConfig.rsu));
            tree.add_child("snmp", RSUSnmpConfig::to_tree(rsuConfig.snmp));

            return tree;
        }

        static RSUConfig from_tree(const message_tree_type& tree)
        {
            RSUConfig rsuConfig;
            rsuConfig.action = tree.get<std::string>("action");
            rsuConfig.event = tree.get<std::string>("event");
            rsuConfig.status = tree.get<std::string>("status", "");
            rsuConfig.rsu = RSUEndpoint::from_tree(tree.get_child("rsu"));
            rsuConfig.snmp = RSUSnmpConfig::from_tree(tree.get_child("snmp"));

            return rsuConfig;
        }


        std::string action;
        std::string  event;
        std::string status;
        RSUEndpoint rsu;
        RSUSnmpConfig snmp;

    };
}