#ifndef _TelematicBRIDGEPLUGIN_H_
#define _TelematicBRIDGEPLUGIN_H_

#include <TmxMessageManager.h>


#include "TelematicBridgeMsgWorker.h"
#include "TelematicUnit.h"
#include "TelematicRsuUnit.h"
#include "RSUConfigWorker.h"
#include <simulation/SimulationEnvUtils.h>
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
        static CONSTEXPR const char *HEALTH_STATUS_TOPIC_SUFFIX = "monitor.plugin.health_status";
        static CONSTEXPR const char *RSU_HEALTH_STATUS_TOPIC_SUFFIX = "monitor.rsu.health_status";
        std::unique_ptr<TelematicUnit> _telematicUnitPtr;
        std::unique_ptr<TelematicRsuUnit> _telematicRsuUnitPtr;
        std::string _unitId;
        std::string _unitName;
        std::string _natsURL;
        std::string _excludedMessages;
        std::string _maxConnections;
        int16_t _pluginHeartBeatInterval;
        int16_t _rsuStatusMonitorInterval;
        unique_ptr<tmx::utils::ThreadTimer> _rsuRegistrationConfigTimer;
        unique_ptr<tmx::utils::ThreadTimer> _healthStatusTimer;  ///< Timer for periodic health status publishing
        bool _started = false;
        int16_t rsuConfigUpdateIntervalInMillisec = 100;
        uint _timerThId;
        uint _healthStatusTimerThId;  ///< Thread ID for health status timer
        bool _isTRU = false;
        std::mutex _configMutex;
        TRUHealthStatusTracker _truHealthStatusTracker;  ///< Thread-safe tracker for TRU health status

        /**
         * @brief Process RSU Status Message and update TRU health status tracker
         * @param routeMsg The routeable message containing RSU status
         */
        void ProcessRSUStatusMessage(const tmx::routeable_message &routeMsg);

        /**
         * @brief Update unit health status in TRU tracker
         * @param status The status string (e.g., "registered", "running", "stopped")
         */
        void UpdateUnitHealthStatus(const std::string &status);

        /**
         * @brief Helper method to publish health status data to NATS
         * @param topicSuffix The suffix to append to "unit.<unit_id>."
         * @param healthData The JSON data to publish
         */
        void PublishHealthStatusToNATS(const std::string &topicSuffix, const Json::Value &healthData);

    public:
        explicit TelematicBridgePlugin(const std::string &name);
        void OnConfigChanged(const char *key, const char *value) override;
        void OnStateChange(IvpPluginState state) override;
        void UpdateConfigSettings();
        void OnMessageReceived(IvpMessage *msg) override;
        void BroadcastRSURegistrationConfigMessage();
        
        /**
         * @brief Update the available topics for a specific RSU
         * @param rsuIp RSU IP address
         * @param rsuPort RSU port number
         * @param topic Topic name to add to the RSU's available topics
         */
        void updateRsuTopics(const std::string &rsuIp, int rsuPort, const std::string &topic);

        /**
         * @brief Update an RSU health status in the TRU tracker
         * @param rsuId The RSU identifier in format "IP:port"
         * @param status The RSU health status message
         */
        void updateRsuHealthStatus(const std::string &rsuId, const RSUHealthStatusMessage &status)
        {
            _truHealthStatusTracker.updateRsuStatus(rsuId, status);
        }

        /**
         * @brief Get the TRU health status snapshot
         * @return Current TRU health status message
         */
        TelematicBridgePlugin::TRUHealthStatusMessage getTruHealthStatusSnapshot() const
        {
            return _truHealthStatusTracker.getSnapshot();
        }

        /**
         * @brief Get the TRU health status tracker
         * @return Reference to the TRU health status tracker
         */
        TelematicBridgePlugin::TRUHealthStatusTracker& getTruHealthStatusTracker()
        {
            return _truHealthStatusTracker;
        }

    };

} // namespace TelematicBridge

#endif