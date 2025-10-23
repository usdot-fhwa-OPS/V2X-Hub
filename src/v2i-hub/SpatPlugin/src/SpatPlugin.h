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
#include <tmx/messages/SaeJ2735Traits.hpp>

#include "SignalControllerConnection.h"
#include "SpatMode.h"

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
			 * @brief Enumeration describing the expected format of received SPaT data.
			 */
			SPAT_MODE spatMode = SPAT_MODE::UNKNOWN;
			/**
			 * @brief Key for state object describing TSC Connection Status.
			 */
			const char* keyConnectionStatus = "Connection Status";
			/**
			 * @brief Connection status when SPAT Plugin is listening for messages but not receiving any from TSC
			 */
			static inline const std::string CONNECTION_STATUS_IDLE = "IDLE";
			/**
			 * @brief Connection status when SPaT Plugin connection to TSC fails
			 */
			static inline const std::string CONNECTION_STATUS_DISONNECTED = "DISCONNECTED";
			/**
			 * @brief Connection status when SPaT Plugin connection is unhealthy according to CTI 4501, messaging frequency and interval requirements
			 */
			static inline const std::string CONNECTION_STATUS_UNHEALTHY = "UNHEALTHY";
			/**
			 * @brief Connection status when SPaT Plugin connection is healthy according to CTI 4501, messaging frequency and interval requirements
			 */
			static inline const std::string CONNECTION_STATUS_HEALTHY = "HEALTHY";
			/**
			 * @brief Key for counting the number of received packets from TSC that 
			 * have been skipped due to errors.
			 */
			const char* keySkippedMessages = "Skipped Messages";
			/**
			 * @brief Key for keeping track of the maximum recorded interval between two sequential SPaT messages.
			 */
			const char* keySpatMaxInterval = "Max SPaT Interval (ms)(all values above allowable range 300 will apear as 301)";
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
			 * Millisecond timestamp of last SPAT message received by plugin 
			 */
			uint64_t lastSpatTimeMs = 0;
			/**
			 * Largest interval between two SPaT messages received by plugin
			 */
			int maxSpatIntervalMs = 0;
			/**
			 * @brief Helper method used by processTSCPacket to process packet assuming it is in SPAT format
			 */
			void processSpat();
			/**
			 * @brief Method to receive and process any packet received from TSC
			 */
			void processTSCPacket();
			/**
			 * @brief Helper method used by processTSCPacket to process packet assuming it is in TSCBM format
			 */
			void processTSCBM();

			/**
			 * @brief Method to measure interval between received SPaT messages. This 
			 * method will measure the time interval in ms between SPAT messages and
			 * maintain the maximum interval observed. If the interval exceeds 300 ms a
			 * warning event log message will be created.
			 */
			void measureSpatInterval();
	};
} /* namespace SpatPlugin */

