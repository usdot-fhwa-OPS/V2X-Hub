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
        
        // NATS subject suffix for RSU available topics. Full topic format: unit.<unit_id>.topic.rsu.available_topics
        static CONSTEXPR const char *RSU_AVAILABLE_TOPICS = ".topic.rsu.available_topics";
        
        // NATS subject suffix for RSU selected topics. Full topic format: unit.<unit_id>.topic.rsu.selected_topics
        static CONSTEXPR const char *RSU_SELECTED_TOPICS = ".topic.rsu.selected_topics";

        std::unique_ptr<truConfigWorker> _truConfigWorkerptr;
        
        // Map to track available topics per RSU (key: "ip:port", value: set of topics)
        std::unordered_map<std::string, std::unordered_set<std::string>> _rsuTopicsMap;
        std::mutex _rsuTopicsMutex;
        
        // Map to track selected topics per RSU (key: "ip:port", value: set of selected topic names)
        std::unordered_map<std::string, std::unordered_set<std::string>> _rsuSelectedTopicsMap;
        std::mutex _rsuSelectedTopicsMutex;


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
         * @brief Update the available topics for a specific RSU
         * @param rsuId RSU identifier in format "ip:port"
         * @param topic Topic name to add to the RSU's available topics
         */
        void updateRsuTopics(const std::string &rsuId, const std::string &topic);
        
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
         * @brief Destructor for TelematicRsuUnit
         * Cleans up NATS subscriptions and connections
        */
        ~TelematicRsuUnit();
    };

}