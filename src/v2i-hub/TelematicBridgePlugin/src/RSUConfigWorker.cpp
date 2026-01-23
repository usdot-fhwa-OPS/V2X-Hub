#include "RSUConfigWorker.h"

using namespace tmx::utils;
namespace TelematicBridge
{
    action truConfigWorker::stringToAction(const std::string& str) {
        if (str == "add") return action::add;
        if (str == "delete") return action::remove;
        return action::unknown;
    }

    std::string truConfigWorker::actionToString(action ac) const{
        switch (ac){
            case action::add :
                return "add";
            case action::remove :
                return "delete";
            case action::update:
                return "update";
            case action::unknown:
                return "unknown";
            default:
                return "unspecified";
        }
    }

    void truConfigWorker::validateRequiredKeys(const Json::Value& json, const std::vector<std::string>& requiredKeys)
    {
        for(const auto& key : requiredKeys)
        {
            if (!json.isMember(key)) {
                std::string error = "Missing required key" + key;
                throw TelematicBridgeException(error);
            }
        }
    }

    bool truConfigWorker::loadRSUConfigListFromFile(const std::string& configPath)
    {
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
            if(root.isMember(TRU_UNIT_CONFIG_KEY) && root[TRU_UNIT_CONFIG_KEY].isArray()){
                setJsonArrayToUnitConfig(root[TRU_UNIT_CONFIG_KEY]);
            }
            if (root.isMember(TRU_RSU_CONFIGS_KEY) && root[TRU_RSU_CONFIGS_KEY].isArray()) {
                setJsonArrayToRsuConfigList(root[TRU_RSU_CONFIGS_KEY]);
            }
            if(root.isMember(TRU_TIMESTAMP_KEY)){
                _lastUpdateTimestamp = root[TRU_TIMESTAMP_KEY].asInt();
            }
        } catch (...)
        {
            PLOG(logERROR) << "Could not load config from from file, RSU configuration needs to be defined on the UI.";
            return false;
        }
        return true;
    }

    bool truConfigWorker::updateTRUStatus(const Json::Value& jsonVal){
        try{
            if(jsonVal.isMember(TRU_UNIT_CONFIG_KEY) && jsonVal[TRU_UNIT_CONFIG_KEY].isArray()){
                for (auto item : jsonVal[TRU_UNIT_CONFIG_KEY]){
                    if (item.isMember(TRU_UNIT_ID_KEY)){
                        auto incomingUnitId = item[TRU_UNIT_ID_KEY].asString();
                        if(incomingUnitId!= _unitId){
                            PLOG(logERROR) << "Incoming Unit ID"<< incomingUnitId <<"does not match assigned ID"<< _unitId <<", ignoring request.";
                            return false;
                        }
                        else{
                            continue;
                        }
                    }
                }
            }
            else{
                PLOG(logERROR) << "Missing Unit Configuration in message ";
                return false;
            }

            if (jsonVal.isMember(TRU_RSU_CONFIGS_KEY) && jsonVal[TRU_RSU_CONFIGS_KEY].isArray()) {
                setJsonArrayToRsuConfigList(jsonVal[TRU_RSU_CONFIGS_KEY]);
            }
            else{
                PLOG(logERROR) << "Missing RSU Configuration in message ";
                return false;
            }
            if(jsonVal.isMember(TRU_TIMESTAMP_KEY)){
                _lastUpdateTimestamp = jsonVal[TRU_TIMESTAMP_KEY].asInt();
            }
            else{
                PLOG(logERROR) << "Missing timestamp in message ";
                return false;
            }
        }catch (...)
        {
            PLOG(logERROR) << "Could not update TRU configuration from request.";
            return false;
        }
        return true;
    }

    bool truConfigWorker::setJsonArrayToUnitConfig(const Json::Value& message){
        if (!message.isArray()) {
            PLOG(logERROR) << "Input is not a JSON array";
            return false;
        }

        for (auto item : message){
            _unitId = item.get(TRU_UNIT_ID_KEY, "").asString();
            _unitName = item.get(TRU_UNIT_NAME_KEY, "").asString();
            _maxConnections = item.get(TRU_MAX_CONNECTIONS_KEY, 5).asInt();
            _pluginHeartBeatInterval = item.get(TRU_PLUGIN_HEARTBEAT_INTERVAL_KEY, 1).asInt();
            _healthMonitorPluginHeartbeatInterval = item.get(TRU_HEALTH_MONITOR_PLUGIN_HEARTBEAT_INTERVAL_KEY, 1).asInt();
            _rsuStatusMonitorInterval = item.get(TRU_RSU_STATUS_MONITOR_INTERVAL_KEY, 1).asInt();
        }
        return true;
    }

    bool truConfigWorker::setJsonArrayToRsuConfigList(const Json::Value& message){
        if (!message.isArray()) {
            PLOG(logERROR) << "Input is not a JSON array";
            return false;
        }
        bool success = true;

        for (Json::ArrayIndex i = 0; i < message.size(); ++i) {
            rsuConfig config;
            if (jsonValueToRsuConfig(message[i], config)) {
                processAddAction(config);
            } else {
                PLOG(logERROR) << "Failed to parse RSU config at index " << i;
                success = false;
            }
        }
        return success;
    }

    bool truConfigWorker::jsonValueToRsuConfig(const Json::Value& json, rsuConfig& config) {
        try {
            if (!json.isMember(TRU_EVENT_KEY)){
                PLOG(logERROR) << "Missing or invalid Event for RSU";
                return false;
            }
            config.event = json.get(TRU_EVENT_KEY, "").asString();

            if(!json.isMember(TRU_ACTION_KEY)){
                // Action defaults to add
                config.actionType = action::add;
            }
            else{
                config.actionType = stringToAction(json.get(TRU_ACTION_KEY,"").asString());
            }

            // Validate top-level structure
            if (!json.isMember(TRU_RSU_KEY) || !json[TRU_RSU_KEY].isObject()) {
                PLOG(logERROR) << "Missing or invalid 'rsu' Endpoint object";
                return false;
            }

            if (!json.isMember(TRU_SNMP_KEY) || !json[TRU_SNMP_KEY].isObject()) {
                PLOG(logERROR) << "Missing or invalid rsu 'snmp' object";
                return false;
            }

            Json::Value rsuEndpoint = json[TRU_RSU_KEY];
            Json::Value snmpConfig = json[TRU_SNMP_KEY];

            // Extract RSU fields
            config.rsu.ip = rsuEndpoint[TRU_IP_KEY].asString();
            config.rsu.port = rsuEndpoint.get(TRU_PORT_KEY, 8080).asInt();


            // Extract SNMP fields
            config.snmp.userKey = snmpConfig[TRU_USER_KEY].asString();
            config.snmp.privProtocol = snmpConfig[TRU_PRIVACY_PROTOCOL_KEY].asString();
            config.snmp.authProtocolKey = snmpConfig[TRU_AUTH_PROTOCOL_KEY].asString();
            config.snmp.authPassPhraseKey = snmpConfig[TRU_AUTH_PASS_PHRASE_KEY].asString();
            config.snmp.privPassPhrase = snmpConfig[TRU_PRIVACY_PASS_PHRASE_KEY].asString();
            config.snmp.rsuMIBVersionKey = snmpConfig[TRU_RSU_MIB_VERSION_KEY].asString();
            config.snmp.securityLevelKey = snmpConfig[TRU_SECURITY_LEVEL_KEY].asString();

            return true;

        } catch (const std::exception& e) {
            PLOG(logERROR) << "Failed to convert JSON to rsuConfig: " << e.what();
            return false;
        }
    }

    bool truConfigWorker::processRSUConfig(const Json::Value& rsuConfigJson){
        try
        {
            if(rsuConfigJson.isMember(TRU_RSU_KEY) && rsuConfigJson[TRU_RSU_KEY].isObject())
            {
                validateRequiredKeys(rsuConfigJson, REQUIRED_RSU_CONFIG_KEYS);

                rsuConfig config;
                config.actionType = stringToAction(rsuConfigJson[TRU_ACTION_KEY].asString());
                config.event = rsuConfigJson[TRU_EVENT_KEY].asString();

                Json::Value rsu = rsuConfigJson[TRU_RSU_KEY];
                validateRequiredKeys(rsu, REQUIRED_RSU_KEYS);
                config.rsu.ip = rsu.get(TRU_IP_KEY, "").asString();
                config.rsu.port = rsu.get(TRU_PORT_KEY, 161).asInt();

                PLOG(logINFO)<<"Added RSU:"<< config.rsu.ip<<":"<< config.rsu.port;

                // Check if RSU is already registered
                if(_truRegistrationMap.find(config.rsu.ip) != _truRegistrationMap.end()){
                    PLOG(logDEBUG) << "RSU "<< config.rsu.ip << " already registered.";
                    return false;
                }

                Json::Value snmp = rsuConfigJson[TRU_SNMP_KEY];
                validateRequiredKeys(snmp, REQUIRED_SNMP_KEYS);
                config.snmp.userKey = snmp.get(TRU_USER_KEY, "").asString();
                config.snmp.privProtocol = snmp.get(TRU_PRIVACY_PROTOCOL_KEY,"").asString();
                config.snmp.authProtocolKey = snmp.get(TRU_AUTH_PROTOCOL_KEY,"").asString();
                config.snmp.authPassPhraseKey = snmp.get(TRU_AUTH_PASS_PHRASE_KEY,"").asString();
                config.snmp.privPassPhrase = snmp.get(TRU_PRIVACY_PASS_PHRASE_KEY,"").asString();
                config.snmp.rsuMIBVersionKey = snmp.get(TRU_RSU_MIB_VERSION_KEY,"").asString();
                config.snmp.securityLevelKey = snmp.get(TRU_SECURITY_LEVEL_KEY,"").asString();

                processAddAction(config);
            }
            else{
                return false;
            }
        }catch(const std::exception& e){
            //Catch error for RSU configuration without crashing
            PLOG(logERROR)<<"Failed to process RSU config: "<< e.what();
            return false;
        }
        return true;
    }

    bool truConfigWorker::processAddAction(rsuConfig config)
    {
        // Log error if max number of connections has been reached
        if(_truRegistrationMap.size() >= _maxConnections)
        {
            PLOG(logWARNING) << "Max number of connections reached("<<_maxConnections <<") . RSU " << config.rsu.ip << "will not be added.";
            return false;
        }

        //Add configured RSU to list
        _truRegistrationMap.insert({config.rsu.ip, config});
        PLOG(logDEBUG) << "Added RSU "<< config.rsu.ip << " to registered list";
        return true;
    }

    bool truConfigWorker::processUpdateAction(rsuConfig config){

        if(_truRegistrationMap.find(config.rsu.ip) == _truRegistrationMap.end()){
            PLOG(logERROR) << "RSU "<< config.rsu.ip<<" currently not registered, attempting to add.";
            processAddAction(config);
        }
        else{
            _truRegistrationMap[config.rsu.ip] = config;
        }
        return true;

    }

    bool truConfigWorker::processDeleteAction(rsuConfig config){
        try{
            if(_truRegistrationMap.find(config.rsu.ip) == _truRegistrationMap.end()){
                PLOG(logERROR) << "RSU "<< config.rsu.ip<<" currently not registering, ignoring request to delete.";
            }
            else{
                _truRegistrationMap.erase(config.rsu.ip);
            }
            return true;
        }catch(...)
        {
            PLOG(logERROR)<<"Could not delete RSU "<< config.rsu.ip << " from registration";
            return false;
        }
    }

    Json::Value truConfigWorker::rsuConfigToJsonValue(const rsuConfig& config) {
        Json::Value json;

        json[TRU_ACTION_KEY] = actionToString(config.actionType);
        json[TRU_EVENT_KEY] = config.event;

        // Create RSU endpoint object
        Json::Value rsuEndpoint;
        rsuEndpoint[TRU_IP_KEY] = config.rsu.ip;
        rsuEndpoint[TRU_PORT_KEY] = config.rsu.port;

        // Create SNMP config object
        Json::Value snmpConfig;
        snmpConfig[TRU_USER_KEY] = config.snmp.userKey;
        snmpConfig[TRU_PRIVACY_PROTOCOL_KEY] = config.snmp.privProtocol;
        snmpConfig[TRU_AUTH_PROTOCOL_KEY] = config.snmp.authProtocolKey;
        snmpConfig[TRU_AUTH_PASS_PHRASE_KEY] = config.snmp.authPassPhraseKey;
        snmpConfig[TRU_PRIVACY_PASS_PHRASE_KEY] = config.snmp.privPassPhrase;
        snmpConfig[TRU_RSU_MIB_VERSION_KEY] = config.snmp.rsuMIBVersionKey;
        snmpConfig[TRU_SECURITY_LEVEL_KEY] = config.snmp.securityLevelKey;

        // Add nested objects to main json
        json[TRU_RSU_KEY] = rsuEndpoint;
        json[TRU_SNMP_KEY] = snmpConfig;

        return json;
    }

    Json::Value truConfigWorker::getTruConfigAsJsonArray() {
        Json::Value message;

        message[TRU_UNIT_CONFIG_KEY] = getUnitConfigAsJsonArray();

        message[TRU_RSU_CONFIGS_KEY] = getRsuConfigListAsJsonArray();
        message[TRU_TIMESTAMP_KEY] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        return message;
    }

    Json::Value truConfigWorker::getTRUConfigResponse(bool isRegistrationSuccessful){
        Json::Value message;

        Json::Value unitObject;
        unitObject[TRU_UNIT_ID_KEY] = _unitId;
        message[TRU_UNIT_CONFIG_KEY].append(unitObject);

        Json::Value rsuConfigList;
        for (const auto& configPair : _truRegistrationMap) {
            Json::Value rsuConfig;
            rsuConfig[TRU_IP_KEY] = configPair.second.rsu.ip;
            rsuConfig[TRU_PORT_KEY] = configPair.second.rsu.port;
            rsuConfigList.append(rsuConfig);
        }
        message[TRU_RSU_CONFIGS_KEY] = rsuConfigList;

        if(isRegistrationSuccessful){
            message[TRU_RESPONSE_STATUS_KEY]="success";
        }
        else{
            message[TRU_RESPONSE_STATUS_KEY]="failed";
        }

        message[TRU_TIMESTAMP_KEY] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        return message;
    }

    Json::Value truConfigWorker::getUnitConfigAsJsonArray() {
        Json::Value unitObject;
        unitObject[TRU_UNIT_ID_KEY] = _unitId;
        unitObject[TRU_UNIT_NAME_KEY] = _unitName;
        unitObject[TRU_MAX_CONNECTIONS_KEY] = _maxConnections;
        unitObject[TRU_PLUGIN_HEARTBEAT_INTERVAL_KEY] = _pluginHeartBeatInterval;
        unitObject[TRU_HEALTH_MONITOR_PLUGIN_HEARTBEAT_INTERVAL_KEY] = _healthMonitorPluginHeartbeatInterval;
        unitObject[TRU_RSU_STATUS_MONITOR_INTERVAL_KEY] = _rsuStatusMonitorInterval;

        return unitObject;
    }

    Json::Value truConfigWorker::getRsuConfigListAsJsonArray() {
        Json::Value jsonArray(Json::arrayValue);

        for (const auto& configPair : _truRegistrationMap) {
            jsonArray.append(rsuConfigToJsonValue(configPair.second));
        }
        return jsonArray;
    }

    std::string truConfigWorker::getUnitId(){
        return _unitId;
    }
    
    std::string truConfigWorker::getEventByRsu(const std::string &rsuIp, int rsuPort) const
    {
        // Search for matching RSU by IP and port in the registration map
        for (const auto& pair : _truRegistrationMap)
        {
            const auto& rsuConfig = pair.second;
            if (rsuConfig.rsu.ip == rsuIp && rsuConfig.rsu.port == rsuPort)
            {
                return rsuConfig.event;
            }
        }
        
        // Return empty string if RSU not found
        PLOG(logDEBUG3) << "Event not found for RSU: " << rsuIp << ":" << rsuPort;
        return "";
    }

    int truConfigWorker::getRsuPortByIp(const std::string &rsuIp) const
    {
        auto it = _truRegistrationMap.find(rsuIp);
        if (it != _truRegistrationMap.end())
        {
            return it->second.rsu.port;
        }
        else
        {
            PLOG(logDEBUG3) << "RSU port not found for IP: " << rsuIp;
            return 0; // Return 0 or an appropriate default value if RSU not found
        }
    }
}