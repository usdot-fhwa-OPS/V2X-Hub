#include "TelematicUnit.h"

namespace TelematicBridge
{
    void TelematicUnit::connect(const string &natsURL)
    {
        auto s = natsConnection_ConnectTo(&_conn, natsURL.c_str());
        PLOG(logINFO) << "NATS connection returned: " << natsStatus_GetText(s);
        if (s == NATS_OK)
        {
            registerUnitRequestor();
        }
        else
        {
            throw TelematicBridgeException(natsStatus_GetText(s));
        }
    }

    void TelematicUnit::registerUnitRequestor()
    {
        // Reset registration status
        _isRegistered = false;

        while (!_isRegistered)
        {
            PLOG(logDEBUG2) << "Inside register unit requestor";
            natsMsg *reply = nullptr;
            string payload = "{\"unit_id\":\"" + _unit.unitId + "\"}";
            auto s = natsConnection_RequestString(&reply, _conn, REGISTER_UNIT_TOPIC, payload.c_str(), TIME_OUT);
            if (s == NATS_OK)
            {
                auto replyStr = natsMsg_GetData(reply);
                PLOG(logINFO) << "Received registered reply: " << replyStr;
                updateRegisterStatus(replyStr);
                natsMsg_Destroy(reply);
            }
            else
            {
                throw TelematicBridgeException(natsStatus_GetText(s));
            }
            sleep(1);
        }

        if (_isRegistered)
        {
            // Provide below services when the unit is registered
            availableTopicsReplier();
            selectedTopicsReplier();
            checkStatusReplier();
        }
    }

    void TelematicUnit::updateRegisterStatus(const string &registerReply)
    {
        auto root = parseJson(registerReply);
        if (root.isMember(LOCATION) && root.isMember(TESTING_TYPE) && root.isMember(EVENT_NAME))
        {
            _eventLocation = root[LOCATION].asString();
            _testingType = root[TESTING_TYPE].asString();
            _eventName = root[EVENT_NAME].asString();

            // Unit is registered when server responds with event information (location, testing_type, event_name)
            _isRegistered = true;
        }
    }

    void TelematicUnit::availableTopicsReplier()
    {
        if (!_subAvailableTopic)
        {
            PLOG(logDEBUG2) << "Inside available topic replier";
            stringstream topic;
            topic << _unit.unitId << AVAILABLE_TOPICS;
            natsConnection_Subscribe(&_subAvailableTopic, _conn, topic.str().c_str(), onAvailableTopicsCallback, this);
        }
    }

    void TelematicUnit::selectedTopicsReplier()
    {
        if (!_subSelectedTopic)
        {
            PLOG(logDEBUG2) << "Inside selected topic replier";
            stringstream topic;
            topic << _unit.unitId << PUBLISH_TOPICS;
            natsConnection_Subscribe(&_subSelectedTopic, _conn, topic.str().c_str(), onSelectedTopicsCallback, this);
        }
    }

    void TelematicUnit::checkStatusReplier()
    {
        if (!_subCheckStatus)
        {
            PLOG(logDEBUG2) << "Inside check status replier";
            stringstream topic;
            topic << _unit.unitId << CHECK_STATUS;
            natsConnection_Subscribe(&_subCheckStatus, _conn, topic.str().c_str(), onCheckStatusCallback, this);
        }
    }

    void TelematicUnit::publishMessage(const string &topic, const Json::Value &payload)
    {
        auto pubMsgTopic = "streets." + _unit.unitId + ".data." + topic;
        auto jsonStr = constructPublishedDataString(_unit, _eventLocation, _testingType, _eventName, topic, payload);
        auto s = natsConnection_PublishString(_conn, pubMsgTopic.c_str(), jsonStr.c_str());
        if (s == NATS_OK)
        {
            PLOG(logINFO) << "Topic: " << pubMsgTopic << ". Published: " << jsonStr;
        }
        else
        {
            throw TelematicBridgeException(natsStatus_GetText(s));
        }
    }

