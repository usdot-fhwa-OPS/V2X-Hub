#include "TelematicRsuUnit.h"
#include "data_selection/TRUTopicsMessage.h"

using namespace std;
using namespace tmx::utils;
using namespace std::chrono;

namespace TelematicBridge
{
    TelematicRsuUnit::TelematicRsuUnit()
    {
        const char* rsuConfigPath = std::getenv("RSU_CONFIG_PATH");
        _truConfigWorkerptr = std::make_unique<truConfigWorker>();
        if (rsuConfigPath != nullptr) {
            PLOG(logINFO) << "Loading RSU configuration from: " << rsuConfigPath;
            if (!_truConfigWorkerptr->loadRSUConfigListFromFile(rsuConfigPath))
            {
                PLOG(logERROR)<<"Could not load RSU Configuration from file: " << rsuConfigPath;
            }
            else
            {
                PLOG(logINFO) << "Successfully loaded RSU configuration. RSU count: " << _truConfigWorkerptr->getRegisteredRSUs().size();
            }
        }
        else
        {
            PLOG(logWARNING) << "RSU_CONFIG_PATH environment variable not set. No RSU configuration loaded.";
        }
    }

    bool TelematicRsuUnit::connect(const string &natsURL)
    {
        bool isConnected = false;
        int attemptsCount = 0;
        natsStatus s = NATS_NO_SERVER;

        while ((s != NATS_OK) && attemptsCount < CONNECTION_MAX_ATTEMPTS)
        {
            attemptsCount++;
            s = natsConnection_ConnectTo(&_conn, natsURL.c_str());
            PLOG(logINFO) << "NATS connection returned: " << natsStatus_GetText(s);
            sleep(1);
        }

        if (s == NATS_OK){
           return registerRsuUnitRequestor();
        }
        else{
            throw TelematicBridgeException(natsStatus_GetText(s));
        }
    }

    bool TelematicRsuUnit::registerRsuUnitRequestor()
    {
        // Reset registration status
        bool isRegistered = false;
        int attemptsCount = 0;

        while (!isRegistered && attemptsCount < REGISTRATION_MAX_ATTEMPTS)
        {
            attemptsCount++;
            natsMsg *reply = nullptr;
            auto natsTopic = "unit." + _truConfigWorkerptr->getUnitId() + REGISTERD_RSU_CONFIG;

            auto s = natsConnection_RequestString(&reply, _conn, natsTopic.c_str(), constructRSURegistrationDataString().c_str(), TIME_OUT);
            if (s == NATS_OK)
            {
                auto replyStr = natsMsg_GetData(reply);
                PLOG(logINFO) << "Received registered reply: " << replyStr;
                //Unit is registered when the server responds with OK
                isRegistered = (std::string(replyStr) == "ok");
                natsMsg_Destroy(reply);
            }
            else
            {
                PLOG(logERROR) << "RSU registration request failed (attempt " << attemptsCount << "/" << REGISTRATION_MAX_ATTEMPTS << "): " << natsStatus_GetText(s);
                if (attemptsCount >= REGISTRATION_MAX_ATTEMPTS)
                {
                    throw TelematicBridgeException(natsStatus_GetText(s));
                }
            }
            sleep(1);
        }

        if (isRegistered)
        {
            PLOG(logINFO) << "RSU Unit successfully registered with RSU Management Service.";
            // Provide below services when the unit is registered
            rsuConfigReplier();
            rsuAvailableTopicsReplier();
            rsuSelectedTopicsReplier();
            // availableTopicsReplier();
            // selectedTopicsReplier();
            // checkStatusReplier();
        }
        return isRegistered;
    }

    void TelematicRsuUnit::rsuConfigReplier()
    {
        // Create a subscriber to the rsu config from RSU Management service
        if (!_subRegisteredRSUStatus)
        {
            PLOG(logDEBUG2) << "Inside RSU Configuration replier";
            natsConnection_Subscribe(&_subRegisteredRSUStatus, _conn, getRsuConfigTopic().c_str(), onRSUConfigStatusCallback, this);
        }

    }

    std::string TelematicRsuUnit::getRsuConfigTopic()
    {
        return "unit." + _truConfigWorkerptr->getUnitId() + REGISTERD_RSU_CONFIG;
    }

    bool TelematicRsuUnit::updateRSUStatus(const Json::Value& jsonVal)
    {
        if (!_truConfigWorkerptr->updateTRUStatus(jsonVal)){
            return false;
        }
        return true;
    }


