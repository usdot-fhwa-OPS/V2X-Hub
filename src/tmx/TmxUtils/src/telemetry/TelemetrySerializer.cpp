#include "TelemetrySerializer.h"

namespace tmx::utils::telemetry{
    tmx::message_container_type TelemetrySerializer::serializeUpdatedTelemetry(const map<string, string>& updates){
        tmx::message_container_type pt;
        for(const auto& update : updates){
            pt.store(message_path_type(update.first), update.second);
        }
        return pt;
    }

    tmx::message_container_type TelemetrySerializer::composeUpdatedTelemetryPayload(const map<string, tmx::message_container_type>& updates){
        tmx::message_container_type outputContainer;
        boost::property_tree::ptree ptArray;
        for(auto update: updates){
            ptArray.push_back(boost::property_tree::ptree::value_type("",update.second.get_storage().get_tree()));
        }
        outputContainer.get_storage().get_tree().put_child(PAYLOAD_STRING, ptArray);
        return outputContainer;
    }

    string TelemetrySerializer::composeCompleteTelemetry(tmx::message_container_type& header, tmx::message_container_type& payload){
        header.get_storage().get_tree().push_back(boost::property_tree::ptree::value_type(PAYLOAD_STRING,payload.get_storage().get_tree().get_child(PAYLOAD_STRING)));
        return treeToString(header);
    }

    tmx::message_container_type TelemetrySerializer::serializeTelemetryHeader(const TelemetryHeader& header){
        tmx::message_container_type headerContainer;
        boost::property_tree::ptree element;
        element.push_back(boost::property_tree::ptree::value_type(TYPE_STRING, header.type));
        element.push_back(boost::property_tree::ptree::value_type(SUBTYPE_STRING, header.subtype));
        element.push_back(boost::property_tree::ptree::value_type(ENCODING_STRING, header.encoding));
        element.push_back(boost::property_tree::ptree::value_type(TIMESTAMP_STRING,to_string(header.timestamp)));
        element.push_back(boost::property_tree::ptree::value_type(FLAGS_STRING,header.flags));
        headerContainer.get_storage().get_tree().push_back(boost::property_tree::ptree::value_type(HEADER_STRING, element));
        return headerContainer;
    }

    string TelemetrySerializer::treeToString(tmx::message_container_type& container){
        stringstream ss;
        ss.clear();
        ss.str(string());
        boost::property_tree::write_json(ss, container.get_storage().get_tree(), false);
        string jsonString = ss.str();
        boost::erase_all(jsonString,"\n");
        return jsonString;
    }
}