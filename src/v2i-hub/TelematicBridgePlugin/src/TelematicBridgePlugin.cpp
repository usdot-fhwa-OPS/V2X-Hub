#include "TelematicBridgePlugin.h"
#include <RSUStatusMessage.h>
#include <chrono>

using namespace tmx::utils;
using namespace std;

namespace TelematicBridge
{
    TelematicBridgePlugin::TelematicBridgePlugin(const string &name) : TmxMessageManager(name)
    {
        // Safely get environment variables with null checks
        const char* infraId = std::getenv("INFRASTRUCTURE_ID");
        const char* infraName = std::getenv("INFRASTRUCTURE_NAME");
        
        _unitId = infraId ? infraId : "";
        _unitName = infraName ? infraName : "";
        _isTRU = true;
        //std::getenv("IS_TRU");
        AddMessageFilter("*", "*", IvpMsgFlags_None);
        AddMessageFilter("J2735", "*", IvpMsgFlags_RouteDSRC);
        SubscribeToMessages();
        PLOG(logINFO) << "TelematicBridgePlugin initialized with Unit ID: " << _unitId << ", Unit Name: " << _unitName;        
        if(_isTRU){
            PLOG(logINFO) << "TelematicBridgePlugin operating as Telematic RSU Unit (TRU)";
            _telematicRsuUnitPtr = std::make_unique<TelematicRsuUnit>();
        }
        else{
            PLOG(logINFO) << "TelematicBridgePlugin operating as standard Telematic Unit";
            _telematicUnitPtr = make_unique<TelematicUnit>();
        }
    }

    void TelematicBridgePlugin::registerTRU()
    {
        const char* natsUrl = std::getenv("NATS_URL");
        if (_isTRU & !_isTRURegistered && _telematicRsuUnitPtr)
        {
            _natsURL = natsUrl ? natsUrl : "nats://localhost:4222";
            try
            {
               _isTRURegistered = _telematicRsuUnitPtr->connect(_natsURL);
            }
            catch (const TelematicBridgeException &e)
            {
                PLOG(tmx::utils::LogLevel::logERROR) << "TelematicBridge encountered unhandled exception during initialization: " << e.what();
                _isTRURegistered = false;
                return; //Stop operations if connection failed or registration failed
            }
            // If using Telematic RSU Unit create timer to broadcast RSU Health Config
            if ( !_startedRegistrationTh ) {
                _rsuRegistrationConfigTimer = std::make_unique<tmx::utils::ThreadTimer>();
                // Send RSU Config message to TMX core periodically at configurable interval.
                _timerThId = _rsuRegistrationConfigTimer->AddPeriodicTick([this]()
                            {
                                this->BroadcastRSURegistrationConfigMessage();
                                PLOG(logDEBUG) << "Updating RSUHealthMonitorPlugin Configuration at interval (milliseconds): " << rsuConfigUpdateIntervalInMillisec; },
                                                            std::chrono::milliseconds(rsuConfigUpdateIntervalInMillisec));
                PLOG(logINFO) << "RSU Health Monitor timer thread ID: " << _timerThId;
                _rsuRegistrationConfigTimer->Start();
                _startedRegistrationTh = true;
            }

            if(!_startedHealthMonitorTh){
                // Create timer to publish TRU health status to NATS
                _healthStatusTimer = std::make_unique<tmx::utils::ThreadTimer>();
                _healthStatusTimerThId = _healthStatusTimer->AddPeriodicTick([this]()
                            {
                                if (_telematicRsuUnitPtr)
                                {
                                    _telematicRsuUnitPtr->PublishPluginHealthStatus();
                                }
                                PLOG(logINFO) << "Publishing TRU health status at interval (seconds): " << _pluginHeartBeatInterval; },
                                                            std::chrono::seconds(_pluginHeartBeatInterval));
                PLOG(logINFO) << "Health status monitor timer thread ID: " << _healthStatusTimerThId;
                _healthStatusTimer->Start();
                _startedHealthMonitorTh = true;
            }
        }
    }

