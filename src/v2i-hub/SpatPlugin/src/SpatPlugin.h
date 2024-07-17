/**
 * Copyright (C) 2024 LEIDOS.
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

#include <atomic>
#include <array>
#include <map>
#include <mutex>
#include <vector>
#include <UdpServer.h>
#include <PluginClientClockAware.h>
#include <ThreadTimer.h>
#include <SNMPClient.h>
#include "SignalControllerConnection.h"

namespace SpatPlugin {
	/**
	 * @brief The SPaT Plugin is responsible for receiving information from the Traffic Signal Controller (TSC or SC) necessary
	 * for broadcasting Signal Phase and Timing (SPaT) messages. This includes querying any SNMP objects to determine
	 * TSC state and listen for any broadcast SPaT information from the TSC.
	 */
	class SpatPlugin: public tmx::utils::PluginClientClockAware {

		public:
			/**
			 * @brief Plugin Constructor.
			 * @param name Plugin Name.
			 */
			explicit SpatPlugin(const std::string &name);
			/**
			 * @brief Plugin Destructor
			 */
			virtual ~SpatPlugin();


		protected:
			/**
			 * @brief Method to update plugin after configuration settings have changed.
			 */
			void UpdateConfigSettings();

			// Virtual method overrides.
			void OnConfigChanged(const char *key, const char *value) override;
			void OnStateChange(IvpPluginState state) override;

		private:
			/**
			 * @brief Mutex for thread safety for configuration parameters.
			 */
			std::mutex data_lock;
			/**
			 * @brief Thread timer used to periodically consume broadcast SPaT
			 * data from the TSC .
			 */
			std::unique_ptr<tmx::utils::ThreadTimer> spatReceiverThread;
			/**
			 * @brief TSC Connection.
			 */
			std::unique_ptr<SignalControllerConnection> scConnection;
			/**
			 * @brief String describing the expected format of received SPaT data.
			 */
			std::string spatMode = "";
			/**
			 * @brief Key for state object describing TSC Connection Status.
			 */
			const char* keyConnectionStatus = "Connection Status";
			/**
			 * @brief Key for counting the number of received packets from TSC that 
			 * have been skipped due to errors.
			 */
			const char* keySkippedMessages = "Skipped Messages";
			/**
			 * @brief Count of received packets from the TSC that have been skipped due to
			 * errors.
			 */
			uint skippedMessages = 0;
			/**
			 * @brief Bool flag for TSC connection status.
			 */
			bool isConnected = false;
			/**
			 * @brief Method to receive and process TSC broadcast SPaT data.
			 */
			void processSpat();
	};
} /* namespace SpatPlugin */

