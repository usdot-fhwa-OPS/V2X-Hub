#include "RSUHealthMonitorPlugin.h"

using namespace RSUHealthMonitor;
using namespace tmx::utils;

namespace RSUHealthMonitor
{

    RSUHealthMonitorPlugin::RSUHealthMonitorPlugin(std::string name) : PluginClient(name)
    {
        _rsuWorker = std::make_shared<RSUHealthMonitorWorker>();
        UpdateConfigSettings();
        // Send SNMP call to RSU periodically at configurable interval.
        std::thread rsuStatus_t(&RSUHealthMonitorPlugin::PeriodicRSUStatusReq, this);
        rsuStatus_t.join();
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
        GetConfigValue<string>("SecurityLevel", _securityLevel);
        GetConfigValue<string>("RSUMIBVersion", _rsuMIBVersionStr);
        boost::trim_left(_rsuMIBVersionStr);
        boost::trim_right(_rsuMIBVersionStr);
        // Support RSU MIB version 4.1
        if (boost::iequals(_rsuMIBVersionStr, RSU4_1_str))
        {
            _rsuMibVersion = RSUMIB_4_1;
        }
        else
        {
            PLOG(logERROR) << "Unknow RSU Mib version: " + _rsuMIBVersionStr;
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
            try
            {
                // Periodic SNMP call to get RSU status based on RSU MIB version 4.1
                auto rsuStatusJson = getRSUStatus();
                // Broadcast the RSU status info when there are RSU responses.
                if (!rsuStatusJson.empty())
                {
                    vector<string> rsuStatusFields;
                    for (auto const &field : rsuStatusJson.getMemberNames())
                    {
                        rsuStatusFields.push_back(field);
                    }
                    // Only broadcast RSU status when all required fields are present.
                    if (_rsuWorker && _rsuWorker->isAllRequiredFieldsPresent(_rsuMibVersion, rsuStatusFields))
                    {
                        Json::FastWriter fasterWirter;
                        string json_str = fasterWirter.write(rsuStatusJson);
                        tmx::messages::RSUStatusMessage sendRsuStatusMsg;
                        sendRsuStatusMsg.set_contents(json_str);
                        string source = RSUHealthMonitorPlugin::GetName();
                        BroadcastMessage(sendRsuStatusMsg, source);
                        PLOG(logINFO) << "Broadcast RSU status:  " << json_str;
                    }
                }
            }
            catch (const std::exception &ex)
            {
                PLOG(logERROR) << ex.what();
            }
            this_thread::sleep_for(chrono::seconds(_interval));
        }
    }

    Json::Value RSUHealthMonitorPlugin::getRSUStatus()
    {
        if (!_rsuWorker)
        {
            PLOG(logERROR) << "RSU status update call failed due to fail to initialize RSU worker!";
            return Json::nullValue;
        }

        auto rsuStatusConfigTbl = _rsuWorker->GetRSUStatusConfig(_rsuMibVersion);
        if (rsuStatusConfigTbl.size() == 0)
        {
            PLOG(logERROR) << "RSU status update call failed due to  RSU stataus config table is empty!";
            return Json::nullValue;
        }
        // Create SNMP client and use SNMP V3 protocol
        PLOG(logINFO) << "Update SNMP client: RSU IP: " << _rsuIp << ", RSU port: " << _snmpPort << ", User: " << _securityUser << ", auth pass phrase: " << _authPassPhrase << ", security level: "
                      << _securityLevel;
        auto _snmpClientPtr = std::make_unique<snmp_client>(_rsuIp, _snmpPort, "", _securityUser, _securityLevel, _authPassPhrase, SNMP_VERSION_3, SEC_TO_MICRO);
        if (_snmpClientPtr == nullptr)
        {
            PLOG(logERROR) << "Error creating SNMP client!";
            return Json::nullValue;
        }

        Json::Value rsuStatuJson;
        // Sending RSU SNMP call for each field as each field has its own OID.
        for (auto &config : rsuStatusConfigTbl)
        {
            try
            {
                PLOG(logINFO) << "SNMP RSU status call for field:" << config.field << ", OID: " << config.oid;
                snmp_response_obj responseVal;
                auto success = _snmpClientPtr->process_snmp_request(config.oid, request_type::GET, responseVal);
                if (!success && config.required)
                    PLOG(logERROR) << "SNMP session stopped as the required field: " << config.field << " failed!";
                    break;

                if (success && responseVal.type == snmp_response_obj::response_type::INTEGER)
                {
                    rsuStatuJson[config.field] = responseVal.val_int;
                }
                else if (success && responseVal.type == snmp_response_obj::response_type::STRING)
                {
                    string response_str(responseVal.val_string.begin(), responseVal.val_string.end());
                    PLOG(logDEBUG) << "String value in response: " << response_str;
                    // Proess GPS nmea string
                    auto gps = _rsuWorker->ParseRSUGPS(response_str);
                    rsuStatuJson["rsuGpsOutputStringLatitude"] = gps.begin()->first;
                    rsuStatuJson["rsuGpsOutputStringLongitude"] = gps.begin()->second;
                    rsuStatuJson[config.field] = response_str;
                }
            }
            catch (const std::exception &ex)
            {
                PLOG(logERROR) << "SNMP call failure due to: " << ex.what();
            }
        }
        return rsuStatuJson;
    }

    RSUHealthMonitorPlugin::~RSUHealthMonitorPlugin()
    {
    }

} // namespace RSUHealthMonitor

int main(int argc, char *argv[])
{
    return run_plugin<RSUHealthMonitorPlugin>("RSU Health Monitor", argc, argv);
}
