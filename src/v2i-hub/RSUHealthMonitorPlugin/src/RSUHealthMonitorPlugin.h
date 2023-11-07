
#ifndef RSUHEALTHMONITORLUGIN_H_
#define RSUHEALTHMONITORLUGIN_H_

#include "PluginClient.h"
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "SNMPClient.h"
#include <jsoncpp/json/json.h>
#include "RSUStatusMessage.h"

using namespace tmx::utils;
using namespace std;
using namespace boost::property_tree;

namespace RSUHealthMonitor
{

    struct RSUOIDConfig
    {
        string field;
        string oid;
    };

    class RSUHealthMonitorPlugin : public PluginClient
    {
    private:
        mutex _configMutex;
        uint16_t _interval;
        string _rsuIp;
        uint16_t _snmpPort;
        string _authPassPhrase;
        string _securityUser;
        vector<RSUOIDConfig> _rsuOIDConfigMap;
        std::shared_ptr<snmp_client> _snmpClientPtr;
        /**
         * @brief Update RSU OID configuration map with input JSON string.
         * @param JSON string with RSU OID configuration.
         */
        void UpdateRSUOIDConfig(string &json_str);
        void PeriodicRSUStatusReq();

    public:
        RSUHealthMonitorPlugin(std::string name);
        virtual ~RSUHealthMonitorPlugin();
        void UpdateConfigSettings();
        void OnConfigChanged(const char *key, const char *value);
    };

} // namespace RSUHealthMonitorPlugin

#endif