#include "TelemetryDeserializer.h"

namespace tmx::utils::telemetry
{
    Json::Value TelemetryDeserializer::stringToJson( const string& jsonString){
        Json::Value root = Json::Value::null;
        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader (builder.newCharReader());
        JSONCPP_STRING error;
        if(!reader->parse(jsonString.c_str(), jsonString.c_str()+jsonString.length(), &root, &error)){
            throw TelemetryDeserializerException("Cannot parse JSON string due to error "+ error);
        }
        return root;
    }


    PluginTelemetry TelemetryDeserializer::populatePluginTelemetry(const Json::Value& value){
        PluginTelemetry telemetry;
        if(value.isNull()){
            throw TelemetryDeserializerException("Cannot deserialize null value!");
        }

        if(value.isMember("id") && value.isMember("name") && value.isMember("description") && value.isMember("version")){
            PluginInfo pluginInfo = {
                value["id"].asString(),
                value["name"].asString(),
                value["description"].asString(),
                value["version"].asString()
            };
            telemetry.setPluginInfo(pluginInfo);  
        }
        
        if(value.isMember("enabled") && value.isMember("path") && value.isMember("exeName") && value.isMember("manifest")&& value.isMember("maxMessageInterval")&& value.isMember("commandLineParameters")){
            PluginInstallation installation = {
                value["enabled"].asInt(),
                value["path"].asString(),
                value["exeName"].asString(),
                value["manifest"].asString(),
                value["maxMessageInterval"].asString(),
                value["commandLineParameters"].asString(),
            };
            telemetry.setPluginInstallation(installation);
        }
        return telemetry;
    }

    vector<PluginTelemetry> TelemetryDeserializer::desrializePluginTelemetryList(const string& jsonString){
        vector<PluginTelemetry> result;
        Json::Value root = stringToJson(jsonString);
        if(root.isNull()){
            throw TelemetryDeserializerException("Cannot deserialize null value!");
        }
        
        if(root.isArray()){
            for(auto itr = root.begin(); itr!=root.end(); ++itr){
                auto telemetry = populatePluginTelemetry(*itr);
                result.push_back(telemetry);
            }
        }else{
            throw TelemetryDeserializerException("JSON string has to be an array!");
        }
        return result;
    }

    PluginTelemetry TelemetryDeserializer::deserialzePluginTelemetry(const string& jsonString){        
        Json::Value root = stringToJson(jsonString);
        if(root.isNull()){
            throw TelemetryDeserializerException("Cannot deserialize null value!");
        }
        if(root.isObject()){
            auto telemetry =  populatePluginTelemetry(root);
            return telemetry;
        }else{
             throw TelemetryDeserializerException("JSON string has to be an object!");
        }
    }
} // namespace tmx::utils::telemetry
