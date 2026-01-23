#pragma once
#include <string>
#include <vector>
#include "PluginLog.h"
#include <jsoncpp/json/json.h>
#include <unordered_map>
#include "TelematicBridgeException.h"


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
        static CONSTEXPR const char *TRU_UNIT_ID_KEY = "unitId";
        static CONSTEXPR const char *TRU_UNIT_NAME_KEY = "name";
        static CONSTEXPR const char *TRU_MAX_CONNECTIONS_KEY = "maxConnections";
        static CONSTEXPR const char *TRU_PLUGIN_HEARTBEAT_INTERVAL_KEY = "bridgePluginHeartbeatInterval";
        static CONSTEXPR const char *TRU_HEALTH_MONITOR_PLUGIN_HEARTBEAT_INTERVAL_KEY = "healthMonitorPluginHeartbeatInterval";
        static CONSTEXPR const char  *TRU_RSU_STATUS_MONITOR_INTERVAL_KEY = "rsuStatusMonitorInterval";

        //RSU configs static keys
        static CONSTEXPR const char *TRU_RSU_KEY = "rsu";                                       // RSU key to find rsu endpoint configurations from JSON
        static CONSTEXPR const char *TRU_IP_KEY = "ip";                                         // IP key to find rsu ip from JSON
        static CONSTEXPR const char *TRU_PORT_KEY = "port";                                     // Port key to find rsu port from JSON
        static CONSTEXPR const char *TRU_RSU_CONFIGS_KEY = "rsuConfigs";
        static CONSTEXPR const char *TRU_ACTION_KEY = "action";                                 // action key used to find registration action value from JSON
        static CONSTEXPR const char *TRU_EVENT_KEY = "event";                                   // event key used to find rsu event value from JSON
        static CONSTEXPR const char *TRU_SNMP_KEY = "snmp";                                     // snmp key used to find rsu snmp configuration from JSON
        static CONSTEXPR const char *TRU_USER_KEY = "user";
        static CONSTEXPR const char *TRU_PRIVACY_PROTOCOL_KEY = "privacyProtocol";
        static CONSTEXPR const char *TRU_AUTH_PROTOCOL_KEY = "authProtocol";
        static CONSTEXPR const char *TRU_AUTH_PASS_PHRASE_KEY = "authPassPhrase";
        static CONSTEXPR const char *TRU_PRIVACY_PASS_PHRASE_KEY = "privacyPassPhrase";
        static CONSTEXPR const char *TRU_RSU_MIB_VERSION_KEY = "rsuMibVersion";
        static CONSTEXPR const char *TRU_SECURITY_LEVEL_KEY = "securityLevel";

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


        public:
        /**
         * @brief Default constructor for truConfigWorker
         *
         */
        truConfigWorker() = default;

        /**
         * @brief Process and validate a single RSU configuration JSON object
         *
         * Validates the RSU configuration has all required keys, checks for duplicates,
         * and populates an rsuConfig struct.
         *
         * @param rsuConfigJson JSON object containing complete RSU configuration
         * @return bool true if RSU config is valid and not a duplicate, false if
         *         validation fails or RSU already registered
         */
        bool processRSUConfig(const Json::Value& rsuConfigJson);

        /**
         * @brief Parse unit configuration from JSON array and update internal state
         *
         * Extracts unit configuration parameters from a JSON array and updates the
         * corresponding member variables (_unitId, _unitName, _maxConnections, etc.).
         *
         * @param message JSON array containing unit configuration items. Each item
         *                should be an object with unit configuration fields.
         * @return bool true if input is a valid array, false if not an array
         */
        bool setJsonArrayToUnitConfig(const Json::Value& message);

        /**
         * @brief Convert RSU configs from JSON array to internal registration map
         *
         * Processes a JSON array of RSU configurations, converting each to an rsuConfig
         * struct and adding it to the internal registration map.
         *
         * @param message JSON array containing RSU configuration objects
         * @return bool true if all configs parsed successfully, false if input is not.
         */
        bool setJsonArrayToRsuConfigList(const Json::Value& message);

        /**
         * @brief Convert an rsuConfig structure to a JSON object
         *
         * Creates a JSON representation of an RSU configuration.
         *
         * @param config rsuConfig structure to convert
         * @return Json::Value JSON object with structure:
         *         {
         *           "action": "...",
         *           "event": "...",
         *           "rsu": { "ip": "...", "port": ... },
         *           "snmp": { "user": "...", ... }
         *         }
         */
        Json::Value rsuConfigToJsonValue(const rsuConfig& rsu);

        /**
         * @brief Convert a JSON object to an rsuConfig structure
         *
         * Parses a JSON object containing RSU configuration data and populates an
         * internal rsuConfig struct.
         *
         * @param json JSON object containing RSU configuration with structure:
         *             {
         *               "event": "...",
         *               "action": "add|delete|update" (optional, defaults to add),
         *               "rsu": { "ip": "...", "port": ... },
         *               "snmp": { ... }
         *             }
         * @param config Reference to rsuConfig struct to populate
         * @return bool true if conversion successful, false if required fields missing
         *         or objects invalid
         */
        bool jsonValueToRsuConfig(const Json::Value& json, rsuConfig& rsu);

        /**
         * @brief Load TRU configuration from a JSON file
         *
         * Reads and parses a JSON configuration file containing unit configuration,
         * RSU configurations, and timestamp. Updates internal state with the loaded
         * configuration.
         *
         * @param configPath Path to the JSON configuration file
         * @return bool true if file loaded and parsed successfully, false if file not
         *         found, JSON parsing fails, or exception occurs
         */
        bool loadRSUConfigListFromFile(const std::string& configPath);

        /**
         * @brief Update TRU status from incoming JSON message
         * Processes an update message containing unit configuration, RSU configurations,
         * and timestamp.
         * @param jsonVal JSON object containing update message with structure:
         *                {
         *                  "unitConfig": [ { "unitId": "..." }, ... ],
         *                  "rsuConfigs": [ { ... }, ... ],
         *                  "timestamp": ...
         *                }
         * @return bool true if update successful.
         */
        bool updateTRUStatus(const Json::Value& jsonVal);

        /**
         * @brief Get complete TRU configuration as JSON object
         * Creates a comprehensive JSON representation of the current TRU state.
         * @return Json::Value JSON object with structure:
         *         {
         *           "unitConfig": { ... },
         *           "rsuConfigs": [ { ... }, ... ],
         *           "timestamp": <current_time_in_milliseconds>
         *         }
         */
        Json::Value getTruConfigAsJsonArray();

        /**
         * @brief Add an RSU configuration to the registration map
         *
         * Adds the provided RSU configuration to the internal registration map if the
         * maximum connection limit has not been reached. The RSU is indexed by its IP
         * address in the map.
         *
         * @param config RSU configuration to add
         * @return bool true if RSU was successfully added, false if maximum connections
         *         limit reached (map size >= _maxConnections)
         */
        bool processAddAction(rsuConfig config);

        /**
         * @brief Update an existing RSU configuration or add if not present
         *
         * Updates the RSU configuration in the registration map if it exists (matched
         * by IP address).
         *
         * @param config RSU configuration to update or add
         * @return bool true if update or add successful, false on exception
         */
        bool processUpdateAction(rsuConfig config);

        /**
         * @brief Remove an RSU configuration from the registration map
         *
         * Removes the RSU configuration from the registration map based on IP address.
         *
         * @param config RSU configuration to delete (only IP address is used for lookup)
         * @return bool true if deletion successful or RSU not found, false on exception
         */
        bool processDeleteAction(rsuConfig config);

        /**
         * @brief Get TRU configuration response with registration status
         * Creates a response message containing unit ID, simplified RSU list (IP and port
         * only), registration status, and timestamp.
         * @param isRegistrationSuccessful Boolean indicating registration outcome
         * @return Json::Value JSON object with structure:
         *         {
         *           "unitConfig": [ { "unitId": "..." } ],
         *           "rsuConfigs": [ { "ip": "...", "port": ... }, ... ],
         *           "status": "success" | "failed",
         *           "timestamp": <current_time_in_milliseconds>
         *         }
         */
        Json::Value getTRUConfigResponse(bool isRegistrationSuccessful);

        /**
         * @brief Convert action enum to string representation
         *
         * @param ac Action enum value to convert
         * @return String representation of the action.
         */
        std::string actionToString(action ac) const;

        /**
         * @brief Convert string to action enum
         *
         * @param str String representation of action ("add", "delete", etc.)
         * @return action Corresponding action enum value.
         */
        action stringToAction(const std::string& str);

        /**
         * @brief Get the current unit ID
         * @return std::string The unit ID string, empty if not set
         */
        std::string getUnitId();

        int getRsuPortByIp(const std::string &rsuIp) const;

        /**
         * @brief Get the plugin heartbeat interval
         * @return int The heartbeat interval in seconds
         */
        int getPluginHeartBeatInterval() const
        {
            return _pluginHeartBeatInterval;
        }

        /**
         * @brief Validate that a JSON object contains all required keys
         *
         * Checks that the provided JSON object contains all keys specified in the
         * requiredKeys vector. Throws an exception if any required key is missing.
         *
         * @param json JSON object to validate
         * @param requiredKeys Vector of required key names
         * @throws std::runtime_error if any required key is missing, with message
         *         "Missing required key<keyname>"
         */
        void validateRequiredKeys(const Json::Value& json, const std::vector<std::string>& requiredKeys);


        /**
         * @brief Convert all registered RSU configs to a JSON array
         *
         * Iterates through the internal registration map and converts each RSU
         * configuration to a JSON object, returning them as a JSON array.
         *
         * @return Json::Value JSON array containing all registered RSU configurations,
         *         empty array if no RSUs registered
         */
        Json::Value getRsuConfigListAsJsonArray();

        /**
         * @brief Get unit configuration as a JSON object
         * Creates a JSON object containing the current unit configuration parameters
         * @return Json::Value JSON object with structure:
         *         {
         *           "unitId": "...",
         *           "maxConnections": ...,
         *           "bridgePluginHeartbeatInterval": ...,
         *           "healthMonitorPluginHeartbeatInterval": ...,
         *           "rsuStatusMonitorINterval": ...
         *         }
         */
        Json::Value getUnitConfigAsJsonArray();
        
        /**
         * @brief Get all registered RSU configurations
         * @return std::vector<rsuConfig> Vector of all registered RSU configurations
         */
        std::vector<rsuConfig> getRegisteredRSUs() const;

        /**
         * @brief Get event name for a specific RSU by IP and port
         * @param rsuIp RSU IP address
         * @param rsuPort RSU port number
         * @return std::string Event name if RSU found, empty string otherwise
         */
        std::string getEventByRsu(const std::string &rsuIp, int rsuPort) const;
    };

}