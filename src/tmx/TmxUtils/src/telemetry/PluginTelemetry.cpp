#include "PluginTelemetry.h"
namespace tmx::utils::telemetry{
    PluginInfo PluginTelemetry::getPluginInfo(){
        return _pluginInfo;
    }

    PluginInstallation PluginTelemetry::getPluginInstallation(){
        return _pluginInstallation;
    }
}