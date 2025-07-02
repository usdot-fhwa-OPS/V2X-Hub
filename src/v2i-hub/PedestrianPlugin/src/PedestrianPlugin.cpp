//==========================================================================
// Name        : PedestrianPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2024 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Pedestrian Plugin
//==========================================================================

#include "include/PedestrianPlugin.hpp"

using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;
using namespace OpenAPI;

namespace PedestrianPlugin
{

/**
 * @brief Construct a new PedestrianPlugin with the given name.
 * @param name The name to give the plugin for identification purposes.
 */
PedestrianPlugin::PedestrianPlugin(const std::string &name) : PluginClient(name)
{
	flirConfigsPtr = std::make_shared<FLIRConfigurations>();
}

void PedestrianPlugin::getMessageToWrite()
{
    std::unordered_set<std::string> parsedValues;
    std::stringstream ss(flirOutput);
    std::string token;

    while (std::getline(ss, token, ',')) 
	{
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        parsedValues.insert(token);
    }

    // Set the flags based on parsed values
    generatePSM = parsedValues.count("PSM") > 0;
    generateTIM = parsedValues.count("TIM") > 0;
    generateSDSM = parsedValues.count("SDSM") > 0;
}

int PedestrianPlugin::StartWebSocket(const FLIRConfiguration & config)
{
	PLOG(logDEBUG) << "In PedestrianPlugin::StartWebSocket ";
	// The io_context is required for all I/O
	net::io_context ioc;

	// Create a session and run it
	auto flirSession = std::make_shared<FLIRWebSockAsyncClnSession>(ioc);
    // Launch the asynchronous operation
	flirSession->run(config.socketIp.c_str(), config.socketPort.c_str(), config.FLIRCameraRotation, config.FLIRCameraViewName.c_str(), config.apiSubscription.c_str(), generatePSM, generateSDSM, generateTIM);	
	flirSessions.push_back(flirSession);
	PLOG(logDEBUG) << "Successfully running the I/O service.";	
    runningWebSocket = true;

    // Run the I/O service. The call will return when the socket is closed.
    ioc.run();
	PLOG(logDEBUG) << "Successfully terminating the I/O service.";	

    return EXIT_SUCCESS;
}

void PedestrianPlugin::StopWebSocket()
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

void PedestrianPlugin::checkXML()
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
				std::queue<std::string> currentMsgQueue = flirSession->getMsgQueue();
				while(!currentMsgQueue.empty())
				{		
					const char* char_arr = &currentMsgQueue.front()[0];
					BroadcastPedDet(char_arr);
					currentMsgQueue.pop();
				}
			}			
		}
		// Sleep for 10 milliseconds
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void PedestrianPlugin::processStaticTimXML()
{
	TIMVariables previousTIMVars;
	int16_t msgCount = 0;
	auto period = static_cast<int>(1.0 / staticTimFrequency * 1000.0);
	while(true)
	{		
		if (flirSessions.empty())
		{
			PLOG(logDEBUG) << "FLIR session not yet initialized.";
		}
		else
		{	
			auto isAnyPedestrainPresent = false;
			auto isAnyFLIRSessionHealthy = false;

			for(auto flirSession: flirSessions)
			{
				//Check if any of the FLIR sessions are healthy.
				isAnyFLIRSessionHealthy |= flirSession->isHealthy();

				//Check if any pedestrain is detected at any of the intersection regions/views.
				auto isPedestrainPresent = flirSession->isPedestrainPresent();
				isAnyPedestrainPresent |= isPedestrainPresent;
				if(isPedestrainPresent)
				{
					PLOG(logINFO) << "At least one pedestrain detected at one of the intersection regions/views! Reset the pedestrain presence flag to false after checking it.";
					//Reset the pedestrain presence flag to false after checking it.
					flirSession->setPedestrainPresence(false);
				}
			}

			if(isAnyFLIRSessionHealthy)
			{
				auto lastFlirSession = flirSessions.back();
				//If at least one pedestrain is detected, broadcast TIM with duration time of 1 minute.
				auto durationTime = isAnyPedestrainPresent? 1 : 0;
				/**If at least one pedestrain is detected, use the start year and moy from the last FLIR session.
				 * Otherwise, use the current year and minute of the year.
				 * **/
				auto startYear = isAnyPedestrainPresent? lastFlirSession->getStartYear() : TIMHelper::calculateCurrentYear();
				auto moy = isAnyPedestrainPresent? lastFlirSession->getMoy() : TIMHelper::calculateMinuteOfCurrentYear();
				TIMVariables currentTIMVars = {msgCount, startYear, moy, durationTime};
				if(currentTIMVars == previousTIMVars)
				{
					PLOG(logDEBUG) << "No change in the TIM variables and do not increase msgCnt.";
				}else{
					//Increase the message count in the TIM when there is a change in the TIM variables.
					msgCount = TIMHelper::increaseMsgCount(msgCount);
					currentTIMVars.msgCnt = msgCount;
				}
				auto updatedTim = TIMHelper::updateTimXML(staticTimXML, currentTIMVars);
				previousTIMVars = currentTIMVars;
				BroadcastPedDet(updatedTim);
			}
			else
			{
				PLOG(logDEBUG) << "No FLIR session is healthy. Skipping the broadcast of TIM message.";
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(period));
	}
}


void PedestrianPlugin::PedestrianRequestHandler(QHttpEngine::Socket *socket)
{
	QByteArray st;
	while(socket->bytesAvailable() > 0)
	{	
		auto readBytes = socket->readAll();
		st.append(readBytes);
	}

	if(st.size() == 0)
	{
		PLOG(logERROR) << "Received data is empty and skipped.";
		socket->setStatusCode(QHttpEngine::Socket::BadRequest);
		socket->writeHeaders();
		socket->close();
		return;
	}
	PLOG(logINFO) << "Received data bytes size: " << st.size();

	std::string msgDef = st.data();
	std::list<std::string> msgSL = {};
	msgSL.push_back(msgDef);

	// Catch parse exceptions
	try {
		for(const auto& msg_s: msgSL)
		{
			BroadcastPedDet(msg_s);
			socket->setStatusCode(QHttpEngine::Socket::Created);
		}
	}
	catch(const J2735Exception &e) {
		PLOG(logERROR) << "Error encoding received message data " << msgDef << std::endl << e.what();
		socket->setStatusCode(QHttpEngine::Socket::BadRequest);
	}
	
	socket->writeHeaders();
	socket->close();
}

int PedestrianPlugin::StartWebService()
{
	PLOG(logDEBUG) << "In PedestrianPlugin::StartWebService";

	// Web services
	std::array<char*, 1> placeholderX = {nullptr};
	int placeholderC = 1;
	QCoreApplication a(placeholderC, placeholderX.data());

 	auto address = QHostAddress(QString::fromStdString (webip));
	
	QHttpEngine::QObjectHandler apiHandler;
	apiHandler.registerMethod(Msg_Receive, [this](QHttpEngine::Socket *socket)
							{
							this->PedestrianRequestHandler(socket);
							});
	QHttpEngine::Server server(&apiHandler);

    if (!server.listen(address, webport))
	{
        qCritical("Unable to listen on the specified port.");
        return 1;
    }

	runningWebService = true;
    return QCoreApplication::exec();
}

void PedestrianPlugin::StopWebService()
{
    if (runningWebService)
    {
        PLOG(logDEBUG) << "Stopping WebService";
        QCoreApplication::quit();
    }
    runningWebService = false;
}

void PedestrianPlugin::UpdateConfigSettings()
{
	// Configuration settings are retrieved from the API using the GetConfigValue template class.
	// This method does NOT execute in the main thread, so variables must be protected
	// (e.g. using std::atomic, std::mutex, etc.).

	GetConfigValue<std::string>("WebServiceIP", webip, &_cfgLock);
	GetConfigValue<uint16_t>("WebServicePort", webport, &_cfgLock);
	GetConfigValue<std::string>("DataProvider", dataprovider, &_cfgLock);
	GetConfigValue<std::string>("FLIROutput", flirOutput, &_cfgLock);

	std::string flirConfigsStr;
	GetConfigValue<std::string>("FLIRConfigurations", flirConfigsStr, &_cfgLock);
	flirConfigsPtr->parseFLIRConfigs(flirConfigsStr);

	PLOG(logDEBUG) << "Pedestrian data provider: " << dataprovider;
	
	if (dataprovider.compare("FLIR") == 0)
    {
		//Read static TIM message from configuration parameter only when provider set to FLIR as the TIM message size is large.
		GetConfigValue<std::string> ("StaticTim", staticTimXML, &_cfgLock);
		staticTimXML = TIMHelper::jsonToXml(staticTimXML);
		GetConfigValue<int>("StaticTimFrequency", staticTimFrequency, &_cfgLock);
		getMessageToWrite();
        StopWebService();
        if (!runningWebSocket)
        {
			PLOG(logDEBUG) << "Starting WebSocket Thread";
			std::vector<std::thread> socketThreads;
			for(const auto & config: flirConfigsPtr->getConfigs()){
				socketThreads.emplace_back(&PedestrianPlugin::StartWebSocket, this, config);
			}			
            PLOG(logDEBUG) << "WebSocket thread started!!";

			PLOG(logDEBUG) << "Starting XML thread";
			//This thread is to check flirsession for any SDSM and PSM messages in the queue and broadcast them.
			std::thread xmlThread(&PedestrianPlugin::checkXML, this);
			PLOG(logDEBUG) << "XML thread started!!";
			
			PLOG(logDEBUG) << "Start thread to process static TIM in XML format.";
			std::thread staticTimThread(&PedestrianPlugin::processStaticTimXML, this);
			PLOG(logDEBUG) << "TIM processing thread started!!";

			// wait for all the socket threads to finish
			for(auto &thread: socketThreads){
				thread.join();
			}
			xmlThread.join(); // wait for the thread to finish
			staticTimThread.join(); // wait for the TIM thread to finish
        }
    }

	else if (dataprovider.compare("REST") == 0) // default if PSM/SDSM/TIM XML data consumed using the webservice implementation
	{
        StopWebSocket();
        if (!runningWebService)
        {
			PLOG(logDEBUG) << "Starting WebService Thread";
            std::thread webServiceThread(&PedestrianPlugin::StartWebService, this);
			PLOG(logDEBUG) << "WebService Thread started";
			webServiceThread.join(); // wait for the thread to finish
        }
    }
	else
	{
		PLOG(logWARNING) << "Invalid configured data provider. Pedestrian Plugin requires valid data provider (FLIR, REST)!";
		StopWebService();
		StopWebSocket();
	}

}

void PedestrianPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
}

void PedestrianPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
	}
}

std::string PedestrianPlugin::parseMessage(const std::string& message) const {
    if (message.find("<PersonalSafetyMessage>") != std::string::npos)
	{
        return "PersonalSafetyMessage";
    }
	else if (message.find("<SensorDataSharingMessage>") != std::string::npos)
	{
        return "SensorDataSharingMessage";
    }
	else if (message.find("<TravelerInformation>") != std::string::npos)
	{
        return "TravelerInformation";
    }
	else return "Unknown Message Type";
}

void PedestrianPlugin::BroadcastPedDet(const std::string &msgXML) 
{
	PsmMessage psmMsg;
	PsmEncodedMessage psmENC;
	SdsmMessage sdsmMsg;
	SdsmEncodedMessage sdsmENC;
	TimMessage timMsg;
	TimEncodedMessage timENC;

	tmx::message_container_type container;
	std::stringstream ss;
	ss << msgXML;

	container.load<XML>(ss);

	PLOG(logDEBUG2) << "In PedestrianPlugin::BroadcastPedDet: Incoming Message: " << std::endl << msgXML;
	auto receivedType = parseMessage(msgXML);

	if(receivedType == "PersonalSafetyMessage")
	{
		psmMsg.set_contents(container.get_storage().get_tree());
		psmENC.encode_j2735_message(psmMsg);

		auto msg = std::make_unique<PsmEncodedMessage>();
		msg.reset();
		msg.reset(dynamic_cast<PsmEncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_PERSONALSAFETYMESSAGE_STRING)));
		std::string enc = psmENC.get_encoding();
		msg->refresh_timestamp();
		msg->set_payload(psmENC.get_payload_str());
		msg->set_encoding(enc);
		msg->set_flags(IvpMsgFlags_RouteDSRC);
		msg->addDsrcMetadata(tmx::messages::api::personalSafetyMessage_PSID);
		msg->refresh_timestamp();

		PLOG(logINFO) << " Pedestrian Plugin :: Broadcast PSM:: " << psmENC.get_payload_str();
		auto *rMsg = dynamic_cast<routeable_message *>(msg.get());
		BroadcastMessage(*rMsg);
	}
	else if(receivedType == "SensorDataSharingMessage")
	{
		sdsmMsg.set_contents(container.get_storage().get_tree());
		sdsmENC.encode_j2735_message(sdsmMsg);

		auto msg = std::make_unique<SdsmEncodedMessage>();
		msg.reset();
		msg.reset(dynamic_cast<SdsmEncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_SENSORDATASHARINGMESSAGE_STRING)));
		std::string enc = sdsmENC.get_encoding();
		msg->refresh_timestamp();
		msg->set_payload(sdsmENC.get_payload_str());
		msg->set_encoding(enc);
		msg->set_flags(IvpMsgFlags_RouteDSRC);
		msg->addDsrcMetadata(tmx::messages::api::sensorDataSharingMessage_PSID);
		msg->refresh_timestamp();
		
		PLOG(logINFO) << " Pedestrian Plugin :: Broadcast SDSM:: " << sdsmENC.get_payload_str();
		auto *rMsg = dynamic_cast<routeable_message *>(msg.get());
		BroadcastMessage(*rMsg);
	}
	else if(receivedType == "TravelerInformation")
	{
		timMsg.set_contents(container.get_storage().get_tree());
		timENC.encode_j2735_message(timMsg);

		auto msg = std::make_unique<TimEncodedMessage>();
		msg.reset();
		msg.reset(dynamic_cast<TimEncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_TRAVELERINFORMATION_STRING)));
		std::string enc = timENC.get_encoding();
		msg->refresh_timestamp();
		msg->set_payload(timENC.get_payload_str());
		msg->set_encoding(enc);
		msg->set_flags(IvpMsgFlags_RouteDSRC);
		msg->addDsrcMetadata(tmx::messages::api::travelerInformation_PSID);
		msg->refresh_timestamp();
		
		PLOG(logINFO) << " Pedestrian Plugin :: Broadcast TIM:: " << timENC.get_payload_str();
		auto *rMsg = dynamic_cast<routeable_message *>(msg.get());
		BroadcastMessage(*rMsg);
	}
	else
	{
		PLOG(logWARNING) << "Received unknown message: " << msgXML;
	}
}


} /* namespace PedestrianPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<PedestrianPlugin::PedestrianPlugin>("PedestrianPlugin", argc, argv);
}
