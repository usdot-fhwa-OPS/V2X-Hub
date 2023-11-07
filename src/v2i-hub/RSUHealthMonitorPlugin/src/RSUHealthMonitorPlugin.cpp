#include "RSUHealthMonitorPlugin.h"

using namespace RSUHealthMonitor;
using namespace tmx::utils;

namespace RSUHealthMonitor
{

    RSUHealthMonitorPlugin::RSUHealthMonitorPlugin(std::string name) : PluginClient(name)
    {
        UpdateConfigSettings();

        // Send SNMP call to RSU status at configurable interval.
        std::thread rsuStatus_t(&RSUHealthMonitorPlugin::PeriodicRSUStatusReq, this);
        rsuStatus_t.detach();
    }

    void RSUHealthMonitorPlugin::UpdateConfigSettings()
    {
        PLOG(logINFO) << "Updating configuration settings.";

        lock_guard<mutex> lock(_configMutex);
        GetConfigValue<uint16_t>("Interval", _interval);
        GetConfigValue<string>("RSUIp", _rsuIp);
        GetConfigValue<uint16_t>("SNMPPort", _snmpPort);
        GetConfigValue<string>("AuthPassPhrase", _authPassPhrase);
        GetConfigValue<string>("SecurityUser", _securityUser);
        string rsuOIDMapJsonStr;
        GetConfigValue<string>("RSUOIDConfigMap", rsuOIDMapJsonStr);
        UpdateRSUOIDConfig(rsuOIDMapJsonStr);
    }

    bool RSUHealthMonitorPlugin::UpdateRSUOIDConfig(string &json_str)
    {

        if (json_str.length() == 0)
        {
            return false;
        }
        try
        {
            // Example:"{\"RSUOIDConfig\": [{\"RsuField\": \"rsuGpsOutpuString\", \"OID\": \"1.0.15628.4.1.8.5.0\"}, {\"RsuField\": \"rsuID\", \"OID\": \"1.0.15628.4.1.8.5.0\"}] }"
            ptree pt;
            istringstream iss(json_str);
            read_json(iss, pt);
            BOOST_FOREACH (ptree::value_type &child, pt.get_child("RSUOIDConfig"))
            {
                // Array elements have no names.
                assert(child.first.empty());
                RSUOIDConfig config;
                config.field = child.second.get<string>("RsuField");
                config.oid = child.second.get<string>("OID");
                PLOG(logINFO) << "RSU OID Config: Field: " << config.field << ", OID: " << config.oid;
                // Add RSU OID to the map
                _rsuOIDConfigMap.push_back(config);
            }
        }
        catch (const std::exception &e)
        {
            PLOG(logERROR) << "Error updating RSU OID config" << e.what();
            return false;
        }
        return true;
    }

    void RSUHealthMonitorPlugin::OnConfigChanged(const char *key, const char *value)
    {
        PluginClient::OnConfigChanged(key, value);
        UpdateConfigSettings();
    }

    void RSUHealthMonitorPlugin::PeriodicRSUStatusReq()
    {
        while (true)
        {
            PLOG(logERROR) << "RSU status update call at every " << _interval;
            this_thread::sleep_for(chrono::seconds(_interval));
        }
    }

    RSUHealthMonitorPlugin::~RSUHealthMonitorPlugin()
    {
    }

} // namespace RSUHealthMonitor

int main(int argc, char *argv[])
{
    return run_plugin<RSUHealthMonitorPlugin>("RSU Health Monitor", argc, argv);
}
