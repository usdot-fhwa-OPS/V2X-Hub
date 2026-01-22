#pragma once

#include <vector>
#include <chrono>
#include <jsoncpp/json/json.h>
#include "UnitHealthStatusMessage.h"
#include "RSUHealthStatusMessage.h"

namespace TelematicBridge
{
    /**
     * @class TRUHealthStatusMessage
     * @brief Represents the health status of a TRU (Telematics RSU Unit)
     * 
     * This class aggregates RSU health status messages and unit health status information,
     * providing a comprehensive view of the overall TRU health.
     * 
     * JSON Properties:
     *  - rsuConfigs: List of RSU health status messages
     *  - unitConfig: Unit health status message
     */
    class TRUHealthStatusMessage
    {
    private:
        std::vector<RSUHealthStatusMessage> rsuHealthStatus;  ///< Collection of RSU health statuses
        UnitHealthStatusMessage unitHealthStatus;                                ///< Unit health status

    public:
        /**
         * @brief Default constructor
         */
        TRUHealthStatusMessage() = default;

        /**
         * @brief Constructor with parameters
         * @param rsuHealthStatus Vector of RSU health status messages
         * @param unitHealthStatus Unit health status message
         */
        TRUHealthStatusMessage(const std::vector<RSUHealthStatusMessage> &rsuHealthStatus,
                              const UnitHealthStatusMessage &unitHealthStatus)
            : rsuHealthStatus(rsuHealthStatus), unitHealthStatus(unitHealthStatus) {}

        /**
         * @brief Destructor
         */
        ~TRUHealthStatusMessage() = default;

        // Getters
        /**
         * @brief Get the RSU health status list
         * @return Vector of RSU health status messages
         */
        const std::vector<RSUHealthStatusMessage>& getRsuHealthStatus() const
        {
            return rsuHealthStatus;
        }

        /**
         * @brief Get the unit health status
         * @return Unit health status message
         */
        const UnitHealthStatusMessage& getUnitHealthStatus() const
        {
            return unitHealthStatus;
        }

        // Setters
        /**
         * @brief Set the RSU health status list
         * @param status Vector of RSU health status messages
         */
        void setRsuHealthStatus(const std::vector<RSUHealthStatusMessage> &status)
        {
            rsuHealthStatus = status;
        }

        /**
         * @brief Add an RSU health status message
         * @param status RSU health status message to add
         */
        void addRsuHealthStatus(const RSUHealthStatusMessage &status)
        {
            rsuHealthStatus.push_back(status);
        }

        /**
         * @brief Clear all RSU health status messages
         */
        void clearRsuHealthStatus()
        {
            rsuHealthStatus.clear();
        }

        /**
         * @brief Set the unit health status
         * @param status Unit health status message
         */
        void setUnitHealthStatus(const UnitHealthStatusMessage &status)
        {
            unitHealthStatus = status;
        }

        /**
         * @brief Convert to JSON representation
         * @return JSON::Value object representing the message
         */
        Json::Value toJson() const
        {
            Json::Value json;
            
            // Convert RSU health status list
            Json::Value rsuConfigsJson(Json::arrayValue);
            for (const auto &rsuStatus : rsuHealthStatus)
            {
                rsuConfigsJson.append(rsuStatus.toJson());
            }
            json["rsuConfigs"] = rsuConfigsJson;
            
            // Convert unit health status
            json["unitConfig"] = unitHealthStatus.toJson();
            
            return json;
        }

        /**
         * @brief Create from JSON representation
         * @param json JSON object
         * @return TRUHealthStatusMessage object
         */
        static TRUHealthStatusMessage fromJson(const Json::Value &json)
        {
            TRUHealthStatusMessage msg;
            
            // Parse RSU health status list
            if (json.isMember("rsuConfigs") && json["rsuConfigs"].isArray())
            {
                for (const auto &rsuJson : json["rsuConfigs"])
                {
                    msg.rsuHealthStatus.push_back(RSUHealthStatusMessage::fromJson(rsuJson));
                }
            }
            
            // Parse unit health status
            if (json.isMember("unitConfig"))
            {
                msg.unitHealthStatus = UnitHealthStatusMessage::fromJson(json["unitConfig"]);
            }
            
            return msg;
        }

        /**
         * @brief Get string representation of the message as JSON
         * @return JSON string representation
         */
        std::string toString() const
        {
            Json::Value json;
            
            // Convert unit health status
            json["unitConfig"] = unitHealthStatus.toJson();
            
            // Convert RSU health status list
            Json::Value rsuConfigsJson(Json::arrayValue);
            for (const auto &rsuStatus : rsuHealthStatus)
            {
                rsuConfigsJson.append(rsuStatus.toJson());
            }
            json["rsuConfigs"] = rsuConfigsJson;
            
            // Add top-level timestamp (current time in milliseconds since epoch)
            json["timestamp"] = std::to_string(getCurrentTimestamp());
            
            // Convert to string with minimal formatting
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "  ";
            return Json::writeString(builder, json);
        }

        /**
         * @brief Get the number of RSU health statuses
         * @return Number of RSU health status messages
         */
        size_t getRsuHealthStatusCount() const
        {
            return rsuHealthStatus.size();
        }

        int64_t getCurrentTimestamp() const
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }

        /**
         * @brief Check if RSU health status list is empty
         * @return True if no RSU statuses are present
         */
        bool isRsuHealthStatusEmpty() const
        {
            return rsuHealthStatus.empty();
        }

        /**
         * @brief Equality operator
         * @param other Other message to compare
         * @return True if messages are equal
         */
        bool operator==(const TRUHealthStatusMessage &other) const
        {
            return rsuHealthStatus == other.rsuHealthStatus &&
                   unitHealthStatus == other.unitHealthStatus;
        }

        /**
         * @brief Inequality operator
         * @param other Other message to compare
         * @return True if messages are not equal
         */
        bool operator!=(const TRUHealthStatusMessage &other) const
        {
            return !(*this == other);
        }
    };

} // namespace TelematicBridge
