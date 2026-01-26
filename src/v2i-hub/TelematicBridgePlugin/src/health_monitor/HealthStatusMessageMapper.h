#pragma once

#include "RSUHealthStatusMessage.h"
#include "UnitHealthStatusMessage.h"
#include "../TelematicJsonKeys.h"
#include <tmx/messages/routeable_message.hpp>
#include <jsoncpp/json/json.h>
#include <sstream>
namespace TelematicBridge
{   
    class HealthStatusMessageMapper
    {
    public:
        static RSUHealthStatusMessage toRsuHealthStatusMessage(tmx::routeable_message &msg) 
        {
            auto payloadMsg = msg.get_payload_str();
            
            // Parse JSON content
            Json::CharReaderBuilder reader;
            Json::Value rsuStatusJson;
            std::string errs;
            std::istringstream s(payloadMsg);
            
            if (Json::parseFromStream(reader, s, &rsuStatusJson, &errs))
            {
                // Extract RSU information from JSON
                if (!rsuStatusJson.isMember(TelematicJsonKeys::RSU_IP_ADDRESS))
                {
                    throw std::runtime_error("Missing required field: rsuIpAddress");
                }
                if (!rsuStatusJson.isMember(TelematicJsonKeys::RSU_SNMP_PORT))
                {
                    throw std::runtime_error("Missing required field: rsuSnmpPort");
                }
                
                std::string rsuIp = rsuStatusJson[TelematicJsonKeys::RSU_IP_ADDRESS].asString();
                
                // Handle rsuSnmpPort as either string or int
                int rsuPort = std::stoi(rsuStatusJson[TelematicJsonKeys::RSU_SNMP_PORT].asString());                    
                
                std::string event = rsuStatusJson.isMember(TelematicJsonKeys::EVENT) ? 
                                  rsuStatusJson[TelematicJsonKeys::EVENT].asString() : "";   

                // Determine health status from rsuMode field
                std::string healthStatus = rsuStatusJson.isMember(TelematicJsonKeys::RSU_MODE) ? 
                                          rsuStatusJson[TelematicJsonKeys::RSU_MODE].asString() : "0";
                
                // Create RSUHealthStatusMessage
                RSUHealthStatusMessage rsuHealthStatus(
                    rsuIp, 
                    rsuPort, 
                    healthStatus, 
                    event
                );
                
                return rsuHealthStatus;
            }
            else
            {
               throw std::runtime_error("Error parsing RSU status JSON: " + errs);
            }
        }

        static UnitHealthStatusMessage toUnitHealthStatusMessage(const std::string& unitId, const std::string& status, int64_t timestamp)
        {
            // Create UnitHealthStatusMessage
            UnitHealthStatusMessage unitStatus;
            unitStatus.setUnitId(unitId.empty() ? "unknown" : unitId);
            unitStatus.setBridgePluginStatus(status);
            unitStatus.setLastUpdatedTimestamp(timestamp);
            return unitStatus;
        }
    };
}