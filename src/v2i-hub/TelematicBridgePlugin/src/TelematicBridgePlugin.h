#ifndef _TelematicBRIDGEPLUGIN_H_
#define _TelematicBRIDGEPLUGIN_H_

#include <TmxMessageManager.h>


#include "TelematicBridgeMsgWorker.h"
#include "TelematicUnit.h"
#include "TelematicRsuUnit.h"
#include "RSUConfigWorker.h"
#include <simulation/SimulationEnvUtils.h>
#include "health_monitor/HealthStatusMessageMapper.h"
#include "health_monitor/TRUHealthStatusTracker.h"


namespace TelematicBridge
{
    class TelematicBridgePlugin : public tmx::utils::TmxMessageManager
    {
    private:
    	const char* Key_SkippedMessages = "Messages Skipped";
        unsigned int _skippedMessages = 0;
        static CONSTEXPR const char *Telematic_MSGTYPE_J2735_STRING = "J2735";
        static CONSTEXPR const char *UNIT_TYPE_INFRASTRUCTURE = "Infrastructure";
        std::unique_ptr<TelematicUnit> _telematicUnitPtr;
        std::unique_ptr<TelematicRsuUnit> _telematicRsuUnitPtr;
        std::string _unitId;
        std::string _unitName;
        std::string _natsURL;
        std::string _excludedMessages;
        std::string _maxConnections;
        int16_t _pluginHeartBeatInterval; //Unit of second
        int16_t _rsuStatusMonitorInterval; //Unit of second
        unique_ptr<tmx::utils::ThreadTimer> _rsuRegistrationConfigTimer;
        unique_ptr<tmx::utils::ThreadTimer> _healthStatusTimer;  ///< Timer for periodic health status publishing
        bool _startedRegistrationTh = false;
        bool _startedHealthMonitorTh = false;
        int16_t rsuConfigUpdateIntervalInMillisec = 1000;
        uint _timerThId;
        uint _healthStatusTimerThId;  ///< Thread ID for health status timer
        bool _isTRU = false;
        bool _isTRURegistered = false;
        std::mutex _configMutex;

        /**
         * @brief Process RSU Status Message and update TRU health status tracker
         * @param routeMsg The routeable message containing RSU status
         */
        void ProcessRSUStatusMessage(tmx::routeable_message &routeMsg);

        void RegisterTRU();

        /**
         * @brief Update unit health status in TRU tracker
         * @param status The status string (e.g., "running", "error")
         */
        void UpdateUnitHealthStatus(const std::string &status);

    public:
        explicit TelematicBridgePlugin(const std::string &name);
        void OnConfigChanged(const char *key, const char *value) override;
        void OnStateChange(IvpPluginState state) override;
        void UpdateConfigSettings();
        void OnMessageReceived(IvpMessage *msg) override;
        void BroadcastRSURegistrationConfigMessage();        
    };

} // namespace TelematicBridge

#endif