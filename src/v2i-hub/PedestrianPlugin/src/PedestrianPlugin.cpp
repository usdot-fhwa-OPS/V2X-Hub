//==========================================================================
// Name        : PedestrianPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
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
 * Construct a new PedestrianPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes.
 */
PedestrianPlugin::PedestrianPlugin(const std::string &name) : PluginClient(name)
{
	if (_signSimClient != nullptr)
		_signSimClient.reset();
	
	UpdateConfigSettings();
	std::lock_guard<mutex> lock(_cfgLock);

	std::thread webServiceThread(&PedestrianPlugin::StartWebService, this);
	webServiceThread.detach(); // wait for the thread to finish
	// StartWebService = std::move(std::jthread{StartWebService});
	runningWebService = true;
}

int PedestrianPlugin::StartWebSocket()
{
	PLOG(logDEBUG) << "In PedestrianPlugin::StartWebSocket ";
	// std::jthread StartWebSocket;

	flirSession = std::make_shared<FLIRWebSockAsyncClnSession>(ioc);

    // Launch the asynchronous operation
	flirSession->run(webSocketIP.c_str(), webSocketURLExt.c_str(), cameraRotation, hostString.c_str());	

	PLOG(logDEBUG) << "Successfully running the I/O service";	
    runningWebSocket = true;

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();
	runningWebSocket = false;

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

[[noreturn]] int PedestrianPlugin::checkXML()
{
	// std::jthread checkXML;
	//if a new psm xml has been generated the FLIR web socket, send it to the BroadcastPSM function
	while (true)
	{
		if (flirSession == nullptr)
		{
			PLOG(logDEBUG) << "flir session not yet initialized: ";
		}
		else
		{	
			//retrieve the PSM queue and send each one to be broadcast, then pop		
			std::queue<std::string> currentPSMQueue = flirSession->getPSMQueue();

			while(!currentPSMQueue.empty())
			{		
				const char* char_arr = &currentPSMQueue.front()[0];

				BroadcastPsm(char_arr);
				currentPSMQueue.pop();
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
		PLOG(logERROR) << "Received PSM is empty and skipped.";
		socket->setStatusCode(QHttpEngine::Socket::BadRequest);
		socket->writeHeaders();
		socket->close();
		return;
	}
	PLOG(logINFO) << "Received PSM bytes size: " << st.size();

	std::string psmMsgdef = st.data();
	std::list<std::string> psmSL = {};
	psmSL.push_back(psmMsgdef);

	// Catch parse exceptions
    try {
		for(const auto& psm_s: psmSL)
		{
			BroadcastPsm(psm_s);
			socket->setStatusCode(QHttpEngine::Socket::Created);
		}
	}
	catch(const J2735Exception &e) {
        PLOG(logERROR) << "Error parsing file: " << e.what();
		socket->setStatusCode(QHttpEngine::Socket::BadRequest);
	}
	
	socket->writeHeaders();
	socket->close();
}

int PedestrianPlugin::StartWebService()
{
	PLOG(logDEBUG) << "In PedestrianPlugin::StartWebService";
	// std::jthread webServiceThread;

	// Web services 
	std::array<char*, 1> placeholderX = {nullptr};
	int placeholderC = 1;
	QCoreApplication a(placeholderC, placeholderX.data());

 	auto address = QHostAddress(QString::fromStdString (webip));
	
	QHttpEngine::QObjectHandler apiHandler;
	apiHandler.registerMethod(PSM_Receive, [this](QHttpEngine::Socket *socket)
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

	std::lock_guard<mutex> lock(_cfgLock);
	GetConfigValue<std::string>("WebServiceIP", webip);
	GetConfigValue<uint16_t>("WebServicePort", webport);
	GetConfigValue<std::string>("WebSocketHost", webSocketIP);
	GetConfigValue<std::string>("WebSocketPort", webSocketURLExt);
	GetConfigValue<int>("Instance", instance);
	GetConfigValue<std::string>("DataProvider", dataprovider);
	GetConfigValue<float>("FLIRCameraRotation", cameraRotation);
	GetConfigValue<std::string>("HostString", hostString);

	PLOG(logDEBUG) << "Pedestrian data provider: " << dataprovider;
	
	if (dataprovider.compare("FLIR") == 0)
    {
		PLOG(logDEBUG) << "Before creating websocket to: " << webSocketIP.c_str() <<  " on port: " << webSocketURLExt.c_str();
		
        StopWebService();
        if (!runningWebSocket)
        {
			PLOG(logDEBUG) << "Starting WebSocket Thread";
            std::thread webSocketThread(&PedestrianPlugin::StartWebSocket, this);
			// StartWebSocket = std::move(std::jthread{StartWebSocket});
            PLOG(logDEBUG) << "WebSocket Thread started!!";
			webSocketThread.detach(); // wait for the thread to finish

			PLOG(logDEBUG) << "Starting XML Thread";
			std::thread xmlThread(&PedestrianPlugin::checkXML, this);
			// checkXML = std::move(std::jthread{checkXML});
			PLOG(logDEBUG) << "XML Thread started!!";
			xmlThread.detach(); // wait for the thread to finish
        }
    }

	else if (dataprovider.compare("PSM") == 0) // default if PSM XML data consumed using the webservice implementation
	{
        StopWebSocket();
        if (!runningWebService)
        {
			PLOG(logDEBUG) << "Starting WebService Thread";
            std::thread webServiceThread(&PedestrianPlugin::StartWebService, this);
			webServiceThread.detach(); // wait for the thread to finish
            PLOG(logDEBUG) << "WebService Thread started";
        }
    }
	else
	{
		PLOG(logWARNING) << "Incorrect DataProvider entered!";
		StopWebService();
		StopWebSocket();
	}
}

void PedestrianPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void PedestrianPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
	}
}

void PedestrianPlugin::BroadcastPsm(const std::string &psmJson) 
{ //overloaded 

	PsmMessage psmmessage;
	PsmEncodedMessage psmENC;
	tmx::message_container_type container;

	std::stringstream ss;
	ss << psmJson;

	container.load<XML>(ss);
	psmmessage.set_contents(container.get_storage().get_tree());

	psmENC.encode_j2735_message(psmmessage);

	// std::unique_ptr<PsmEncodedMessage> msg;
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

	auto *rMsg = dynamic_cast<routeable_message *>(msg.get());
	BroadcastMessage(*rMsg);

	PLOG(logINFO) << " Pedestrian Plugin :: Broadcast PSM:: " << psmENC.get_payload_str();
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
