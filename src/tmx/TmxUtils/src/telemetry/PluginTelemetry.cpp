#include "PluginTelemetry.h"
namespace tmx::utils::telemetry{
    PluginInfo PluginTelemetry::getPluginInfo(){
        return _pluginInfo;
    }

    PluginInstallation PluginTelemetry::getPluginInstallation(){
        return _pluginInstallation;
    }

    void PluginTelemetry::setPluginInfo(PluginInfo pluginInfo){
        _isPluginInfoSet=true;
        _pluginInfo = pluginInfo;
    }
    
    void PluginTelemetry::setPluginInstallation(PluginInstallation pluginInstallation){
        _isInstallationSet=true;
        _pluginInstallation = pluginInstallation;
    }

    bool PluginTelemetry::isInstallationSet(){
        return _isInstallationSet;
    }

    bool PluginTelemetry::isPluginInfoSet(){
        return _isPluginInfoSet;
    }
}