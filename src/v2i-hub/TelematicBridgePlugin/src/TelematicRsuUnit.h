#pragma once
#include <nats/nats.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <mutex>
#include "PluginLog.h"
#include "ThreadTimer.h"
#include "TelematicBridgeException.h"
#include "RSUConfigWorker.h"
#include "TelematicUnit.h"
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string.hpp>
#include "health_monitor/TRUHealthStatusTracker.h"
#include "health_monitor/RSUHealthStatusMessage.h"
#include "health_monitor/HealthStatusMessageMapper.h"
#include "data_selection/TRUTopicsMessage.h"
#include <boost/algorithm/string/replace.hpp>

namespace TelematicBridge
{

    class TelematicRsuUnit: public TelematicUnit
    {
    private:
        // NATS subscription for RSU configuration status updates. Used to receive RSU configuration changes from the management service
        natsSubscription *_subRegisteredRSUStatus = nullptr;
        
        // NATS subscription for RSU available topics requests
        natsSubscription *_subRsuAvailableTopics = nullptr;
        
        // NATS subscription for RSU selected topics updates
        natsSubscription *_subRsuSelectedTopics = nullptr;

        //Maximum number of connection attempts to NATS server before failing
        static const int CONNECTION_MAX_ATTEMPTS = 30;

        //Unit topic names
        // NATS subject suffix for RSU registration configuration. Full topic format: {unitId}.register.rsu.config ; Used to publish/subscribe RSU registration data
        static CONSTEXPR const char *REGISTERD_RSU_CONFIG = ".register.rsu.config";
        
        // NATS subject suffix for plugin health status monitoring. Full topic format: unit.<unit_id>.monitor.plugin.health_status
        static CONSTEXPR const char *HEALTH_STATUS_TOPIC_SUFFIX = "monitor.plugin.health_status";
        
        // NATS subject suffix for RSU health status monitoring. Full topic format: unit.<unit_id>.monitor.rsu.health_status
        static CONSTEXPR const char *RSU_HEALTH_STATUS_TOPIC_SUFFIX = "monitor.rsu.health_status";
        
        // NATS subject suffix for RSU available topics. Full topic format: unit.<unit_id>.topic.rsu.available_topics
        static CONSTEXPR const char *RSU_AVAILABLE_TOPICS = ".topic.rsu.available_topics";
        
        // NATS subject suffix for RSU selected topics. Full topic format: unit.<unit_id>.topic.rsu.selected_topics
        static CONSTEXPR const char *RSU_SELECTED_TOPICS = ".topic.rsu.selected_topics";

        std::unique_ptr<truConfigWorker> _truConfigWorkerptr;
        
        // Map to track available topics per RSU (key: "ip:port", value: set of topics)
        std::unordered_map<std::string, std::unordered_set<std::string>> _rsuAvailableTopicsMap;
        std::mutex _rsuAvailableTopicsMutex;
        
        // Map to track selected topics per RSU (key: "ip:port", value: set of selected topic names)
        std::unordered_map<std::string, std::unordered_set<std::string>> _rsuSelectedTopicsMap;
        std::mutex _rsuSelectedTopicsMutex;

        // TRU health status tracker for monitoring TRU and RSU health
        TRUHealthStatusTracker _truHealthStatusTracker;


    public:
        /**
         *@brief Construct telematic unit
         */
        explicit TelematicRsuUnit();

        /**
         * @brief A function for telematic unit to connect to NATS server. Throw exception is connection failed.         *
         * @param string string NATS server URL
         */
        bool connect(const std::string &natsURL) override;

        /**
         * @brief A NATS requestor for telematic unit to send register request to NATS server.
         * If receives a response, it will update the isRegistered flag to indicate the unit is registered.
         * If no response after the specified time out (unit of second) period, it considered register failed.
         * */
        bool registerRsuUnitRequestor();

        /**
         * @brief Set up RSU configuration replier to handle RSU config updates via NATS
         * Publishes initial RSU configuration and subscribes to receive config updates
         */
        void rsuConfigReplier();
        
        /**
         * @brief A NATS replier to subscribe and receive RSU available topics request.
         * Publishes list of available topics grouped by RSU after receiving/processing the request.
         */
        void rsuAvailableTopicsReplier();
        
        /**
         * @brief A NATS replier to subscribe and receive RSU selected topics updates.
         * Processes the selected topics and updates the internal state, then responds with confirmation.
         */
        void rsuSelectedTopicsReplier();
        
        /**
         * @brief Update available topics for a specific RSU
         * Adds the topic to the available topics list for the specified RSU
         * @param rsuIp RSU IP address
         * @param topic Topic name to add to the RSU's available topics
         */
        void updateRsuAvailableTopics(const std::string &rsuIp, const std::string &topic);
        
        /**
         * @brief Check if a topic is selected for a specific RSU
         * @param rsuIp RSU IP address
         * @param topic Topic name to check
         * @return true if the topic is selected for the specified RSU, false otherwise
         */
        bool inRsuSelectedTopics(const std::string &rsuIp, const std::string &topic);
        
