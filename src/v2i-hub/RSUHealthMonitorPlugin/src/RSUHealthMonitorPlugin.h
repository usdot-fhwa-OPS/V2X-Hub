
#pragma once

#include <PluginClient.h>
#include <jsoncpp/json/json.h>
#include <RSUStatusMessage.h>
#include <rsu/RSUSpec.h>

#include "RSUHealthMonitorWorker.h"
#include "RSUConfigurationList.h"

using namespace tmx::utils;
using namespace std;

namespace RSUHealthMonitor
{

    class RSUHealthMonitorPlugin : public PluginClient
    {
    private:
        mutex _configMutex;
        uint16_t _interval;
        string _rsuConfigListStr;
        shared_ptr<RSUConfigurationList> _rsuConfigListPtr;
        shared_ptr<RSUHealthMonitorWorker> _rsuWorker;
        unique_ptr<ThreadTimer> _rsuStatusTimer;
        uint _timerThId;
        bool started = false;
        const long SEC_TO_MICRO = 1000000;
        const long SEC_TO_MILLI = 1000;
        // prefix for RSU connection status key
        // Supported status are "CONNECTED" and "DISCONNECTED"
        const std::string _keyRSUConnectedPrefix = "RSU at ";
        // vector of RSU connection status keys for all configured RSUs to clear previous status when config changes
        std::vector<std::string> _rsuConnectedStatusKeys;

        /**
         * @brief Broadcast RSU status
         * @param rsuStatusJson RSU status in JSON format
         */
        void BroadcastRSUStatus(const Json::Value &rsuStatusJson, const tmx::utils::rsu::RSU_SPEC &mibVersion);

    public:
        explicit RSUHealthMonitorPlugin(const std::string &name);
        void UpdateConfigSettings();
        void OnConfigChanged(const char *key, const char *value) override;
        void monitorRSUs();
    };

} // namespace RSUHealthMonitorPlugin