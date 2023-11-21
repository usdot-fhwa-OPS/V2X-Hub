#include "TelematicUnit.h"

namespace TelematicBridge
{
    void TelematicUnit::connect(const string &natsURL, uint16_t natsConnAttempts, uint16_t natsConnTimeout)
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
        setRegistered(false);

        while (!_isRegistered)
        {
            PLOG(logDEBUG2) << "Inside register unit requestor";
            natsMsg *reply = nullptr;
            string payload = "{\"unit_id\":\"" + _unit.unitId + "\"}";
            auto s = natsConnection_RequestString(&reply, _conn, REGISTER_UNIT_TOPIC, payload.c_str(), TIME_OUT);
            if (s == NATS_OK)
            {
                auto responseStr = natsMsg_GetData(reply);
                PLOG(logINFO) << "Received registered reply: " << responseStr;
                auto root = parseJson(responseStr);
                if (root.isMember(LOCATION) && root.isMember(TESTING_TYPE) && root.isMember(EVENT_NAME))
                {
                    _eventLocation = root[LOCATION].asString();
                    _testingType = root[TESTING_TYPE].asString();
                    _eventName = root[EVENT_NAME].asString();

                    // Unit is registered when server responds with event information (location, testing_type, event_name)
                    setRegistered(true);
                }
                natsMsg_Destroy(reply);
            }
            else
            {
                PLOG(logERROR) << "NATS regsiter Error: " << s << "-" << natsStatus_GetText(s);
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

    void TelematicUnit::availableTopicsReplier()
    {
        if (!_subAvailableTopic)
        {
            PLOG(logDEBUG2) << "Inside available topic replier";
            stringstream topic;
            topic << _unit.unitId << AVAILABLE_TOPICS;
            auto s = natsConnection_Subscribe(&_subAvailableTopic, _conn, topic.str().c_str(), onAvailableTopicsCallback, this);
        }
    }

    void TelematicUnit::selectedTopicsReplier()
    {
        if (!_subSelectedTopic)
        {
            PLOG(logDEBUG2) << "Inside selected topic replier";
            stringstream topic;
            topic << _unit.unitId << PUBLISH_TOPICS;
            auto s = natsConnection_Subscribe(&_subSelectedTopic, _conn, topic.str().c_str(), onSelectedTopicsCallback, this);
        }
    }

    void TelematicUnit::checkStatusReplier()
    {
        if (!_subCheckStatus)
        {
            PLOG(logDEBUG2) << "Inside check status replier";
            stringstream topic;
            topic << _unit.unitId << CHECK_STATUS;
            auto s = natsConnection_Subscribe(&_subCheckStatus, _conn, topic.str().c_str(), onCheckStatusCallback, this);
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
        if (natsMsg_GetReply(msg) != nullptr)
        {
            TelematicUnit *obj = (TelematicUnit *)object;
            auto reply = constructAvailableTopicsReplyString(obj->_unit, obj->_availableTopics, obj->_excludedTopics);
            PLOG(logDEBUG3) << "Available topics replied! " << reply;
            natsConnection_PublishString(nc, natsMsg_GetReply(msg), reply.c_str());
            natsMsg_Destroy(msg);
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
            if (root.isMember(TOPICS) && root[TOPICS].isArray())
            {
                TelematicUnit *obj = (TelematicUnit *)object;
                // clear old selected topics
                obj->_selectedTopics.clear();

                // update selected topics with selected topics from latest request
                for (auto itr = root[TOPICS].begin(); itr != root[TOPICS].end(); itr++)
                {
                    obj->_selectedTopics.push_back(itr->asString());
                }
            }
            string reply = "request received!";
            PLOG(logDEBUG3) << "Selected topics replied: " << reply;
            natsConnection_PublishString(nc, natsMsg_GetReply(msg), reply.c_str());
        }
        natsMsg_Destroy(msg);
    }

    void TelematicUnit::onCheckStatusCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object)
    {
        if (natsMsg_GetReply(msg) != nullptr)
        {
            PLOG(logDEBUG3) << "Received check status msg: " << natsMsg_GetSubject(msg) << " " << natsMsg_GetData(msg);
            string payload = "OK";
            PLOG(logDEBUG3) << "Status Check: " << payload;
            natsConnection_PublishString(nc, natsMsg_GetReply(msg), payload.c_str());
            natsMsg_Destroy(msg);
        }
    }

    string TelematicUnit::constructPublishedDataString(const unit_st &unit, const string &_eventLocation, const string &_testingType, const string &_eventName, const string &topicName, const Json::Value payload)
    {
        Json::Value message;
        message[UNIT_ID] = unit.unitId;
        message[UNIT_NAME] = unit.unitName;
        message[UNIT_TYPE] = unit.unitType;
        message[LOCATION] = _eventLocation;
        message[TESTING_TYPE] = _testingType;
        message[EVENT_NAME] = _eventName;
        message[TOPIC_NAME] = topicName;
        message[TIMESTAMP] = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        message[PAYLOAD] = payload;
        Json::FastWriter fasterWirter;
        string jsonStr = fasterWirter.write(message);
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

    string TelematicUnit::constructAvailableTopicsReplyString(const unit_st &unit, const vector<string> &availableTopicList, const string &excludedTopics)
    {
        Json::Value message;
        message[UNIT_ID] = unit.unitId;
        message[UNIT_NAME] = unit.unitName;
        message[UNIT_TYPE] = unit.unitType;
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

    void TelematicUnit::setRegistered(bool isRegistered)
    {
        _isRegistered = isRegistered;
    }

    void TelematicUnit::setUnit(unit_st unit)
    {
        lock_guard<mutex> lock(_unitMutex);
        _unit = unit;
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