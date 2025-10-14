#include "RSUHealthMonitorPlugin.h"

using namespace RSUHealthMonitor;
using namespace tmx::utils;

namespace RSUHealthMonitor
{

    RSUHealthMonitorPlugin::RSUHealthMonitorPlugin(const std::string &name) : PluginClient(name)
    {
        _rsuWorker = std::make_shared<RSUHealthMonitorWorker>();
        _rsuStatusTimer = std::make_unique<ThreadTimer>();
        _rsuConfigListPtr = std::make_shared<RSUConfigurationList>();
        
    }

    void RSUHealthMonitorPlugin::monitorRSUs()
    {
        for (auto rsuConfig : _rsuConfigListPtr->getConfigs())
        {   
            std::string statusKey =  _keyRSUConnectedPrefix +rsuConfig.rsuIp;
            if (std::find(_rsuConnectedStatusKeys.begin(), _rsuConnectedStatusKeys.end(), statusKey) == _rsuConnectedStatusKeys.end())
            {
                _rsuConnectedStatusKeys.push_back(statusKey);
            }
            try {
                auto rsuStatusJson = _rsuWorker->getRSUStatus(rsuConfig.mibVersion, rsuConfig.rsuIp, rsuConfig.snmpPort, rsuConfig.user, rsuConfig.authProtocol, rsuConfig.authPassPhrase, rsuConfig.privProtocol, rsuConfig.privPassPhrase, rsuConfig.securityLevel, SEC_TO_MICRO);
                BroadcastRSUStatus(rsuStatusJson, rsuConfig.mibVersion);
                SetStatus<std::string>(statusKey.c_str(), CONNECTED);

            }
            catch (const std::exception &ex)
            {
                PLOG(logERROR) << "Failed to CONNECT to at RSU IP: " << rsuConfig.rsuIp << " due to error: " << ex.what();
                SetStatus<std::string>(statusKey.c_str(), DISCONNECTED);
                // Create TmxEventLogMessage for RSU disconnection
                tmx::messages::TmxEventLogMessage eventLogMsg;
                eventLogMsg.set_level(IvpLogLevel::IvpLogLevel_error);
                eventLogMsg.set_description("Failed to CONNECT to at RSU IP: " + rsuConfig.rsuIp + " due to error: " + ex.what());
                BroadcastMessage(eventLogMsg, RSUHealthMonitorPlugin::GetName());

            }
        }
    }

    void RSUHealthMonitorPlugin::UpdateConfigSettings()
    {
        // Clear previous plugin status in database
        PLOG(logDEBUG) << "Clearing previous Plugin status.";
        for (const auto &rsuConnectStatusKey : _rsuConnectedStatusKeys)
        {
            RemoveStatus(rsuConnectStatusKey.c_str());
        }
        _rsuConnectedStatusKeys.clear();

        PLOG(logINFO) << "Updating configuration settings.";

        lock_guard<mutex> lock(_configMutex);
        GetConfigValue<uint16_t>("Interval", _interval);
        GetConfigValue<string>("RSUConfigurationList", _rsuConfigListStr);
        
        PLOG(logDEBUG) << "RSU Configuration " << _rsuConfigListStr;
        _rsuConfigListPtr->parseRSUs(_rsuConfigListStr);
        try
        {
            if ( !started ) {       
                // Send SNMP call to RSU periodically at configurable interval.
                _timerThId = _rsuStatusTimer->AddPeriodicTick([this]()
                                                            {
                                this->monitorRSUs();
                                PLOG(logINFO) << "Monitoring RSU at interval (second): " << _interval; },
                                                            std::chrono::milliseconds(_interval * SEC_TO_MILLI));
                PLOG(logDEBUG1) << "RSU Health Monitor timer thread ID: " << _timerThId;
                _rsuStatusTimer->Start();
                started = true;
            }
            else  {
                PLOG(logDEBUG1) << "Updating Health Monitor timer frequency for thread ID: " << _timerThId;
              
                _rsuStatusTimer->ChangeFrequency(_timerThId, std::chrono::milliseconds(_interval * SEC_TO_MILLI));
            }
        }
        catch (const RSUConfigurationException &ex)
        {
            PLOG(logERROR) << "Cannot update RSU configurations due to error: " << ex.what();
        }
        catch (const tmx::TmxException &ex)
        {
            PLOG(logERROR) << ex.what();
        }
    }

    void RSUHealthMonitorPlugin::OnConfigChanged(const char *key, const char *value)
    {
        PluginClient::OnConfigChanged(key, value);
        // If state is registered, then update the config settings.
        UpdateConfigSettings();
    }

    void RSUHealthMonitorPlugin::BroadcastRSUStatus(const Json::Value &rsuStatusJson, const tmx::utils::rsu::RSU_SPEC &mibVersion)
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
