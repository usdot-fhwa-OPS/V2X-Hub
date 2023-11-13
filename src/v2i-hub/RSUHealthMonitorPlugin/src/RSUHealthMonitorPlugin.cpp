#include "RSUHealthMonitorPlugin.h"

using namespace RSUHealthMonitor;
using namespace tmx::utils;

namespace RSUHealthMonitor
{

    RSUHealthMonitorPlugin::RSUHealthMonitorPlugin(const std::string &name) : PluginClient(name)
    {
        _rsuWorker = std::make_shared<RSUHealthMonitorWorker>();
        UpdateConfigSettings();

        // Send SNMP call to RSU periodically at configurable interval.
        _rsuStatusTimer = make_unique<ThreadTimer>();
        _rsuStatusTimer->AddPeriodicTick([this]()
                                         {
            // Periodic SNMP call to get RSU status based on RSU MIB version 4.1
            auto rsuStatusJson =  _rsuWorker->getRSUStatus(_rsuMibVersion, _rsuIp, _snmpPort, _authPassPhrase, _securityUser, _securityLevel, SEC_TO_MICRO);

            //Broadcast RSU status periodically at _interval
            BroadcastRSUStatus(rsuStatusJson); },
                                         chrono::seconds(_interval));
        _rsuStatusTimer->Start();
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
            _rsuMibVersion = RSUMibVersion::RSUMIB_V_4_1;
        }
        else
        {
            _rsuMibVersion = RSUMibVersion::UNKOWN_MIB_V;
            PLOG(logERROR) << "Uknown RSU MIB version: " << _rsuMIBVersionStr;
        }
    }

    void RSUHealthMonitorPlugin::OnConfigChanged(const char *key, const char *value)
    {
        PluginClient::OnConfigChanged(key, value);
        UpdateConfigSettings();
    }

    void RSUHealthMonitorPlugin::BroadcastRSUStatus(const Json::Value &rsuStatusJson)
    {
        try
        {
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
    }

} // namespace RSUHealthMonitor

int main(int argc, char *argv[])
{
    return run_plugin<RSUHealthMonitorPlugin>("RSU Health Monitor", argc, argv);
}
