
#pragma once

#include <PluginClient.h>
#include <jsoncpp/json/json.h>
#include <RSUStatusMessage.h>
#include <rsu/RSUSpec.h>
#include <RSURegistrationConfigMessage.h>

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
        // Supported status are "CONNECTED" and "DISCONNECTED"
        static inline const std::string CONNECTED = "CONNECTED";
        static inline const std::string DISCONNECTED = "DISCONNECTED"; 
        // prefix for RSU connection status key
        static inline const std::string _keyRSUConnectedPrefix = "RSU at ";
        // vector of RSU connection status keys for all configured RSUs to clear previous status when config changes
        std::vector<std::string> _rsuConnectedStatusKeys;

        /**
         * @brief Broadcast RSU status
         * @param rsuStatusJson RSU status in JSON format
         */
        void BroadcastRSUStatus(const Json::Value &rsuStatusJson, const tmx::utils::rsu::RSU_SPEC &mibVersion);

        /**
         * @brief Handle RSURegistrationConfigMessage to update configurations
         * @param msg RSURegistrationConfigMessage containing updated RSU configurations
         * @param routeableMsg Routeable message wrapper
         */
        void OnRSURegistrationConfigMessage(tmx::messages::RSURegistrationConfigMessage &msg, routeable_message &routeableMsg);

    public:
        explicit RSUHealthMonitorPlugin(const std::string &name);
        void UpdateConfigSettings();
        void OnConfigChanged(const char *key, const char *value) override;
        void monitorRSUs();
    };

} // namespace RSUHealthMonitorPlugin