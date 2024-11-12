#include "TelemetrySerializer.h"

namespace tmx::utils::telemetry{

    tmx::message_container_type TelemetrySerializer::serializePluginTelemetry(const PluginTelemetry& pluginTelemetry){
        tmx::message_container_type output;

        auto name = pluginTelemetry.getPluginInfo().name;
        output.store(message_path_type("name"), name);

        auto id = pluginTelemetry.getPluginInfo().id;
        output.store(message_path_type("id"), id); 

        auto description = pluginTelemetry.getPluginInfo().description;
        output.store(message_path_type("description"), description); 

        auto version = pluginTelemetry.getPluginInfo().version;
            output.store(message_path_type("version"), version);

        auto enabled = pluginTelemetry.getPluginInstallation().enabled;
        //enabled value -> 0: disabled, 1 enabled
        output.store(message_path_type("enabled"), !enabled ? "Disabled": enabled > 0 ? "Enabled" : "External");

        //When enabled is -1, this plugin is not installed
        if(enabled < 0){
            return output;
        }

        auto path = pluginTelemetry.getPluginInstallation().path;
        output.store(message_path_type("path"), path);

        auto exeName = pluginTelemetry.getPluginInstallation().exeName;
        output.store(message_path_type("exeName"), exeName);

        auto manifest = pluginTelemetry.getPluginInstallation().manifest;
        output.store(message_path_type("path"), manifest);

        auto maxMessageInterval = pluginTelemetry.getPluginInstallation().maxMessageInterval;
        output.store(message_path_type("maxMessageInterval"), maxMessageInterval);

        auto commandLineParameters = pluginTelemetry.getPluginInstallation().commandLineParameters;
        output.store(message_path_type("commandLineParameters"), commandLineParameters);
        return output;
    }

    tmx::message_container_type TelemetrySerializer::serializeFullPluginTelemetryPayload(const vector<PluginTelemetry>& pluginTelemetryList){
        tmx::message_container_type outputContainer;
        boost::property_tree::ptree ptArray;
        for(const auto& pluginTelemetry: pluginTelemetryList){
            auto output = serializePluginTelemetry(pluginTelemetry);
            ptArray.push_back(boost::property_tree::ptree::value_type("",output.get_storage().get_tree()));
        }
        outputContainer.get_storage().get_tree().put_child("payload", ptArray);
        return outputContainer;
    }

    tmx::message_container_type TelemetrySerializer::serializeUpdatedTelemetry(const map<string, string>& updates){
        tmx::message_container_type pt;
        for(const auto update : updates){
            pt.store(message_path_type(update.first), update.second);
        }
        return pt;
    }

    string TelemetrySerializer::composeUpdatedTelemetryPayload(const map<string, string>& updates){
        string payload = "{\"payload\":[";
        //loop through updates
        bool first = true;
        for  (auto it = updates.begin();it != updates.end();it++)
        {
            if (first)
                first = false;
            else
                payload+=",";
            //append plugin json
            payload+=it->second;
        }
        payload += "]}";
        return payload;
    }

    string TelemetrySerializer::composeCompleteTelemetry(const string& headerJsonStr, const string& payloadJsonStr){
        //Remove the tailing bracket "}" from headerJsonStr
        string fullJsonString = headerJsonStr.substr(0,headerJsonStr.length()-1);
        fullJsonString+=",";
        //Remove the first bracket "{" from payloadJsonStr
        fullJsonString += payloadJsonStr.substr(1);
        return fullJsonString;
    }

    tmx::message_container_type TelemetrySerializer::serializeTelemetryHeader(const TelemetryHeader& header){
        tmx::message_container_type headerContainer;
        boost::property_tree::ptree element;
        element.push_back(boost::property_tree::ptree::value_type("type", header.type));
        element.push_back(boost::property_tree::ptree::value_type("subtype", header.subtype));
        element.push_back(boost::property_tree::ptree::value_type("encoding", header.encoding));
        element.push_back(boost::property_tree::ptree::value_type("timestamp",to_string(header.timestamp)));
        element.push_back(boost::property_tree::ptree::value_type("flags","0"));
        headerContainer.get_storage().get_tree().push_back(boost::property_tree::ptree::value_type("header", element));
        return headerContainer;
    }

    string TelemetrySerializer::jsonToString(tmx::message_container_type& container){
        stringstream ss;
        ss.clear();
        ss.str(string());
        boost::property_tree::write_json(ss, container.get_storage().get_tree(), false);
        string jsonString = ss.str();
        boost::erase_all(jsonString,"\n");
        boost::replace_all(jsonString, "\\\"", "\"");
        return jsonString;
    }
}