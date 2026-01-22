#pragma once

#include <string>
#include <jsoncpp/json/json.h>

namespace TelematicBridge
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
        std::string rsuId;      ///< Unique RSU identifier (IP:port)

        /**
         * @brief Generate RSU ID from IP and port
         * @param ip IP address
         * @param port Port number
         * @return RSU ID in format "ip:port"
         */
        static std::string generateRsuId(const std::string &ip, int port)
        {
            return ip + ":" + std::to_string(port);
        }

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
            rsuId = generateRsuId(ip, port);
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
            json["rsuId"] = rsuId;
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
            if (json.isMember("rsuId")) msg.rsuId = json["rsuId"].asString();
            return msg;
        }

        /**
         * @brief Equality operator
         * @param other Other message to compare
         * @return True if messages are equal
         */
        bool operator==(const RSUHealthStatusMessage &other) const
        {
            return ip == other.ip &&
                   port == other.port &&
                   status == other.status &&
                   event == other.event &&
                   rsuId == other.rsuId;
        }

        /**
         * @brief Inequality operator
         * @param other Other message to compare
         * @return True if messages are not equal
         */
        bool operator!=(const RSUHealthStatusMessage &other) const
        {
            return !(*this == other);
        }
    };

} // namespace TelematicBridge
