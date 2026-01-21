#pragma once

#include <string>
#include <jsoncpp/json/json.h>

namespace TelematicBridgePlugin
{
    /**
     * @class RSUHealthStatusMessage
     * @brief Represents RSU health status information
     * 
     * This class encapsulates the health status message for an RSU including
     * IP address, port, status, and event information.
     */
    class RSUHealthStatusMessage
    {
    public:
        std::string ip;          ///< IP address of the RSU
        int port;                ///< SNMP port of the RSU
        std::string status;      ///< Health status of the RSU
        std::string event;       ///< Event description or information
        std::string rsu_id;      ///< Unique RSU identifier (IP:port)

        /**
         * @brief Default constructor
         */
        RSUHealthStatusMessage() : port(0) {}

        /**
         * @brief Constructor with parameters
         * @param ip IP address
         * @param port SNMP port
         * @param status Health status
         * @param event Event information
         */
        RSUHealthStatusMessage(const std::string &ip, int port, 
                              const std::string &status, 
                              const std::string &event)
            : ip(ip), port(port), status(status), event(event)
        {
            // Generate rsu_id as combination of IP and port
            rsu_id = ip + ":" + std::to_string(port);
        }

        /**
         * @brief Convert to JSON representation
         * @return JSON::Value object representing the message
         */
        Json::Value toJson() const
        {
            Json::Value json;
            json["ip"] = ip;
            json["port"] = port;
            json["status"] = status;
            json["event"] = event;
            json["rsu_id"] = rsu_id;
            return json;
        }

        /**
         * @brief Create from JSON representation
         * @param json JSON object
         * @return RSUHealthStatusMessage object
         */
        static RSUHealthStatusMessage fromJson(const Json::Value &json)
        {
            RSUHealthStatusMessage msg;
            if (json.isMember("ip")) msg.ip = json["ip"].asString();
            if (json.isMember("port")) msg.port = json["port"].asInt();
            if (json.isMember("status")) msg.status = json["status"].asString();
            if (json.isMember("event")) msg.event = json["event"].asString();
            if (json.isMember("rsu_id")) msg.rsu_id = json["rsu_id"].asString();
            return msg;
        }
    };

} // namespace RSUHealthMonitor
