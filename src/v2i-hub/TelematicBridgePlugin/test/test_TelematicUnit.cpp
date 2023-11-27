#include <gtest/gtest.h>
#include "TelematicUnit.h"

namespace TelematicBridge
{
    class test_TelematicUnit : public ::testing::Test
    {
    public:
        shared_ptr<TelematicUnit> _telematicUnitPtr = make_shared<TelematicUnit>();
        unit_st unit =
            {
                "test_id",
                "test_name",
                "infrastructure"};
    };

    TEST_F(test_TelematicUnit, setUnit)
    {
        ASSERT_NO_THROW(_telematicUnitPtr->setUnit(unit));
        ASSERT_EQ(unit.unitId, _telematicUnitPtr->getUnit().unitId);
    }

    TEST_F(test_TelematicUnit, updateExcludedTopics)
    {
        ASSERT_NO_THROW(_telematicUnitPtr->updateExcludedTopics("test_topic"));
    }

    TEST_F(test_TelematicUnit, updateAvailableTopics)
    {
        ASSERT_NO_THROW(_telematicUnitPtr->updateAvailableTopics("test_topic"));
    }

    TEST_F(test_TelematicUnit, constructAvailableTopicsReplyString)
    {
        vector<string> topics = {"test_topic", "excluded_topic"};
        string excluded_topic = "excluded_topic";
        string eventLocation = "location";
        string testingType = "unit_test";
        string eventName = "testing";
        auto reply = TelematicUnit::constructAvailableTopicsReplyString(unit, eventLocation, testingType, eventName, topics, excluded_topic);
        auto json = TelematicUnit::parseJson(reply);
        ASSERT_EQ("test_topic", json["topics"][0]["name"].asString());
    }

    TEST_F(test_TelematicUnit, constructPublishedDataString)
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
    }

    TEST_F(test_TelematicUnit, onCheckStatusCallback)
    {
        natsMsg *msg;
        string data = "{\"data\":\"test\"}";
        natsMsg_Create(&msg, "test_subject", "Test_reply", data.c_str(), data.size());
        ASSERT_NO_THROW(TelematicUnit::onCheckStatusCallback(nullptr, nullptr, msg, nullptr));
    }

    TEST_F(test_TelematicUnit, onSelectedTopicsCallback)
    {
        natsMsg *msg;
        string data = "{\"topics\":[\"test_topic\"]}";
        natsMsg_Create(&msg, "test_subject", "Test_reply", data.c_str(), data.size());
        ASSERT_NO_THROW(TelematicUnit::onSelectedTopicsCallback(nullptr, nullptr, msg, _telematicUnitPtr.get()));
        ASSERT_TRUE(_telematicUnitPtr->inSelectedTopics("test_topic"));
    }

    TEST_F(test_TelematicUnit, onAvailableTopicsCallback)
    {
        natsMsg *msg;
        string data = "{\"data\":\"test\"}";
        natsMsg_Create(&msg, "test_subject", "Test_reply", data.c_str(), data.size());
        ASSERT_NO_THROW(TelematicUnit::onAvailableTopicsCallback(nullptr, nullptr, msg, _telematicUnitPtr.get()));
    }

    TEST_F(test_TelematicUnit, publishMessage)
    {
        string topicName = "test_topic";
        Json::Value payload;
        payload["body"] = "test_body";
        ASSERT_THROW(_telematicUnitPtr->publishMessage(topicName, payload), TelematicBridgeException);
    }

    TEST_F(test_TelematicUnit, checkStatusReplier)
    {
        _telematicUnitPtr->checkStatusReplier();
    }

    TEST_F(test_TelematicUnit, selectedTopicsReplier)
    {
        _telematicUnitPtr->selectedTopicsReplier();
    }

    TEST_F(test_TelematicUnit, availableTopicsReplier)
    {
        _telematicUnitPtr->availableTopicsReplier();
    }

    TEST_F(test_TelematicUnit, registerUnitRequestor)
    {
        ASSERT_THROW(_telematicUnitPtr->registerUnitRequestor(), TelematicBridgeException);
    }

    TEST_F(test_TelematicUnit, connect)
    {
        ASSERT_THROW(_telematicUnitPtr->connect("nats://127.0.0.1:4222"), TelematicBridgeException);
    }

    TEST_F(test_TelematicUnit, getters)
    {
        ASSERT_EQ(0, _telematicUnitPtr->getAvailableTopics().size());
        ASSERT_EQ("", _telematicUnitPtr->getEventLocation());
        ASSERT_EQ("", _telematicUnitPtr->getEventName());
        ASSERT_EQ("", _telematicUnitPtr->getExcludedTopics());
        ASSERT_EQ("", _telematicUnitPtr->getTestingType());
    }

    TEST_F(test_TelematicUnit, selectedTopics)
    {
        string selectedTopic = "test_selected_topics";
        _telematicUnitPtr->addSelectedTopic(selectedTopic);
        ASSERT_TRUE(_telematicUnitPtr->inSelectedTopics(selectedTopic));
        _telematicUnitPtr->clearSelectedTopics();
        ASSERT_FALSE(_telematicUnitPtr->inSelectedTopics(selectedTopic));
    }
    TEST_F(test_TelematicUnit, validateRegisterStatus)
    {
        string replyStr = "{\"event_name\":\"Test\",\"location\":\"Local\",\"testing_type\":\"Integration\"}";        
        _telematicUnitPtr->validateRegisterStatus(replyStr);
        ASSERT_EQ("Local", _telematicUnitPtr->getEventLocation());
        ASSERT_EQ("Test", _telematicUnitPtr->getEventName());
        ASSERT_EQ("Integration", _telematicUnitPtr->getTestingType());
    }
    
}