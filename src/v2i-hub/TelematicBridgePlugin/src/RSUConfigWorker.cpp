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
        if(rsuConfigJson.isMember("rsu") && rsuConfigJson["rsu"].isObject())
        {
            try
            {
                validateRequiredKeys(rsuConfigJson, REQUIRED_RSU_CONFIG_KEYS);

                rsuConfig config;
                config.action = rsuConfigJson["action"].asString();
                config.event = rsuConfigJson["event"].asString();

                Json::Value rsu = rsuConfigJson["rsu"];
                validateRequiredKeys(rsu, REQUIRED_RSU_KEYS);

                config.rsu.ip = rsu.get("IP", "").asString();
                config.rsu.port = rsu.get("Port", 161).asInt();

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
                if(!rsuConfigJson.isMember("snmp") || !rsuConfigJson["snmp"].isObject()){
                    PLOG(logERROR) << "Missing or invalid snmp object for RSU with IP "<< config.rsu.ip << " , ignoring request to register.";
                    return false;
                }

                Json::Value snmp = rsuConfigJson["snmp"];
                validateRequiredKeys(snmp, REQUIRED_SNMP_KEYS);

                config.snmp.userKey = snmp.get("User", "").asString();
                config.snmp.privProtocol = snmp.get("PrivacyProtocol","").asString();
                config.snmp.authProtocolKey = snmp.get("AuthProtocol","").asString();
                config.snmp.authPassPhraseKey = snmp.get("AuthPassPhrase","").asString();
                config.snmp.privPassPhrase = snmp.get("PrivacyPassPhrase","").asString();
                config.snmp.rsuMIBVersionKey = snmp.get("RSUMIBVersion","").asString();
                config.snmp.securityLevelKey = snmp.get("SecurityLevel","").asString();

                //Add configured RSU to list
                rsuRegisteredList.push_back(config);
                PLOG(logDEBUG) << "Added RSU "<< config.rsu.ip << " to registered list";
                return true;
            }
            catch(const std::exception& e)
            {
                //Catch error for RSU configuration without crashing
                PLOG(logERROR)<<"Failed to process RSU config: "<< e.what();
                return false;
            }

        }
    }


    Json::Value rsuConfigToJsonValue(const rsuConfig& config) {
        Json::Value json;

        json["action"] = config.action;
        json["event"] = config.event;

        // Create RSU endpoint object
        Json::Value rsuEndpoint;
        rsuEndpoint["IP"] = config.rsu.ip;
        rsuEndpoint["Port"] = config.rsu.port;

        // Create SNMP config object
        Json::Value snmpConfig;
        snmpConfig["User"] = config.snmp.userKey;
        snmpConfig["PrivacyProtocol"] = config.snmp.privProtocol;
        snmpConfig["AuthProtocol"] = config.snmp.authProtocolKey;
        snmpConfig["AuthPassPhrase"] = config.snmp.authPassPhraseKey;
        snmpConfig["PrivacyPassPhrase"] = config.snmp.privPassPhrase;
        snmpConfig["RSUMIBVersion"] = config.snmp.rsuMIBVersionKey;
        snmpConfig["SecurityLevel"] = config.snmp.securityLevelKey;

        // Add nested objects to main json
        json["rsu"] = rsuEndpoint;
        json["snmp"] = snmpConfig;

        return json;
    }


    bool jsonValueToRsuConfig(const Json::Value& json, rsuConfig& config) {
        try {
            // Validate top-level structure
            if (!json.isMember("rsu") || !json["rsu"].isObject()) {
                PLOG(logERROR) << "Missing or invalid 'rsu' Endpoint object";
                return false;
            }

            if (!json.isMember("snmp") || !json["snmp"].isObject()) {
                PLOG(logERROR) << "Missing or invalid rsu 'snmp' object";
                return false;
            }

            Json::Value rsuEndpoint = json["rsu"];
            Json::Value snmpConfig = json["snmp"];

            // Extract RSU fields
            config.rsu.ip = rsuEndpoint["IP"].asString();
            config.rsu.port = rsuEndpoint.get("Port", "8080").asString();


            // Extract SNMP fields
            config.snmp.userKey = snmpConfig["User"].asString();
            config.snmp.privProtocol = snmpConfig["PrivacyProtocol"].asString();
            config.snmp.authProtocolKey = snmpConfig["AuthProtocol"].asString();
            config.snmp.authPassPhraseKey = snmpConfig["AuthPassPhrase"].asString();
            config.snmp.privPassPhrase = snmpConfig["PrivacyPassPhrase"].asString();
            config.snmp.rsuMIBVersionKey = snmpConfig["RSUMIBVersion"].asString();
            config.snmp.securityLevelKey = snmpConfig["SecurityLevel"].asString();

            config.event = json.get("event", "").asString();
            config.action = json.get("action","").asString();

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

            if (root.isMember("RSUConfigs") && root["RSUConfigs"].isArray()) {
                return rsuConfigMessageToVector(root["RSUConfigs"], rsuConfigList);
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


}