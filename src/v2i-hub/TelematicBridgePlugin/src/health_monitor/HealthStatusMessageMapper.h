#pragma once
#include "RSUHealthStatusMessage.h"
#include "UnitHealthStatusMessage.h"
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
                if (rsuStatusJson.isMember("rsuIpAddress") && rsuStatusJson.isMember("rsuSnmpPort"))
                {
                    std::string rsuIp = rsuStatusJson["rsuIpAddress"].asString();
                    
                    // Handle rsuSnmpPort as either string or int
                    int rsuPort = std::stoi(rsuStatusJson["rsuSnmpPort"].asString());                    
                    
                    std::string event = rsuStatusJson.isMember("event") ? 
                                      rsuStatusJson["event"].asString() : "";                    
                    
                                      // Create RSU ID in format "IP:port"
                    std::string rsuId = rsuIp + ":" + std::to_string(rsuPort);

                    // Determine health status from rsuMode field
                    std::string healthStatus = rsuStatusJson.isMember("rsuMode") ? 
                                              rsuStatusJson["rsuMode"].asString() : "0";
                    
                    // Create RSUHealthStatusMessage
                    RSUHealthStatusMessage rsuHealthStatus(
                        rsuIp, 
                        rsuPort, 
                        healthStatus, 
                        event
                    );
                    
                    return rsuHealthStatus;
                }
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