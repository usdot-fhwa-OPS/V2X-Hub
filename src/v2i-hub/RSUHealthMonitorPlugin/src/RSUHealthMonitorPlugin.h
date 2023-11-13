
#pragma once

#include "PluginClient.h"
#include <jsoncpp/json/json.h>
#include "RSUStatusMessage.h"
#include "RSUHealthMonitorWorker.h"

using namespace tmx::utils;
using namespace std;

namespace RSUHealthMonitor
{

    class RSUHealthMonitorPlugin : public PluginClient
    {
    private:
        mutex _configMutex;
        uint16_t _interval;
        string _rsuIp;
        uint16_t _snmpPort;
        string _authPassPhrase;
        string _securityUser;
        string _securityLevel;
        string _rsuMIBVersionStr;
        RSUMibVersion _rsuMibVersion;
        const char *RSU4_1_str = "RSU4.1";
        const char *RSU1218_str = "RSU1218";
        shared_ptr<RSUHealthMonitorWorker> _rsuWorker;
        unique_ptr<ThreadTimer> _rsuStatusTimer;
        uint _timerThId;
        const long SEC_TO_MICRO = 1000000;
        const long SEC_TO_MILLI= 1000;
        /**
         * @brief Broadcast RSU status
         * @param Json::Value RSU status in JSON format
         */
        void BroadcastRSUStatus(const Json::Value& rsuStatusJson);

    public:
        explicit RSUHealthMonitorPlugin(const std::string &name);
        void UpdateConfigSettings();
        void OnConfigChanged(const char *key, const char *value) override;
    };

} // namespace RSUHealthMonitorPlugin