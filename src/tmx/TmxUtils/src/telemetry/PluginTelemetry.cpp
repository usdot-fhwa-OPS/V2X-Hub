#include "PluginTelemetry.h"
namespace tmx::utils::telemetry{
    PluginInfo PluginTelemetry::getPluginInfo() const{
        return _pluginInfo;
    }

    PluginInstallation PluginTelemetry::getPluginInstallation() const{
        return _pluginInstallation;
    }

    void PluginTelemetry::setPluginInfo(const PluginInfo& pluginInfo){
        _pluginInfo = pluginInfo;
    }
    
    void PluginTelemetry::setPluginInstallation(const PluginInstallation& pluginInstallation){
        _pluginInstallation = pluginInstallation;
    }

    tmx::message_container_type PluginTelemetry::toTree() const{
        tmx::message_container_type output;

        auto name = _pluginInfo.name;
        output.store(message_path_type(NAME_STRING), name);

        auto id = _pluginInfo.id;
        output.store(message_path_type(ID_STRING), id); 

        auto description = _pluginInfo.description;
        output.store(message_path_type(DESCRIPTION_STRING), description); 

        auto version = _pluginInfo.version;
            output.store(message_path_type(VERSION_STRING), version);

        auto enabled = _pluginInstallation.enabled;
        //enabled value -> 0: disabled, 1 enabled, -1 external
        string enabledStr;
        if(enabled < 0){
            enabledStr = EXTERNAL_STRING;
        }else{
            enabledStr = !enabled? DISABLED_STRING: ENABLED_STRING;
        }
        
        output.store(message_path_type(ENABLED_STRING), enabledStr);

        //When enabled equals -1, this plugin is not installed
        if(enabled < 0){
            return output;
        }

        auto path = _pluginInstallation.path;
        output.store(message_path_type(PATH_STRING), path);

        auto exeName = _pluginInstallation.exeName;
        output.store(message_path_type(EXE_NAME_STRING), exeName);

        auto manifest = _pluginInstallation.manifest;
        output.store(message_path_type(PATH_STRING), manifest);

        auto maxMessageInterval = _pluginInstallation.maxMessageInterval;
        output.store(message_path_type(MAX_MESSAGE_INTERVAL_STRING), maxMessageInterval);

        auto commandLineParameters = _pluginInstallation.commandLineParameters;
        output.store(message_path_type(COMMAND_LINE_PARAMETERS_STRING), commandLineParameters);
        return output;
    }
    
    void PluginTelemetry::fromTree(const boost::property_tree::ptree& jsonContainer){
        try{
            if(jsonContainer.empty()){
                throw tmx::TmxException("Cannot deserialize empty JSON!");
            }

            PluginInfo pluginInfo = {
                jsonContainer.get<string>(ID_STRING,""),
                jsonContainer.get<string>(NAME_STRING,""),
                jsonContainer.get<string>(DESCRIPTION_STRING,""),
                jsonContainer.get<string>(VERSION_STRING,"")
            };
            setPluginInfo(pluginInfo);  
            string enabledStr = jsonContainer.get<string>(ENABLED_STRING,"");
            int enabled = 0; //Default set to 0 when "disabled"
            if(enabledStr != DISABLED_STRING){
                //enabled: 1, external -1
                enabled = enabledStr == ENABLED_STRING? 1:-1;
            }
            PluginInstallation installation = {
                enabled,
                jsonContainer.get<string>(PATH_STRING,""),
                jsonContainer.get<string>(EXE_NAME_STRING,""),
                jsonContainer.get<string>(MANIFEST_STRING,""),
                jsonContainer.get<string>(MAX_MESSAGE_INTERVAL_STRING,""),
                jsonContainer.get<string>(COMMAND_LINE_PARAMETERS_STRING,""),
            };
            setPluginInstallation(installation);
        }catch(const boost::property_tree::ptree_error& error){
            throw tmx::TmxException("Cannot get value from JSON due to error: "+  string(error.what()));
        }
    }
}