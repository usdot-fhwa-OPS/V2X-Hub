#pragma once
#include <string>
#include <vector>
#include "PluginLog.h"
#include <jsoncpp/json/json.h>
#include <unordered_map>


namespace TelematicBridge
{
    /* Struct to store snmp configuration
    */
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

    enum class action
    {
        add,
        remove,
        update,
        unknown
    };

    struct rsuConfig
    {
        action actionType;
        std::string event;
        rsuEndpoint rsu;
        snmpConfig snmp;
    };


    class truConfigWorker{

        private:
        int test;
        std::string _unitId;   // Unique identifier for each unit
        std::string _unitName;
        int16_t _maxConnections=5;   // Number of maximum RSUs supported by plugin
        int16_t _pluginHeartBeatInterval; // Configurable interval at which the plugin heartbeat should be monitored
        int16_t _healthMonitorPluginHeartbeatInterval; // Configurable interval at which the RSU Health Monitor heartbeat should be monitored
        int16_t _rsuStatusMonitorInterval; // Configurable interval at which the RSU status should be monitored
        std::unordered_map<std::string, rsuConfig> _truRegistrationMap;
        int64_t _lastUpdateTimestamp;

        //Unit Configs Key static keys
        static CONSTEXPR const char *TRU_UNIT_CONFIG_KEY = "unitConfig";
        static CONSTEXPR const char *TRU_UNIT_ID_KEY = "unitID";
        static CONSTEXPR const char *TRU_UNIT_NAME_KEY = "name";
        static CONSTEXPR const char *TRU_MAX_CONNECTIONS_KEY = "maxConnections";
        static CONSTEXPR const char *TRU_PLUGIN_HEARTBEAT_INTERVAL_KEY = "pluginHeartbeatInterval";
        static CONSTEXPR const char *TRU_HEALTH_MONITOR_PLUGIN_HEARTBEAT_INTERVAL_KEY = "healthMonitorPluginHeartbeatInterval";
        static CONSTEXPR const char  *TRU_RSU_STATUS_MONITOR_INTERVAL_KEY = "rsuStatusMonitorINterval";

        //RSU configs static keys
        static CONSTEXPR const char *TRU_RSU_KEY = "rsu";                                       // RSU key to find rsu endpoint configurations from JSON
        static CONSTEXPR const char *TRU_IP_KEY = "ip";                                         // IP key to find rsu ip from JSON
        static CONSTEXPR const char *TRU_PORT_KEY = "port";                                     // Port key to find rsu port from JSON
        static CONSTEXPR const char *TRU_RSU_CONFIGS_KEY = "rsuConfigs";
        static CONSTEXPR const char *TRU_ACTION_KEY = "action";                                 // action key used to find registration action value from JSON
        static CONSTEXPR const char *TRU_EVENT_KEY = "event";                                   // event key used to find rsu event value from JSON
        static CONSTEXPR const char *TRU_SNMP_KEY = "snmp";                                     // snmp key used to find rsu snmp configuration from JSON
        static CONSTEXPR const char *TRU_USER_KEY = "user";
        static CONSTEXPR const char *TRU_PRIVACY_PROTOCOL_KEY = "privacyprotocol";
        static CONSTEXPR const char *TRU_AUTH_PROTOCOL_KEY = "authprotocol";
        static CONSTEXPR const char *TRU_AUTH_PASS_PHRASE_KEY = "authpassphrase";
        static CONSTEXPR const char *TRU_PRIVACY_PASS_PHRASE_KEY = "privacypassphrase";
        static CONSTEXPR const char *TRU_RSU_MIB_VERSION_KEY = "rsumibversion";
        static CONSTEXPR const char *TRU_SECURITY_LEVEL_KEY = "securitylevel";

        static CONSTEXPR const char *TRU_TIMESTAMP_KEY = "timestamp";

        static CONSTEXPR const char *TRU_RESPONSE_STATUS_KEY = "status";

        // Required Keys for validation

        // Required keys for RSUConfig
        const std::vector<std::string> REQUIRED_RSU_CONFIG_KEYS = {TRU_ACTION_KEY, TRU_EVENT_KEY, TRU_RSU_KEY,TRU_SNMP_KEY};
        // Required keys for RSUendpoint
        const std::vector<std::string> REQUIRED_RSU_KEYS = {TRU_IP_KEY,TRU_PORT_KEY};
        const std::vector<std::string> REQUIRED_SNMP_KEYS = {
            TRU_USER_KEY,
            TRU_PRIVACY_PROTOCOL_KEY,
            TRU_AUTH_PROTOCOL_KEY,
            TRU_AUTH_PASS_PHRASE_KEY,
            TRU_PRIVACY_PASS_PHRASE_KEY,
            TRU_RSU_MIB_VERSION_KEY,
            TRU_SECURITY_LEVEL_KEY
        };

        //Helper utility functions
        constexpr const char* actionToString(action ac);
        action stringToAction(const std::string& str);
        void validateRequiredKeys(const Json::Value& json, const std::vector<std::string>& requiredKeys);

        bool setJsonArrayToUnitConfig(const Json::Value& message);

        /**
         * @brief Convert RSUConfigs from a message to vector of rsuConfig
         * @param message The JSON::Value containing "RSUConfigs" array
         * @param rsus Reference to vector to populate
         * @return bool true if successful, false otherwise
         */
        bool setJsonArrayToRsuConfigList(const Json::Value& message);


        /**
         * @brief Process a single RSU configuration item.
         * @param rsuConfigItem JSON object containing RSU configuration
         */
        bool processRSUConfig(const Json::Value& rsuConfigJson);

        bool processAddAction(rsuConfig config);

        bool processUpdateAction(rsuConfig config);

        bool processDeleteAction(rsuConfig config);



        /**
         * @brief Convert a vector of rsuConfig structures to JSON array
         * @param rsus Vector of rsuConfig structures
         * @return Json::Value containing array of RSU configurations
         */
        Json::Value getRsuConfigListAsJsonArray();

        Json::Value getUnitConfigAsJsonArray();

        public:
        truConfigWorker() = default;

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
         * @brief Load RSU configurations from a JSON file
         * @param configPath Path to the JSON configuration file
         * @param rsus Reference to vector to populate with RSU configs
         * @return bool true if successful, false otherwise
         */
        bool loadRSUConfigListFromFile(const std::string& configPath);

        bool updateTRUStatus(const Json::Value& jsonVal);

        Json::Value getTruConfigAsJsonArray();

        Json::Value getTRUConfigResponse(bool isRegistrationSuccessful);

        std::string getUnitId();
    };

}