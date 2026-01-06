#pragma once
#include <string>
#include <vector>
#include "PluginLog.h"
#include <jsoncpp/json/json.h>

namespace TelematicBridge
{
    struct snmpConfig
    {
        std::string privProtocol;
        std::string securityLevelKey;
        std::string authProtocolKey;
        std::string authPassPhraseKey;
        std::string userKey;
        std::string privPassPhrase;
        std::string rsuMIBVersionKey;
    };

    struct rsuEndpoint
    {
        std::string ip;
        int port;
    };

    struct rsuConfig
    {
        std::string action;
        std::string event;
        rsuEndpoint rsu;
        snmpConfig snmp;
    };

    static CONSTEXPR const char *RSUCONFIG_KEY = "rsuconfigs";                   // location key used to find location value from JSON
    static CONSTEXPR const char *RSU_KEY = "rsu";
    static CONSTEXPR const char *IP_KEY = "ip";           // testing_type key used to find testing_type value from JSON
    static CONSTEXPR const char *PORT_KEY = "port";               // event_name key used to find event_name value from JSON
    static CONSTEXPR const char *ACTION_KEY = "action";                     // unit_id key used to find unit_id value from JSON
    static CONSTEXPR const char *EVENT_KEY = "event";                 // unit_name key used to find unit_name value from JSON
    static CONSTEXPR const char *SNMP_KEY = "snmp";
    static CONSTEXPR const char *USER_KEY = "user";
    static CONSTEXPR const char *PRIVACY_PROTOCOL_KEY = "privacyprotocol";                 // unit_type key used to find unit_type value from JSON
    static CONSTEXPR const char *AUTH_PROTOCOL_KEY = "authprotocol";               // topic_name key used to find topic_name value from JSON
    static CONSTEXPR const char *AUTH_PASS_PHRASE_KEY = "authpassphrase";                 // timestamp key used to find timestamp value from JSON
    static CONSTEXPR const char *PRIVACY_PASS_PHRASE_KEY = "privacypassphrase";
    static CONSTEXPR const char *RSU_MIB_VERSION_KEY = "rsumibversion";                     // payload key used to find payload value from JSON
    static CONSTEXPR const char *SECURITY_LEVEL_KEY = "securitylevel";                       // topics key used to find topics value from JSON

    // Constants
    static constexpr int RSU_STATUS_MONITOR_INTERVAL_IN_HZ = 10;
    // Required keys for RSUConfig
    const std::vector<std::string> REQUIRED_RSU_CONFIG_KEYS = {"action", "event", "rsu","snmp"};
    // Required keys for RSUendpoint
    const std::vector<std::string> REQUIRED_RSU_KEYS = {"ip","port"};
    // Required keys for SNMP config
    const std::vector<std::string> REQUIRED_SNMP_KEYS = {
        USER_KEY,
        PRIVACY_PROTOCOL_KEY,
        AUTH_PROTOCOL_KEY,
        AUTH_PASS_PHRASE_KEY,
        PRIVACY_PASS_PHRASE_KEY,
        RSU_MIB_VERSION_KEY,
        SECURITY_LEVEL_KEY
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

    std::string toLower(std::string s);
    Json::Value convertKeysToLowerCase(const Json::Value& input);

}