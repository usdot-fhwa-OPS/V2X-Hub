#include "RSUHealthMonitorPlugin.h"

using namespace RSUHealthMonitor;
using namespace tmx::utils;

namespace RSUHealthMonitor
{

    RSUHealthMonitorPlugin::RSUHealthMonitorPlugin(const std::string &name) : PluginClient(name)
    {
        _rsuWorker = std::make_shared<RSUHealthMonitorWorker>();
        _rsuStatusTimer = make_unique<ThreadTimer>();
        _rsuConfigListPtr = std::make_shared<RSUConfigurationList>();
        UpdateConfigSettings();

        // Send SNMP call to RSU periodically at configurable interval.
        _timerThId = _rsuStatusTimer->AddPeriodicTick([this]()
                                                      {
                        this->monitorRSUs();
                        PLOG(logINFO) << "Updating RSU _interval: " << _interval; },
                                                      std::chrono::milliseconds(_interval * SEC_TO_MILLI));
        _rsuStatusTimer->Start();
    }

    void RSUHealthMonitorPlugin::monitorRSUs()
    {
        for (auto rsuConfig : _rsuConfigListPtr->getConfigs())
        {
            auto rsuStatusJson = _rsuWorker->getRSUStatus(rsuConfig.mibVersion, rsuConfig.rsuIp, rsuConfig.snmpPort, rsuConfig.user, rsuConfig.authPassPhrase, rsuConfig.securityLevel, SEC_TO_MICRO);
            BroadcastRSUStatus(rsuStatusJson, rsuConfig.mibVersion);
        }
    }

    void RSUHealthMonitorPlugin::UpdateConfigSettings()
    {
        PLOG(logINFO) << "Updating configuration settings.";

        lock_guard<mutex> lock(_configMutex);
        GetConfigValue<uint16_t>("Interval", _interval);
        GetConfigValue<string>("RSUConfigurationList", _rsuConfigListStr);
        _rsuConfigListPtr->parseRSUs(_rsuConfigListStr);

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
    }

    void RSUHealthMonitorPlugin::BroadcastRSUStatus(const Json::Value &rsuStatusJson, const RSUMibVersion &mibVersion)
    {
        // Broadcast the RSU status info when there are RSU responses.
        if (!rsuStatusJson.empty() && _rsuWorker)
        {
            auto rsuStatusFields = _rsuWorker->getJsonKeys(rsuStatusJson);
            auto configTbl = _rsuWorker->GetRSUStatusConfig(mibVersion);

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
