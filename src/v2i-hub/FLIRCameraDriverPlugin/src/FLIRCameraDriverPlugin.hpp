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
#include <string>

#include <PluginClientClockAware.h>

#include <atomic>
#include <thread>
#include <mutex>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <queue>
#include "FLIRWebsockAsyncClnSession.hpp"
#include "FLIRConfigurations.hpp"


namespace FLIRCameraDriverPlugin
{

	/**
	 * @brief Plugin used to encode messages from an XML input via HTTP POST or metadata received from the FLIR API.
	 */
	class FLIRCameraDriverPlugin: public tmx::utils::PluginClientClockAware
	{
		public:
			explicit FLIRCameraDriverPlugin(const std::string &name);

		protected:
			/**
			 * @brief Called everytime a configuration value is changed for the plugin.
			 */
			void UpdateConfigSettings();
			/**
			 * @brief Overrides PluginClient OnStateChange(IvpPluginState state) method.
			 * @param state new state of the plugin.
			 */
			void OnStateChange(IvpPluginState state) override;
			/**
			 * @brief Starts Asyncronous WebSocket Client to connect to FLIR WebSocket Server.
			 */
			int  StartWebSocket(const FLIRConfiguration &config);
			/**
			 * @brief Stops WebSocket Client session before HTTP POST WebService is enabled. 
			 */
			void StopWebSocket();
			/**
			 * @brief Loops over Websocket FLIR connections, checks for messages in the queue and sends them on the TMX message bus.
			 */
			__attribute__((noreturn)) void sendDetections();
			/**
			 * @brief Applies the link monitoring and link timeout settings to a session.
			 */
			void applyLinkSettings(FLIRWebsockAsyncClnSession &flirSession) const;
			/**
			 * @brief Copy of the session list, safe to iterate while WebSocket threads add sessions.
			 */
			std::vector<std::shared_ptr<FLIRWebsockAsyncClnSession>> getSessions();
			
		private:
			
			
			std::vector<std::shared_ptr<FLIRWebsockAsyncClnSession>> flirSessions;
			// Guards flirSessions, which the WebSocket threads add to while other threads read it
			std::mutex flirSessionsMutex;
			std::shared_ptr<FLIRConfigurations> flirConfigsPtr;
			bool runningWebSocket = false;
			// Status data and keys
			const char* Key_DroppedPedestrianCount = "Dropped Pedestrian Detections";
			// Total number of dropped pedestrian detections
			uint droppedPedCount = 0;
			const char* Key_ModifiedPedestrianCount = "Modified Pedestrian Detections (Bad Data: Assumed Stationary)";
			// Total number of modified pedestrian detections
			uint modifiedPedCount = 0;
			const char* Key_UniquePedestrianCount = "Unique Pedestrian Detections";
			// Total number of unique pedestrian detections
			unsigned int uniquePedCount = 0;
			// Vector to store unique pedestrian IDs
			std::unordered_set<int> uniquePedestrianIds;
			const char* Key_TotalPedestrianCount = "Total Pedestrian Detections";
			// Total number of pedestrian detections
			unsigned int totalPedCount = 0;
			// Live number of currently detected pedestrians
			const char* Key_LivePedestrianCount = "Live number of currently detected pedestrians";

			// Camera link monitoring status keys (totals since the camera connections started)
			const char* Key_LinkStallCount = "Camera Link Stalls";
			const char* Key_LinkMaxStall = "Camera Link Longest Stall (ms)";
			const char* Key_LinkLastStall = "Camera Link Last Stall (UTC)";
			const char* Key_LinkLateFrames = "Camera Link Late Frames";
			const char* Key_LinkMinorHolds = "Camera Link Minor Holds";
			const char* Key_CameraMissingFrames = "Camera Missing Frames";
			const char* Key_LinkReconnects = "Camera Link Reconnects";
			const char* Key_LinksConnected = "Camera Links Connected";
			// Extra transit time (ms) at which a camera frame counts as a link stall. Set by LinkStallThresholdMs.
			std::atomic<int64_t> linkStallThresholdMs{500};
			// Link watchdog timeout (ms), 0 disables it. Set by LinkTimeoutMs.
			std::atomic<int64_t> linkTimeoutMs{500};


	};

}
