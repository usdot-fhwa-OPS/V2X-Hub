#include "PluginTelemetry.h"
namespace tmx::utils::telemetry{
    PluginInfo PluginTelemetry::getPluginInfo() const{
        return _pluginInfo;
    }

    PluginInstallation PluginTelemetry::getPluginInstallation() const{
        return _pluginInstallation;
    }

    void PluginTelemetry::setPluginInfo(PluginInfo pluginInfo){
        _pluginInfo = pluginInfo;
    }
    
    void PluginTelemetry::setPluginInstallation(PluginInstallation pluginInstallation){
        _pluginInstallation = pluginInstallation;
    }

    tmx::message_container_type PluginTelemetry::serialize() const{
        tmx::message_container_type output;

        auto name = _pluginInfo.name;
        output.store(message_path_type("name"), name);

        auto id = _pluginInfo.id;
        output.store(message_path_type("id"), id); 

        auto description = _pluginInfo.description;
        output.store(message_path_type("description"), description); 

        auto version = _pluginInfo.version;
            output.store(message_path_type("version"), version);

        auto enabled = _pluginInstallation.enabled;
        //enabled value -> 0: disabled, 1 enabled
        output.store(message_path_type("enabled"), !enabled ? "Disabled": enabled > 0 ? "Enabled" : "External");

        //When enabled is -1, this plugin is not installed
        if(enabled < 0){
            return output;
        }

        auto path = _pluginInstallation.path;
        output.store(message_path_type("path"), path);

        auto exeName = _pluginInstallation.exeName;
        output.store(message_path_type("exeName"), exeName);

        auto manifest = _pluginInstallation.manifest;
        output.store(message_path_type("path"), manifest);

        auto maxMessageInterval = _pluginInstallation.maxMessageInterval;
        output.store(message_path_type("maxMessageInterval"), maxMessageInterval);

        auto commandLineParameters = _pluginInstallation.commandLineParameters;
        output.store(message_path_type("commandLineParameters"), commandLineParameters);
        return output;
    }
    
    void PluginTelemetry::deserialize(const boost::property_tree::ptree& jsonContainer){
        try{
            if(jsonContainer.empty()){
                throw tmx::TmxException("Cannot deserialize empty JSON!");
            }

            PluginInfo pluginInfo = {
                jsonContainer.get<string>("id",""),
                jsonContainer.get<string>("name",""),
                jsonContainer.get<string>("description",""),
                jsonContainer.get<string>("version","")
            };
            setPluginInfo(pluginInfo);  
            string enabledStr = jsonContainer.get<string>("enabled","");
            int enabled = enabledStr =="enabled"? 1: enabledStr=="Disabled"?0:-1; 
            PluginInstallation installation = {
                enabled,
                jsonContainer.get<string>("path",""),
                jsonContainer.get<string>("exeName",""),
                jsonContainer.get<string>("manifest",""),
                jsonContainer.get<string>("maxMessageInterval",""),
                jsonContainer.get<string>("commandLineParameters",""),
            };
            setPluginInstallation(installation);
        }catch(const boost::property_tree::ptree_error& error){
            throw tmx::TmxException("Cannot get value from JSON due to error: "+  string(error.what()));
        }
    }
}