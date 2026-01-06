#include "RSUConfigWorker.h"

using namespace tmx::utils;
using namespace std;


namespace TelematicBridge
{

    void validateRequiredKeys(const Json::Value& json, const std::vector<std::string>& requiredKeys)
    {
        for(const auto& key : requiredKeys)
        {
            if (!json.isMember(key)) {
                std::string error = "Missing required key" + key;
                throw std::runtime_error(error);
            }
        }
    }

    bool processRSUConfig(const Json::Value& rsuConfigJson, int16_t& maxConnections, std::vector<rsuConfig>& rsuRegisteredList)
    {

        try
        {
            if(rsuConfigJson.isMember(RSU_KEY) && rsuConfigJson[RSU_KEY].isObject())
            {
                validateRequiredKeys(rsuConfigJson, REQUIRED_RSU_CONFIG_KEYS);

                rsuConfig config;
                config.action = rsuConfigJson[ACTION_KEY].asString();
                config.event = rsuConfigJson[EVENT_KEY].asString();

                Json::Value rsu = rsuConfigJson[RSU_KEY];
                validateRequiredKeys(rsu, REQUIRED_RSU_KEYS);

                config.rsu.ip = rsu.get(IP_KEY, "").asString();
                config.rsu.port = rsu.get(PORT_KEY, 161).asInt();

                PLOG(logINFO)<<"Added RSU:"<< config.rsu.ip<<":"<< config.rsu.port;

                // Check if RSU is already registered
                for(auto registeredRsu : rsuRegisteredList)
                {
                    if (config.rsu.ip == registeredRsu.rsu.ip){
                        PLOG(logDEBUG) << "RSU "<< config.rsu.ip << " already registered.";
                        return false;
                    }
                }

                // Log error if max number of connections has been reached
                if(rsuRegisteredList.size() >= maxConnections)
                {
                    PLOG(logWARNING) << "Max number of connections reached("<<maxConnections <<") . RSU " << config.rsu.ip << "will not be added.";
                    return false;
                }

                // Log error if RSU is defined without snmp configuration
                if(!rsuConfigJson.isMember(SNMP_KEY) || !rsuConfigJson[SNMP_KEY].isObject()){
                    PLOG(logERROR) << "Missing or invalid snmp object for RSU with IP "<< config.rsu.ip << " , ignoring request to register.";
                    return false;
                }

                Json::Value snmp = rsuConfigJson[SNMP_KEY];
                validateRequiredKeys(snmp, REQUIRED_SNMP_KEYS);

                config.snmp.userKey = snmp.get(USER_KEY, "").asString();
                config.snmp.privProtocol = snmp.get(PRIVACY_PROTOCOL_KEY,"").asString();
                config.snmp.authProtocolKey = snmp.get(AUTH_PROTOCOL_KEY,"").asString();
                config.snmp.authPassPhraseKey = snmp.get(AUTH_PASS_PHRASE_KEY,"").asString();
                config.snmp.privPassPhrase = snmp.get(PRIVACY_PASS_PHRASE_KEY,"").asString();
                config.snmp.rsuMIBVersionKey = snmp.get(RSU_MIB_VERSION_KEY,"").asString();
                config.snmp.securityLevelKey = snmp.get(SECURITY_LEVEL_KEY,"").asString();

                //Add configured RSU to list
                rsuRegisteredList.push_back(config);
                PLOG(logDEBUG) << "Added RSU "<< config.rsu.ip << " to registered list";
                return true;
            }
        } catch(const std::exception& e){
            //Catch error for RSU configuration without crashing
            PLOG(logERROR)<<"Failed to process RSU config: "<< e.what();
            return false;
        }

    }


    Json::Value rsuConfigToJsonValue(const rsuConfig& config) {
        Json::Value json;

        json[ACTION_KEY] = config.action;
        json[EVENT_KEY] = config.event;

        // Create RSU endpoint object
        Json::Value rsuEndpoint;
        rsuEndpoint[IP_KEY] = config.rsu.ip;
        rsuEndpoint[PORT_KEY] = config.rsu.port;

        // Create SNMP config object
        Json::Value snmpConfig;
        snmpConfig[USER_KEY] = config.snmp.userKey;
        snmpConfig[PRIVACY_PROTOCOL_KEY] = config.snmp.privProtocol;
        snmpConfig[AUTH_PROTOCOL_KEY] = config.snmp.authProtocolKey;
        snmpConfig[AUTH_PASS_PHRASE_KEY] = config.snmp.authPassPhraseKey;
        snmpConfig[PRIVACY_PASS_PHRASE_KEY] = config.snmp.privPassPhrase;
        snmpConfig[RSU_MIB_VERSION_KEY] = config.snmp.rsuMIBVersionKey;
        snmpConfig[SECURITY_LEVEL_KEY] = config.snmp.securityLevelKey;

        // Add nested objects to main json
        json[RSU_KEY] = rsuEndpoint;
        json[SNMP_KEY] = snmpConfig;

        return json;
    }


