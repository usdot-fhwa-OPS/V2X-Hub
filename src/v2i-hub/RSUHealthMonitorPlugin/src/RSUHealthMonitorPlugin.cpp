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

        // Update the OID to RSU field mapping
        string rsuOIDMapJsonStr;
        GetConfigValue<string>("RSUOIDConfigMap", rsuOIDMapJsonStr);
        UpdateRSUOIDConfig(rsuOIDMapJsonStr);

        // Create SNMP client and use SNMP V3 protocol
        try
        {
            _snmpClientPtr = std::make_shared<snmp_client>(_rsuIp, _snmpPort, "", _securityUser, "authNoPriv", _authPassPhrase, 3);
            PLOG(logINFO) << "Updated SNMP client call: RSU IP: " << _rsuIp << ", RSU port: " << _snmpPort << ", User: " << _securityUser << ", auth pass phrase: " << _authPassPhrase << ", security level: "
                          << "authNoPriv";
        }
        catch (std::exception &ex)
        {
            PLOG(logERROR) << "Cannot create SNMP client due to an error. The error message is: " << ex.what();
        }
    }

    void RSUHealthMonitorPlugin::UpdateRSUOIDConfig(string &json_str)
    {

        if (json_str.length() == 0)
        {
            PLOG(logERROR) << "Error updating RSU OID config due to JSON is empty.";
        }
        try
        {
            // Example:"{\"RSUOIDConfig\": [{\"RsuField\": \"rsuGpsOutpuString\", \"OID\": \"1.0.15628.4.1.8.5.0\"}, {\"RsuField\": \"rsuID\", \"OID\": \"1.0.15628.4.1.8.5.0\"}] }"
            ptree pt;
            istringstream iss(json_str);
            read_json(iss, pt);
            _rsuOIDConfigMap.clear();
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
        }
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
            PLOG(logDEBUG) << "RSU status update call at every " << _interval << "seconds!";
            Json::Value rsuStatuJson;
            //Sending RSU SNMP call for each field as each field has its own OID.
            for_each(_rsuOIDConfigMap.begin(), _rsuOIDConfigMap.end(), [this, &rsuStatuJson](RSUOIDConfig &config)
                     {
                        try
                        {
                            PLOG(logINFO) << "SNMP RSU status call for field:"<< config.field << ", OID: " << config.oid;
                            snmp_response_obj responseVal;
                            if(_snmpClientPtr != nullptr)
                            {
                                _snmpClientPtr->process_snmp_request(config.oid, request_type::GET, responseVal);
                                if(responseVal.type == snmp_response_obj::response_type::INTEGER)
                                {
                                    rsuStatuJson[config.field] = responseVal.val_int;
                                }
                                else if(responseVal.type == snmp_response_obj::response_type::STRING)
                                {
                                    string response_str(responseVal.val_string.begin(), responseVal.val_string.end());
                                    rsuStatuJson[config.field] = response_str;
                                }                                
                            }
                        }
                        catch (std::exception &ex)
                        {
                            PLOG(logERROR) << "SNMP call failure due to: " << ex.what();
                        } });
            //Broadcast the RSU status info when there are any RSU responses.
            if (!rsuStatuJson.empty())
            {
                Json::FastWriter fasterWirter;
                string json_str = fasterWirter.write(rsuStatuJson);
                tmx::messages::RSUStatusMessage sendRsuStatusMsg;
                sendRsuStatusMsg.set_contents(json_str);
                BroadcastMessage(sendRsuStatusMsg);
            }
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