    void TelematicRsuUnit::onRSUConfigStatusCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object)
    {
        if (natsMsg_GetReply(msg) != nullptr)
        {
            auto msgStr = natsMsg_GetData(msg);
            auto root = parseJson(msgStr);
            auto obj = (TelematicRsuUnit *)object;

            auto [isSuccessful, response] = obj->processConfigUpdateAndGenerateResponse(root);

            auto s = natsConnection_PublishString(nc, natsMsg_GetReply(msg), response.c_str());
            if (s == NATS_OK)
            {
                PLOG(logDEBUG3) << "Received RSU status msg: " << natsMsg_GetSubject(msg)
                            << " " << natsMsg_GetData(msg) << ". Replied: "<< response;
            }
            natsMsg_Destroy(msg);
        }
    }

    std::pair<bool, std::string> TelematicRsuUnit::processConfigUpdateAndGenerateResponse(const Json::Value& incomingConfig)
    {
        bool isSuccessful = updateRSUStatus(incomingConfig);

        if (!isSuccessful) {
            PLOG(logERROR) << "Error processing incoming RSU Config, ignoring update.";
        }

        std::string response = constructRSUConfigResponseDataString(isSuccessful);
        return {isSuccessful, response};
    }

    std::string TelematicRsuUnit::constructRSUConfigResponseDataString(bool isRegistrationSuccessful)
    {
        Json::Value message = _truConfigWorkerptr->getTRUConfigResponse(isRegistrationSuccessful);

        Json::FastWriter fasterWirter;
        string jsonStr = fasterWirter.write(message);
        return jsonStr;

    }

    std::string TelematicRsuUnit::constructRSURegistrationDataString()
    {
        lock_guard<mutex> lock(_unitMutex);

        Json::Value message = _truConfigWorkerptr->getTruConfigAsJsonArray();
        Json::FastWriter fasterWirter;
        string jsonStr = fasterWirter.write(message);
        return jsonStr;
    }

    void TelematicRsuUnit::rsuAvailableTopicsReplier()
    {
        if (!_subRsuAvailableTopics)
        {
            PLOG(logDEBUG2) << "Inside RSU available topics replier";
            stringstream topic;
            topic << "unit." << _truConfigWorkerptr->getUnitId() << RSU_AVAILABLE_TOPICS;
            natsConnection_Subscribe(&_subRsuAvailableTopics, _conn, topic.str().c_str(), onRsuAvailableTopicsCallback, this);
        }
    }

    void TelematicRsuUnit::rsuSelectedTopicsReplier()
    {
        if (!_subRsuSelectedTopics)
        {
            PLOG(logDEBUG2) << "Inside RSU selected topics replier";
            stringstream topic;
            topic << "unit." << _truConfigWorkerptr->getUnitId() << RSU_SELECTED_TOPICS;
            natsConnection_Subscribe(&_subRsuSelectedTopics, _conn, topic.str().c_str(), onRsuSelectedTopicsCallback, this);
        }
    }


    void TelematicRsuUnit::updateRsuAvailableTopics(const std::string &rsuIp, int rsuPort, const std::string &topic)
    {
        // Construct RSU ID from IP and port
        string rsuId = rsuIp + ":" + to_string(rsuPort);
        
        lock_guard<mutex> lock(_rsuAvailableTopicsMutex);
        _rsuAvailableTopicsMap[rsuId].insert(topic);
        
        PLOG(logDEBUG3) << "Updated available topic for RSU " << rsuId << ": " << topic;
    }

    bool TelematicRsuUnit::inRsuSelectedTopics(const std::string &rsuIp, int rsuPort, const std::string &topic) 
    {
        // Construct RSU ID from IP and port
        string rsuId = rsuIp + ":" + to_string(rsuPort);
        
        lock_guard<mutex> lock(_rsuSelectedTopicsMutex);
        
        // Check if this specific RSU has the topic selected
        auto it = _rsuSelectedTopicsMap.find(rsuId);
        if (it != _rsuSelectedTopicsMap.end())
        {
            return it->second.find(topic) != it->second.end();
        }
        
        return false;
    }

    void TelematicRsuUnit::publishRsuDataStream(const std::string &rsuIp, int rsuPort, const std::string &topic, const Json::Value &message)
    {
        string unitId = _truConfigWorkerptr->getUnitId();        
        // Construct topic: unit.<unit_id>.stream.rsu.<rsu_ip>.<topic_name>
        string natsTopic = "unit." + unitId + ".stream.rsu." + rsuIp + "." + topic;        
        auto jsonStr = constructPublishedRsuDataStream(unitId, rsuIp, rsuPort, topic, message);
        publishToNats(natsTopic, jsonStr);
    }

    

    std::string TelematicRsuUnit::constructPublishedRsuDataStream(const std::string &unitId,                                                      const std::string &rsuIp, int rsuPort,
                                                                   const std::string &topicName, const Json::Value &payload) const
    {
        // Get event name for this RSU from config worker
        std::string eventName = _truConfigWorkerptr->getEventByRsu(rsuIp, rsuPort);
        
        Json::Value message;
        
        // Construct metadata section
        Json::Value metadata;
        metadata["unitId"] = unitId;
        metadata["topicName"] = topicName;
        
        // Add RSU endpoint
        Json::Value rsu;
        rsu["ip"] = rsuIp;
        rsu["port"] = rsuPort;
        metadata["rsu"] = rsu;
        
        // Add timestamp (in microseconds)
        metadata["timestamp"] = std::to_string(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
        
        // Add event name
        metadata["event"] = eventName;
        
        // Add metadata and payload to message
        message["metadata"] = metadata;
        message["payload"] = payload;
        
        // Convert to string
        Json::FastWriter writer;
        string jsonStr = writer.write(message);
        return jsonStr;
    }

    std::string TelematicRsuUnit::constructRsuAvailableTopicsReplyString()
    {
        lock_guard<mutex> lock(_rsuAvailableTopicsMutex);
        
        // Create TRUTopicsMessage
        TRUTopicsMessage truTopicsMsg;
        truTopicsMsg.setUnitId(_truConfigWorkerptr->getUnitId());
        truTopicsMsg.setCurrentTimestamp();
        
        // Get all registered RSUs from the config worker
        auto registeredRsus = _truConfigWorkerptr->getRegisteredRSUs();
        
        for (const auto& rsuConfig : registeredRsus)
        {
            // Create RSU ID as "ip:port"
            string rsuId = rsuConfig.rsu.ip + ":" + to_string(rsuConfig.rsu.port);
            
            // Create RSUTopicsMessage
            RSUTopicsMessage rsuTopicsMsg;
            rsuTopicsMsg.setRsuEndpoint(rsuConfig.rsu);
            
            // Build topics list for this RSU
            std::vector<TopicMessage> topics;
            
            auto it = _rsuAvailableTopicsMap.find(rsuId);
            if (it != _rsuAvailableTopicsMap.end())
            {
                for (const auto& topicName : it->second)
                {
                    topics.emplace_back(topicName, false);
                }
            }
            
            rsuTopicsMsg.setTopics(topics);
            truTopicsMsg.addRsuTopic(rsuTopicsMsg);
        }

        return truTopicsMsg.toString();
    }

    void TelematicRsuUnit::onRsuAvailableTopicsCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object)
    {
        PLOG(logDEBUG3) << "Received RSU available topics request: " << natsMsg_GetSubject(msg) << " " << natsMsg_GetData(msg);
        
        // Send reply
        if (object && natsMsg_GetReply(msg) != nullptr)
        {
            const auto obj = (TelematicRsuUnit *)object;
            auto reply = obj->constructRsuAvailableTopicsReplyString();
            auto s = natsConnection_PublishString(nc, natsMsg_GetReply(msg), reply.c_str());
            natsMsg_Destroy(msg);
            if (s == NATS_OK)
            {
                PLOG(logDEBUG3) << "RSU available topics replied: " << reply;
            }
            else
            {
                PLOG(logERROR) << "Failed to reply RSU available topics: " << natsStatus_GetText(s);
            }
        }
    }

    void TelematicRsuUnit::onRsuSelectedTopicsCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object)
    {
        PLOG(logDEBUG3) << "Received RSU selected topics: " << natsMsg_GetSubject(msg) << " " << natsMsg_GetData(msg);
        
        // Send reply
        if (object && natsMsg_GetReply(msg) != nullptr)
        {
            auto obj = (TelematicRsuUnit *)object;
            auto msgStr = natsMsg_GetData(msg);
            
            try
            {
                // Parse incoming TRUTopicsMessage
                Json::CharReaderBuilder reader;
                Json::Value root;
                std::string errs;
                std::istringstream s(msgStr);
                
                if (Json::parseFromStream(reader, s, &root, &errs))
                {
                    TRUTopicsMessage incomingMsg = TRUTopicsMessage::fromJson(root);
                    
                    // Clear old selected topics for all RSUs
                    {
                        lock_guard<mutex> lock(obj->_rsuSelectedTopicsMutex);
                        obj->_rsuSelectedTopicsMap.clear();
                    }
                    
                    // Process selected topics from the incoming message
                    for (const auto& rsuTopicsMsg : incomingMsg.getRsuTopics())
                    {
                        const auto& endpoint = rsuTopicsMsg.getRsuEndpoint();
                        string rsuId = endpoint.ip + ":" + to_string(endpoint.port);
                        
                        lock_guard<mutex> lock(obj->_rsuSelectedTopicsMutex);
                        for (const auto& topic : rsuTopicsMsg.getTopics())
                        {
                            if (topic.isSelected())
                            {
                                obj->_rsuSelectedTopicsMap[rsuId].insert(topic.getName());
                            }
                        }
                    }
                    
                    // Build response with selected set to true
                    TRUTopicsMessage responseMsg;
                    responseMsg.setUnitId(obj->_truConfigWorkerptr->getUnitId());
                    responseMsg.setCurrentTimestamp();
                    
                    // Construct response with selected topics marked as true
                    for (const auto& rsuTopicsMsg : incomingMsg.getRsuTopics())
                    {
                        RSUTopicsMessage rsuResponse;
                        rsuResponse.setRsuEndpoint(rsuTopicsMsg.getRsuEndpoint());
                        
                        std::vector<TopicMessage> responseTopics;
                        for (const auto& topic : rsuTopicsMsg.getTopics())
                        {
                            // Set selected to true in response for topics that were in the request
                            TopicMessage responseTopic;
                            responseTopic.setName(topic.getName());
                            responseTopic.setSelected(true);
                            responseTopics.push_back(responseTopic);
                        }
                        rsuResponse.setTopics(responseTopics);
                        responseMsg.addRsuTopic(rsuResponse);
                    }
                    
                    // Send response
                    Json::FastWriter writer;
                    string reply = writer.write(responseMsg.toJson());
                    auto s = natsConnection_PublishString(nc, natsMsg_GetReply(msg), reply.c_str());
                    
                    if (s == NATS_OK)
                    {
                        PLOG(logDEBUG3) << "RSU selected topics replied: " << reply;
                    }
                    else
                    {
                        PLOG(logERROR) << "Failed to reply RSU selected topics: " << natsStatus_GetText(s);
                    }
                }
                else
                {
                    PLOG(logERROR) << "Failed to parse RSU selected topics JSON: " << errs;
                    string errorReply = "Error: Invalid JSON format";
                    natsConnection_PublishString(nc, natsMsg_GetReply(msg), errorReply.c_str());
                }
            }
            catch (const std::exception &e)
            {
                PLOG(logERROR) << "Error processing RSU selected topics: " << e.what();
                string errorReply = "Error: Processing failed";
                natsConnection_PublishString(nc, natsMsg_GetReply(msg), errorReply.c_str());
            }
            
            natsMsg_Destroy(msg);
        }
    }


    void TelematicRsuUnit::PublishHealthStatusToNATS(const std::string &topicSuffix)
    {
        try
        {
            // Get TRU health status snapshot
            auto truHealthStatus = _truHealthStatusTracker.getSnapshot();
            auto health = truHealthStatus.toString();

            // Construct NATS topic: unit.<unit_id>.<topicSuffix>
            std::string unitId = _truConfigWorkerptr->getUnitId();
            std::string topic = "unit." + (unitId.empty() ? "unknown" : unitId) + "." + topicSuffix;

            // Publish to NATS using publishMessage
            publishToNats(topic, health);
            
            PLOG(logDEBUG1) << "Published health status to topic: " << topic;
        }
        catch (const std::exception &e)
        {
            PLOG(logERROR) << "Error publishing health status: " << e.what();
        }
    }

    void TelematicRsuUnit::PublishRSUHealthStatus()
    {
        try
        {
            PublishHealthStatusToNATS(RSU_HEALTH_STATUS_TOPIC_SUFFIX);
            PLOG(logDEBUG2) << "Published RSU health status";
        }
        catch (const std::exception &e)
        {
            PLOG(logERROR) << "Error publishing RSU health status: " << e.what();
        }
    }

    void TelematicRsuUnit::PublishPluginHealthStatus()
    {
        try
        {
            PublishHealthStatusToNATS(HEALTH_STATUS_TOPIC_SUFFIX);
            PLOG(logDEBUG2) << "Published plugin health status";
        }
        catch (const std::exception &e)
        {
            PLOG(logERROR) << "Error publishing plugin health status: " << e.what();
        }
    }

    TelematicRsuUnit::~TelematicRsuUnit()
    {
        natsSubscription_Destroy(_subRegisteredRSUStatus);
        natsSubscription_Destroy(_subRsuAvailableTopics);
        natsSubscription_Destroy(_subRsuSelectedTopics);
        natsConnection_Destroy(_conn);
        _conn = nullptr;
        _subRegisteredRSUStatus = nullptr;
        _subRsuAvailableTopics = nullptr;
        _subRsuSelectedTopics = nullptr;
    }
}