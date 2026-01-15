#pragma once
#include <tmx/messages/message.hpp>

namespace tmx::messages{
    struct RSUSnmpConfig
    {
        RSUSnmpConfig(): privacyProtocol(""), securityLevel(""),
                 authProtocol(""), authPassPhrase(""), user(""), privacyPassPhrase(""), rsuMIBVersion("") {}

        RSUSnmpConfig(const std::string& privacyProtocol, const std::string& securityLevel, const std::string&authProtocol,
        const std::string& authPassPhrase, const std::string& user, const std::string& privacyPassPhrase, const std::string& rsuMIBVersion):
                privacyProtocol(privacyProtocol), securityLevel(securityLevel), authProtocol(authProtocol),authPassPhrase(authPassPhrase),
                user(user), privacyPassPhrase(privacyPassPhrase), rsuMIBVersion(rsuMIBVersion){}

        static message_tree_type to_tree(const RSUSnmpConfig& snmpConfig){
            message_tree_type tree;
            tree.put("privacyProtocol", snmpConfig.privacyProtocol);
            tree.put("securityLevel", snmpConfig.securityLevel);
            tree.put("authProtocol", snmpConfig.authProtocol);
            tree.put("authPassPhrase", snmpConfig.authPassPhrase);
            tree.put("user", snmpConfig.user);
            tree.put("privacyPassPhrase", snmpConfig.privacyPassPhrase);
            tree.put("rsuMIBVersion", snmpConfig.rsuMIBVersion);

            return tree;
        }

        static RSUSnmpConfig from_tree(const message_tree_type& tree){
            RSUSnmpConfig snmpConfig;
            snmpConfig.privacyProtocol = tree.get<std::string>("privacyProtocol");
            snmpConfig.securityLevel = tree.get<std::string>("securityLevel");
            snmpConfig.authProtocol = tree.get<std::string>("authProtocol");
            snmpConfig.authPassPhrase = tree.get<std::string>("authPassPhrase");
            snmpConfig.user = tree.get<std::string>("user");
            snmpConfig.privacyPassPhrase = tree.get<std::string>("privacyPassPhrase");
            snmpConfig.rsuMIBVersion = tree.get<std::string>("rsuMIBVersion");

            return snmpConfig;
        }

        std::string privacyProtocol;
        std::string securityLevel;
        std::string authProtocol;
        std::string authPassPhrase;
        std::string user;
        std::string privacyPassPhrase;
        std::string rsuMIBVersion;
    };
}