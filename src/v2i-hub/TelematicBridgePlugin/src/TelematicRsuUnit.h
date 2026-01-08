#pragma once
#include <nats/nats.h>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include "PluginLog.h"
#include "ThreadTimer.h"
#include "TelematicBridgeException.h"
#include "RSUConfigWorker.h"
#include "TelematicUnit.h"
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string.hpp>

namespace TelematicBridge
{
    struct unitConfig
    {
        std::string unitId;   // Unique identifier for each unit
        int16_t maxConnections;   // Number of maximum RSUs supported by plugin
        int16_t pluginHeartBeatInterval; // Configurable interval at which the plugin heartbeat should be monitored
        int16_t healthMonitorPluginHeartbeatInterval; // Configurable interval at which the RSU Health Monitor heartbeat should be monitored
        int16_t rsuStatusMonitorInterval; // Configurable interval at which the RSU status should be monitored
    };

    struct truUnit
    {
        unitConfig unit;
        std::vector<rsuConfig> registeredRsuList; //List of configurations for connected RSUs
        int64_t timestamp;
    };

    class TelematicRsuUnit: public TelematicUnit
    {
    private:
        // The TRU (Telematic RSU) unit configuration. Contains unit settings, registered RSU list, and timestamp
        truUnit _truUnit;
        // NATS subscription for RSU configuration status updates. Used to receive RSU configuration changes from the management service
        natsSubscription *_subRegisteredRSUStatus = nullptr;

        //Maximum number of connection attempts to NATS server before failing
        static const int CONNECTION_MAX_ATTEMPTS = 30;

        //Unit topic names
        // NATS subject suffix for RSU registration configuration. Full topic format: {unitId}.register.rsu.config ; Used to publish/subscribe RSU registration data
        static CONSTEXPR const char *REGISTERD_RSU_CONFIG = ".register.rsu.config";

        //Unit json keys
        static CONSTEXPR const char *UNIT_KEY = "unit";
        static CONSTEXPR const char *UNIT_ID_KEY = "unitid";
        static CONSTEXPR const char *MAX_CONNECTIONS_KEY = "maxconnections";
        static CONSTEXPR const char *PLUGIN_HEARTBEAT_INTERVAL_KEY = "bridgepluginheartbeatinterval";
        static CONSTEXPR const char *HEALTHMONITOR_HEARTBEAT_INTERVAL_KEY = "healthmonitorpluginheartbeatinterval";
        static CONSTEXPR const char *RSU_STATUS_MONITOR_INTERVAL_KEY = "rsustatusmonitorinterval";
        static CONSTEXPR const char *RSU_CONFIGS_KEY = "rsuconfigs";


    public:
        /**
         *@brief Construct telematic unit
         */
        explicit TelematicRsuUnit();

        /**
         * @brief A function for telematic unit to connect to NATS server. Throw exception is connection failed.         *
         * @param string string NATS server URL
         */
        void connect(const std::string &natsURL) override;

        /**
         * @brief A NATS requestor for telematic unit to send register request to NATS server.
         * If receives a response, it will update the isRegistered flag to indicate the unit is registered.
         * If no response after the specified time out (unit of second) period, it considered register failed.
         * */
        void registerUnitRequestor() override;

        /**
         * @brief Set up RSU configuration replier to handle RSU config updates via NATS
         * Publishes initial RSU configuration and subscribes to receive config updates
         */
        void rsuConfigReplier();

        /**
         * @brief Update RSU status from incoming configuration message
         *
         * Processes an incoming RSU configuration message by:
         * 1. Validating the message is intended for this unit by checking unit ID
         * 2. Processing any RSU configurations in the message
         * 3. Updating the unit's timestamp if present in the message
         *
         * @param jsonVal JSON value containing RSU configuration update message.
         *
         * @return bool true if message was successfully processed or if message is valid but empty,
         *              false if message is for different unit or if RSU config processing fails
         */
        bool updateRSUStatus(const Json::Value& jsonVal);

        /**
         * @brief Publish the RSU registration message containing unit and RSU configurations
         * @param topic The NATS topic to publish the registration message to
         * @throws TelematicBridgeException if publish fails
         */
        void publishRSURegistrationMessage(const std::string &topic);

        /**
         * @brief Construct a JSON string containing RSU registration data
         * Includes unit configuration, registered RSU list, and timestamp
         * @return std::string JSON formatted RSU registration data
        */
        std::string constructRSURegistrationDataString();

        /**
         * @brief NATS callback handler for RSU configuration status updates
         * Processes incoming RSU configuration changes and updates the registered RSU list
         * @param nc NATS connection pointer
         * @param sub NATS subscription pointer
         * @param msg NATS message containing RSU configuration updates
         * @param object Pointer to TelematicRsuUnit instance
         */
        static void onRSUConfigStatusCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object);

        /**
         * @brief Destructor for TelematicRsuUnit
         * Cleans up NATS subscriptions and connections
        */
        ~TelematicRsuUnit();
    };

}