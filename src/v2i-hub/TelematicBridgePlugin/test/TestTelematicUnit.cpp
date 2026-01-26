#include <gtest/gtest.h>
#include "TelematicUnit.h"
using namespace std;
using namespace tmx::utils;
using namespace std::chrono;

namespace TelematicBridge
{
    class TestTelematicUnit : public ::testing::Test
    {
    public:
        shared_ptr<TelematicUnit> _telematicUnitPtr = make_shared<TelematicUnit>();
        unit_st unit =
            {
                "test_id",
                "test_name",
                "infrastructure"};
    };

    TEST_F(TestTelematicUnit, setUnit)
    {
        ASSERT_NO_THROW(_telematicUnitPtr->setUnit(unit));
        ASSERT_EQ(unit.unitId, _telematicUnitPtr->getUnit().unitId);
    }

    TEST_F(TestTelematicUnit, updateExcludedTopics)
    {
        ASSERT_NO_THROW(_telematicUnitPtr->updateExcludedTopics("test_topic"));
    }

    TEST_F(TestTelematicUnit, updateAvailableTopics)
    {
        ASSERT_NO_THROW(_telematicUnitPtr->updateAvailableTopics("test_topic"));
    }

    TEST_F(TestTelematicUnit, constructAvailableTopicsReplyString)
    {
        vector<string> topics = {"test_topic", "excluded_topic"};
        string excluded_topic = "excluded_topic";
        string eventLocation = "location";
        string testingType = "unit_test";
        string eventName = "testing";
        auto reply = TelematicUnit::constructAvailableTopicsReplyString(unit, eventLocation, testingType, eventName, topics, excluded_topic);
        auto json = TelematicUnit::parseJson(reply);
        ASSERT_EQ("test_topic", json["topics"][0]["name"].asString());

        reply = TelematicUnit::constructAvailableTopicsReplyString(unit, eventLocation, testingType, eventName, {}, excluded_topic);
        json = TelematicUnit::parseJson(reply);
        ASSERT_EQ(1, json["topics"].isArray());
    }

    TEST_F(TestTelematicUnit, constructPublishedDataString)
    {
        string eventLocation = "location";
        string testingType = "unit_test";
        string eventName = "testing";
        string topicName = "test_topic";
        Json::Value payload;
        payload["timestamp"] = 1701099016033;

        auto reply = _telematicUnitPtr->constructPublishedDataString(unit, eventLocation, testingType, eventName, topicName, payload);
        auto json = TelematicUnit::parseJson(reply);
        ASSERT_EQ(eventLocation, json["location"].asString());
        ASSERT_EQ(testingType, json["testing_type"].asString());
        ASSERT_EQ(eventName, json["event_name"].asString());
        ASSERT_EQ(1701099016033000, json["timestamp"].asUInt64());
        ASSERT_THROW(TelematicUnit::parseJson("Invalid Json"), TelematicBridgeException);

        Json::Value payload2;
        payload2["body"] = "invalid";
        reply = _telematicUnitPtr->constructPublishedDataString(unit, eventLocation, testingType, eventName, topicName, payload2);
        json = TelematicUnit::parseJson(reply);
        ASSERT_NEAR(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count(), json["timestamp"].asUInt64(), 10000);
    }

    TEST_F(TestTelematicUnit, onCheckStatusCallback)
    {
        natsMsg *msg;
        string data = "{\"data\":\"test\"}";
        natsMsg_Create(&msg, "test_subject", "Test_reply", data.c_str(), data.size());
        ASSERT_NO_THROW(TelematicUnit::onCheckStatusCallback(nullptr, nullptr, msg, nullptr));
    }

    TEST_F(TestTelematicUnit, onSelectedTopicsCallback)
    {
        natsMsg *msg;
        string data = "{\"topics\":[\"test_topic\"]}";
        natsMsg_Create(&msg, "test_subject", "Test_reply", data.c_str(), data.size());
        ASSERT_NO_THROW(TelematicUnit::onSelectedTopicsCallback(nullptr, nullptr, msg, _telematicUnitPtr.get()));
        ASSERT_TRUE(_telematicUnitPtr->inSelectedTopics("test_topic"));
    }

