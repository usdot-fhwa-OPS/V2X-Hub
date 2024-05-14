//==========================================================================
// Name        : PedestrianPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Pedestrian Plugin
//==========================================================================

#include "include/PedestrianPlugin.hpp"


namespace PedestrianPlugin
{

/**
 * Construct a new PedestrianPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes.
 */
PedestrianPlugin::PedestrianPlugin(string name): PluginClient(name)
{
	// The log level can be changed from the default here.
	FILELog::ReportingLevel() = FILELog::FromString("DEBUG");
}

void PedestrianPlugin::PedestrianRequestHandler(QHttpEngine::Socket *socket)
{
	auto router = QSharedPointer<OpenAPI::OAIApiRouter>::create();
	QString st; 
	while(socket->bytesAvailable()>0)
	{	
		st.append(socket->readAll());
	}
	QByteArray array = st.toLocal8Bit();

	char* psmMsgdef = array.data();	
	// Catch parse exceptions
    try {
	    BroadcastPsm(psmMsgdef);
		writeResponse(QHttpEngine::Socket::Created, socket);
	}
	catch(const J2735Exception &e) {
        PLOG(logERROR) << "Error parsing file: " << e.what();
		writeResponse(QHttpEngine::Socket::BadRequest, socket);
	}
}

int PedestrianPlugin::StartWebSocket()
{
	PLOG(logDEBUG) << "In PedestrianPlugin::StartWebSocket ";
	// The io_context is required for all I/O
    net::io_context ioc;

	flirSession = std::make_shared<FLIRWebSockAsyncClnSession>(ioc);

    // Launch the asynchronous operation
	flirSession->run(webSocketIP.c_str(), webSocketURLExt.c_str(), cameraRotation, hostString.c_str());	

	PLOG(logDEBUG) << "Successfully running the I/O service";	

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();

	return EXIT_SUCCESS;
}

void PedestrianPlugin::OnWebSocketClosed()
{
	if (flirSession)
	{
		boost::beast::error_code ec;
		flirSession->on_close(ec);
	}
	else
	{
		PLOG(logDEBUG) << "WebSocket session already closed.";
	}
}

int PedestrianPlugin::checkXML()
{
	//first xml will be empty string
	std::string lastGeneratedXML = "";

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
			std::queue<string> currentPSMQueue = flirSession->getPSMQueue();

			while(!currentPSMQueue.empty())
			{		
				char* char_arr = &currentPSMQueue.front()[0];

				BroadcastPsm(char_arr);
				currentPSMQueue.pop();
			}			 
		}
		
	}	
	return EXIT_SUCCESS;
}

int PedestrianPlugin::StartWebService()
{
	//Web services 
	char *placeholderX[1]={0};
	int placeholderC=1;
	QCoreApplication a(placeholderC,placeholderX);

 	QHostAddress address = QHostAddress(QString::fromStdString (webip));
    quint16 port = static_cast<quint16>(webport);


	QSharedPointer<OpenAPI::OAIApiRequestHandler> handler(new OpenAPI::OAIApiRequestHandler());
	handler = QSharedPointer<OpenAPI::OAIApiRequestHandler> (new OpenAPI::OAIApiRequestHandler());

    QObject::connect(handler.data(), &OpenAPI::OAIApiRequestHandler::requestReceived, [&](QHttpEngine::Socket *socket) {

		this->PedestrianRequestHandler(socket);
    });

    QHttpEngine::Server server(handler.data());

    if (!server.listen(address, port)) {
        qCritical("Unable to listen on the specified port.");
        return 1;
    }
	return a.exec();

}

PedestrianPlugin::~PedestrianPlugin()
{
	if (_signSimClient != NULL)
		delete _signSimClient;
}

void PedestrianPlugin::UpdateConfigSettings()
{
	// Configuration settings are retrieved from the API using the GetConfigValue template class.
	// This method does NOT execute in the main thread, so variables must be protected
	// (e.g. using std::atomic, std::mutex, etc.).

	int instance;
	std::lock_guard<mutex> lock(_cfgLock);

	GetConfigValue<string>("WebServiceIP",webip);
	GetConfigValue<uint16_t>("WebServicePort",webport);
	GetConfigValue<string>("WebSocketHost",webSocketIP);
	GetConfigValue<string>("WebSocketPort",webSocketURLExt);
	GetConfigValue<int>("Instance", instance);
	GetConfigValue<string>("DataProvider", dataprovider);
	GetConfigValue<float>("FLIRCameraRotation",cameraRotation);
	GetConfigValue<string>("HostString",hostString);

	PLOG(logDEBUG) << "Pedestrian data provider: "<< dataprovider.c_str();
	PLOG(logDEBUG) << "Before creating websocket to: " << webSocketIP.c_str() <<  " on port: " << webSocketURLExt.c_str();

	if (dataprovider.compare("FLIR") == 0)
	{
		try
		{
			std::thread webthread(&PedestrianPlugin::StartWebSocket,this);
			PLOG(logDEBUG) << "Thread started!!: ";
			
			webthread.detach(); // wait for the thread to finish

			std::thread xmlThread(&PedestrianPlugin::checkXML,this);
			PLOG(logDEBUG) << "XML Thread started!!: ";
			
			xmlThread.detach(); // wait for the thread to finish

		}
		catch(const std::exception& e)
		{
			PLOG(logERROR) << "Error connecting to websocket: " << e.what();
		}
				
			
	}
	else  // default if PSM XML data consumed using the webservice implementation
	{
		OnWebSocketClosed();
		std::thread webthread(&PedestrianPlugin::StartWebService,this);
		webthread.detach(); // wait for the thread to finish
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
		SetStatus("ReceivedMaps", 0);
	}
}

