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
        string unitName; // Descriptive name for each unit
        string unitType; // Unit categorized base on unit type: platform or infrastructure
    };

    class TelematicUnit
    {
    private:
        mutex _unitMutex;
        mutex _availableTopicsMutex;
        mutex _excludedTopicsMutex;
        unit_st _unit;                                                        // Global variable to store the unit information
        vector<string> _availableTopics;                                      // Global variable to store available topics
        string _excludedTopics;                                               // Global variable to store topics that are excluded by the users
        vector<string> _selectedTopics;                                       // Global variable to store selected topics confirmed by users
        static CONSTEXPR const char *AVAILABLE_TOPICS = ".available_topics";  // NATS subject to pub/sub available topics
        static CONSTEXPR const char *REGISTER_UNIT_TOPIC = "*.register_unit"; // NATS subject to pub/sub registering unit
        static CONSTEXPR const char *PUBLISH_TOPICS = ".publish_topics";      // NATS subject to publish data stream
        static CONSTEXPR const char *CHECK_STATUS = ".check_status";          // NATS subject to pub/sub checking unit status
        natsConnection *_conn = nullptr;                                      // Global NATS connection object
        natsSubscription *_subAvailableTopic = nullptr;                       // Global NATS subscription object
        natsSubscription *_subSelectedTopic = nullptr;                        // Global NATS subscription object
        natsSubscription *_subCheckStatus = nullptr;                          // Global NATS subscription object
        int64_t TIME_OUT = 10000;                                             // NATS Connection time out in  milliseconds
        string _eventName;                                                    // Testing event the unit is assigned to
        string _eventLocation;                                                // Testing event location
        string _testingType;                                                  // Testing type
        static CONSTEXPR const char *LOCATION = "location";                   // location key used to find location value from JSON
        static CONSTEXPR const char *TESTING_TYPE = "testing_type";           // testing_type key used to find testing_type value from JSON
        static CONSTEXPR const char *EVENT_NAME = "event_name";               // event_name key used to find event_name value from JSON
        static CONSTEXPR const char *UNIT_ID = "unit_id";                     // unit_id key used to find unit_id value from JSON
        static CONSTEXPR const char *UNIT_NAME = "unit_name";                 // unit_name key used to find unit_name value from JSON
        static CONSTEXPR const char *UNIT_TYPE = "unit_type";                 // unit_type key used to find unit_type value from JSON
        static CONSTEXPR const char *TOPIC_NAME = "topic_name";               // topic_name key used to find topic_name value from JSON
        static CONSTEXPR const char *TIMESTAMP = "timestamp";                 // timestamp key used to find timestamp value from JSON
        static CONSTEXPR const char *PAYLOAD = "payload";                     // payload key used to find payload value from JSON
        static CONSTEXPR const char *TOPICS = "topics";                       // topics key used to find topics value from JSON
        static CONSTEXPR const char *NAME = "name";                           // topics key used to find topics value from JSON
        static const uint16_t MILLI_TO_MICRO = 1000;

    public:
        /**
         *@brief Construct telematic unit
         */
        explicit TelematicUnit() = default;
        /**
         * @brief A function for telematic unit to connect to NATS server. Throw exception is connection failed.         *
         * @param string string NATS server URL
         */
        void connect(const string &natsURL);

        /**
         * @brief A NATS requestor for telematic unit to send register request to NATS server.
         * If receives a response, it will update the isRegistered flag to indicate the unit is registered.
         * If no response after the specified time out (unit of second) period, it considered register failed.
         * */
        void registerUnitRequestor();

        /**
         * @brief A NATS replier to subscribe to NATS server and receive available topics request.
         * Publish list of available topics after receiving/processing the request.
         */
        void availableTopicsReplier();

        /**
         * @brief A NATS replier to subscribe to NATS server and receive requested topics from telematic server.
         * Process the request and update the selectedTopics global variable.
         * Respond the telematic server with acknowledgement.
         */
        void selectedTopicsReplier();

        /**
         * @brief A NATS replier to subscribe to NATS server and receive request for status check from telematic server.
         * Publish unit status upon receiving a request.
         */
        void checkStatusReplier();

        /**
         * @brief A function to publish message stream into NATS server
         */
        void publishMessage(const string &topic, const Json::Value &payload);

        /**
         * @brief A function to parse a JSON string and create a JSON object.
         * @param string input json string
         * @return Json::Value
         */
        static Json::Value parseJson(const string &jsonStr);

        /**
         * @brief construct available topic response
         * @param unit_st struct that contains unit related information
         * @param vector of available topics
         * @param string Excluded topics separated by commas
         */
        static string constructAvailableTopicsReplyString(const unit_st &unit, const string &eventLocation, const string &testingType, const string &eventName, const vector<string> &availableTopicList, const string &excludedTopics);

        /**
         * @brief A function to update available topics global variables when discovering a new topic.
         */
        void updateAvailableTopics(const string &newTopic);

        /**
         * @brief Update telematic unit registration status with the registration reply from NATS server
         * @param string Register reply in Json format
         * @return True when status are validated, otherwise false.
        */
        bool validateRegisterStatus(const string& registerReply);

        /**
         * @brief construct Json data string that will be streamed into the cloud by a publisher
         * @param unit_st struct that contains unit related information
         * @param string Event location
         * @param string Testing type
         * @param string Event name
         * @param string Topic name is a combination of type_subtype_source from TMX IVPMessage
         * @param Json::Value Payload is the actual data generated by V2xHub plugin
         */
        string constructPublishedDataString(const unit_st &unit, const string &eventLocation, const string &testingType, const string &eventName, const string &topicName, const Json::Value &payload) const;

        /**
         * @brief A function to update global unit variable
         * @param unit_st object that has the unit id, type and name information
         */
        void setUnit(const unit_st &unit);

        /**
         * @brief Return unit structure
         */
        unit_st getUnit() const;

        /**
         * @brief Return list of available topics
         */
        vector<string> getAvailableTopics() const;

        /**
         * @brief Return excluded topics string.
         */
        string getExcludedTopics() const;

        /**
         * @brief Return event name
         */
        string getEventName() const;

        /**
         * @brief Return event location
         */
        string getEventLocation() const;

        /**
         * @brief Return testing type
         */
        string getTestingType() const;

        /**
         * @brief Add new selected topic into the selected topics list
         */
        void addSelectedTopic(const string &newSelectedTopic);

        /**
         * @brief Clear selected topics list
         */
        void clearSelectedTopics();

        /**
         * @brief A function to update excluded topics.
         * @param string Excluded topics separated by commas
         */
        void updateExcludedTopics(const string &excludedTopics);

        /**
         * @brief Check if the given topic is inside the selectedTopics list
         * @param string A topic to check for existence
         * @return boolean indicator whether the input topic eixst.
         */
        bool inSelectedTopics(const string &topic);

        /**
         * @brief A callback function for available topic replier
         */
        static void onAvailableTopicsCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object);

        /**
         * @brief A callback function for selected topic replier
         */
        static void onSelectedTopicsCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object);

        /**
         * @brief A callback function for check status replier
         */
        static void onCheckStatusCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object);

        ~TelematicUnit();
    };

} // namespace TelematicBridge
