#ifndef _TelematicBRIDGEPLUGIN_H_
#define _TelematicBRIDGEPLUGIN_H_

#include "PluginClient.h"
#include "TelematicBridgeJ2735MsgWorker.h"
#include "xml2json.h"

using namespace tmx::utils;
using namespace std;
namespace TelematicBridge
{

   
    class TelematicBridgePlugin : public tmx::utils::PluginClient
    {
    private:
        static CONSTEXPR const char *Telematic_MSGTYPE_J2735_STRING = "J2735";
        void OnMessageReceived(IvpMessage *msg);

    public:
        TelematicBridgePlugin(string name);
        virtual ~TelematicBridgePlugin();
    };

} // namespace TelematicBridge

#endif