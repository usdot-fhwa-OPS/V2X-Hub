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

        std::string dir = logDir;
        if(!createdLogDirectory(dir)) return;    

        // RX Logger
        boost::log::add_file_log
        (
            boost::log::keywords::file_name = dir + "j2735Rx_%Y-%m-%d.log",
            boost::log::keywords::rotation_size = maxFileSize * 1024 * 1024,
            boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
            boost::log::keywords::format = boost::log::expressions::stream << boost::log::expressions::smessage,
            boost::log::keywords::filter = boost::log::expressions::attr<std::string>("Channel") == "rx",
            boost::log::keywords::auto_flush = true,
            boost::log::keywords::max_files = maxFiles // Set maximum number of log files
        );

        // TX Logger
        boost::log::add_file_log
        (
            boost::log::keywords::file_name = dir + "j2735Tx_%Y-%m-%d.log",
            boost::log::keywords::rotation_size = maxFileSize * 1024 * 1024,
            boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
            boost::log::keywords::format = boost::log::expressions::stream << boost::log::expressions::smessage,
            boost::log::keywords::filter = boost::log::expressions::attr<std::string>("Channel") == "tx",
            boost::log::keywords::auto_flush = true,
            boost::log::keywords::max_files = maxFiles // Set maximum number of log files
        );

        try{
            rxLogger = boost::log::sources::severity_channel_logger< boost::log::trivial::severity_level , std::string>(boost::log::keywords::channel = "rx");
            BOOST_LOG_SEV(rxLogger, boost::log::trivial::info) << "Initialized RX Logger.";
        }catch(const std::exception &e){
            PLOG(tmx::utils::logERROR) << "Exception initializing RX Logger: " << e.what();
        }

        try{
            txLogger = boost::log::sources::severity_channel_logger< boost::log::trivial::severity_level , std::string> (boost::log::keywords::channel = "tx");
            BOOST_LOG_SEV(txLogger, boost::log::trivial::info) << "Initialized TX Logger.";
        }catch(const std::exception &e){
            PLOG(tmx::utils::logERROR) << "Exception initializing TX Logger: " << e.what();
        }
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
        // Cast routeable message as J2735 Message
        if (tmx::utils::PluginClient::IsJ2735Message(routeMsg)) {
            try {
                // Convert routeable message to J2735 encoded message
                tmx::messages::TmxJ2735EncodedMessage<tmx::messages::MessageFrameMessage> rMsg = 
                    routeMsg.get_payload<tmx::messages::TmxJ2735EncodedMessage<tmx::messages::MessageFrameMessage>>();
                // Decode Encode J2735 Message
                auto j2735Data = rMsg.decode_j2735_message().get_j2735_data();
                // Convert J2735 data to TmxJ2735Message for JSON serialization
                auto j2735Message = tmx::messages::TmxJ2735Message<MessageFrame_t, tmx::JSON>(j2735Data);
                // Serial J2735 message to JSON
                std::string json_payload_str = j2735Message.to_string();
                ASN_STRUCT_FREE(asn_DEF_MessageFrame, j2735Data.get());

                // Free the J2735 data structure
                PLOG(tmx::utils::logDEBUG1) << json_payload_str;
           
                if ( routeMsg.get_flags() & IvpMsgFlags_RouteDSRC ) {
                    BOOST_LOG_SEV(txLogger, boost::log::trivial::info) << json_payload_str;
                }
                else {
                    BOOST_LOG_SEV(rxLogger, boost::log::trivial::info) << json_payload_str;
                }
            }
            catch (const boost::exception &e) {
                PLOG(tmx::utils::logERROR) << "Boost exception while logging message: " << boost::diagnostic_information(e);
            }
            catch (const std::exception &e) {
                PLOG(tmx::utils::logERROR) << "Exception while logging message: " << e.what();
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

    bool JSONMessageLoggerPlugin::createdLogDirectory(std::string &dir) const {
        if(dir.empty()) {
            PLOG(tmx::utils::logERROR) << "Log directory path is empty.";
            return false;
        }
        // Ensure directory exists and has a trailing slash
        if (!dir.empty() && dir.back() != '/') dir.push_back('/');
        std::error_code ec;
        auto created = std::filesystem::create_directories(dir, ec);
        if (ec) {
            PLOG(tmx::utils::logERROR) << "Failed to create log dir '" << dir << "': " << ec.message();
            return false;
        }
        if (created) {
            PLOG(tmx::utils::logINFO) << "Created log dir '" << dir << "'";
        }else{
            PLOG(tmx::utils::logDEBUG1) << "Log dir '" << dir << "' already exists";
        }
        return true;
    }
    
} // namespace JSONMessageLoggerPlugin
// The main entry point for this application.
int main(int argc, char *argv[])
{
	return tmx::utils::run_plugin<JSONMessageLoggerPlugin::JSONMessageLoggerPlugin>("JSON Message Logger Plugin", argc, argv);
}