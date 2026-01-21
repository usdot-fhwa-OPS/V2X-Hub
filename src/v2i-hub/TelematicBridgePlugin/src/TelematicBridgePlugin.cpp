#include "TelematicBridgePlugin.h"
#include <RSUStatusMessage.h>
#include <chrono>

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

            // Create timer to publish TRU health status to NATS
            _healthStatusTimer = std::make_unique<tmx::utils::ThreadTimer>();
            _healthStatusTimerThId = _healthStatusTimer->AddPeriodicTick([this]()
                        {
                            this->PublishHealthStatusToNATS(HEALTH_STATUS_TOPIC_SUFFIX, _truHealthStatusTracker.getSnapshot().toJson());
                            PLOG(logDEBUG1) << "Publishing TRU health status at interval (ms): " << _pluginHeartBeatInterval; },
                                                        std::chrono::milliseconds(_pluginHeartBeatInterval));
            PLOG(logDEBUG1) << "Health status monitor timer thread ID: " << _healthStatusTimerThId;
            _healthStatusTimer->Start();
        }
        else{
            _telematicUnitPtr = make_unique<TelematicUnit>();
        }
    }

    void TelematicBridgePlugin::OnMessageReceived(IvpMessage *msg)
    {
        auto hasError = false;
        tmx::routeable_message routeMsg(msg);
        
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
        GetConfigValue<int16_t>("PluginHeartBeatInterval", _pluginHeartBeatInterval);
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
            // Update unit health status to registered and running
            UpdateUnitHealthStatus("running");
        }
        else if (state == IvpPluginState_error)
        {
            UpdateUnitHealthStatus("error");
        }
    }

    void TelematicBridgePlugin::OnConfigChanged(const char *key, const char *value)
    {
        TmxMessageManager::OnConfigChanged(key, value);
        UpdateConfigSettings();
    }


    void TelematicBridgePlugin::ProcessRSUStatusMessage(const tmx::routeable_message &routeMsg)
    {
        try
        {
            // Parse RSU status message
            tmx::messages::RSUStatusMessage rsuStatusMsg;
            rsuStatusMsg.set_contents(routeMsg.get_payload());
            
            // Parse JSON content
            Json::CharReaderBuilder reader;
            Json::Value rsuStatusJson;
            std::string errs;
            std::istringstream s(rsuStatusMsg.get_contents());
            
            if (Json::parseFromStream(reader, s, &rsuStatusJson, &errs))
            {
                // Extract RSU information from JSON
                if (rsuStatusJson.isMember("rsuIpAddress") && rsuStatusJson.isMember("rsuSnmpPort"))
                {
                    std::string rsuIp = rsuStatusJson["rsuIpAddress"].asString();
                    int rsuPort = rsuStatusJson["rsuSnmpPort"].asInt();
                    std::string event = rsuStatusJson.isMember("event") ? 
                                      rsuStatusJson["event"].asString() : "";
                    
                    // Create RSU ID in format "IP:port"
                    std::string rsuId = rsuIp + ":" + std::to_string(rsuPort);
                    
                    // Determine health status from rsuChanStatus field
                    std::string healthStatus = rsuStatusJson.isMember("rsuChanStatus") ? 
                                              rsuStatusJson["rsuChanStatus"].asString() : "unknown";
                    
                    // Create RSUHealthStatusMessage
                    ::TelematicBridgePlugin::RSUHealthStatusMessage rsuHealthStatus(
                        rsuIp, 
                        rsuPort, 
                        healthStatus, 
                        event
                    );
                    
                    // Update the TRU health status tracker
                    _truHealthStatusTracker.updateRsuStatus(rsuId, rsuHealthStatus);
                    
                    PLOG(logINFO) << "Updated RSU health status for " << rsuId 
                                  << " with status: " << healthStatus;
                    PublishHealthStatusToNATS(RSU_HEALTH_STATUS_TOPIC_SUFFIX, _truHealthStatusTracker.getSnapshot().toJson());
                }
            }
            else
            {
                FILE_LOG(tmx::utils::LogLevel::logERROR) << "Error parsing RSU status JSON: " << errs;
            }
        }
        catch (const std::exception &e)
        {
            FILE_LOG(tmx::utils::LogLevel::logERROR) << "Error processing RSU status message: " << e.what();
        }
    }

    void TelematicBridgePlugin::PublishHealthStatusToNATS(const std::string &topicSuffix, const Json::Value &healthData)
    {
        try
        {
            if (!_telematicRsuUnitPtr)
            {
                PLOG(logWARNING) << "TelematicRsuUnit not available for health status publishing";
                return;
            }

            // Get TRU health status snapshot
            auto truHealthStatus = _truHealthStatusTracker.getSnapshot();
            Json::Value healthJson = truHealthStatus.toJson();

            // Construct NATS topic: unit.<unit_id>.<topicSuffix>
            std::string topic = "unit." + (_unitId.empty() ? "unknown" : _unitId) + "." + topicSuffix;

            // Publish to NATS using TelematicRsuUnit's publishMessage
            _telematicRsuUnitPtr->publishMessage(topic, healthData);
            
            PLOG(logDEBUG1) << "Published health status to topic: " << topic;
        }
        catch (const std::exception &e)
        {
            FILE_LOG(tmx::utils::LogLevel::logERROR) << "Error publishing health status: " << e.what();
        }
    }

    void TelematicBridgePlugin::UpdateUnitHealthStatus(const std::string &status)
    {
        try
        {
            // Get current timestamp in milliseconds
            auto now = std::chrono::system_clock::now();
            auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();
            
            // Create UnitHealthStatusMessage
            ::TelematicBridgePlugin::UnitHealthStatusMessage unitStatus;
            unitStatus.setUnitId(_unitId.empty() ? "unknown" : _unitId);
            unitStatus.setBridgePluginStatus(status);
            unitStatus.setLastCommunicationTimestamp(timestamp);
            
            // Update the TRU health status tracker
            _truHealthStatusTracker.updateUnitStatus(unitStatus);
            
            PLOG(logINFO) << "Updated unit health status: unitId=" << unitStatus.getUnitId()
                          << ", status=" << status
                          << ", timestamp=" << timestamp;
        }
        catch (const std::exception &e)
        {
            FILE_LOG(tmx::utils::LogLevel::logERROR) << "Error updating unit health status: " << e.what();
        }
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
    
    void TelematicBridgePlugin::updateRsuTopics(const std::string &rsuIp, int rsuPort, const std::string &topic)
    {
        if (_telematicRsuUnitPtr)
        {
            std::string rsuId = rsuIp + ":" + std::to_string(rsuPort);
            _telematicRsuUnitPtr->updateRsuTopics(rsuId, topic);
        }
    }
}

// The main entry point for this application.
int main(int argc, char *argv[])
{
    return run_plugin<TelematicBridge::TelematicBridgePlugin>("Telematic Bridge", argc, argv);
}