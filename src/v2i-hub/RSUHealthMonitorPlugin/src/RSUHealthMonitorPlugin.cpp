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
        
        // Subscribe to RSURegistrationConfigMessage
        AddMessageFilter<tmx::messages::RSURegistrationConfigMessage>(this, &RSUHealthMonitorPlugin::OnRSURegistrationConfigMessage);
        
        SubscribeToMessages();
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
                auto rsuStatusJson = _rsuWorker->getRSUStatus(rsuConfig.mibVersion, rsuConfig.rsuIp, rsuConfig.snmpPort, rsuConfig.user, rsuConfig.authProtocol, rsuConfig.authPassPhrase, rsuConfig.privProtocol, rsuConfig.privPassPhrase, rsuConfig.securityLevel, rsuConfig.event, SEC_TO_MICRO);
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

    void RSUHealthMonitorPlugin::OnRSURegistrationConfigMessage(tmx::messages::RSURegistrationConfigMessage &msg, routeable_message &routeableMsg)
    {
        PLOG(logINFO) << "Received RSURegistrationConfigMessage: " << msg.to_string();
        
        lock_guard<mutex> lock(_configMutex);
        
        // Update the monitor interval from the message
        uint16_t newInterval = msg.get_unitConfig().rsuStatusMonitorInterval;
        if (newInterval > 0 && newInterval != _interval)
        {
            _interval = newInterval;
            PLOG(logINFO) << "Updated RSU status monitor interval to: " << _interval << " seconds";
            
            if (started && _rsuStatusTimer)
            {
                PLOG(logINFO) << "Updating Health Monitor timer frequency for thread ID: " << _timerThId;
                _rsuStatusTimer->ChangeFrequency(_timerThId, std::chrono::milliseconds(_interval * SEC_TO_MILLI));
            }
        }
        
        // Clear previous plugin status in database
        PLOG(logDEBUG) << "Clearing previous RSU connection statuses.";
        for (const auto &rsuConnectStatusKey : _rsuConnectedStatusKeys)
        {
            RemoveStatus(rsuConnectStatusKey.c_str());
        }
        _rsuConnectedStatusKeys.clear();
        
        // Purge existing configuration and replace with new RSU configs from message
        PLOG(logINFO) << "Updating RSU configurations from RSURegistrationConfigMessage";
        _rsuConfigListPtr->clearConfigs();
        
        try
        {
            // Convert TMX RSU configs to internal RSUConfiguration format
            for (const auto &rsuConfig : msg.get_rsuConfigs().rsuConfigs)
            {
                RSUConfiguration config;
                config.rsuIp = rsuConfig.rsu.ip;
                config.snmpPort = rsuConfig.rsu.port;
                config.user = rsuConfig.snmp.user;
                config.authProtocol = rsuConfig.snmp.authProtocol;
                config.privProtocol = rsuConfig.snmp.privacyProtocol;
                config.authPassPhrase = rsuConfig.snmp.authPassPhrase;
                config.privPassPhrase = rsuConfig.snmp.privacyPassPhrase;
                config.securityLevel = rsuConfig.snmp.securityLevel;
                config.mibVersion = tmx::utils::rsu::stringToRSUSpec(rsuConfig.snmp.rsuMIBVersion);
                config.event = rsuConfig.event;
                
                _rsuConfigListPtr->addConfig(config);
                PLOG(logDEBUG) << "Added RSU config: " << config.rsuIp << ":" << config.snmpPort;
            }
            
            PLOG(logINFO) << "Successfully updated RSU configurations. Total RSUs: " << _rsuConfigListPtr->getConfigs().size();
        }
        catch (const std::exception &ex)
        {
            PLOG(logERROR) << "Error processing RSU configurations from message: " << ex.what();
        }
    }

} // namespace RSUHealthMonitor

int main(int argc, char *argv[])
{
    return run_plugin<RSUHealthMonitorPlugin>("RSU Health Monitor", argc, argv);
}
