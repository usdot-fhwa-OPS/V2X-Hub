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
    JSONMessageLoggerPlugin::~JSONMessageLoggerPlugin()
    {
         // Cleanup code if needed
        boost::log::core::get()->flush();
        boost::log::core::get()->remove_all_sinks();
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
        // Tmx Message Manager OnStateChange will call Start() when entering the registered state
        tmx::utils::TmxMessageManager::OnStateChange(state);
		if (state == IvpPluginState_registered) {
			UpdateConfigSettings();
		}
    }

    void JSONMessageLoggerPlugin::OnConfigChanged(const char *key, const char *value)
    {
        // Tmx Message Manager OnConfigChanged will call Start() if TmxMessageManager related configs like NUMBER_WORKER_THREADS_CFG are changed
        tmx::utils::TmxMessageManager::OnConfigChanged(key, value);
        // Reset BSM Count on config update
        PLOG(tmx::utils::logWARNING) << "Message count before resetting JsonMessageLogger: "<< _bsmCount;
        _bsmCount = 0;

		UpdateConfigSettings();
    }
    void JSONMessageLoggerPlugin::OnMessageReceived(tmx::routeable_message &msg)
    {
        PLOG(tmx::utils::logDEBUG1) << "Routable Message " << msg.to_string();
        // Cast routeable message as J2735 Message
        if (tmx::utils::PluginClient::IsJ2735Message(msg)) {
            try {
                if (msg.get_flags() & IvpMsgFlags_RouteDSRC) {
                    PLOG(tmx::utils::logDEBUG1) << "Logging TX J2735 Message";
                    logRouteableMessage(msg, txLogger);

                }
                else {
                    PLOG(tmx::utils::logDEBUG1) << "Logging RX J2735 Message";

                    _bsmCount++;
					PLOG(tmx::utils::logINFO) << "Received BSM Message count JsonMessageLogger: "<< _bsmCount;

                    logRouteableMessage(msg, rxLogger);
                }
            }
            catch (const boost::exception &e) {
                std::string errorMessage = "Boost exception while logging message: " + boost::diagnostic_information(e);
                FILE_LOG(tmx::utils::logERROR) << errorMessage;
                tmx::messages::TmxEventLogMessage eventLogMsg;
                eventLogMsg.set_level(IvpLogLevel::IvpLogLevel_error);
                eventLogMsg.set_description(errorMessage);
                BroadcastMessage(eventLogMsg, JSONMessageLoggerPlugin::GetName());
                _skippedMessages++;
                SetStatus<unsigned long>(_keySkippedMessages, _skippedMessages);
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
	return tmx::utils::run_plugin<JSONMessageLoggerPlugin::JSONMessageLoggerPlugin>("JSON Message Logger", argc, argv);
}