void PedestrianPlugin::BroadcastPsm(char * msgJson) {  //overloaded 

	PsmMessage psmmessage;
	PsmEncodedMessage psmENC;
	SdsmMessage sdsmMsg;
	SdsmEncodedMessage sdsmENC;
	tmx::message_container_type container;
	std::unique_ptr<PsmEncodedMessage> pMsg;	
	std::unique_ptr<SdsmEncodedMessage> sMsg;

	try
	{
		std::stringstream ss;
		ss << msgJson;

		container.load<XML>(ss);

		if(dataprovider.compare("PSM") == 0)
		{
			psmmessage.set_contents(container.get_storage().get_tree());

			const std::string psmString(msgJson);

			psmENC.encode_j2735_message(psmmessage);

			pMsg.reset();
			pMsg.reset(dynamic_cast<PsmEncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_PERSONALSAFETYMESSAGE_STRING)));

			string enc = psmENC.get_encoding();
			pMsg->refresh_timestamp();
			pMsg->set_payload(psmENC.get_payload_str());
			pMsg->set_encoding(enc);
			pMsg->set_flags(IvpMsgFlags_RouteDSRC);
			pMsg->addDsrcMetadata(tmx::messages::api::personalSafetyMessage_PSID);
			pMsg->refresh_timestamp();

			routeable_message *rMsg = dynamic_cast<routeable_message *>(pMsg.get());
			BroadcastMessage(*rMsg);
			PLOG(logINFO) << " Pedestrian Plugin :: Broadcast PSM:: " << psmENC.get_payload_str();
		}
		else if (dataprovider.compare("SDSM") == 0)
		{
			sdsmMsg.set_contents(container.get_storage().get_tree());

			const std::string sdsmString(msgJson);

			sdsmENC.encode_j2735_message(sdsmMsg);

			sMsg.reset();
			sMsg.reset(dynamic_cast<SdsmEncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_SENSORDATASHARINGMESSAGE_STRING)));

			string enc = sdsmENC.get_encoding();
			sMsg->refresh_timestamp();
			sMsg->set_payload(sdsmENC.get_payload_str());
			sMsg->set_encoding(enc);
			sMsg->set_flags(IvpMsgFlags_RouteDSRC);
			sMsg->addDsrcMetadata(tmx::messages::api::sensorDataSharingMessage_PSID);
			sMsg->refresh_timestamp();

			routeable_message *rMsg = dynamic_cast<routeable_message *>(sMsg.get());
			BroadcastMessage(*rMsg);
			PLOG(logINFO) << " Pedestrian Plugin :: Broadcast SDSM:: " << sdsmENC.get_payload_str();
		}
	}
	catch(const std::exception& e)
	{
		PLOG(logWARNING) << "Error: " << e.what() << " broadcasting message for xml: " << msgJson;

	}

	

}

/**
 * Write HTTP response. 
 */
void PedestrianPlugin::writeResponse(int responseCode , QHttpEngine::Socket *socket) {
	socket->setStatusCode(responseCode);
    socket->writeHeaders();
    if(socket->isOpen()){
        socket->close();
    }

}


int PedestrianPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	uint msCount = 0;
	while (_plugin->state != IvpPluginState_error)
	{

		msCount += 10;

		if (_plugin->state == IvpPluginState_registered)
		{
			if(dataprovider.compare("PSM") == 0)
			{
				PersonalSafetyMessage psm_1;
				PersonalSafetyMessage &psm = psm_1;
				//BroadcastPsm(psm);
			}
			else if(dataprovider.compare("SDSM") == 0)
			{
				SensorDataSharingMessage sdsm_1;
				SensorDataSharingMessage &sdsm = sdsm_1;
			}

			this_thread::sleep_for(chrono::milliseconds(100));

			msCount = 0;
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
