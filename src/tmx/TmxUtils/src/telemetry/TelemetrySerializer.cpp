#include "TelemetrySerializer.h"

namespace tmx::utils::telemetry{

    string TelemetrySerializer::serializePluginTelemetryList(vector<PluginTelemetry> pluginTelemetryList){
        if(pluginTelemetryList.empty()){
            throw new TelemetrySerializerException("Plugin telemetry list cannot be empty!");
        }

        Json::Value root;
        for(auto& telemetry: pluginTelemetryList){
            Json::Value value;
            PluginInfo plugin =  telemetry.getPluginInfo();
            value["id"] = plugin.id;
            value["name"] = plugin.name;
            value["description"] = plugin.description;
            value["version"] = plugin.version;

            PluginInstallation installation = telemetry.getPluginInstallation();
            value["path"] = installation.path;
            value["enabled"] = installation.enabled;
            value["exeName"] = installation.exeName;
            value["manifest"] = installation.manifest;
            value["maxMessageInterval"] = installation.maxMessageInterval;
            value["commandLineParameters"] = installation.commandLineParameters;
            root.append(value);
        }
        return jsonToString(root);
    }

    string TelemetrySerializer::jsonToString(Json::Value root){
        Json::StreamWriterBuilder builder;
        string jsonString = Json::writeString(builder, root);
        //Remove format which includes \t \n from the telemetry json string
        boost::erase_all(jsonString,"\n");
        boost::erase_all(jsonString,"\t");
        return jsonString;
    }
}