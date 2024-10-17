#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string.hpp>
#include "RSUConfigurationException.h"

namespace RSUHealthMonitor
{
    static constexpr const char *RSUSKey = "RSUS";
    static constexpr const char *RSUIpKey = "RSUIp";
    static constexpr const char *SNMPPortKey = "SNMPPort";
    static constexpr const char *UserKey = "User";
    static constexpr const char *AuthPassPhraseKey = "AuthPassPhrase";
    static constexpr const char *RSUMIBVersionKey = "RSUMIBVersion";
    static constexpr const char *SecurityLevelKey = "SecurityLevel";
    static constexpr const char *RSU4_1_str = "RSU4.1";
    static constexpr const char *RSU1218_str = "NTCIP1218";

    enum class RSUMibVersion
    {
        UNKOWN_MIB_V = 0,
        RSUMIB_V_4_1 = 1,
        RSUMIB_V_1218 = 2
    };

    struct RSUConfiguration
    {
        std::string rsuIp;
        uint16_t snmpPort;
        std::string user;
        std::string authPassPhrase;
        std::string securityLevel;
        RSUMibVersion mibVersion;
        friend std::ostream &operator<<(std::ostream &os, const RSUConfiguration &config);
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
        Json::Value parseJson(const std::string &rsuConfigsStr) const;
        RSUMibVersion strToMibVersion(std::string &mibVersionStr) const;

    public:
        RSUConfigurationList() = default;
        ~RSUConfigurationList() = default;
        /**
         * @brief Parse RSUs configrations in JSON string representation, and update the memeber of list of RSUConfiguration struct.
         * @param rsuConfigsStr A JSON string includes all RSUs related configrations.
         */
        void parseRSUs(const std::string &rsuConfigsStr);
        /**
         * @brief Get a list of RSUConfiguration struct.
         */
        std::vector<RSUConfiguration> getConfigs() const;
    };

} // namespace RSUHealthMonitor
