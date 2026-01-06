#include "TelematicBridgePlugin.h"

using namespace tmx::utils;
using namespace std;

namespace TelematicBridge
{
    TelematicBridgePlugin::TelematicBridgePlugin(const string &name) : TmxMessageManager(name)
    {
        _unitId = std::getenv("INFRASTRUCTURE_ID");
        _unitName = std::getenv("INFRASTRUCTURE_NAME");
        _isTRU = std::getenv("IS_TRU");
        AddMessageFilter("*", "*", IvpMsgFlags_None);
        AddMessageFilter("J2735", "*", IvpMsgFlags_RouteDSRC);
        SubscribeToMessages();

        if (_isTRU){
            _natsURL = std::getenv("NATS_URL");
            _telematicRsuUnitPtr = std::make_unique<TelematicRsuUnit>();
            _telematicRsuUnitPtr->connect(_natsURL);
            // If using Telematic RSU Unit create timer to broadcast RSU Health Config
            _rsuRegistrationConfigTimer = std::make_unique<tmx::utils::ThreadTimer>();
            if ( !_started ) {
                // Send RSU Config message to TMX core periodically at configurable interval.
                _timerThId = _rsuRegistrationConfigTimer->AddPeriodicTick([this]()
                            {
                                this->BroadcastRSURegistrationConfigMessage();
                                PLOG(logINFO) << "Updating RSU Health Configuration at interval (second): " << rsuConfigUpdateIntervalInMillisec; },
                                                            std::chrono::milliseconds(rsuConfigUpdateIntervalInMillisec));
                PLOG(logDEBUG1) << "RSU Health Monitor timer thread ID: " << _timerThId;
                _rsuRegistrationConfigTimer->Start();
                _started = true;
            }
        }
        else{
            _telematicUnitPtr = make_unique<TelematicUnit>();
        }
    }

    void TelematicBridgePlugin::OnMessageReceived(IvpMessage *msg)
    {
        auto hasError = false;
        tmx::routeable_message routeMsg(msg);
        // Convert IVP message to JSON CPP Value
        Json::Value json = routeableMessageToJsonValue(routeMsg);
        // Overwrite HEX String payload with JER encode JSON payload for J2735 Messages
        if (PluginClient::IsJ2735Message(routeMsg))
        {
            // Convert routeable message to J2735 encoded message
            std::string json_payload_str = j2735MessageToJson(routeMsg);
            // Update the JSON payload
            try {
                json["payload"] = stringToJsonValue(json_payload_str);
            }
            catch (const TelematicBridgeException &e) {
                FILE_LOG(tmx::utils::LogLevel::logERROR) << "Error converting J2735 message to JSON: " << e.what();
                hasError = true;
                tmx::utils::TmxMessageManager::SetStatus<int>(Key_SkippedMessages, ++_skippedMessages);
            }
        }
        if (!hasError) {
            stringstream topic;
            topic << (routeMsg.get_type()) << "_" << (routeMsg.get_subtype()) << "_" << (routeMsg.get_source());
            auto topicStr = topic.str();
            if (_telematicUnitPtr){
                _telematicUnitPtr->updateAvailableTopics(topicStr);
                if (_telematicUnitPtr->inSelectedTopics(topicStr))
                {
                    _telematicUnitPtr->publishMessage(topicStr, json);
                }
            }
        }

    }

    void TelematicBridgePlugin::UpdateConfigSettings()
    {
        lock_guard<mutex> lock(_configMutex);
        GetConfigValue<string>("NATSUrl", _natsURL);
        GetConfigValue<string>("MessageExclusionList", _excludedMessages);
        unit_st unit = {_unitId, _unitName, UNIT_TYPE_INFRASTRUCTURE};
        if (_telematicUnitPtr)
        {
            _telematicUnitPtr->setUnit(unit);
            _telematicUnitPtr->updateExcludedTopics(_excludedMessages);
        }
    }

    void TelematicBridgePlugin::OnStateChange(IvpPluginState state)
    {
        TmxMessageManager::OnStateChange(state);
        if (state == IvpPluginState_registered)
        {
            UpdateConfigSettings();
            if (_telematicUnitPtr)
            {
                _telematicUnitPtr->connect(_natsURL);
            }
        }
    }

    void TelematicBridgePlugin::OnConfigChanged(const char *key, const char *value)
    {
        TmxMessageManager::OnConfigChanged(key, value);
        UpdateConfigSettings();
    }


    void TelematicBridgePlugin::BroadcastRSURegistrationConfigMessage()
    {
        Json::Value rsuRegistrationConfigJsonArray = _telematicRsuUnitPtr->constructRSURegistrationDataString();

        if(!rsuRegistrationConfigJsonArray.empty())
        {

            tmx::messages::RSURegistrationConfigMessage sendRsuRegistrationConfigMsg;
            if(jsonValueToRouteableMessage(rsuRegistrationConfigJsonArray, sendRsuRegistrationConfigMsg))
            {
                BroadcastMessage<tmx::messages::RSURegistrationConfigMessage>(sendRsuRegistrationConfigMsg, TelematicBridgePlugin::GetName());
            }
            else{
                PLOG(logERROR) <<"Error converting rsu health config to TMX message";
            }

        }

    }
}

// The main entry point for this application.
int main(int argc, char *argv[])
{
    return run_plugin<TelematicBridge::TelematicBridgePlugin>("Telematic Bridge", argc, argv);
}