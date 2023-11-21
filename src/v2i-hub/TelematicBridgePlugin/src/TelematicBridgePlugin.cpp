#include "TelematicBridgePlugin.h"

namespace TelematicBridge
{
    TelematicBridgePlugin::TelematicBridgePlugin(const string &name) : PluginClient(name)
    {
        _telematicUnitPtr = make_shared<TelematicUnit>();
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
        GetConfigValue<int64_t>("NATSConnectionTimeOut", _natsConnTimeOut);
        GetConfigValue<int>("NATSConnectionAttempts", _natsConnAttempts);
        GetConfigValue<string>("NATSUrl", _natsURL);
        GetConfigValue<string>("UnitId", _unitId);
        GetConfigValue<string>("UnitName", _unitName);
        GetConfigValue<string>("UnitType", _unitType);
        unit_st unit;
        unit.unitId = _unitId;
        unit.unitName = _unitName;
        unit.unitType = _unitType;
        if (_telematicUnitPtr)
        {
            _telematicUnitPtr->setUnit(unit);
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
                _telematicUnitPtr->connect(_natsURL, _natsConnAttempts, _natsConnTimeOut);
            }
        }
    }

    void TelematicBridgePlugin::OnConfigChanged(const char *key, const char *value)
    {
        PLOG(logDEBUG1) << "OnConfigChanged called";
        PluginClient::OnConfigChanged(key, value);
        UpdateConfigSettings();
    }
}

// The main entry point for this application.
int main(int argc, char *argv[])
{
    return run_plugin<TelematicBridge::TelematicBridgePlugin>("Telematic Bridge", argc, argv);
}