

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
		auto flirSession = std::make_shared<FLIRWebSockAsyncClnSession>(ioc);
		// Launch the asynchronous operation
		flirSession->run(config.socketIp, config.socketPort, config.cameraRotation, config.sensorId, config.apiSubscription);	
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
					// Retrieve the message queue in each session and send each one to be broadcast, then pop.
					std::queue<tmx::messages::SensorDetectedObject> currentMsgQueue = flirSession->getMsgQueue();
					while(!currentMsgQueue.empty())
					{		
						auto message = currentMsgQueue.front();
						PLOG(logDEBUG1) << "Sending Simulated SensorDetectedObject Message " << message;
						this->BroadcastMessage<tmx::messages::SensorDetectedObject>(message, _name, 0 , IvpMsgFlags_None);
						currentMsgQueue.pop();
					}
					flirSession->clearMsgQueue();
				}			
			}
			// Sleep for 10 milliseconds
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}


	void FLIRCameraDriverPlugin::UpdateConfigSettings()
	{

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
			std::thread detectionThread(&FLIRCameraDriverPlugin::sendDetections, this);
			PLOG(logDEBUG) << "XML thread started!!";
			

			// wait for all the socket threads to finish
			for(auto &thread: socketThreads){
				thread.detach();
			}
			detectionThread.detach(); // wait for the thread to finish
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
