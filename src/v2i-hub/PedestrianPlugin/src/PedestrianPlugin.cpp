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
}

int PedestrianPlugin::StartWebSocket()
{
	PLOG(logDEBUG) << "In PedestrianPlugin::StartWebSocket ";
	flirSession = std::make_shared<FLIRWebSockAsyncClnSession>(ioc);

    // Launch the asynchronous operation
	flirSession->run(webSocketIP.c_str(), webSocketURLExt.c_str(), cameraRotation, hostString.c_str());	

	PLOG(logDEBUG) << "Successfully running the I/O service";	
    runningWebSocket = true;

    // Run the I/O service. The call will return when the socket is closed.
    ioc.run();

    return EXIT_SUCCESS;
}

void PedestrianPlugin::StopWebSocket()
{
    if (flirSession && runningWebSocket)
    {
        PLOG(logDEBUG) << "Stopping WebSocket session";
		beast::error_code ec;
        flirSession->on_close(ec);
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
		if (flirSession == nullptr)
		{
			PLOG(logDEBUG) << "FLIR session not yet initialized: ";
		}
		else
		{	
			// Retrieve the message queue and send each one to be broadcast, then pop.
			std::queue<std::string> currentMsgQueue = flirSession->getMsgQueue();

			while(!currentMsgQueue.empty())
			{		
				const char* char_arr = &currentMsgQueue.front()[0];

				BroadcastPedDet(char_arr);
				currentMsgQueue.pop();
			}
		}
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
	GetConfigValue<std::string>("WebSocketHost", webSocketIP, &_cfgLock);
	GetConfigValue<std::string>("WebSocketPort", webSocketURLExt, &_cfgLock);
	GetConfigValue<std::string>("DataProvider", dataprovider, &_cfgLock);
	GetConfigValue<float>("FLIRCameraRotation", cameraRotation, &_cfgLock);
	GetConfigValue<std::string>("HostString", hostString, &_cfgLock);

	PLOG(logDEBUG) << "Pedestrian data provider: " << dataprovider;
	
	if (dataprovider.compare("FLIR") == 0)
    {
        StopWebService();
        if (!runningWebSocket)
        {
			PLOG(logDEBUG) << "Starting WebSocket Thread";
            std::thread webSocketThread(&PedestrianPlugin::StartWebSocket, this);
            PLOG(logDEBUG) << "WebSocket Thread started!!";

			PLOG(logDEBUG) << "Starting XML Thread";
			std::thread xmlThread(&PedestrianPlugin::checkXML, this);
			PLOG(logDEBUG) << "XML Thread started!!";
			webSocketThread.join(); // wait for the thread to finish
			xmlThread.join(); // wait for the thread to finish
        }
    }

	else if (dataprovider.compare("XML") == 0) // default if PSM/SDSM/TIM XML data consumed using the webservice implementation
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
		PLOG(logWARNING) << "Invalid configured data provider. Pedestrian Plugin requires valid data provider (FLIR, PSM, SDSM)!";
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

std::string PedestrianPlugin::parseMessage(const std::string& message) {
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

void PedestrianPlugin::BroadcastPedDet(const std::string &msgJson) 
{
	PsmMessage psmMsg;
	PsmEncodedMessage psmENC;
	SdsmMessage sdsmMsg;
	SdsmEncodedMessage sdsmENC;
	TimMessage timMsg;
	TimEncodedMessage timENC;

	tmx::message_container_type container;
	std::stringstream ss;
	ss << msgJson;

	container.load<XML>(ss);

	PLOG(logDEBUG) << "In PedestrianPlugin::BroadcastPedDet: Incoming Message: " << std::endl << msgJson;
	auto receivedType = parseMessage(msgJson);

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
		PLOG(logWARNING) << "Received unknown message: " << msgJson;
	}
}

int PedestrianPlugin::Main()
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

} /* namespace PedestrianPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<PedestrianPlugin::PedestrianPlugin>("PedestrianPlugin", argc, argv);
}
