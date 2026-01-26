#pragma once

#include <string>
#include <jsoncpp/json/json.h>
#include "../TelematicJsonKeys.h"

namespace TelematicBridge
{
    /**
     * @class UnitHealthStatusMessage
     * @brief Represents unit health status information
     * 
     * This class encapsulates the health status message for a communication unit
     * including unit ID, bridge plugin status, and last communication timestamp.
     * 
     * JSON Properties:
     *  - unitId: Unique identifier for the unit
     *  - bridgePluginStatus: Status of the bridge plugin for this unit
     *  - LastUpdatedTimestamp: Timestamp of the last status update of the unit
     */
    class UnitHealthStatusMessage
    {
    private:
        std::string unitId;                    ///< Unique identifier for the unit
        std::string bridgePluginStatus;        ///< Status of the bridge plugin for this unit
        long LastUpdatedTimestamp;       ///< Timestamp of the last status update of the unit

    public:
        /**
         * @brief Default constructor
         */
        UnitHealthStatusMessage() : LastUpdatedTimestamp(0) {}

        /**
         * @brief Constructor with parameters
         * @param unitId Unique identifier for the unit
         * @param bridgePluginStatus Status of the bridge plugin
         * @param LastUpdatedTimestamp Last status change timestamp
         */
        UnitHealthStatusMessage(const std::string &unitId, 
                               const std::string &bridgePluginStatus,
                               long LastUpdatedTimestamp)
            : unitId(unitId), 
              bridgePluginStatus(bridgePluginStatus),
              LastUpdatedTimestamp(LastUpdatedTimestamp) {}

        /**
         * @brief Destructor
         */
        ~UnitHealthStatusMessage() = default;

        // Getters
        /**
         * @brief Get the unit ID
         * @return Unit ID string
         */
        const std::string& getUnitId() const { return unitId; }

        /**
         * @brief Get the bridge plugin status
         * @return Bridge plugin status string
         */
        const std::string& getBridgePluginStatus() const { return bridgePluginStatus; }

        /**
         * @brief Get the last communication timestamp
         * @return Last communication timestamp in milliseconds
         */
        long getLastUpdatedTimestamp() const { return LastUpdatedTimestamp; }

        // Setters
        /**
         * @brief Set the unit ID
         * @param id Unit ID to set
         */
        void setUnitId(const std::string &id) { unitId = id; }

        /**
         * @brief Set the bridge plugin status
         * @param status Bridge plugin status to set
         */
        void setBridgePluginStatus(const std::string &status) { bridgePluginStatus = status; }

        /**
         * @brief Set the last communication timestamp
         * @param timestamp Timestamp to set
         */
        void setLastUpdatedTimestamp(long timestamp) { LastUpdatedTimestamp = timestamp; }

        /**
         * @brief Convert to JSON representation
         * @return JSON::Value object representing the message
         */
        Json::Value toJson() const
        {
            Json::Value json;
            json[TelematicJsonKeys::UNIT_ID] = unitId;
            json[TelematicJsonKeys::BRIDGE_PLUGIN_STATUS] = bridgePluginStatus;
            json[TelematicJsonKeys::LAST_UPDATED_TIMESTAMP] = LastUpdatedTimestamp;
            return json;
        }

        /**
         * @brief Create from JSON representation
         * @param json JSON object
         * @return UnitHealthStatusMessage object
         */
        static UnitHealthStatusMessage fromJson(const Json::Value &json)
        {
            UnitHealthStatusMessage msg;
            if (json.isMember(TelematicJsonKeys::UNIT_ID)) 
                msg.unitId = json[TelematicJsonKeys::UNIT_ID].asString();
            if (json.isMember(TelematicJsonKeys::BRIDGE_PLUGIN_STATUS)) 
                msg.bridgePluginStatus = json[TelematicJsonKeys::BRIDGE_PLUGIN_STATUS].asString();
            if (json.isMember(TelematicJsonKeys::LAST_UPDATED_TIMESTAMP)) 
                msg.LastUpdatedTimestamp = json[TelematicJsonKeys::LAST_UPDATED_TIMESTAMP].asInt64();
            return msg;
        }

        /**
         * @brief Get string representation of the message as JSON
         * @return JSON string representation
         */
        std::string toString() const
        {
            Json::FastWriter writer;
            return writer.write(toJson());
        }

        /**
         * @brief Equality operator
         * @param other Other message to compare
         * @return True if messages are equal
         */
        bool operator==(const UnitHealthStatusMessage &other) const
        {
            return unitId == other.unitId &&
                   bridgePluginStatus == other.bridgePluginStatus &&
                   LastUpdatedTimestamp == other.LastUpdatedTimestamp;
        }

        /**
         * @brief Inequality operator
         * @param other Other message to compare
         * @return True if messages are not equal
         */
        bool operator!=(const UnitHealthStatusMessage &other) const
        {
            return !(*this == other);
        }
    };

} // namespace TelematicBridge
