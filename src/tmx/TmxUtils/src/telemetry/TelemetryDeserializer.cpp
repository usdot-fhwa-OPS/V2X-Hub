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
} // namespace tmx::utils::telemetry
