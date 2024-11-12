#include "TelemetryDeserializer.h"

namespace tmx::utils::telemetry
{
    boost::property_tree::ptree TelemetryDeserializer::stringToJson( const string& jsonString){
        try{
            boost::property_tree::ptree pt;
            stringstream ss;
            ss << jsonString;
            boost::property_tree::read_json(ss, pt);
            return pt;
        }catch(boost::property_tree::json_parser::json_parser_error error){
            throw TelemetryDeserializerException("Cannot parse JSON string due to error: "+  error.message());
        }
    }


    PluginTelemetry TelemetryDeserializer::populatePluginTelemetry(const boost::property_tree::ptree& value){
        try{
            PluginTelemetry telemetry;
            if(value.empty()){
                throw TelemetryDeserializerException("Cannot deserialize empty JSON!");
            }

            PluginInfo pluginInfo = {
                value.get<string>("id",""),
                value.get<string>("name",""),
                value.get<string>("description",""),
                value.get<string>("version","")
            };
            telemetry.setPluginInfo(pluginInfo);  
            string enabledStr = value.get<string>("enabled","");
            int enabled = enabledStr =="enabled"? 1: enabledStr=="Disabled"?0:-1; 
            PluginInstallation installation = {
                enabled,
                value.get<string>("path",""),
                value.get<string>("exeName",""),
                value.get<string>("manifest",""),
                value.get<string>("maxMessageInterval",""),
                value.get<string>("commandLineParameters",""),
            };
            telemetry.setPluginInstallation(installation);
            return telemetry;
        }catch(const boost::property_tree::ptree_error& error){
            throw TelemetryDeserializerException("Cannot get value from JSON due to error: "+  string(error.what()));
        }
    }

    vector<PluginTelemetry> TelemetryDeserializer::desrializeFullPluginTelemetryPayload(const string& jsonString){
        vector<PluginTelemetry> result;
        auto root = stringToJson(jsonString);        
        if(root.empty()){
            throw TelemetryDeserializerException("JSON cannot be empty!");
        }
        try{
            auto payload = root.get_child("payload");
            if(payload.empty()){
                throw TelemetryDeserializerException("JSON payload cannot be empty!");
            }
            //Payload content is an array of pluginTelemetry  
            BOOST_FOREACH(auto& itr, payload){
                auto telemetry = populatePluginTelemetry(itr.second);
                result.push_back(telemetry);
            }
        }catch(const boost::property_tree::ptree_bad_path & error){
            throw TelemetryDeserializerException("Cannot deserialize JSON as the JSON string has no \"payload\" field!");
        }
        return result;
    }

    PluginTelemetry TelemetryDeserializer::deserialzePluginTelemetry(const string& jsonString){        
        auto root = stringToJson(jsonString);
        if(root.empty()){
            throw TelemetryDeserializerException("Cannot deserialize null value!");
        }
        auto telemetry = populatePluginTelemetry(root);
        return telemetry;
    }
} // namespace tmx::utils::telemetry
