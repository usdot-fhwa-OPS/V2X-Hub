#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string.hpp>
#include <rsu/RSUSpec.h>
#include "RSUConfigurationException.h"
#include <RSURegistrationConfigMessage.h>

namespace RSUHealthMonitor
{
    static constexpr const char *RSUSKey = "rsuConfigs";
    static constexpr const char *EventKey = "event";
    static constexpr const char *RSUKey = "rsu";
    static constexpr const char *RSUIpKey = "ip";
    static constexpr const char *SNMPPortKey = "port";
    static constexpr const char *SNMPKey = "snmp";
    static constexpr const char *UserKey = "user";
    static constexpr const char *AuthProtocolKey = "authProtocol";
    static constexpr const char *PrivProtocolKey = "privacyProtocol";
    static constexpr const char *AuthPassPhraseKey = "authPassPhrase";
    static constexpr const char *PrivPassPhraseKey = "privacyPassPhrase";
    static constexpr const char *RSUMIBVersionKey = "rsuMibVersion";
    static constexpr const char *SecurityLevelKey = "securityLevel";

  

    struct RSUConfiguration
    {
        std::string rsuIp;
        uint16_t snmpPort;
        std::string user;
        std::string authProtocol;
        std::string privProtocol;
        std::string authPassPhrase;
        std::string privPassPhrase;
        std::string securityLevel;
        tmx::utils::rsu::RSU_SPEC mibVersion;
        std::string event;
        friend std::ostream &operator<<(std::ostream &os, const RSUConfiguration &config);
    };

    class RSUConfigurationList
    {
    private:
        std::vector<RSUConfiguration> configs;
        mutable std::mutex _configMutex;
        
        /***
         * @brief Parse JSON string and return the corresponding JSON value.
         * @param rsuConfigsStr  A JSON string includes all RSUs related configrations.
         * @return JSON::Value A JSON object that includes RSUS information.
         */
        Json::Value parseJson(const std::string &rsuConfigsStr) const;

    public:
        RSUConfigurationList() = default;
        ~RSUConfigurationList() = default;
        /**
         * @brief Parse RSUs configrations in JSON string representation, and update the memeber of list of RSUConfiguration struct.
         * @param rsuConfigsStr A JSON string includes all RSUs related configrations.
         */
        void parseRSUs(const std::string &rsuConfigsStr);
        /**
         * @brief Parse RSUs configrations from RSURegistrationConfigMessage, and update the memeber of list of RSUConfiguration struct.
         * @param msg RSURegistrationConfigMessage that includes all RSUs related configrations.
         */
        void parseRSUs(tmx::messages::RSURegistrationConfigMessage &msg);
        
        /**
         * @brief Add a single RSU configuration to the list
         * @param config RSUConfiguration to add
         */
        void addConfig(const RSUConfiguration &config);
        
        /**
         * @brief Clear all RSU configurations
         */
        void clearConfigs();
        
        /**
         * @brief Get a list of RSUConfiguration struct.
         */
        std::vector<RSUConfiguration> getConfigs() const;
    };

} // namespace RSUHealthMonitor
