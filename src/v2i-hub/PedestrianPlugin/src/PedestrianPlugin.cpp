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

	AddMessageFilter<MapDataMessage>(this, &PedestrianPlugin::HandleMapDataMessage);

	AddMessageFilter <BsmMessage> (this, &PedestrianPlugin::HandleBasicSafetyMessage);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();

	// fire up the web service on a thread PROTECTION required

	std::lock_guard<mutex> lock(_cfgLock); 
	GetConfigValue<string>("IPAddress",webip);
	GetConfigValue<uint16_t>("WebPort",webport);
	GetConfigValue<string>("PedDataProvider",dataprovider);
	GetConfigValue<string>("WebSocketIP",webSocketIP);
	GetConfigValue<string>("WebSocketURLExt",webSocketURLExt);
	
	
		PLOG(logERROR) << "Pedestrian data provider: "<< dataprovider.c_str() << std::endl;
	

	PLOG(logERROR) << "Before creating websocket: " << std::endl;
	
	
	if (dataprovider.compare("FLIR") == 0)
	{
		try
		{
			std::thread webthread(&PedestrianPlugin::StartWebSocket,this);
			webthread.detach(); // wait for the thread to finish
		}
		catch(const std::exception& e)
		{
			PLOG(logERROR) << "Error connecting to websocket: " << e.what() << std::endl;
		}
				
			
	}
	else  // default if PSM XML data consumed using the webservice implementation
	{
		std::thread webthread(&PedestrianPlugin::StartWebService,this);
		webthread.detach(); // wait for the thread to finish
	}



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
        PLOG(logERROR) << "Error parsing file: " << e.what() << std::endl;
		writeResponse(QHttpEngine::Socket::BadRequest, socket);
	}
}

int PedestrianPlugin::StartWebSocket()
{
	PLOG(logERROR) << "In PedestrianPlugin::StartWebSocket " << std::endl;
	// The io_context is required for all I/O
    net::io_context ioc;

    // Launch the asynchronous operation
    std::make_shared<FLIRWebSockAsyncClnSession>(ioc)->run(webSocketIP.c_str(), webSocketURLExt.c_str());

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();

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
	GetConfigValue<string>("WebServicePort",webSocketURLExt);
	GetConfigValue<int>("Instance", instance);
	GetConfigValue<string>("DataProvider", dataprovider);
	

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

void PedestrianPlugin::HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg)
{
	static std::atomic<int> count {0};

	PLOG(logINFO) << "New MAP: " << msg;

	int mapCount = count;
	SetStatus("ReceivedMaps", mapCount);
}



void PedestrianPlugin::HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg) {
	PLOG(logDEBUG)<<"HandleBasicSafetyMessage";
}


void PedestrianPlugin::BroadcastPsm(char * psmJson) {  //overloaded 


	PsmMessage psmmessage;
	PsmEncodedMessage psmENC;
	tmx::message_container_type container;
	std::unique_ptr<PsmEncodedMessage> msg;


	std::stringstream ss;
	ss << psmJson;

	container.load<XML>(ss);
	psmmessage.set_contents(container.get_storage().get_tree());

	const std::string psmString(psmJson);
	psmENC.encode_j2735_message(psmmessage);


	msg.reset();
	msg.reset(dynamic_cast<PsmEncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_PERSONALSAFETYMESSAGE_STRING)));


	string enc = psmENC.get_encoding();
	msg->refresh_timestamp();
	msg->set_payload(psmENC.get_payload_str());
	msg->set_encoding(enc);
	msg->set_flags(IvpMsgFlags_RouteDSRC);
	msg->addDsrcMetadata(172, 0x8002);
	msg->refresh_timestamp();



	routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
	BroadcastMessage(*rMsg);



	PLOG(logINFO) << " Pedestrian Plugin :: Broadcast PSM:: " << psmENC.get_payload_str();

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
			PersonalSafetyMessage psm_1;
			PersonalSafetyMessage &psm = psm_1;
			//BroadcastPsm(psm);

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
