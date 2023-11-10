
#pragma once

#include "PluginClient.h"
#include "SNMPClient.h"
#include <jsoncpp/json/json.h>
#include "RSUStatusMessage.h"
#include "RSUHealthMonitorWorker.h"

using namespace tmx::utils;
using namespace std;
using namespace boost::property_tree;

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
        std::shared_ptr<RSUHealthMonitorWorker> _rsuWorker;
        const long SEC_TO_MICRO = 1000000;
        // std::shared_ptr<snmp_client> _snmpClientPtr;
        /**
         * @brief Update RSU OID configuration map with input JSON string.
         * @param JSON string with RSU OID configuration.
         */
        void UpdateRSUOIDConfig(string &json_str);
        /**
         * @brief Periodically sending SNMP requests to get RSU status info.
         */
        void PeriodicRSUStatusReq();
        /**
         * @brief Sending SNMP requests to get info for each field in the RSUStatusConfigTable, and return the RSU status in JSON
         * Use RSU Status configuration table include RSU field, OIDs, and whether fields  are required or optional
         */
        Json::Value getRSUStatus();

    public:
        RSUHealthMonitorPlugin(std::string name);
        virtual ~RSUHealthMonitorPlugin();
        void UpdateConfigSettings();
        void OnConfigChanged(const char *key, const char *value);
    };

} // namespace RSUHealthMonitorPlugin