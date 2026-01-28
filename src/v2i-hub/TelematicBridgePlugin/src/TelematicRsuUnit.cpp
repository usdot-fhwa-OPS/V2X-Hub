#include "TelematicRsuUnit.h"
#include "data_selection/TRUTopicsMessage.h"
#include "TelematicJsonKeys.h"

using namespace std;
using namespace tmx::utils;
using namespace std::chrono;

namespace TelematicBridge
{
    TelematicRsuUnit::TelematicRsuUnit()
    {
        const char* rsuConfigPath = std::getenv("RSU_CONFIG_PATH");
        _truConfigWorkerptr = std::make_unique<truConfigWorker>();
        _truHealthStatusTracker = std::make_shared<TRUHealthStatusTracker>();
        _dataSelectionTracker = std::make_shared<DataSelectionTracker>();
        if (rsuConfigPath != nullptr) {
            PLOG(logINFO) << "Loading RSU configuration from: " << rsuConfigPath;
            if (!_truConfigWorkerptr->loadRSUConfigListFromFile(std::string(rsuConfigPath)))
            {
                throw std::runtime_error("Could not load RSU Configuration from file: " + std::string(rsuConfigPath));
            }
        }
        else
        {
            throw std::runtime_error("RSU_CONFIG_PATH environment variable not set. No RSU configuration loaded.");
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
            auto payload = constructRSURegistrationDataString();

            auto s = natsConnection_RequestString(&reply, _conn, natsTopic.c_str(), payload.c_str(), TIME_OUT);
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
            rsuConfigReplier();
            rsuAvailableTopicsReplier();
            rsuSelectedTopicsReplier();
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
        Json::Value message = constructRSURegistrationJson();
        Json::FastWriter fasterWirter;
        string jsonStr = fasterWirter.write(message);
        return jsonStr;
    }

    Json::Value TelematicRsuUnit::constructRSURegistrationJson()
    {
        Json::Value message = _truConfigWorkerptr->getTruConfigAsJsonArray();
        return message;
    }

    void TelematicRsuUnit::rsuAvailableTopicsReplier()
    {
        if (!_subRsuAvailableTopics)
        {
            PLOG(logDEBUG2) << "Inside RSU available topics replier";
            stringstream topic;
            topic << "unit." << _truConfigWorkerptr->getUnitId() << RSU_AVAILABLE_TOPICS;
            std::string topicStr = topic.str();
            natsConnection_Subscribe(&_subRsuAvailableTopics, _conn, topicStr.c_str(), onRsuAvailableTopicsCallback, this);
        }
    }

    void TelematicRsuUnit::rsuSelectedTopicsReplier()
    {
        if (!_subRsuSelectedTopics)
        {
            PLOG(logDEBUG2) << "Inside RSU selected topics replier";
            stringstream topic;
            topic << "unit." << _truConfigWorkerptr->getUnitId() << RSU_SELECTED_TOPICS;
            std::string topicStr = topic.str();
            natsConnection_Subscribe(&_subRsuSelectedTopics, _conn, topicStr.c_str(), onRsuSelectedTopicsCallback, this);
        }
    }

    void TelematicRsuUnit::publishRsuDataStream(const std::string &rsuIp, int rsuPort, const std::string &topic, const Json::Value &message)
    {
        // Construct topic: unit.<unit_id>.stream.rsu.<rsu_ip>.<topic_name>
        string unitId = _truConfigWorkerptr->getUnitId(); 
        string rsuIpTmp = rsuIp;       
        boost::replace_all(rsuIpTmp, ".", "_"); 
        string natsTopic = "unit." + unitId + ".stream.rsu." + rsuIpTmp + "." + topic;        
        auto jsonStr = constructPublishedRsuDataStream(unitId, rsuIp, rsuPort, topic, message);
        publishToNats(natsTopic, jsonStr);
    }    

    std::string TelematicRsuUnit::constructPublishedRsuDataStream(const std::string &unitId, const std::string &rsuIp, int rsuPort,
                                                                   const std::string &topicName, const Json::Value &payload) const
    {
        // Get event name for this RSU from config worker
        std::string eventName = _truConfigWorkerptr->getEventByRsu(rsuIp, rsuPort);
        
        Json::Value message;
        
        // Construct metadata section
        Json::Value metadata;
        metadata[TelematicJsonKeys::UNIT_ID] = unitId;
        metadata[TelematicJsonKeys::TOPIC_NAME_METADATA] = topicName;
        
        // Add RSU endpoint
        Json::Value rsu;
        rsu[TelematicJsonKeys::RSU_IP] = rsuIp;
        rsu[TelematicJsonKeys::RSU_PORT] = rsuPort;
        metadata[TelematicJsonKeys::RSU] = rsu;
        
        // Add timestamp (in milliseconds)
        metadata[TelematicJsonKeys::TIMESTAMP] = std::to_string(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
        
        // Add event name
        metadata[TelematicJsonKeys::EVENT] = eventName;
        
        // Add metadata and payload to message
        message[TelematicJsonKeys::METADATA] = metadata;
        if(payload.isObject() || payload.isArray()){
            message[TelematicJsonKeys::PAYLOAD] = payload;
        }
        
        // Convert to string
        Json::FastWriter writer;
        string jsonStr = writer.write(message);
        return jsonStr;
    }

    std::string TelematicRsuUnit::constructRsuAvailableTopicsReplyString()
    {
        return _dataSelectionTracker->latestAvailableTopicsMessageToJsonString();
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
        PLOG(logINFO) << "Received RSU selected topics request";
        PLOG(logINFO) << "  Subject: " << (natsMsg_GetSubject(msg) ? natsMsg_GetSubject(msg) : "NULL");
        PLOG(logINFO) << "  Data: " << (natsMsg_GetData(msg) ? natsMsg_GetData(msg) : "NULL");
        PLOG(logINFO) << "  Reply subject: " << (natsMsg_GetReply(msg) ? natsMsg_GetReply(msg) : "NULL");
        PLOG(logINFO) << "Received RSU selected topics: " << natsMsg_GetSubject(msg) << " " << natsMsg_GetData(msg);

        if (object && natsMsg_GetReply(msg) != nullptr)
        {
            auto obj = (TelematicRsuUnit *)object;
            auto msgStr = natsMsg_GetData(msg);
            
            PLOG(logINFO) << "About to call constructRsuSelectedTopicsReplyString with: " << msgStr;
            
            try
            {
                auto reply = obj->constructRsuSelectedTopicsReplyString(msgStr);
                PLOG(logINFO) << "Sending reply: " << reply;
                auto s = natsConnection_PublishString(nc, natsMsg_GetReply(msg), reply.c_str());
                if (s == NATS_OK)
                {
                    PLOG(logINFO) << "Successfully sent RSU selected topics reply";
                }
                else
                {
                    PLOG(logERROR) << "Failed to send RSU selected topics reply: " << natsStatus_GetText(s);
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
        else
        {
            PLOG(logERROR) << "Cannot send reply - object or reply subject is NULL!";
            natsMsg_Destroy(msg);
        }
    }

    std::string TelematicRsuUnit::constructRsuSelectedTopicsReplyString(const std::string &msgStr){
        _dataSelectionTracker->updateLatestSelectedTopics(msgStr.c_str());
        return _dataSelectionTracker->latestSelectedTopicsMessageToJsonString();
    }

    void TelematicRsuUnit::processRsuDataStream(const std::string &rsuIp, const std::string &topic, const Json::Value &json)
    {
        auto rsuPort = _truConfigWorkerptr->getRsuPortByIp(rsuIp);
        auto unitId = _truConfigWorkerptr->getUnitId();        
        _dataSelectionTracker->updateRsuAvailableTopics(rsuIp, rsuPort, topic, unitId);
        if (_dataSelectionTracker->inRsuSelectedTopics(rsuIp, topic))
        {
            publishRsuDataStream(rsuIp,  rsuPort, topic, json);
        }
    }

    void TelematicRsuUnit::updateRsuHealthStatus(const RSUHealthStatusMessage &status)
    {
        _truHealthStatusTracker->updateRsuStatus(status);
    }

    void TelematicRsuUnit::updateUnitHealthStatus(const std::string &status)
    {
        // Get current timestamp in milliseconds
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();            
        auto unitStatus = HealthStatusMessageMapper::toUnitHealthStatusMessage(_truConfigWorkerptr->getUnitId(), status, timestamp);            
        _truHealthStatusTracker->updateUnitStatus(unitStatus);
    }

    void TelematicRsuUnit::publishHealthStatusToNATS(const std::string &topicSuffix)
    {
        try
        {
            // Get TRU health status snapshot
            auto truHealthStatus = _truHealthStatusTracker->getSnapshot();
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