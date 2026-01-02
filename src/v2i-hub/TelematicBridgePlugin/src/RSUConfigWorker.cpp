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

    bool processRSUConfig(const Json::Value& rsuConfigJson, int16_t& maxConnections, std::vector<rsu_st>& rsuRegisteredList)
    {
        if(rsuConfigJson.isMember("rsu") && rsuConfigJson["rsu"].isObject())
        {
            try
            {
                rsu_st rsuConfig;
                Json::Value rsu = rsuConfigJson["rsu"];
                rsuConfig.ip = rsu.get("IP", "").asString();
                rsuConfig.port = rsu.get("Port", 161).asInt();

                PLOG(logINFO)<<"Added RSU:"<<rsuConfig.ip<<":"<<rsuConfig.port;

                // Check if RSU is already registered
                for(auto registeredRsu : rsuRegisteredList)
                {
                    if (rsuConfig.ip == registeredRsu.ip){
                        PLOG(logDEBUG) << "RSU "<< rsuConfig.ip << " already registered.";
                        return false;
                    }
                }

                // Log error if max number of connections has been reached
                if(rsuRegisteredList.size() >= maxConnections)
                {
                    PLOG(logWARNING) << "Max number of connections reached("<<maxConnections <<") . RSU " << rsuConfig.ip << "will not be added.";
                    return false;
                }

                // Log error if RSU is defined without snmp configuration
                if(!rsuConfigJson.isMember("snmp") || !rsuConfigJson["snmp"].isObject()){
                    PLOG(logERROR) << "Missing or invalid snmp object for RSU with IP "<< rsuConfig.ip << " , ignoring request to register.";
                    return false;
                }

                Json::Value snmp = rsuConfigJson["snmp"];
                validateRequiredKeys(snmp, REQUIRED_SNMP_KEYS);

                rsuConfig.snmp.userKey = snmp.get("User", "").asString();
                rsuConfig.snmp.snmpPort = snmp.get("SNMPPort", "").asString();
                rsuConfig.snmp.privProtocol = snmp.get("PrivacyProtocol","").asString();
                rsuConfig.snmp.authProtocolKey = snmp.get("AuthProtocol","").asString();
                rsuConfig.snmp.authPassPhraseKey = snmp.get("AuthPassPhrase","").asString();
                rsuConfig.snmp.privPassPhrase = snmp.get("PrivacyPassPhrase","").asString();
                rsuConfig.snmp.rsuMIBVersionKey = snmp.get("RSUMIBVersion","").asString();
                rsuConfig.snmp.securityLevelKey = snmp.get("SecurityLevel","").asString();

                //Add configured RSU to list
                rsuRegisteredList.push_back(rsuConfig);
                PLOG(logDEBUG) << "Added RSU "<< rsuConfig.ip << " to registered list";
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


    Json::Value rsuToJsonValue(const rsu_st& rsu) {
        Json::Value json;

        // Create RSU endpoint object
        Json::Value rsuEndpoint;
        rsuEndpoint["IP"] = rsu.ip;
        rsuEndpoint["Port"] = rsu.port;
        rsuEndpoint["timestamp"] = rsu.timestamp;

        // Create SNMP config object
        Json::Value snmpConfig;
        snmpConfig["User"] = rsu.snmp.userKey;
        snmpConfig["SNMPPort"] = rsu.snmp.snmpPort;
        snmpConfig["PrivacyProtocol"] = rsu.snmp.privProtocol;
        snmpConfig["AuthProtocol"] = rsu.snmp.authProtocolKey;
        snmpConfig["AuthPassPhrase"] = rsu.snmp.authPassPhraseKey;
        snmpConfig["PrivacyPassPhrase"] = rsu.snmp.privPassPhrase;
        snmpConfig["RSUMIBVersion"] = rsu.snmp.rsuMIBVersionKey;
        snmpConfig["SecurityLevel"] = rsu.snmp.securityLevelKey;

        // Add nested objects to main json
        json["rsu"] = rsuEndpoint;
        json["snmp"] = snmpConfig;

        // Add event if present
        if (!rsu.event.empty()) {
            json["event"] = rsu.event;
        }
        return json;
    }


    bool jsonValueToRsu(const Json::Value& json, rsu_st& rsu) {
        try {
            // Validate top-level structure
            if (!json.isMember("rsu") || !json["rsu"].isObject()) {
                PLOG(logERROR) << "Missing or invalid 'rsu' object";
                return false;
            }

            if (!json.isMember("snmp") || !json["snmp"].isObject()) {
                PLOG(logERROR) << "Missing or invalid 'snmp' object";
                return false;
            }

            Json::Value rsuEndpoint = json["rsu"];
            Json::Value snmpConfig = json["snmp"];

            // Validate required keys
            validateRequiredKeys(rsuEndpoint, REQUIRED_RSU_KEYS);
            validateRequiredKeys(snmpConfig, REQUIRED_SNMP_KEYS);

            // Extract RSU fields
            rsu.ip = rsuEndpoint["IP"].asString();
            rsu.port = rsuEndpoint.get("Port", "8080").asString();
            rsu.timestamp = rsuEndpoint.get("timestamp", 0).asInt64();
            rsu.event = json.get("event", "").asString();

            // Extract SNMP fields
            rsu.snmp.userKey = snmpConfig["User"].asString();
            rsu.snmp.snmpPort = snmpConfig["SNMPPort"].asString();
            rsu.snmp.privProtocol = snmpConfig["PrivacyProtocol"].asString();
            rsu.snmp.authProtocolKey = snmpConfig["AuthProtocol"].asString();
            rsu.snmp.authPassPhraseKey = snmpConfig["AuthPassPhrase"].asString();
            rsu.snmp.privPassPhrase = snmpConfig["PrivacyPassPhrase"].asString();
            rsu.snmp.rsuMIBVersionKey = snmpConfig["RSUMIBVersion"].asString();
            rsu.snmp.securityLevelKey = snmpConfig["SecurityLevel"].asString();

            return true;

        } catch (const std::exception& e) {
            PLOG(logERROR) << "Failed to convert JSON to rsu_st: " << e.what();
            return false;
        }
    }

    bool rsuConfigMessageToVector(const Json::Value& jsonArray, std::vector<rsu_st>& rsus) {

        if (!jsonArray.isArray()) {
            PLOG(logERROR) << "Input is not a JSON array";
            return false;
        }

        bool success = true;

        for (Json::ArrayIndex i = 0; i < jsonArray.size(); ++i) {
            rsu_st rsu;
            if (jsonValueToRsu(jsonArray[i], rsu)) {
                rsus.push_back(rsu);
            } else {
                PLOG(logERROR) << "Failed to parse RSU config at index " << i;
                success = false;
                // Continue processing other configs
            }
        }

        return success;
    }

    bool loadRSUConfigFromFile(const std::string& configPath, std::vector<rsu_st>& rsus) {
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
                return rsuConfigMessageToVector(root["RSUConfigs"], rsus);
            }
        } catch (...)
        {
            PLOG(logERROR) << "Could not load config from from file, RSU configuration needs to be defined on the UI.";
            return false;
        }
    }


    Json::Value rsuVectorToJsonArray(const std::vector<rsu_st>& rsus) {
        Json::Value jsonArray(Json::arrayValue);

        for (const auto& rsu : rsus) {
            jsonArray.append(rsuToJsonValue(rsu));
        }

        return jsonArray;
    }


}