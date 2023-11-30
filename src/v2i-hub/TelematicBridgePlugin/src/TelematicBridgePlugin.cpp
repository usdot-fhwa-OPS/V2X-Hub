#include "TelematicBridgePlugin.h"

using namespace tmx::utils;
using namespace std;

namespace TelematicBridge
{
    TelematicBridgePlugin::TelematicBridgePlugin(const string &name) : PluginClient(name)
    {
        _telematicUnitPtr = make_unique<TelematicUnit>();
        _unitId = std::getenv("INFRASTRUCTURE_ID");
        _unitName = std::getenv("INFRASTRUCTURE_NAME");
        UpdateConfigSettings();
        AddMessageFilter("*", "*", IvpMsgFlags_None);
        AddMessageFilter("J2735", "*", IvpMsgFlags_RouteDSRC);
        SubscribeToMessages();
    }

    void TelematicBridgePlugin::OnMessageReceived(IvpMessage *msg)
    {
        if (msg && msg->type)
        {
            auto json = IvpMessageToJson(msg);
            // Process J2735 message payload hex string
            if (strcasecmp(msg->type, Telematic_MSGTYPE_J2735_STRING) == 0)
            {
                auto messageFm = (MessageFrame_t *)calloc(1, sizeof(MessageFrame_t));
                DecodeJ2735Msg(msg->payload->valuestring, messageFm);
                string xml_payload_str = ConvertJ2735FrameToXML(messageFm);
                ASN_STRUCT_FREE(asn_DEF_MessageFrame, messageFm);
                string json_payload_str = xml2Json(xml_payload_str.c_str());
                json["payload"] = StringToJson(json_payload_str);
            }

            stringstream topic;
            topic << (msg->type ? msg->type : "") << "_" << (msg->subtype ? msg->subtype : "") << "_" << (msg->source ? msg->source : "");
            auto topicStr = topic.str();
            _telematicUnitPtr->updateAvailableTopics(topicStr);
            if (_telematicUnitPtr->inSelectedTopics(topicStr))
            {
                _telematicUnitPtr->publishMessage(topicStr, json);
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
        PluginClient::OnStateChange(state);
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
        PluginClient::OnConfigChanged(key, value);
        UpdateConfigSettings();
    }
}

// The main entry point for this application.
int main(int argc, char *argv[])
{
    return run_plugin<TelematicBridge::TelematicBridgePlugin>("Telematic Bridge", argc, argv);
}