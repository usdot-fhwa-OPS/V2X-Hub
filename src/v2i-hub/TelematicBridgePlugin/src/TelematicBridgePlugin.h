#ifndef _TelematicBRIDGEPLUGIN_H_
#define _TelematicBRIDGEPLUGIN_H_

#include "PluginClient.h"
#include "TelematicBridgeMsgWorker.h"
#include "TelematicUnit.h"

using namespace tmx::utils;
using namespace std;
namespace TelematicBridge
{
    class TelematicBridgePlugin : public tmx::utils::PluginClient
    {
    private:
        static CONSTEXPR const char *Telematic_MSGTYPE_J2735_STRING = "J2735";
        shared_ptr<TelematicUnit> _telematicUnitPtr;
        string _unitId;
        string _unitType;
        string _unitName;
        string _natsURL;
        string _excludedTopics;
        mutex _configMutex;
        void OnMessageReceived(IvpMessage *msg);

    public:
        explicit TelematicBridgePlugin(const string &name);
        void OnConfigChanged(const char *key, const char *value) override;
        void OnStateChange(IvpPluginState state) override;
        void UpdateConfigSettings();
        ~TelematicBridgePlugin() override = default;
    };

} // namespace TelematicBridge

#endif