        /**
         * @brief Publish message to RSU-specific NATS topic
         * Publishes the message to topic format: unit.<unit_id>.stream.rsu.<rsu_ip>.<topic_name>
         * @param rsuIp RSU IP address
         * @param rsuPort RSU port number
         * @param topic Topic name
         * @param message JSON message to publish
         */
        void publishRsuDataStream(const std::string &rsuIp, int rsuPort, const std::string &topic, const Json::Value &message);

        
        /**
         * @brief Construct published RSU data string with metadata and payload
         * Creates a JSON message with metadata section containing unit info, RSU endpoint, topic, and timestamp,
         * and a payload section containing the actual message data
         * @param unitId Unit identifier
         * @param rsuIp RSU IP address
         * @param rsuPort RSU port number
         * @param topicName Topic name
         * @param payload Message payload
         * @return JSON formatted string
         */
        std::string constructPublishedRsuDataStream(const std::string &unitId,                                                      const std::string &rsuIp, int rsuPort,
                                                     const std::string &topicName, const Json::Value &payload) const;
        
        /**
         * @brief Construct JSON response for RSU available topics request
         * @return std::string JSON formatted string with structure:
         *         {
         *           "unitId": "unit_id",
         *           "rsuTopics": [
         *             {
         *               "topics": [{"name": "bsm", "selected": false}, ...],
         *               "rsu": {"ip": "192.168.1.11", "port": 502}
         *             }
         *           ],
         *           "timestamp": "1765343631000"
         *         }
         */
        std::string constructRsuAvailableTopicsReplyString();

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
         * @brief Construct a JSON string containing RSU registration data
         * Includes unit configuration, registered RSU list, and timestamp
         * @return std::string JSON formatted RSU registration data
        */
        std::string constructRSURegistrationDataString();

        /**
         * @brief Construct a JSON response string for RSU configuration registration status
         *
         * Creates a JSON response message containing the unit information, list of registered
         * RSU configurations, registration status, and timestamp.
         *
         * @param isRegistrationSuccessful Boolean indicating if RSU registration was successful
         * @return std::string JSON-formatted response string with structure:
         *         {
         *           "unit": { "unitId": "..." },
         *           "rsuConfig": [ { "eventName": "...", "rsu": { "IP": "...", "Port": ... } }, ... ],
         *           "status": "success" | "failed",
         *           "timestamp": <timestamp>
         *         }
         */
        std::string constructRSUConfigResponseDataString(bool isRegistrationSuccessful);

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
         * @brief NATS callback handler for RSU available topics requests
         * Processes incoming requests for RSU available topics and sends reply with topics grouped by RSU
         * @param nc NATS connection pointer
         * @param sub NATS subscription pointer
         * @param msg NATS message containing the request
         * @param object Pointer to TelematicRsuUnit instance
         */
        static void onRsuAvailableTopicsCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object);
        
        /**
         * @brief NATS callback handler for RSU selected topics updates
         * Processes incoming selected topics updates and sends confirmation reply
         * @param nc NATS connection pointer
         * @param sub NATS subscription pointer
         * @param msg NATS message containing the selected topics
         * @param object Pointer to TelematicRsuUnit instance
         */
        static void onRsuSelectedTopicsCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object);

        /**
         * @brief Process incoming config update and generate response
         * @param incomingConfig JSON configuration from server
         * @return std::pair<bool, std::string> Success status and response JSON string
         */
        std::pair<bool, std::string> processConfigUpdateAndGenerateResponse(const Json::Value& incomingConfig);

        /**
         * @brief Get Topic string for RSU Configuration subscription
         * @return std::string for rsu configuration topic.
         */
        std::string getRsuConfigTopic();

        /**
         * @brief Update an RSU health status in the TRU tracker
         * @param status The RSU health status message
         */
        void updateRsuHealthStatus(const RSUHealthStatusMessage &status);

        /**
         * @brief Update unit health status in TRU tracker
         * @param status The unit health status message
         */
        void updateUnitHealthStatus(const std::string &status);

        /**
         * @brief Helper method to publish health status data to NATS
         * @param topicSuffix The suffix to append to "unit.<unit_id>."
         */
        void PublishHealthStatusToNATS(const std::string &topicSuffix);
        
        /**
         * @brief Get plugin heartbeat interval from configuration
         * @return int Heartbeat interval in seconds
         */
        int getPluginHeartBeatInterval() const
        {
            return _truConfigWorkerptr->getPluginHeartBeatInterval();
        }
        /**
         * @brief Publish RSU health status to NATS
         * Publishes the current TRU health status snapshot to the RSU health status topic
         */
        void PublishRSUHealthStatus();

        /**
         * @brief Publish plugin health status to NATS
         * Publishes the current TRU health status snapshot to the plugin health status topic
         */
        void PublishPluginHealthStatus();

        /**
         * @brief Destructor for TelematicRsuUnit
         * Cleans up NATS subscriptions and connections
        */
        ~TelematicRsuUnit();
    };

}