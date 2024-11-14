#pragma once
#include <string>
#include <iostream>
#include <tmx/TmxException.hpp>
#include <tmx/messages/message.hpp>
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
        //Whether the plugin is current enabled or disabled. When a plugin is not installed, enabled default value is set to -1
        int enabled = -1;
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
    public:
        PluginTelemetry()=default;
        PluginInfo getPluginInfo() const;
        PluginInstallation getPluginInstallation() const;
        void setPluginInfo(const PluginInfo& pluginInfo);
        void setPluginInstallation(const PluginInstallation& pluginInstallation);
         /***
         * @brief Serialize PluginTelemetry object into JSON container (tmx::message_container_type)
         * @return A JSON container of tmx::message_container_type
         */
        tmx::message_container_type serialize() const;
        void deserialize(const boost::property_tree::ptree& jsonContainer);
        ~PluginTelemetry()=default;
    };
    
}
