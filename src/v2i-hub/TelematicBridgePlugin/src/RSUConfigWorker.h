#pragma once
#include <string>
#include <vector>
#include "PluginLog.h"
#include <jsoncpp/json/json.h>

namespace TelematicBridge
{
    struct snmpConfig
    {
        std::string privProtocol = "PrivacyProtocol";
        std::string securityLevelKey="SecurityLevel";
        std::string authProtocolKey="AuthProtocol";
        std::string authPassPhraseKey="AuthPassPhrase";
        std::string userKey="User";
        std::string privPassPhrase="PrivacyPassPhrase";
        std::string rsuMIBVersionKey="RSUMIBVersion";
    };

    struct rsuEndpoint
    {
        std::string ip="127.0.0.1";
        std::string port="8080";
    };

    struct rsuConfig
    {
        std::string action;
        std::string event;
        rsuEndpoint rsu;
        snmpConfig snmp;
    };


    // Constants
    static constexpr int RSU_STATUS_MONITOR_INTERVAL_IN_HZ = 10;
    // Required keys for RSUConfig
    const std::vector<std::string> REQUIRED_RSU_CONFIG_KEYS = {"action", "event", "rsu","snmp"};
    // Required keys for RSUendpoint
    const std::vector<std::string> REQUIRED_RSU_KEYS = {"IP","Port"};
    // Required keys for SNMP config
    const std::vector<std::string> REQUIRED_SNMP_KEYS = {
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
    bool processRSUConfig(const Json::Value& rsuConfigJson, int16_t& maxConnections, std::vector<rsuConfig>& rsuRegisteredList);

    /**
     * @brief Validate that a JSON object contains all required keys
     * @param json The JSON object to validate
     * @param requiredKeys Vector of required key names
     * @throws std::runtime_error if any required key is missing
     */
    void validateRequiredKeys(const Json::Value& json, const std::vector<std::string>& requiredKeys);

    /**
     * @brief Convert an rsuConfig structure to JSON::Value
     * @param rsu The rsuConfig structure to convert
     * @return Json::Value containing the RSU configuration
     */
    Json::Value rsuConfigToJsonValue(const rsuConfig& rsu);

    /**
     * @brief Convert a JSON::Value to an rsuConfig structure
     * @param json The JSON::Value containing RSU configuration
     * @param rsu Reference to rsuConfig structure to populate
     * @return bool true if successful, false otherwise
     */
    bool jsonValueToRsuConfig(const Json::Value& json, rsuConfig& rsu);


    /**
     * @brief Convert RSUConfigs from a message to vector of rsuConfig
     * @param message The JSON::Value containing "RSUConfigs" array
     * @param rsus Reference to vector to populate
     * @return bool true if successful, false otherwise
     */
    bool rsuConfigMessageToVector(const Json::Value& message, std::vector<rsuConfig>& rsus);

    /**
     * @brief Load RSU configurations from a JSON file
     * @param configPath Path to the JSON configuration file
     * @param rsus Reference to vector to populate with RSU configs
     * @return bool true if successful, false otherwise
     */
    bool loadRSUConfigListFromFile(const std::string& configPath, std::vector<rsuConfig>& rsus);

    /**
     * @brief Convert a vector of rsuConfig structures to JSON array
     * @param rsus Vector of rsuConfig structures
     * @return Json::Value containing array of RSU configurations
     */
    Json::Value rsuConfigListToJsonArray(const std::vector<rsuConfig>& rsus);


}