    bool jsonValueToRsuConfig(const Json::Value& json, rsuConfig& config) {
        try {
            // Validate top-level structure
            if (!json.isMember(RSU_KEY) || !json[RSU_KEY].isObject()) {
                PLOG(logERROR) << "Missing or invalid 'rsu' Endpoint object";
                return false;
            }

            if (!json.isMember(SNMP_KEY) || !json[SNMP_KEY].isObject()) {
                PLOG(logERROR) << "Missing or invalid rsu 'snmp' object";
                return false;
            }

            Json::Value rsuEndpoint = json[RSU_KEY];
            Json::Value snmpConfig = json[SNMP_KEY];

            // Extract RSU fields
            config.rsu.ip = rsuEndpoint[IP_KEY].asString();
            config.rsu.port = rsuEndpoint.get(PORT_KEY, 8080).asInt();


            // Extract SNMP fields
            config.snmp.userKey = snmpConfig[USER_KEY].asString();
            config.snmp.privProtocol = snmpConfig[PRIVACY_PROTOCOL_KEY].asString();
            config.snmp.authProtocolKey = snmpConfig[AUTH_PROTOCOL_KEY].asString();
            config.snmp.authPassPhraseKey = snmpConfig[AUTH_PASS_PHRASE_KEY].asString();
            config.snmp.privPassPhrase = snmpConfig[PRIVACY_PASS_PHRASE_KEY].asString();
            config.snmp.rsuMIBVersionKey = snmpConfig[RSU_MIB_VERSION_KEY].asString();
            config.snmp.securityLevelKey = snmpConfig[SECURITY_LEVEL_KEY].asString();

            config.event = json.get(EVENT_KEY, "").asString();
            config.action = json.get(ACTION_KEY,"").asString();

            return true;

        } catch (const std::exception& e) {
            PLOG(logERROR) << "Failed to convert JSON to rsuConfig: " << e.what();
            return false;
        }
    }

    bool rsuConfigMessageToVector(const Json::Value& jsonArray, std::vector<rsuConfig>& rsuConfigList) {

        if (!jsonArray.isArray()) {
            PLOG(logERROR) << "Input is not a JSON array";
            return false;
        }

        bool success = true;

        for (Json::ArrayIndex i = 0; i < jsonArray.size(); ++i) {
            rsuConfig config;
            if (jsonValueToRsuConfig(jsonArray[i], config)) {
                rsuConfigList.push_back(config);
            } else {
                PLOG(logERROR) << "Failed to parse RSU config at index " << i;
                success = false;
                // Continue processing other configs
            }
        }

        return success;
    }

    bool loadRSUConfigListFromFile(const std::string& configPath, std::vector<rsuConfig>& rsuConfigList) {
        try{
            std::ifstream configFile(configPath);
            if (!configFile.is_open()) {
                PLOG(logWARNING) << "Config file not found: " << configPath;
                return false;
            }

            Json::Value root;
            Json::CharReaderBuilder builder;
            std::string errs;

            if (!Json::parseFromStream(builder, configFile, &root, &errs)) {
                PLOG(logERROR) << "Failed to parse config file: " << errs;
                return false;
            }

            auto rootLowerCase = convertKeysToLowerCase(root);

            if (rootLowerCase.isMember("rsuconfigs") && rootLowerCase["rsuconfigs"].isArray()) {
                return rsuConfigMessageToVector(rootLowerCase["rsuconfigs"], rsuConfigList);
            }
        } catch (...)
        {
            PLOG(logERROR) << "Could not load config from from file, RSU configuration needs to be defined on the UI.";
            return false;
        }
    }


    Json::Value rsuConfigListToJsonArray(const std::vector<rsuConfig>& rsuConfigList) {
        Json::Value jsonArray(Json::arrayValue);

        for (const auto& config : rsuConfigList) {
            jsonArray.append(rsuConfigToJsonValue(config));
        }
        return jsonArray;
    }

    std::string toLower(std::string s) {
    std::string lower_s = s;
    std::transform(lower_s.begin(), lower_s.end(), lower_s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return lower_s;
    }

    // Recursive function to convert all keys in a Json::Value to lowercase
    Json::Value convertKeysToLowerCase(const Json::Value& input) {
        Json::Value output;

        if (input.isObject()) {
            for (Json::ValueConstIterator it = input.begin(); it != input.end(); ++it) {
                std::string originalKey = it.name();
                std::string lowerKey = toLower(originalKey);
                // Recursively process nested values
                output[lowerKey] = convertKeysToLowerCase(*it);
            }
        } else if (input.isArray()) {
            // If it's an array, iterate through elements and recursively process them
            for (const auto& item : input) {
                output.append(convertKeysToLowerCase(item));
            }
        } else {
            // For primitive types (string, int, bool, etc.), just copy the value
            output = input;
        }
        return output;
    }


}