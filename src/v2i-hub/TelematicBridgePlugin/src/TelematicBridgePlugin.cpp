#include "TelematicBridgePlugin.h"

namespace TelematicBridge
{
    TelematicBridgePlugin::TelematicBridgePlugin(const string &name) : PluginClient(name)
    {
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
                json["payload"] = StringToJson(xml2Json(xml_payload_str));
            }

            auto jsonStr = JsonToString(json);
            PLOG(logINFO) << jsonStr;
        }
    }
}

// The main entry point for this application.
int main(int argc, char *argv[])
{
    return run_plugin<TelematicBridge::TelematicBridgePlugin>("Telematic Bridge", argc, argv);
}