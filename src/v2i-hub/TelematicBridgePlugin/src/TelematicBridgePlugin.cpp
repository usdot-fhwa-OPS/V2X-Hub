#include "TelematicBridgePlugin.h"

using namespace tmx::utils;
using namespace std;

namespace TelematicBridge
{
    TelematicBridgePlugin::TelematicBridgePlugin(const string &name) : TmxMessageManager(name)
    {
        // _telematicUnitPtr = make_unique<TelematicUnit>();
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
        Json::Value json = IvpMessageToJson(msg);
        // Overwrite HEX String payload with JER encode JSON payload for J2735 Messages
        if (PluginClient::IsJ2735Message(msg))
        {
            // Convert routeable message to J2735 encoded message
            tmx::messages::TmxJ2735EncodedMessage<tmx::messages::MessageFrameMessage> rMsg = msg.get_payload<tmx::messages::TmxJ2735EncodedMessage<tmx::messages::MessageFrameMessage>>();
            // Decode Encode J2735 Message
            auto j2735Data = rMsg.decode_j2735_message().get_j2735_data();
            tmx::messages::TmxJ2735Message<MessageFrame_t, tmx::JSON> j2735Message =
                tmx::messages::TmxJ2735Message<MessageFrame_t, tmx::JSON>(j2735Data);
            // Convert J2735 message to JSON
            std::string json_payload_str = j2735Message.to_string();
            // Create a Json::CharReaderBuilder and Json::CharReader to parse the string
            Json::CharReaderBuilder builder;
            Json::Value parsedParam;
            std::string errs;
            std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

            // Parse the JSON string
            if (reader->parse(json_payload_str.data(), json_payload_str.data() + json_payload_str.size(), &parsedParam, &errs)) {
                json["payload"] = parsedParam;
            } else {
                PLOG(tmx::utils::LogLevel::logERROR) << "Failed to parse JSON string: " << json_payload_str << " with errors " << errs;
            }
            // Free the J2735 data structure
            ASN_STRUCT_FREE(asn_DEF_MessageFrame, j2735Data.get());
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