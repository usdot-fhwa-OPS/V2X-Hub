
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
#include <nmeaparse/nmea.h>
#include <algorithm>

using namespace tmx::utils;
using namespace std;
using namespace boost::property_tree;

namespace RSUHealthMonitor
{

    struct RSUOIDConfig
    {
        string field;
        string oid;
        bool required; // Indicate whether this field is required to before broadcasting the RSU status.
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
        string _securityLevel;
        vector<RSUOIDConfig> _rsuOIDConfigMap;
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
         * @brief Sending SNMP requests to get info for each field in the _rsuOIDConfigMap, and return the RSU status in JSON
         */
        Json::Value getRSUstatus();
        /**
         * @brief Parse NMEA GPS sentense and return GPS related data
         * @param gps_nmea_data NMEA GPS sentense
         * @return map<double, double>  A map of latitude and longitude
         */
        std::map<double, double> ParseGPS(const std::string &gps_nmea_data);
        /**
         * @brief determine if all required fields in the RSU config map _rsuOIDConfigMap present in the input fields
         * @return True if all required fields found. Otherwise, false.
         */
        bool isAllRequiredFieldsPresent(vector<string> fields);

    public:
        RSUHealthMonitorPlugin(std::string name);
        virtual ~RSUHealthMonitorPlugin();
        void UpdateConfigSettings();
        void OnConfigChanged(const char *key, const char *value);
    };

} // namespace RSUHealthMonitorPlugin

#endif