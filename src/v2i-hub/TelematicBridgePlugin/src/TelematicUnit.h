#pragma once
#include <nats/nats.h>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include "PluginLog.h"
#include "ThreadTimer.h"
#include "TelematicBridgeException.h"
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace tmx::utils;
using namespace std::chrono;

namespace TelematicBridge
{
    using unit_st = struct unit
    {
        string unitId;   // Unique identifier for each unit
        string unitType; // Unit categorized base on unit type: platform or infrastructure
        string unitName; // Descriptive name for each unit
    };

    class TelematicUnit
    {
    public:
        mutex _unitMutex;
        mutex _availableTopicsMutex;
        atomic<bool> _isConnected{false};
        atomic<bool> _isRegistered{false};
        unit_st _unit;                                                        // Global variable to store the unit information
        vector<string> availableTopics;                                       // Global variable to store available topics
        string excludedTopics;                                                // Global variable to store topics that are excluded by the users
        vector<string> selectedTopics;                                        // Global variable to store selected topics confirmed by users
        static CONSTEXPR const char *AVAILABLE_TOPICS = ".available_topics";  // NATS subject to pub/sub available topics
        static CONSTEXPR const char *REGISTER_UNIT_TOPIC = "*.register_unit"; // NATS subject to pub/sub registering unit
        static CONSTEXPR const char *PUBLISH_TOPICS = ".publish_topics";      // NATS subject to publish data stream
        static CONSTEXPR const char *CHECK_STATUS = ".check_status";          // NATS subject to pub/sub checking unit status
        unique_ptr<ThreadTimer> _natsRegisterTh;
        natsConnection *_conn = nullptr; // Global NATS connection object
        natsOptions *_opts = nullptr;
        natsSubscription *subAvailableTopic = nullptr;
        natsSubscription *subSelectedTopic = nullptr;
        natsSubscription *subCheckStatus = nullptr;
        int64_t TIME_OUT = 10000;
        string _eventName;
        string _eventLocation;
        string _testingType;

        /**
         *@brief Construct telematic unit
         */
        explicit TelematicUnit() = default;
        /**
         * @brief A function for telematic unit to connect to NATS server. Throw exception is connection failed.         *
         * @param const string NATS server URL
         * @param uint16_t The numbers of attempts to make connections to NATS server
         * @param uint16_t The timeout for between connection attempts
         */
        void connect(const string &natsURL, uint16_t natsConnAttempts, uint16_t natsConnTimeout);

        /**
         * @brief A function to update available topics global variables when discovering a new topic.
         */
        void updateAvailableTopics(const string &newTopic);

        /**
         * @brief A function to publish message stream into NATS server
         */
        void publishMessage(const string &topic, const Json::Value &payload);

        /**
         * @brief A function to update global unit variable
         * @param unit_st object that has the unit id, type and name information
         */
        void setUnit(unit_st unit);

        /**
         * @brief Check if the given topic is inside the selectedTopics list
         * @param string A topic to check for existence
         * @return boolean indicator whether the input topic eixst.
         */
        bool inSelectedTopics(const string &topic);

        /**
         * @brief A NATS requestor for telematic unit to send register request to NATS server.
         * If receives a response, it will update the isRegistered flag to indicate the unit is registered.
         * If no response after the specified time out (unit of second) period, it considered register failed.
         * */
        void registerUnitRequestor();

        /**
         * @brief A NATS replier to publish available topics upon receiving a request for a list of available topics.
         */
        void availableTopicsReplier();

        /**
         * @brief A NATS replier to subscribe to NATS server and receive requested topics from telematic server.
         * Process the request and update the selectedTopics global variable.
         * Respond the telematic server with acknowledgement.
         */
        void selectedTopicsReplier();

        /**
         * @brief A NATS replier to publish unit status upon receiving a request for status check from telematic server.
         */
        void checkStatusReplier();

        static void onAvailableTopicsCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object);
        static void onSelectedTopicsCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object);
        static void onCheckStatusCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object);

        ~TelematicUnit();
    };

} // namespace TelematicBridge
