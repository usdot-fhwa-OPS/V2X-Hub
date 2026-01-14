#include "TelematicRsuUnit.h"

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
            if (!_truConfigWorkerptr->loadRSUConfigListFromFile(rsuConfigPath))
            {
                PLOG(logERROR)<<"Could not load RSU Configuration from file.";
            }
        }
    }

    void TelematicRsuUnit::connect(const string &natsURL)
    {
        bool isConnected = false;
        int attemptsCount = 0;
        natsStatus s;
        while ((s != NATS_OK) && attemptsCount < CONNECTION_MAX_ATTEMPTS)
        {
            attemptsCount++;
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
        int attemptsCount = 0;

        while (!isRegistered && attemptsCount < REGISTRATION_MAX_ATTEMPTS)
        {
            attemptsCount++;
            PLOG(logDEBUG2) << "Inside register unit requestor";
            natsMsg *reply = nullptr;
            string payload = constructRSURegistrationDataString();
            auto s = natsConnection_RequestString(&reply, _conn, REGISTERD_RSU_CONFIG, payload.c_str(), TIME_OUT);
            if (s == NATS_OK)
            {
                auto replyStr = natsMsg_GetData(reply);
                PLOG(logINFO) << "Received registered reply: " << replyStr;
                //Unit is registered when the server responds with OK
                if (replyStr == "ok"){
                    isRegistered = true;
                }

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
        // Create a subscriber to the rsu config from RSU Management service
        if (!_subRegisteredRSUStatus)
        {
            std::string rsuConfigTopic = "unit." + _truConfigWorkerptr->getUnitId() + REGISTERD_RSU_CONFIG;
            PLOG(logDEBUG2) << "Inside rsu config status replier";
            stringstream topic;
            topic << rsuConfigTopic;
            natsConnection_Subscribe(&_subRegisteredRSUStatus, _conn, topic.str().c_str(), onRSUConfigStatusCallback, this);
        }

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
        // Update list of registered RSUs on receiving a message
        if (natsMsg_GetReply(msg) != nullptr)
        {
            auto msgStr = natsMsg_GetData(msg);
            auto root = parseJson(msgStr);

            auto obj = (TelematicRsuUnit *)object;
            bool isRegistrationSuccessful = true;
            if(!obj->updateRSUStatus(root))
            {
                PLOG(logERROR) << "Error processing incoming RSU Config, ignoring update.";
                isRegistrationSuccessful = false;
            }
            //Respond with the latest registration configuration information
            auto latestRSUConfig = obj->constructRSUConfigResponseDataString(isRegistrationSuccessful);
            auto s = natsConnection_PublishString(nc, natsMsg_GetReply(msg), latestRSUConfig.c_str());
            if (s == NATS_OK)
            {
                PLOG(logDEBUG3) << "Received RSU status msg: " << natsMsg_GetSubject(msg) << " " << natsMsg_GetData(msg) << ". Replied: "<< latestRSUConfig;
            }
            natsMsg_Destroy(msg);
        }
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


    TelematicRsuUnit::~TelematicRsuUnit()
    {
        natsSubscription_Destroy(_subRegisteredRSUStatus);
        natsConnection_Destroy(_conn);
        _conn = nullptr;
        _subRegisteredRSUStatus = nullptr;
    }
}