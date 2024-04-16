#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <jsoncpp/json/json.h>
#include "RSUConfigurationException.h"

namespace RSUHealthMonitor
{
    static constexpr const char *RSUIpKey = "RSUIp";
    static constexpr const char *SNMPPortKey = "SNMPPort";
    static constexpr const char *UserKey = "User";
    static constexpr const char *AuthPassPhraseKey = "AuthPassPhrase";
    static constexpr const char *RSUMIBVersionKey = "RSUMIBVersion";
    static constexpr const char *SecurityLevelKey = "SecurityLevel";
    struct RSUConfiguration
    {
        std::string rsuIp;
        uint16_t snmpPort;
        std::string user;
        std::string authPassPhrase;
        std::string securityLevel = "authPriv";
        std::string mibVersion;
        friend std::ostream &operator<<(std::ostream &os, const RSUConfiguration &config)
        {
            os << RSUIpKey << ": " << config.rsuIp << ", " << SNMPPortKey << ": " << config.snmpPort << ", " << UserKey << ": " << config.user << ", " << AuthPassPhraseKey << ": " << config.authPassPhrase << ", " << SecurityLevelKey << ": " << config.securityLevel << ", " << RSUMIBVersionKey << ": " << config.mibVersion;
            return os;
        }
    };

    class RSUConfigurationList
    {
    private:
        std::vector<RSUConfiguration> configs;
        /***
         * @brief Parse JSON string and return the corresponding JSON value.
         * @param rsuConfigsStr  A JSON string includes all RSUs related configrations.
         * @return JSON::Value A JSON object that includes RSUS information.
         */
        Json::Value parseJson(std::string rsuConfigsStr);

    public:
        RSUConfigurationList() = default;
        ~RSUConfigurationList() = default;
        /**
         * @brief Parse RSUs configrations in JSON string representation, and update the memeber of list of RSUConfiguration struct.
         * @param rsuConfigsStr A JSON string includes all RSUs related configrations.
         */
        void parseRSUs(std::string rsuConfigsStr);
        /**
         * @brief Get a list of RSUConfiguration struct.
         */
        std::vector<RSUConfiguration> getConfigs();
    };

} // namespace RSUHealthMonitor
