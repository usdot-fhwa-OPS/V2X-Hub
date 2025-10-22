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
#pragma once
#include <TmxMessageManager.h>
#include <tmx/messages/TmxJ2735.hpp>
#include <tmx/messages/TmxJ2735Codec.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_channel_logger.hpp> // For severity channel logger
#include <string>
#include "MessageLogger.hpp"

namespace JSONMessageLoggerPlugin {
    BOOST_LOG_ATTRIBUTE_KEYWORD(a_channel, "Channel", std::string) // Declare a_channel as a Boost.Log attribute keyword

    class JSONMessageLoggerPlugin : public tmx::utils::TmxMessageManager {
        public:
            explicit JSONMessageLoggerPlugin(const std::string &name);
            void OnStateChange(IvpPluginState state) override;
            void OnConfigChanged(const char *key, const char *value) override;
            void OnMessageReceived(tmx::routeable_message &msg) override;
            void initLogging(unsigned int maxFileSize, unsigned int maxFiles, const std::string &logDir);
            void BroadcastEventLog(const std::string &messageContent);
            // Define desctructor
            ~JSONMessageLoggerPlugin();
        protected:
            void UpdateConfigSettings();

        private:
            std::mutex _configMutex;
            /**
             * Logger to record received J2735 messages in JSON format.
             */
            boost::log::sources::severity_channel_logger< boost::log::trivial::severity_level , std::string> rxLogger;
            /**
             * Logger to record transmitted J2735 messages in JSON format.
             */
            boost::log::sources::severity_channel_logger< boost::log::trivial::severity_level , std::string> txLogger;
            unsigned long _skippedMessages = 0;
            const char* _keySkippedMessages = "Skipped Messages";


    };


} /* namespace JSONMessageLoggerPlugin */