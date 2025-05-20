//==========================================================================
// Name        : PedestrianPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2024 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Pedestrian Plugin
//==========================================================================
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
#include "FLIRWebSockAsyncClnSession.hpp"
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
			 * @brief Overrides PluginClient OnConfigChanged(const char *key, const char *value) method
			 * and calls UpdateConfigSettings() on each configuration change.
			 * @param key string key of the configuration value that has changed.
			 * @param value new value of the configuration that has changed.
			 */
			void OnConfigChanged(const char *key, const char *value) override;
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
			void sendDetections();
			
		private:
			
			std::mutex _cfgLock;
			
			uint16_t webport;
			std::string webip; 
			std::vector<std::shared_ptr<FLIRWebSockAsyncClnSession>> flirSessions;
			std::shared_ptr<FLIRConfigurations> flirConfigsPtr;
			bool runningWebSocket = false;
	};

}