    void TelematicUnit::onAvailableTopicsCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object)
    {
        PLOG(logDEBUG3) << "Received available topics: " << natsMsg_GetSubject(msg) << " " << natsMsg_GetData(msg);
        // Sends a reply
        if (object && natsMsg_GetReply(msg) != nullptr)
        {
            const auto obj = (TelematicUnit *)object;
            auto reply = constructAvailableTopicsReplyString(obj->getUnit(), obj->getEventLocation(), obj->getTestingType(), obj->getEventName(), obj->getAvailableTopics(), obj->getExcludedTopics());
            auto s = natsConnection_PublishString(nc, natsMsg_GetReply(msg), reply.c_str());
            natsMsg_Destroy(msg);
            if (s == NATS_OK)
            {
                PLOG(logDEBUG3) << "Available topics replied: " << reply;
            }
        }
    }

    void TelematicUnit::onSelectedTopicsCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object)
    {
        PLOG(logDEBUG3) << "Received selected topics: " << natsMsg_GetSubject(msg) << " " << natsMsg_GetData(msg);
        // Sends a reply
        if (natsMsg_GetReply(msg) != nullptr)
        {
            auto msgStr = natsMsg_GetData(msg);
            auto root = parseJson(msgStr);
            if (object && root.isMember(TOPICS) && root[TOPICS].isArray())
            {
                auto obj = (TelematicUnit *)object;
                // clear old selected topics
                obj->clearSelectedTopics();

                // update selected topics with selected topics from latest request
                for (auto itr = root[TOPICS].begin(); itr != root[TOPICS].end(); itr++)
                {
                    obj->addSelectedTopic(itr->asString());
                }
            }
            string reply = "request received!";
            auto s = natsConnection_PublishString(nc, natsMsg_GetReply(msg), reply.c_str());
            natsMsg_Destroy(msg);
            if (s == NATS_OK)
            {
                PLOG(logDEBUG3) << "Selected topics replied: " << reply;
            }
        }
    }

    void TelematicUnit::onCheckStatusCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object)
    {
        if (natsMsg_GetReply(msg) != nullptr)
        {
            auto s = natsConnection_PublishString(nc, natsMsg_GetReply(msg), "OK");
            if (s == NATS_OK)
            {
                PLOG(logDEBUG3) << "Received check status msg: " << natsMsg_GetSubject(msg) << " " << natsMsg_GetData(msg) << ". Replied: OK";
            }
            natsMsg_Destroy(msg);
        }
    }

    string TelematicUnit::constructPublishedDataString(const unit_st &unit, const string &eventLocation, const string &testingType, const string &eventName, const string &topicName, const Json::Value &payload) const
    {
        Json::Value message;
        message[UNIT_ID] = unit.unitId;
        message[UNIT_NAME] = unit.unitName;
        message[UNIT_TYPE] = unit.unitType;
        message[LOCATION] = eventLocation;
        message[TESTING_TYPE] = testingType;
        message[EVENT_NAME] = eventName;
        message[TOPIC_NAME] = topicName;
        message[TIMESTAMP] = payload.isMember("timestamp") ? payload["timestamp"].asUInt64() : duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        message[PAYLOAD] = payload;
        Json::FastWriter fasterWirter;
        string jsonStr = fasterWirter.write(message);
        return jsonStr;
    }

    Json::Value TelematicUnit::parseJson(const string &jsonStr)
    {
        Json::Value root;
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(jsonStr, root);
        if (!parsingSuccessful)
        {
            throw TelematicBridgeException("Error parsing the reply message");
        }
        return root;
    }

    string TelematicUnit::constructAvailableTopicsReplyString(const unit_st &unit, const string &eventLocation, const string &testingType, const string &eventName, const vector<string> &availableTopicList, const string &excludedTopics)
    {
        Json::Value message;
        message[UNIT_ID] = unit.unitId;
        message[UNIT_NAME] = unit.unitName;
        message[UNIT_TYPE] = unit.unitType;
        message[LOCATION] = eventLocation;
        message[TESTING_TYPE] = testingType;
        message[EVENT_NAME] = eventName;
        message[TIMESTAMP] = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        Json::Value topics;
        for (const auto &topic : availableTopicList)
        {
            if (!boost::icontains(excludedTopics, topic))
            {
                Json::Value topicJson;
                topicJson[NAME] = topic;
                topics.append(topicJson);
            }
        }
        message[TOPICS] = topics;
        Json::FastWriter fasterWirter;
        string reply = fasterWirter.write(message);
        return reply;
    }

    void TelematicUnit::updateAvailableTopics(const string &newTopic)
    {
        if (find(_availableTopics.begin(), _availableTopics.end(), newTopic) == _availableTopics.end())
        {
            lock_guard<mutex> lock(_availableTopicsMutex);
            _availableTopics.push_back(newTopic);
            PLOG(logINFO) << "Add topic (= " << newTopic << ") to available topics list. Size: " << _availableTopics.size();
        }
    }

    void TelematicUnit::updateExcludedTopics(const string &excludedTopics)
    {
        lock_guard<mutex> lock(_excludedTopicsMutex);
        _excludedTopics = excludedTopics;
    }

    bool TelematicUnit::inSelectedTopics(const string &topic)
    {
        if (find(_selectedTopics.begin(), _selectedTopics.end(), topic) == _selectedTopics.end())
        {
            return false;
        }
        return true;
    }

    void TelematicUnit::setUnit(const unit_st &unit)
    {
        lock_guard<mutex> lock(_unitMutex);
        _unit = unit;
    }

    unit_st TelematicUnit::getUnit() const
    {
        return _unit;
    }

    string TelematicUnit::getEventName() const
    {
        return _eventName;
    }

    string TelematicUnit::getEventLocation() const
    {
        return _eventLocation;
    }

    string TelematicUnit::getTestingType() const
    {
        return _testingType;
    }

    vector<string> TelematicUnit::getAvailableTopics() const
    {
        return _availableTopics;
    }

    string TelematicUnit::getExcludedTopics() const
    {
        return _excludedTopics;
    }

    void TelematicUnit::addSelectedTopic(const string &newSelectedTopic)
    {
        _selectedTopics.push_back(newSelectedTopic);
    }

    void TelematicUnit::clearSelectedTopics()
    {
        _selectedTopics.clear();
    }

    TelematicUnit::~TelematicUnit()
    {
        natsSubscription_Destroy(_subAvailableTopic);
        natsSubscription_Destroy(_subSelectedTopic);
        natsSubscription_Destroy(_subCheckStatus);
        natsConnection_Destroy(_conn);
        _conn = nullptr;
        _subAvailableTopic = nullptr;
        _subSelectedTopic = nullptr;
        _subCheckStatus = nullptr;
    }
}