    TEST_F(TestTelematicUnit, onAvailableTopicsCallback)
    {
        natsMsg *msg;
        string data = "{\"data\":\"test\"}";
        natsMsg_Create(&msg, "test_subject", "Test_reply", data.c_str(), data.size());
        ASSERT_NO_THROW(TelematicUnit::onAvailableTopicsCallback(nullptr, nullptr, msg, _telematicUnitPtr.get()));
    }

    TEST_F(TestTelematicUnit, publishMessage)
    {
        string topicName = "test_topic";
        Json::Value payload;
        payload["body"] = "test_body";
        ASSERT_THROW(_telematicUnitPtr->publishMessage(topicName, payload), TelematicBridgeException);
    }

    TEST_F(TestTelematicUnit, publishToNats_WithoutConnection)
    {
        string natsTopic = "test.topic";
        string message = "{\"test\":\"data\"}";
        // Should throw exception when not connected to NATS
        ASSERT_THROW(_telematicUnitPtr->publishToNats(natsTopic, message), TelematicBridgeException);
    }

    TEST_F(TestTelematicUnit, publishToNats_EmptyTopic)
    {
        string natsTopic = "";
        string message = "{\"test\":\"data\"}";
        // Should throw exception with empty topic
        ASSERT_THROW(_telematicUnitPtr->publishToNats(natsTopic, message), TelematicBridgeException);
    }

    TEST_F(TestTelematicUnit, publishToNats_EmptyMessage)
    {
        string natsTopic = "test.topic";
        string message = "";
        // Should throw exception when not connected, even with empty message
        ASSERT_THROW(_telematicUnitPtr->publishToNats(natsTopic, message), TelematicBridgeException);
    }

    TEST_F(TestTelematicUnit, checkStatusReplier)
    {
        _telematicUnitPtr->checkStatusReplier();
    }

    TEST_F(TestTelematicUnit, selectedTopicsReplier)
    {
        _telematicUnitPtr->selectedTopicsReplier();
    }

    TEST_F(TestTelematicUnit, availableTopicsReplier)
    {
        _telematicUnitPtr->availableTopicsReplier();
    }

    TEST_F(TestTelematicUnit, registerUnitRequestor)
    {
        ASSERT_THROW(_telematicUnitPtr->registerUnitRequestor(), TelematicBridgeException);
    }

    TEST_F(TestTelematicUnit, connect)
    {
        ASSERT_THROW(_telematicUnitPtr->connect("nats://127.0.0.1:4222"), TelematicBridgeException);
    }

    TEST_F(TestTelematicUnit, getters)
    {
        ASSERT_EQ(0, _telematicUnitPtr->getAvailableTopics().size());
        ASSERT_EQ("", _telematicUnitPtr->getEventLocation());
        ASSERT_EQ("", _telematicUnitPtr->getEventName());
        ASSERT_EQ("", _telematicUnitPtr->getExcludedTopics());
        ASSERT_EQ("", _telematicUnitPtr->getTestingType());
    }

    TEST_F(TestTelematicUnit, selectedTopics)
    {
        string selectedTopic = "test_selected_topics";
        _telematicUnitPtr->addSelectedTopic(selectedTopic);
        ASSERT_TRUE(_telematicUnitPtr->inSelectedTopics(selectedTopic));
        _telematicUnitPtr->clearSelectedTopics();
        ASSERT_FALSE(_telematicUnitPtr->inSelectedTopics(selectedTopic));
    }
    TEST_F(TestTelematicUnit, validateRegisterStatus)
    {
        string replyStr = "{\"event_name\":\"Test\",\"location\":\"Local\",\"testing_type\":\"Integration\"}";
        _telematicUnitPtr->validateRegisterStatus(replyStr);
        ASSERT_EQ("Local", _telematicUnitPtr->getEventLocation());
        ASSERT_EQ("Test", _telematicUnitPtr->getEventName());
        ASSERT_EQ("Integration", _telematicUnitPtr->getTestingType());
    }

}