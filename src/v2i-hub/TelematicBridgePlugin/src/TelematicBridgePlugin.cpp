#include "TelematicBridgePlugin.h"

namespace TelematicBridge
{
    TelematicBridgePlugin::TelematicBridgePlugin(string name) : PluginClient(name)
    {
        AddMessageFilter("*", "*", IvpMsgFlags_None);
        SubscribeToMessages();
    }

    TelematicBridgePlugin::~TelematicBridgePlugin()
    {
    }

    void TelematicBridgePlugin::OnMessageReceived(IvpMessage *msg)
    {
        PLOG(logINFO) << "Message Received."
                      << " Type: " << msg->type << ", SubType: " << msg->subtype << ", DSRC metadata: " << msg->dsrcMetadata << ", encoding: " << msg->encoding << ", source: " << msg->source << ", sourceId: " << msg->sourceId << ", flags: " << msg->flags << ", timestamp: " << msg->timestamp << ", payload: " << msg->payload->valuestring;
        // Process J2735 message
        if (strcasecmp(msg->type, Telematic_MSGTYPE_J2735_STRING) == 0)
        {
            auto messageFm = (MessageFrame_t *)malloc(sizeof(MessageFrame_t));
            TelematicBridgeJ2735MsgWorker::DecodeJ2735Msg(msg->payload->valuestring, messageFm);
            string xml_payload_str = TelematicBridgeJ2735MsgWorker::ConvertJ2735FrameToXML(messageFm);
            ASN_STRUCT_FREE(asn_DEF_MessageFrame, messageFm);
            string json_payload_str = xml2json(xml_payload_str.c_str());
            PLOG(logINFO) << json_payload_str;
        }
        else
        {
            PLOG(logINFO) << msg;
        }
    }
}

// The main entry point for this application.
int main(int argc, char *argv[])
{
    return run_plugin<TelematicBridge::TelematicBridgePlugin>("Telematic Bridge", argc, argv);
}