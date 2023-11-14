#include "RSUHealthMonitorPlugin.h"

using namespace RSUHealthMonitor;
using namespace tmx::utils;

namespace RSUHealthMonitor
{

    RSUHealthMonitorPlugin::RSUHealthMonitorPlugin(const std::string &name) : PluginClient(name)
    {
        _rsuWorker = std::make_shared<RSUHealthMonitorWorker>();
        _rsuStatusTimer = make_unique<ThreadTimer>();
        UpdateConfigSettings();

        // Send SNMP call to RSU periodically at configurable interval.
        _timerThId = _rsuStatusTimer->AddPeriodicTick([this]()
                                                      {
            // Periodic SNMP call to get RSU status based on RSU MIB version 4.1
            auto rsuStatusJson =  _rsuWorker->getRSUStatus(_rsuMibVersion, _rsuIp, _snmpPort, _securityUser, _authPassPhrase, _securityLevel, SEC_TO_MICRO);
            PLOG(logINFO) << "Updating _interval: " << _interval;
            //Broadcast RSU status periodically at _interval
            BroadcastRSUStatus(rsuStatusJson); },
                                                      std::chrono::milliseconds(_interval * SEC_TO_MILLI));
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

        try
        {
            _rsuStatusTimer->ChangeFrequency(_timerThId, std::chrono::milliseconds(_interval * SEC_TO_MILLI));
        }
        catch (const tmx::TmxException &ex)
        {
            PLOG(logERROR) << ex.what();
        }
    }

    void RSUHealthMonitorPlugin::OnConfigChanged(const char *key, const char *value)
    {
        PluginClient::OnConfigChanged(key, value);
        UpdateConfigSettings();
        _rsuStatusTimer->Start();
    }

    void RSUHealthMonitorPlugin::BroadcastRSUStatus(const Json::Value &rsuStatusJson)
    {
        // Broadcast the RSU status info when there are RSU responses.
        if (!rsuStatusJson.empty() && _rsuWorker)
        {
            auto rsuStatusFields = _rsuWorker->getJsonKeys(rsuStatusJson);
            auto configTbl = _rsuWorker->GetRSUStatusConfig(_rsuMibVersion);

            // Only broadcast RSU status when all required fields are present.
            if (_rsuWorker->validateAllRequiredFieldsPresent(configTbl, rsuStatusFields))
            {
                auto sendRsuStatusMsg = _rsuWorker->convertJsonToTMXMsg(rsuStatusJson);
                BroadcastMessage(sendRsuStatusMsg, RSUHealthMonitorPlugin::GetName());
            }
        }
    }

} // namespace RSUHealthMonitor

int main(int argc, char *argv[])
{
    return run_plugin<RSUHealthMonitorPlugin>("RSU Health Monitor", argc, argv);
}
