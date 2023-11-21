#include "TelematicUnit.h"

namespace TelematicBridge
{
    TelematicUnit::TelematicUnit()
    {
        // natsStatus s;
        // s = natsOptions_SetTimeout(_opts, natsConnTimeout);
        // if (s == NATS_OK)
        // {
        //     natsOptions_SetMaxReconnect(_opts, natsConnAttempts);
        // }

        // if (s == NATS_OK)
        // {
        //     s = natsOptions_SetRetryOnFailedConnect(_opts, true, nullptr, nullptr);
        // }

        // if (s == NATS_OK)
        // {
        // s = natsOptions_SetURL(_opts, natsURL);
        // }

        // if (s != NATS_OK)
        // {
        //     nats_PrintLastErrorStack(stderr);
        //     throw TelematicBridgeException(natsStatus_GetText(s));
        // }
    }

    void TelematicUnit::connect(const string &natsURL, uint16_t natsConnAttempts, uint16_t natsConnTimeout)
    {
        if (!isConnected)
        {
            PLOG(logINFO) << "Trying to connect to " << natsURL << " attempts: " << natsConnAttempts << ", nats connect timeout: " << natsConnTimeout;
            // auto s = natsConnection_Connect(&_conn, _opts);
            auto s = natsConnection_ConnectTo(&_conn, natsURL.c_str());
            PLOG(logINFO) << "natsConnection_Connect returned: " << natsStatus_GetText(s);
            if (s == NATS_OK)
            {
                isConnected = true;
                registerUnitRequestor();
            }
            else
            {
                isConnected = false;
                nats_PrintLastErrorStack(stderr);
                printf("NATS Connection Error: %u - %s\n", s, natsStatus_GetText(s));
                throw TelematicBridgeException(natsStatus_GetText(s));
            }
        }
    }

    void TelematicUnit::setUnit(unit_st unit)
    {
        lock_guard<mutex> lock(_unitMutex);
        _unit = unit;
    }

    void TelematicUnit::updateAvailableTopics(const string &newTopic)
    {
        if (find(availableTopics.begin(), availableTopics.end(), newTopic) == availableTopics.end())
        {
            lock_guard<mutex> lock(_availableTopicsMutex);
            availableTopics.push_back(newTopic);
            PLOG(logDEBUG2) << "Add topic (= " << newTopic << ") to available topics list. Size: " << availableTopics.size();
        }
    }

    void TelematicUnit::registerUnitRequestor()
    {
        while (!isRegistered)
        {
            PLOG(logINFO) << "Inside register unit requestor";
            natsMsg *reply = nullptr;
            string payload = "{\"unit_id\":\"" + _unit.unitId + "\"}";
            auto s = natsConnection_RequestString(&reply, _conn, REGISTER_UNIT_TOPIC, payload.c_str(), TIME_OUT);
            if (s == NATS_OK)
            {
                isRegistered = true;
                PLOG(logINFO) << "Received regitered reply: " << natsMsg_GetData(reply);
                Json::Value root;
                Json::Reader reader;
                bool parsingSuccessful = reader.parse(natsMsg_GetData(reply), root);
                if (!parsingSuccessful)
                {
                    throw TelematicBridgeException("Error parsing the reply message");
                }
                if (root.isMember("location"))
                {
                    _eventLocation = root["location"].asString();
                }
                if (root.isMember("location"))
                {
                    _testingType = root["testing_type"].asString();
                }
                if (root.isMember("event_name"))
                {
                    _eventName = root["event_name"].asString();
                }
                availableTopicsReplier();
                selectedTopicsReplier();
                checkStatusReplier();
            }
            else
            {
                isRegistered = false;
                nats_PrintLastErrorStack(stderr);
                printf("NATS regiter Error: %u - %s\n", s, natsStatus_GetText(s));
            }
            natsMsg_Destroy(reply);
        }
    }

    void TelematicUnit::availableTopicsReplier()
    {
        if (!subAvailableTopic)
        {
            PLOG(logDEBUG2) << "Inside available topic replier";
            stringstream topic;
            topic << _unit.unitId << AVAILABLE_TOPICS;
            auto s = natsConnection_Subscribe(&subAvailableTopic, _conn, topic.str().c_str(), onAvailableTopicsCallback, this);
        }
    }

    void TelematicUnit::selectedTopicsReplier()
    {
        if (!subSelectedTopic)
        {
            PLOG(logDEBUG2) << "Inside selected topic replier";
            stringstream topic;
            topic << _unit.unitId << PUBLISH_TOPICS;
            auto s = natsConnection_Subscribe(&subSelectedTopic, _conn, topic.str().c_str(), onSelectedTopicsCallback, this);
        }
    }

