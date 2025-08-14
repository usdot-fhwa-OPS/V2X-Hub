#include "TelematicBridgePlugin.h"

using namespace tmx::utils;
using namespace std;

namespace TelematicBridge
{
    TelematicBridgePlugin::TelematicBridgePlugin(const string &name) : TmxMessageManager(name)
    {
        _telematicUnitPtr = make_unique<TelematicUnit>();
        _unitId = std::getenv("INFRASTRUCTURE_ID");
        _unitName = std::getenv("INFRASTRUCTURE_NAME");
        AddMessageFilter("*", "*", IvpMsgFlags_None);
        AddMessageFilter("J2735", "*", IvpMsgFlags_RouteDSRC);
        SubscribeToMessages();
    }

    void TelematicBridgePlugin::OnMessageReceived(tmx::routeable_message &msg)
    {
        TmxMessageManager::OnMessageReceived(msg);
        // Convert IVP message to JSON CPP Value
        Json::Value json = routeableMessageToJsonValue(msg);
        // Overwrite HEX String payload with JER encode JSON payload for J2735 Messages
        if (PluginClient::IsJ2735Message(msg))
        {
            // Convert routeable message to J2735 encoded message
            std::string json_payload_str = j2735MessageToJson(msg);
            // Update the JSON payload
            json["payload"] = stringToJsonValue(json_payload_str);
        }
       
        stringstream topic;
        topic << (msg.get_type()) << "_" << (msg.get_subtype()) << "_" << (msg.get_source());
        auto topicStr = topic.str();
        _telematicUnitPtr->updateAvailableTopics(topicStr);
        if (_telematicUnitPtr->inSelectedTopics(topicStr))
        {
            _telematicUnitPtr->publishMessage(topicStr, json);
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
}

// The main entry point for this application.
int main(int argc, char *argv[])
{
    return run_plugin<TelematicBridge::TelematicBridgePlugin>("Telematic Bridge", argc, argv);
}