#include "TelematicBridgePlugin.h"

namespace TelematicBridge
{
    TelematicBridgePlugin::TelematicBridgePlugin(const string &name) : PluginClient(name)
    {
        AddMessageFilter("*", "*", IvpMsgFlags_None);
        SubscribeToMessages();
    }

    void TelematicBridgePlugin::OnMessageReceived(IvpMessage *msg)
    {
        if (msg && msg->type)
        {
            PLOG(logDEBUG2) << "Message Received."
                            << " Type: " << msg->type << ", SubType: " << msg->subtype << ", DSRC metadata: " << msg->dsrcMetadata << ", encoding: " << msg->encoding << ", source: " << msg->source << ", sourceId: " << msg->sourceId << ", flags: " << msg->flags << ", timestamp: " << msg->timestamp << ", payload: " << cJSON_Print(msg->payload);
            auto json = TelematicBridgeMsgWorker::IvpMessageToJson(msg);
            // Process J2735 message payload hex string
            if (strcasecmp(msg->type, Telematic_MSGTYPE_J2735_STRING) == 0)
            {
                auto messageFm = (MessageFrame_t *)malloc(sizeof(MessageFrame_t));
                TelematicBridgeMsgWorker::DecodeJ2735Msg(msg->payload->valuestring, messageFm);
                string xml_payload_str = TelematicBridgeMsgWorker::ConvertJ2735FrameToXML(messageFm);
                ASN_STRUCT_FREE(asn_DEF_MessageFrame, messageFm);
                string json_payload_str = xml2json(xml_payload_str.c_str());
                json["payload"] = TelematicBridgeMsgWorker::StringToJson(json_payload_str);
            }

            auto jsonStr = TelematicBridgeMsgWorker::JsonToString(json);
            PLOG(logINFO) << jsonStr;
        }
    }
}

// The main entry point for this application.
int main(int argc, char *argv[])
{
    return run_plugin<TelematicBridge::TelematicBridgePlugin>("Telematic Bridge", argc, argv);
}