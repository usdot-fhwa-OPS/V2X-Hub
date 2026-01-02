#pragma once
#include <string>
#include <vector>
#include "PluginLog.h"
#include <jsoncpp/json/json.h>

namespace TelematicBridge
{
    using snmp_st = struct snmpconfig
    {
        std::string rsuKey="RSUS";
        std::string snmpPort="SNMPPort";
        std::string userKey="User";
        std::string privProtocol = "PrivacyProtocol";
        std::string authProtocolKey="AuthProtocol";
        std::string authPassPhraseKey="AuthPassPhrase";
        std::string privPassPhrase="PrivacyPassPhrase";
        std::string rsuMIBVersionKey="RSUMIBVersion";
        std::string securityLevelKey="SecurityLevel";
    };

    using rsu_st = struct rsu_unit
    {
        std::string ip="127.0.0.1";
        std::string port="8080";
        int64_t timestamp = 0;
        std::string event = "";
        snmp_st snmp;
    };


    // Constants
    static constexpr int RSU_STATUS_MONITOR_INTERVAL_IN_HZ = 10;
    // Required keys for RSU endpoint
    const std::vector<std::string> REQUIRED_RSU_KEYS = {"IP"};
    // Required keys for SNMP config
    const std::vector<std::string> REQUIRED_SNMP_KEYS = {
        "User",
        "SNMPPort",
        "PrivacyProtocol",
        "AuthProtocol",
        "AuthPassPhrase",
        "PrivacyPassPhrase",
        "RSUMIBVersion",
        "SecurityLevel"
    };

    /**
     * @brief Process a single RSU configuration item.
     * @param rsuConfigItem JSON object containing RSU configuration
     */
    bool processRSUConfig(const Json::Value& rsuConfigJson, int16_t& maxConnections, std::vector<rsu_st>& rsuRegisteredList);

    /**
     * @brief Validate that a JSON object contains all required keys
     * @param json The JSON object to validate
     * @param requiredKeys Vector of required key names
     * @throws std::runtime_error if any required key is missing
     */
    void validateRequiredKeys(const Json::Value& json, const std::vector<std::string>& requiredKeys);

    /**
     * @brief Convert a JSON::Value to an rsu_st structure
     * @param json The JSON::Value containing RSU configuration
     * @param rsu Reference to rsu_st structure to populate
     * @return bool true if successful, false otherwise
     */
    bool jsonValueToRsu(const Json::Value& json, rsu_st& rsu);

    /**
     * @brief Convert an rsu_st structure to JSON::Value
     * @param rsu The rsu_st structure to convert
     * @return Json::Value containing the RSU configuration
     */
    Json::Value rsuToJsonValue(const rsu_st& rsu);

    /**
     * @brief Convert RSUConfigs from a message to vector of rsu_st
     * @param message The JSON::Value containing "RSUConfigs" array
     * @param rsus Reference to vector to populate
     * @return bool true if successful, false otherwise
     */
    bool rsuConfigMessageToVector(const Json::Value& message, std::vector<rsu_st>& rsus);

    /**
     * @brief Load RSU configurations from a JSON file
     * @param configPath Path to the JSON configuration file
     * @param rsus Reference to vector to populate with RSU configs
     * @return bool true if successful, false otherwise
     */
    bool loadRSUConfigFromFile(const std::string& configPath, std::vector<rsu_st>& rsus);

    /**
     * @brief Convert a vector of rsu_st structures to JSON array
     * @param rsus Vector of rsu_st structures
     * @return Json::Value containing array of RSU configurations
     */
    Json::Value rsuVectorToJsonArray(const std::vector<rsu_st>& rsus);


}