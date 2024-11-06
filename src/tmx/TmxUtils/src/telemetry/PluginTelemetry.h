#pragma once
#include <string>
using namespace std;

namespace tmx::utils::telemetry
{
    struct PluginInfo
    {
        string id;
        //Brief name for a plugin
        string name; 
        //Detailed description of the plugin and its usage
        string description;
        //Latest version of the plugin
        string version;
    };

    struct PluginInstallation
    {  
        //Whether the plugin is current enabled or disabled
        string enabled;
        //Where the plugin executable is located
        string path;
        //The plugin executable name
        string exeName;
        //The manifest file that contains some configuration parameters
        string manifest;
        //Message broadcast interval
        string maxMessageInterval;
        string commandLineParameters;
    };

    class PluginTelemetry
    {
    private:
        PluginInfo _pluginInfo;
        PluginInstallation _pluginInstallation;
        bool _isInstallationSet;
        bool _isPluginInfoSet;
    public:
        PluginTelemetry()=default;
        PluginInfo getPluginInfo();
        PluginInstallation getPluginInstallation();
        void setPluginInfo(PluginInfo pluginInfo);
        void setPluginInstallation(PluginInstallation pluginInstallation);
        bool isInstallationSet();
        bool isPluginInfoSet();
        ~PluginTelemetry()=default;
    };
    
}
