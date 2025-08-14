#ifndef _TelematicBRIDGEPLUGIN_H_
#define _TelematicBRIDGEPLUGIN_H_

#include <TmxMessageManager.h>
#include <tmx/messages/TmxJ2735.hpp>
#include <tmx/messages/TmxJ2735Codec.hpp>

#include "TelematicBridgeMsgWorker.h"
#include "TelematicUnit.h"
#include <simulation/SimulationEnvUtils.h>


namespace TelematicBridge
{
    class TelematicBridgePlugin : public tmx::utils::TmxMessageManager
    {
    private:
        static CONSTEXPR const char *Telematic_MSGTYPE_J2735_STRING = "J2735";
        static CONSTEXPR const char *UNIT_TYPE_INFRASTRUCTURE = "Infrastructure";
        std::unique_ptr<TelematicUnit> _telematicUnitPtr;
        std::string _unitId;
        std::string _unitName;
        std::string _natsURL;
        std::string _excludedMessages;
        std::mutex _configMutex;
        void OnMessageReceived(tmx::routeable_message &msg) override;

    public:
        explicit TelematicBridgePlugin(const std::string &name);
        void OnConfigChanged(const char *key, const char *value) override;
        void OnStateChange(IvpPluginState state) override;
        void UpdateConfigSettings();
        ~TelematicBridgePlugin() override = default;
    };

} // namespace TelematicBridge

#endif