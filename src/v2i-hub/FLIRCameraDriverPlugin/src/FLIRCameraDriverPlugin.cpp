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
#include "FLIRCameraDriverPlugin.hpp"

using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace FLIRCameraDriverPlugin
{

	/**
	 * @brief Construct a new FLIRCameraDriverPlugin with the given name.
	 * @param name The name to give the plugin for identification purposes.
	 */
	FLIRCameraDriverPlugin::FLIRCameraDriverPlugin(const std::string &name) : PluginClientClockAware(name)
	{
		flirConfigsPtr = std::make_shared<FLIRConfigurations>();
	}



	int FLIRCameraDriverPlugin::StartWebSocket(const FLIRConfiguration & config)
	{
		PLOG(logDEBUG) << "In FLIRCameraDriverPlugin::StartWebSocket ";
		// The io_context is required for all I/O
		net::io_context ioc;

		// Create a session and run it
		auto flirSession = std::make_shared<FLIRWebsockAsyncClnSession>(
			ioc, 
			config.socketIp, 
			config.socketPort, 
			config.cameraRotation, 
			config.sensorId, 
			config.apiSubscription,
			config.cameraRefPoint);
		// Launch the asynchronous operation
		flirSession->run();	
		flirSessions.push_back(flirSession);
		PLOG(logDEBUG) << "Successfully running the I/O service.";	
		runningWebSocket = true;

		// Run the I/O service. The call will return when the socket is closed.
		ioc.run();
		PLOG(logDEBUG) << "Successfully terminating the I/O service.";	

		return EXIT_SUCCESS;
	}

	void FLIRCameraDriverPlugin::StopWebSocket()
	{
		if (!flirSessions.empty() && runningWebSocket)
		{
			PLOG(logDEBUG) << "Stopping WebSocket session";
			beast::error_code ec;
			for(const auto &flirSession: flirSessions){
				flirSession->on_close(ec);
			}        
			runningWebSocket = false;
		}
		else
		{
			PLOG(logDEBUG) << "WebSocket session was not running or already stopped.";
		}
	}

	__attribute__((noreturn)) void FLIRCameraDriverPlugin::sendDetections()
	{
		while (true)
		{
			if (flirSessions.empty())
			{
				PLOG(logDEBUG) << "FLIR session not yet initialized: ";
			}
			else
			{	
				//Loop through all FLIR sessions to check each session for any messages in the queue and broadcast them.
				for(const auto &flirSession: flirSessions)
				{
					// Check for dropped pedestrians and add it to status count
					if ( flirSession->getDroppedPedCount()> 0) {
						droppedPedCount += flirSession->getDroppedPedCount();
						flirSession->clearDroppedPedCount();
					}
					// A set used to store incoming pedestrian IDs.This set is used to 
					// update the uniquePedestrianIds. By comparing incoming ids to previously
					// incoming ids (uniquePedestrianIds), we can determine if a new pedestrian has been detected.
					// If a new pedestrian is detected, we increment the uniquePedCount.
					std::unordered_set<int> incomingPedestrianIds;
					// Retrieve the message queue in each session and send each one to be broadcast, then pop.
					std::queue<tmx::messages::SensorDetectedObject> currentMsgQueue = flirSession->getMsgQueue();
					// Update total pedestrian detection count.
					totalPedCount += currentMsgQueue.size();
					while(!currentMsgQueue.empty())
					{		
						auto message = currentMsgQueue.front();
						// Add ID of the detected pedestrian to the incomingPedestrianIds set.
						incomingPedestrianIds.insert(message.get_objectId());
						// If pedestrian ID is not in current uniquePedestrianIds, it is a new pedestrian detection.
						// Increment uniquePedCount
						if ( uniquePedestrianIds.find(message.get_objectId()) == uniquePedestrianIds.end() )
						{
							PLOG(logDEBUG1) << "New pedestrian detected with ID: " << message.get_objectId();
							uniquePedCount++;
						}
						
						PLOG(logDEBUG1) << "Sending Simulated SensorDetectedObject Message " << message;
						this->BroadcastMessage<tmx::messages::SensorDetectedObject>(message, _name, 0 , IvpMsgFlags_None);
						if (message.get_isModified()) {
							modifiedPedCount++;
						}
						currentMsgQueue.pop();
					}
					// Processed clear msg queue
					flirSession->clearMsgQueue();
					// Set the uniquePedestrianIds to the incomingPedestrianIds
					if (!incomingPedestrianIds.empty()) {
						uniquePedestrianIds = incomingPedestrianIds;
					}
					
				}
				SetStatus<uint>(Key_DroppedPedestrianCount, droppedPedCount);
				SetStatus<uint>(Key_ModifiedPedestrianCount, modifiedPedCount);
				SetStatus<uint>(Key_UniquePedestrianCount, uniquePedCount);
				SetStatus<uint>(Key_TotalPedestrianCount, totalPedCount);
			
			}
			// Sleep for 10 milliseconds
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}


	void FLIRCameraDriverPlugin::UpdateConfigSettings()
	{
		PLOG(logINFO) << "Resetting FLIR Camera Driver Status.";
		// Resetting all status data.
		droppedPedCount = 0;
		modifiedPedCount = 0;
		uniquePedCount = 0;
		totalPedCount = 0;
		SetStatus<uint>(Key_DroppedPedestrianCount, droppedPedCount);
		SetStatus<uint>(Key_ModifiedPedestrianCount, modifiedPedCount);
		SetStatus<uint>(Key_UniquePedestrianCount, uniquePedCount);
		SetStatus<uint>(Key_TotalPedestrianCount, totalPedCount);
		std::string flirConfigsStr;
		GetConfigValue<std::string>("FLIRConfigurations", flirConfigsStr, &_cfgLock);
		flirConfigsPtr->parseFLIRConfigs(flirConfigsStr);		

		if (!runningWebSocket)
		{
			std::vector<std::thread> socketThreads;
			for(const auto & config: flirConfigsPtr->getConfigs()){
				socketThreads.emplace_back(&FLIRCameraDriverPlugin::StartWebSocket, this, config);
				PLOG(logDEBUG) << "Starting WebSocket session for camera: " << config.sensorId << " at " << config.socketIp << ":" << config.socketPort;
			}			

			// Starting thread to consume detections from FLIR Camera Web Socket sessions and 
			// broadcast them to TMX core.
			std::thread detectionThread(&FLIRCameraDriverPlugin::sendDetections, this);
			PLOG(logDEBUG) << "Starting detection thread to send messages to TMX core.";

			// Detaching all threads to prevent plugin configuration thread from blocking causing 
			// configuration changes to fail.
			for(auto &thread: socketThreads){
				thread.detach();
			}
			detectionThread.detach(); 
		}
	

	}

	void FLIRCameraDriverPlugin::OnStateChange(IvpPluginState state)
	{
		PluginClientClockAware::OnStateChange(state);

		if (state == IvpPluginState_registered)
		{
			UpdateConfigSettings();
		}
	}


} /* namespace FLIRCameraDriverPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<FLIRCameraDriverPlugin::FLIRCameraDriverPlugin>("FLIRCameraDriverPlugin", argc, argv);
}
