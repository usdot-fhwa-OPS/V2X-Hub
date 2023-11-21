#ifndef _TelematicBRIDGEPLUGIN_H_
#define _TelematicBRIDGEPLUGIN_H_

#include "PluginClient.h"
#include "TelematicBridgeMsgWorker.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace tmx::utils;
using namespace std;
namespace pt = boost::property_tree;
namespace TelematicBridge
{

   
    class TelematicBridgePlugin : public tmx::utils::PluginClient
    {
    private:
        static CONSTEXPR const char *Telematic_MSGTYPE_J2735_STRING = "J2735";
        void OnMessageReceived(IvpMessage *msg);

    public:
        explicit TelematicBridgePlugin(const string& name);
        ~TelematicBridgePlugin() override = default;
    };

} // namespace TelematicBridge

#endif