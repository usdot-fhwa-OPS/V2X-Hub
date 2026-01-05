#include "TelematicRsuUnit.h"

using namespace std;
using namespace tmx::utils;
using namespace std::chrono;

namespace TelematicBridge
{
    TelematicRsuUnit::TelematicRsuUnit()
    {
        const char* rsuConfigPath = std::getenv("RSU_CONFIG_PATH");
        if (rsuConfigPath != nullptr) {
            if (!loadRSUConfigListFromFile(rsuConfigPath, _truUnit.registeredRsuList))
            {
                PLOG(logERROR)<<"Could not load RSU Configuration from file.";
            }
        }
    }

    void TelematicRsuUnit::connect(const string &natsURL)
    {
        bool isConnected = false;
        int attempt_counts = 0;
        natsStatus s;
        while ((s != NATS_OK) && attempt_counts < CONNECTION_MAX_ATTEMPTS)
        {
            auto s = natsConnection_ConnectTo(&_conn, natsURL.c_str());
            PLOG(logINFO) << "NATS connection returned: " << natsStatus_GetText(s);
            sleep(1);
        }

        if (s == NATS_OK){
            registerUnitRequestor();
        }
        else{
            throw TelematicBridgeException(natsStatus_GetText(s));
        }

    }

    void TelematicRsuUnit::registerUnitRequestor()
    {
        // Reset registration status
        bool isRegistered = false;
        int attempts_count = 0;

        while (!isRegistered && attempts_count < REGISTRATION_MAX_ATTEMPTS)
        {
            attempts_count++;
            PLOG(logDEBUG2) << "Inside register unit requestor";
            natsMsg *reply = nullptr;
            string payload = "{\"unit_id\":\"" + _truUnit.unit.unitId + "\"}";
            auto s = natsConnection_RequestString(&reply, _conn, REGISTER_UNIT_TOPIC, payload.c_str(), TIME_OUT);
            if (s == NATS_OK)
            {
                auto replyStr = natsMsg_GetData(reply);
                PLOG(logINFO) << "Received registered reply: " << replyStr;

                // Unit is registered when server responds with event information (location, testing_type, event_name)
                isRegistered = validateRegisterStatus(replyStr);
                natsMsg_Destroy(reply);
            }
            else
            {
                throw TelematicBridgeException(natsStatus_GetText(s));
            }
            sleep(1);
        }

        if (isRegistered)
        {
            // Provide below services when the unit is registered
            rsuConfigReplier();
            availableTopicsReplier();
            selectedTopicsReplier();
            checkStatusReplier();
        }
    }

    void TelematicRsuUnit::rsuConfigReplier()
    {
        std::string rsuConfigTopic = _truUnit.unit.unitId + REGISTERD_RSU_CONFIG;
        // Publish list of initially loaded RSUs to telematics RSU Management service
        publishRSURegistrationMessage(rsuConfigTopic);

        // Create a subscriber to the rsu config from RSU Management service
        if (!_subRegisteredRSUStatus)
        {
            PLOG(logDEBUG2) << "Inside rsu config status replier";
            stringstream topic;
            topic << rsuConfigTopic;
            natsConnection_Subscribe(&_subRegisteredRSUStatus, _conn, topic.str().c_str(), onRSUConfigStatusCallback, this);
        }

    }


    void TelematicRsuUnit::publishRSURegistrationMessage(const std::string &topic)
    {
        // Pass current stored configuration information
        auto jsonStr = constructRSURegistrationDataString();
        auto s = natsConnection_PublishString(_conn, topic.c_str(), jsonStr.c_str());
        if (s == NATS_OK)
        {
            PLOG(logINFO) << "Topic: " << topic << ". Published: " << jsonStr;
        }
        else
        {
            throw TelematicBridgeException(natsStatus_GetText(s));
        }
    }



    void TelematicRsuUnit::onRSUConfigStatusCallback(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *object)
    {
        // Update list of registered RSUs on receiving a message
        if (natsMsg_GetReply(msg) != nullptr)
        {
            auto msgStr = natsMsg_GetData(msg);
            auto root = parseJson(msgStr);

            auto obj = (TelematicRsuUnit *)object;

            if (object && root.isMember(UNIT_ID_KEY) && root[UNIT_ID_KEY].isArray())
            {

                Json::Value unitArray = root[UNIT_ID_KEY];

                auto unitID = unitArray.get(UNIT_ID_KEY, "").asString();
                if (unitID != obj->_truUnit.unit.unitId)
                {
                    //Ignore messages not implied for this unit
                    PLOG(logWARNING) <<  "Ignoring RSU Configuration message for unit: " << unitID << " received in unit: "<< obj->_truUnit.unit.unitId;
                    return;
                }
            }

            if (object && root.isMember(RSU_CONFIGS_KEY) && root[RSU_CONFIGS_KEY].isArray())
            {
                Json::Value rsuConfigsArray = root[RSU_CONFIGS_KEY];

                PLOG(logDEBUG) << "Processing " << rsuConfigsArray.size() << " RSU configs";

                // Iterate through RSUConfig and update list of RSUs registered to unit
                for (const auto& rsuConfig : rsuConfigsArray)
                {
                    if(!processRSUConfig(rsuConfig, obj->_truUnit.unit.maxConnections, obj->_truUnit.registeredRsuList))
                    {
                        PLOG(logERROR) << "Error processing incoming RSU Config, ignoring.";
                    }
                }
            }

            if (object && root.isMember("timestamp"))
            {
                obj->_truUnit.timestamp = root[TIMESTAMP_KEY].asInt();
            }

            auto s = natsConnection_PublishString(nc, natsMsg_GetReply(msg), "OK");
            if (s == NATS_OK)
            {
                PLOG(logDEBUG3) << "Received RSU status msg: " << natsMsg_GetSubject(msg) << " " << natsMsg_GetData(msg) << ". Replied: OK";
            }
            natsMsg_Destroy(msg);
        }
    }

    std::string TelematicRsuUnit::constructRSURegistrationDataString()
    {
        lock_guard<mutex> lock(_unitMutex);

        Json::Value message;
        Json::Value unitObject;
        unitObject[UNIT_ID_KEY] = _truUnit.unit.unitId;
        unitObject[MAX_CONNECTIONS_KEY] = _truUnit.unit.maxConnections;
        unitObject[PLUGIN_HEARTBEAT_INTERVAL_KEY] = _truUnit.unit.pluginHeartBeatInterval;
        unitObject[HEALTHMONITOR_HEARTBEAT_INTERVAL_KEY] = _truUnit.unit.healthMonitorPluginHeartbeatInterval;
        unitObject[RSU_STATUS_MONITOR_INTERVAL_KEY] = _truUnit.unit.rsuStatusMonitorInterval;
        message[UNIT_KEY] = unitObject;
        message[RSU_CONFIGS_KEY] = rsuConfigListToJsonArray(_truUnit.registeredRsuList);
        message[TIMESTAMP_KEY] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        Json::FastWriter fasterWirter;
        string jsonStr = fasterWirter.write(message);
        return jsonStr;
    }




    TelematicRsuUnit::~TelematicRsuUnit()
    {
        natsSubscription_Destroy(_subRegisteredRSUStatus);
        natsConnection_Destroy(_conn);
        _conn = nullptr;
        _subRegisteredRSUStatus = nullptr;
    }
}