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

        static std::string statusToLabel (const std::string &status)
        {
            if (status == "1")
                return "other";
            else if (status == "2")
                return "standby";
            else if (status == "3")
                return "operate";
            else
                return "unknown";
        }

        /**
         * @brief Convert to JSON representation
         * @return JSON::Value object representing the message
         */
        Json::Value toJson() const
        {
            Json::Value json;
            
            // Create nested RSU object
            Json::Value rsuObject;
            rsuObject["ip"] = ip;
            rsuObject["port"] = port;
                        
            json["rsu"] = rsuObject;
            json["status"] = statusToLabel(status);
            json["event"] = event;
            return json;
        }

        /**
         * @brief Create from JSON representation
         * @param json JSON object
         * @return RSUHealthStatusMessage object
         */
        static RSUHealthStatusMessage fromJson(const Json::Value &json)
        {
            std::string ip;
            int port = 0;
            std::string status;
            std::string event;
            // Parse from nested RSU object
            if (json.isMember("rsu") && json["rsu"].isObject())
            {
                const Json::Value &rsuObject = json["rsu"];
                if (rsuObject.isMember("ip")) ip = rsuObject["ip"].asString();
                if (rsuObject.isMember("port")) port = rsuObject["port"].asInt();
            }
            
            if (json.isMember("status")) status = json["status"].asString();
            if (json.isMember("event")) event = json["event"].asString();

            RSUHealthStatusMessage msg(ip, port, status, event );
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

        std::string getRsuId() const { 
            return rsuId;
        }

        std::string getStatus() const { 
            return statusToLabel(status); 
        }

        std::string toString() const
        {
            Json::Value json = toJson();
            Json::FastWriter writer;
            return writer.write(json);
        }
    };

} // namespace TelematicBridge
