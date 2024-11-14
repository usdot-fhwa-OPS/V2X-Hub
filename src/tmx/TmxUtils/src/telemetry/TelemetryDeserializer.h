#pragma once
#include <vector>
#include <jsoncpp/json/json.h>
#include <tmx/messages/message.hpp>
#include <boost/foreach.hpp>

#include "PluginTelemetry.h"
#include "TelemetryDeserializerException.h"
#include "TelemetryMetadata.h"

using namespace std;

namespace tmx::utils::telemetry
{
    class TelemetryDeserializer
    {
    public:
        TelemetryDeserializer() = default;
        /***
         * @brief Deserialize JSON string into a vector of telemetry objects
         * @param JSON string object 
         * @return Vector of telemetry objects
         */
        template <typename T>
        static vector<T> desrializeFullTelemetryPayload(const boost::property_tree::ptree& jsonContainer){
            vector<T> result;      
            if(jsonContainer.empty()){
                throw TelemetryDeserializerException("JSON cannot be empty!");
            }
            try{
                auto payload = jsonContainer.get_child(PAYLOAD_STRING);
                if(payload.empty()){
                    throw TelemetryDeserializerException("JSON payload cannot be empty!");
                }
                //Payload content is an array of T  
                BOOST_FOREACH(auto& itr, payload){
                    T telemetry;
                    telemetry.deserialize(itr.second);
                    result.push_back(telemetry);
                }
            }catch(const boost::property_tree::ptree_bad_path& error){
                throw TelemetryDeserializerException("Cannot deserialize JSON as the JSON string has no \"payload\" field!");
            }
            return result;
        }
        /***
         * @brief Convert JSON String into boost::property_tree::ptree object
         * @param JSON string object 
         * @return boost::property_tree::ptree
         */
        static boost::property_tree::ptree stringToJson( const string& jsonString);
        ~TelemetryDeserializer() = default;
    };

} // namespace tmx::utils::telemetry