    void TelematicUnit::checkStatusReplier()
    {
        if (!subCheckStatus)
        {
            PLOG(logDEBUG2) << "Inside check status replier";
            stringstream topic;
            topic << _unit.unitId << CHECK_STATUS;
            auto s = natsConnection_Subscribe(&subCheckStatus, _conn, topic.str().c_str(), onCheckStatusCallback, this);
        }
    }

    void TelematicUnit::publishMessage(const string &topic, const Json::Value &payload)
    {
        auto pubMsgTopic = "streets." + _unit.unitId + ".data." + topic;
        Json::Value message;
        message["unit_id"] = _unit.unitId;
        message["unit_name"] = _unit.unitName;
        message["unit_type"] = _unit.unitType;
        message["location"] = _eventLocation;
        message["testing_type"] = _testingType;
        message["event_name"] = _eventName;
        message["topic_name"] = topic;
        message["timestamp"] = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        message["payload"] = payload;
        Json::FastWriter fasterWirter;
        string jsonStr = fasterWirter.write(message);
        auto s = natsConnection_PublishString(_conn, pubMsgTopic.c_str(), jsonStr.c_str());
        if (s != NATS_OK)
        {
            PLOG(logINFO) << "Topic: " << pubMsgTopic << ". Published: " << jsonStr;
        }
        else
        {
            nats_PrintLastErrorStack(stderr);
            throw TelematicBridgeException(natsStatus_GetText(s));
        }
    }

    bool TelematicUnit::inSelectedTopics(const string &topic)
    {
        if (find(selectedTopics.begin(), selectedTopics.end(), topic) == selectedTopics.end())
        {
            return false;
        }
        return true;
    }

    void TelematicUnit::onAvailableTopicsCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object)
    {
        PLOG(logDEBUG3) << "Received available topics: " << natsMsg_GetSubject(msg) << " " << natsMsg_GetData(msg);
        // Sends a reply
        if (natsMsg_GetReply(msg) != NULL)
        {
            TelematicUnit *obj = (TelematicUnit *)object;
            Json::Value payload;
            payload["unit_id"] = obj->_unit.unitId;
            payload["unit_name"] = obj->_unit.unitName;
            payload["unit_type"] = obj->_unit.unitType;
            payload["timestamp"] = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            Json::Value topics;
            for (const auto &topic : obj->availableTopics)
            {
                Json::Value topicJson;
                topicJson["name"] = topic;
                topics.append(topicJson);
            }
            payload["topics"] = topics;
            Json::FastWriter fasterWirter;
            string jsonStr = fasterWirter.write(payload);
            PLOG(logDEBUG2) << "Available topics replied! " << jsonStr;
            natsConnection_PublishString(nc, natsMsg_GetReply(msg), jsonStr.c_str());
        }
        natsMsg_Destroy(msg);
    }

    void TelematicUnit::onSelectedTopicsCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object)
    {
        PLOG(logDEBUG3) << "Received selected topics: " << natsMsg_GetSubject(msg) << " " << natsMsg_GetData(msg);
        // Sends a reply
        if (natsMsg_GetReply(msg) != NULL)
        {
            TelematicUnit *obj = (TelematicUnit *)object;
            Json::Value root;
            Json::Reader reader;
            auto msgStr = natsMsg_GetData(msg);
            bool parsingSuccessful = reader.parse(msgStr, root);
            if (!parsingSuccessful)
            {
                throw TelematicBridgeException("Error parsing the string");
            }
            if (root["topics"].isArray())
            {
                for (auto itr = root["topics"].begin(); itr != root["topics"].end(); itr++)
                {
                    obj->selectedTopics.push_back(itr->asString());
                }
            }
            string payload = "request received!";
            PLOG(logDEBUG2) << "Selected topics replied: " << payload;
            natsConnection_PublishString(nc, natsMsg_GetReply(msg), payload.c_str());
        }
        natsMsg_Destroy(msg);
    }

    void TelematicUnit::onCheckStatusCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object)
    {
        if (natsMsg_GetReply(msg) != NULL)
        {
            PLOG(logDEBUG3) << "Received check status msg: " << natsMsg_GetSubject(msg) << " " << natsMsg_GetData(msg);
            string payload = "OK";
            PLOG(logDEBUG3) << "Status Check: " << payload;
            natsConnection_PublishString(nc, natsMsg_GetReply(msg), payload.c_str());
            natsMsg_Destroy(msg);
        }
    }

    TelematicUnit::~TelematicUnit()
    {
        natsOptions_Destroy(_opts);
        natsConnection_Destroy(_conn);
        _conn = nullptr;
    }
}