//==========================================================================
// Name        : PedestrianPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2024 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Pedestrian Plugin
//==========================================================================

#include "include/FLIRCameraDriverPlugin.hpp"

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
	auto flirSession = std::make_shared<FLIRWebSockAsyncClnSession>(ioc);
    // Launch the asynchronous operation
	flirSession->run(config.socketIp.c_str(), config.socketPort.c_str(), config.FLIRCameraRotation, config.FLIRCameraViewName.c_str(), config.apiSubscription.c_str());	
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
		for(auto& flirSession: flirSessions){
			flirSession->on_close(ec);
		}        
        runningWebSocket = false;
    }
    else
    {
        PLOG(logDEBUG) << "WebSocket session was not running or already stopped.";
    }
}

void FLIRCameraDriverPlugin::checkXML()
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
			for(auto flirSession: flirSessions)
			{
				// Retrieve the message queue in each session and send each one to be broadcast, then pop.
				std::queue<tmx::messages::SensorDetectedObject> currentMsgQueue = flirSession->getMsgQueue();
				while(!currentMsgQueue.empty())
				{		
					auto message = currentMsgQueue.front();
					PLOG(logDEBUG1) << "Sending Simulated SensorDetectedObject Message " << message;
					this->BroadcastMessage<tmx::messages::SensorDetectedObject>(message, _name, 0 , IvpMsgFlags_None);
					currentMsgQueue.pop();
				}
			}			
		}
		// Sleep for 10 milliseconds
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}


void FLIRCameraDriverPlugin::UpdateConfigSettings()
{
	// Configuration settings are retrieved from the API using the GetConfigValue template class.
	// This method does NOT execute in the main thread, so variables must be protected
	// (e.g. using std::atomic, std::mutex, etc.).

	GetConfigValue<std::string>("WebServiceIP", webip, &_cfgLock);
	GetConfigValue<uint16_t>("WebServicePort", webport, &_cfgLock);
	GetConfigValue<std::string>("FLIROutput", flirOutput, &_cfgLock);

	std::string flirConfigsStr;
	GetConfigValue<std::string>("FLIRConfigurations", flirConfigsStr, &_cfgLock);
	flirConfigsPtr->parseFLIRConfigs(flirConfigsStr);

	
	

	if (!runningWebSocket)
	{
		PLOG(logDEBUG) << "Starting WebSocket Thread";
		std::vector<std::thread> socketThreads;
		for(const auto & config: flirConfigsPtr->getConfigs()){
			socketThreads.emplace_back(&FLIRCameraDriverPlugin::StartWebSocket, this, config);
		}			
		PLOG(logDEBUG) << "WebSocket thread started!!";

		PLOG(logDEBUG) << "Starting XML thread";
		//This thread is to check flirsession for any SDSM and PSM messages in the queue and broadcast them.
		std::thread xmlThread(&FLIRCameraDriverPlugin::checkXML, this);
		PLOG(logDEBUG) << "XML thread started!!";
		

		// wait for all the socket threads to finish
		for(auto &thread: socketThreads){
			thread.join();
		}
		xmlThread.join(); // wait for the thread to finish
	}
   

}

void FLIRCameraDriverPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClientClockAware::OnConfigChanged(key, value);
}

void FLIRCameraDriverPlugin::OnStateChange(IvpPluginState state)
{
	PluginClientClockAware::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
	}
}


int FLIRCameraDriverPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	while (_plugin->state != IvpPluginState_error)
	{
		if (_plugin->state == IvpPluginState_registered)
		{
			this_thread::sleep_for(chrono::milliseconds(100));
		}
	}

	PLOG(logINFO) << "Plugin terminating gracefully.";
	return EXIT_SUCCESS;
}

} /* namespace FLIRCameraDriverPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<FLIRCameraDriverPlugin::FLIRCameraDriverPlugin>("FLIRCameraDriverPlugin", argc, argv);
}
