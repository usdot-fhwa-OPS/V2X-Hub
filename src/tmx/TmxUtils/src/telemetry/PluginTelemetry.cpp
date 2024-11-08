#include "PluginTelemetry.h"
namespace tmx::utils::telemetry{
    PluginInfo PluginTelemetry::getPluginInfo() const{
        return _pluginInfo;
    }

    PluginInstallation PluginTelemetry::getPluginInstallation() const{
        return _pluginInstallation;
    }

    void PluginTelemetry::setPluginInfo(PluginInfo pluginInfo){
        _pluginInfo = pluginInfo;
    }
    
    void PluginTelemetry::setPluginInstallation(PluginInstallation pluginInstallation){
        _pluginInstallation = pluginInstallation;
    }

    
    ostream& operator <<(ostream& os, const PluginTelemetry& telemetry){
        os << "id="<<telemetry._pluginInfo.id
        <<",name="<<telemetry._pluginInfo.name
        <<",description="<<telemetry._pluginInfo.description
        <<",version="<<telemetry._pluginInfo.version
        <<",enabled="<<telemetry._pluginInstallation.enabled
        <<",exeName="<<telemetry._pluginInstallation.exeName
        <<",manifest="<<telemetry._pluginInstallation.manifest
        <<",path="<<telemetry._pluginInstallation.path
        <<",commandLineParameters="<<telemetry._pluginInstallation.commandLineParameters
        <<",maxMessageInterval="<<telemetry._pluginInstallation.maxMessageInterval;
        return os;
    }
}