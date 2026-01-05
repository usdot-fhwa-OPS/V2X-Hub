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
        truUnit _truUnit;

        natsSubscription *_subRegisteredRSUStatus = nullptr;

        static const int CONNECTION_MAX_ATTEMPTS = 30;

        //Unit topic names
        static CONSTEXPR const char *REGISTERD_RSU_CONFIG = ".register.rsu.config"; // NATS subject to pub/sub connected RSU units

        //Unit json keys
        static CONSTEXPR const char *UNIT_KEY = "unit";
        static CONSTEXPR const char *UNIT_ID_KEY = "unitId";
        static CONSTEXPR const char *MAX_CONNECTIONS_KEY = "maxConnections";
        static CONSTEXPR const char *PLUGIN_HEARTBEAT_INTERVAL_KEY = "bridgePluginHeartbeatInterval";
        static CONSTEXPR const char *HEALTHMONITOR_HEARTBEAT_INTERVAL_KEY = "healthMonitorPluginHeartbeatInterval";
        static CONSTEXPR const char *RSU_STATUS_MONITOR_INTERVAL_KEY = "rsuStatusMonitorInterval";
        static CONSTEXPR const char *RSU_CONFIGS_KEY = "rsuConfigs";


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
        void registerUnitRequestor();


        void rsuConfigReplier();

        Json::Value getRSURegistrationConfigJson();


        void publishMessage(const std::string &topic, const Json::Value &payload);


        /**
         * @brief construct available topic response
         * @param truUnit struct that contains unit related information
         * @param vector of available topics
         * @param string Excluded topics separated by commas
         */
        static std::string constructAvailableTopicsReplyString(const truUnit &unit, const std::string &eventLocation, const std::string &testingType, const std::string &eventName, const std::vector<std::string> &availableTopicList, const std::string &excludedTopics);

        void publishRSURegistrationMessage(const std::string &topic);


        std::string constructRSURegistrationDataString();

        static void onRSUConfigStatusCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object);


        ~TelematicRsuUnit();
    };

}