    void TelematicBridgePlugin::OnMessageReceived(IvpMessage *msg)
    {
        auto hasError = false;
        tmx::routeable_message routeMsg(msg);
        PLOG(logDEBUG) << "TelematicBridgePlugin received message: Type=" << routeMsg.get_type()
                       << ", Subtype=" << routeMsg.get_subtype()
                       << ", Source=" << routeMsg.get_source()
                       << ", Timestamp=" << routeMsg.get_timestamp()
                       << ", RSU IP=" << routeMsg.get_rsuIp()
                       << ", RSU Port=" << routeMsg.get_rsuPort()
                       << ", Encoding=" << routeMsg.get_encoding()
                       << ", Flags=" << routeMsg.get_flags()
                       << ", Payload=" << routeMsg.get_payload_str();
        
        // Check if this is an RSU Status Message
        if (routeMsg.get_type() == tmx::messages::RSUStatusMessage::MessageType && 
            routeMsg.get_subtype() == tmx::messages::RSUStatusMessage::MessageSubType)
        {
            ProcessRSUStatusMessage(routeMsg);
        }
        
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
            if(_telematicRsuUnitPtr){
                // Extract RSU IP and port from DSRC metadata (populated by TmxMessageManager)
                std::string rsuIp = routeMsg.get_rsuIp();
                int rsuPort = routeMsg.get_rsuPort();
                
                // Only process if we have valid RSU information
                if (!rsuIp.empty() && rsuPort > 0)
                {
                    _telematicRsuUnitPtr->updateRsuAvailableTopics(rsuIp, rsuPort, topicStr);
                    if (_telematicRsuUnitPtr->inRsuSelectedTopics(rsuIp, rsuPort, topicStr))
                    {
                        _telematicRsuUnitPtr->publishRsuDataStream(rsuIp, rsuPort, topicStr, json);
                    }
                }
                else
                {
                    PLOG(logDEBUG) << "Message does not contain RSU metadata (rsuIp/rsuPort), skipping RSU-specific routing";
                }
            }
        }

    }

    void TelematicBridgePlugin::UpdateConfigSettings()
    {
        lock_guard<mutex> lock(_configMutex);
        GetConfigValue<string>("NATSUrl", _natsURL);
        GetConfigValue<string>("MessageExclusionList", _excludedMessages);
        GetConfigValue<int16_t>("bridgePluginHeartbeatInterval", _pluginHeartBeatInterval);
        unit_st unit = {_unitId, _unitName, UNIT_TYPE_INFRASTRUCTURE};

        registerTRU();
        
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
                try
                {
                    _telematicUnitPtr->connect(_natsURL);
                }
                catch (const TelematicBridgeException &e)
                {
                    FILE_LOG(tmx::utils::LogLevel::logERROR) << "TelematicBridge encountered unhandled exception: " << e.what();
                    _telematicRsuUnitPtr->updateUnitHealthStatus(_unitId, "error");
                    return;
                }
            }
            // Update unit health status to registered and running
            _telematicRsuUnitPtr->updateUnitHealthStatus(_unitId, "running");
        }
        else if (state == IvpPluginState_error)
        {
            _telematicRsuUnitPtr->updateUnitHealthStatus(_unitId, "error");
        }
    }

    void TelematicBridgePlugin::OnConfigChanged(const char *key, const char *value)
    {
        TmxMessageManager::OnConfigChanged(key, value);
        UpdateConfigSettings();
    }


    void TelematicBridgePlugin::ProcessRSUStatusMessage(tmx::routeable_message &routeMsg)
    {
        try
        {
            auto rsuHealthStatus = HealthStatusMessageMapper::toRsuHealthStatusMessage(routeMsg);
            if (_telematicRsuUnitPtr)
            {
                std::string rsuId = rsuHealthStatus.getRsuId();
                _telematicRsuUnitPtr->updateRsuHealthStatus(rsuId, rsuHealthStatus);
                PLOG(logINFO) << "Updated health status for RSU: " << rsuId
                              << " with status: " << rsuHealthStatus.getStatus();
                _telematicRsuUnitPtr->PublishRSUHealthStatus();
            }
        }        
        catch (const std::exception &e)
        {
            FILE_LOG(tmx::utils::LogLevel::logERROR) << "Error processing RSU status message: " << e.what();
        }
    }

    void TelematicBridgePlugin::BroadcastRSURegistrationConfigMessage()
    {
        try
        {
            std::string rsuRegistrationConfigJsonStr = _telematicRsuUnitPtr->constructRSURegistrationDataString();

            if(!rsuRegistrationConfigJsonStr.empty())
            {
                // Parse the JSON string to Json::Value
                Json::CharReaderBuilder reader;
                Json::Value rsuRegistrationConfigJsonArray;
                std::string errs;
                std::istringstream s(rsuRegistrationConfigJsonStr);
                
                if (Json::parseFromStream(reader, s, &rsuRegistrationConfigJsonArray, &errs))
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
                else
                {
                    PLOG(logERROR) << "Error parsing RSU registration config JSON: " << errs;
                }
            }
        }
        catch (const std::exception &e)
        {
            PLOG(logERROR) << "Error broadcasting RSU registration config message: " << e.what();
        }

    }
}

// The main entry point for this application.
int main(int argc, char *argv[])
{
    return run_plugin<TelematicBridge::TelematicBridgePlugin>("Telematic Bridge", argc, argv);
}