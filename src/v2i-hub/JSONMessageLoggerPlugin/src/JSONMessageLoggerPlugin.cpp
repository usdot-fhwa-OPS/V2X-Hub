/**
 * Copyright (C) 2025 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
#include "JSONMessageLoggerPlugin.hpp"

namespace JSONMessageLoggerPlugin {
    JSONMessageLoggerPlugin::JSONMessageLoggerPlugin(const std::string &name) : tmx::utils::TmxMessageManager(name)
    {
        AddMessageFilter("*", "*", IvpMsgFlags_None);
        AddMessageFilter("J2735", "*", IvpMsgFlags_RouteDSRC);
        SubscribeToMessages();

    }

    void JSONMessageLoggerPlugin::initLogging(unsigned int maxFileSize, unsigned int maxFiles, const std::string &logDir)
    {
         // Common attributes (timestamp, etc.)
        boost::log::add_common_attributes();

        // RX Logger
        boost::log::add_file_log
        (
            boost::log::keywords::file_name = logDir + "j2735Rx_%Y-%m-%d.log",
            boost::log::keywords::rotation_size = maxFileSize * 1024 * 1024,
            boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
            boost::log::keywords::format = boost::log::expressions::stream << boost::log::expressions::smessage,
            boost::log::keywords::filter = a_channel == "rx", // Filter for "rx" channel messages
            boost::log::keywords::max_files = maxFiles // Set maximum number of log files
        );

        // TX Logger
        boost::log::add_file_log
        (
            boost::log::keywords::file_name = logDir+"j2735Tx_%Y-%m-%d.log",
            boost::log::keywords::rotation_size = maxFileSize * 1024 * 1024,
            boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
            boost::log::keywords::format = boost::log::expressions::stream << boost::log::expressions::smessage,
            boost::log::keywords::filter = a_channel == "tx", // Filter for "tx" channel messages
            boost::log::keywords::max_files = maxFiles // Set maximum number of log files

        );
        rxLogger = boost::log::sources::severity_channel_logger< boost::log::trivial::severity_level , std::string>(boost::log::keywords::channel = "rx");
        txLogger = boost::log::sources::severity_channel_logger< boost::log::trivial::severity_level , std::string> (boost::log::keywords::channel = "tx");
    }

    void JSONMessageLoggerPlugin::OnStateChange(IvpPluginState state)
    {
        tmx::utils::TmxMessageManager::OnStateChange(state);
		if (state == IvpPluginState_registered) {
			UpdateConfigSettings();
		}
    }

    void JSONMessageLoggerPlugin::OnConfigChanged(const char *key, const char *value)
    {
        tmx::utils::TmxMessageManager::OnConfigChanged(key, value);
		UpdateConfigSettings();
    }
    void JSONMessageLoggerPlugin::OnMessageReceived(IvpMessage *msg)
    {
        tmx::utils::TmxMessageManager::OnMessageReceived(msg);
        tmx::routeable_message routeMsg(msg);
        PLOG(tmx::utils::logDEBUG1) << "Routable Message " << routeMsg.to_string();
        // Cast routeable message as J2735 Message
        if (tmx::utils::PluginClient::IsJ2735Message(routeMsg)) {
            if (routeMsg.get_flags() & IvpMsgFlags_RouteDSRC) {
                PLOG(tmx::utils::logDEBUG1) << "Logging TX J2735 Message";
                logRouteableMessage(routeMsg, txLogger);

            }
            else {
                PLOG(tmx::utils::logDEBUG1) << "Logging RX J2735 Message";
                logRouteableMessage(routeMsg, rxLogger);
            }
        }  
    }

    void JSONMessageLoggerPlugin::UpdateConfigSettings()
    {
        unsigned int maxFileSize = 0;
        unsigned int maxFiles = 0;
        std::string logDir;
        GetConfigValue<unsigned int>("MaxFileSize",maxFileSize, &_configMutex);
        GetConfigValue<unsigned int>("MaxFiles",maxFiles, &_configMutex);
        GetConfigValue<std::string>("LogDir", logDir, &_configMutex);
        initLogging(maxFileSize, maxFiles, logDir);
    }
    
} // namespace JSONMessageLoggerPlugin
// The main entry point for this application.
int main(int argc, char *argv[])
{
	return tmx::utils::run_plugin<JSONMessageLoggerPlugin::JSONMessageLoggerPlugin>("JSON Message Logger Plugin", argc, argv);
}