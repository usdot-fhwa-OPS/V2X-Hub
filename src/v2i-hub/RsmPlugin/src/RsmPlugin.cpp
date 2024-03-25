//==========================================================================
// Name        : RsmPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2023 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Rsm Plugin
//==========================================================================

#include "include/RsmPlugin.hpp"

namespace RsmPlugin
{
/**
 * Construct a new RsmPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes.
 */
RsmPlugin::RsmPlugin(string name): PluginClient(name)
{
	// The log level can be changed from the default here.
	FILELog::ReportingLevel() = FILELog::FromString("DEBUG");

	AddMessageFilter<RsmMessage>(this, &RsmPlugin::HandleRoadSafetyMessage);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();
}

void RsmPlugin::RsmRequestHandler(QHttpEngine::Socket *socket)
{
	if(socket->bytesAvailable() == 0)
	{
		PLOG(logERROR) << "RSM Plugin does not receive web service request content!" << endl;
		writeResponse(QHttpEngine::Socket::BadRequest, socket);
		return;
	}

	// should read from the websocket and parse 
	QString st; 
	while(socket->bytesAvailable()>0)
	{	
		st.append(socket->readAll());
	}
	QByteArray array = st.toLocal8Bit();

	char* rsmMsgdef = array.data();	
	// Catch parse exceptions

	stringstream ss;
	ss << rsmMsgdef;
	PLOG(logDEBUG) << "Received from webservice: " << ss.str() << endl;
	
    try {
	    BroadcastRsm(rsmMsgdef);
		writeResponse(QHttpEngine::Socket::Created, socket);
	}
	catch (TmxException &ex) {
		PLOG(logERROR) << "Failed to encode message : " << ex.what();
		writeResponse(QHttpEngine::Socket::BadRequest, socket);
	}
}


int RsmPlugin::StartWebService()
{
	//Web services 
	char *placeholderX[1]={0};
	int placeholderC=1;
	QCoreApplication a(placeholderC,placeholderX);

 	QHostAddress address = QHostAddress(QString::fromStdString (webip));
    quint16 port = static_cast<quint16>(webport);


	QSharedPointer<OpenAPI::OAIApiRequestHandler> handler(new OpenAPI::OAIApiRequestHandler());
	handler = QSharedPointer<OpenAPI::OAIApiRequestHandler> (new OpenAPI::OAIApiRequestHandler());

	auto router = QSharedPointer<OpenAPI::OAIApiRouter>::create();
    router->setUpRoutes();

    QObject::connect(handler.data(), &OpenAPI::OAIApiRequestHandler::requestReceived, [&](QHttpEngine::Socket *socket) {

		this->RsmRequestHandler(socket);
    });

	QObject::connect(handler.data(), &OpenAPI::OAIApiRequestHandler::requestReceived, [&](QHttpEngine::Socket *socket) {
		router->processRequest(socket);
    });

    QHttpEngine::Server server(handler.data());

    if (!server.listen(address, port)) {
        qCritical("RsmPlugin:: Unable to listen on the specified port.");
        return 1;
    }
	PLOG(logINFO)<<"RsmPlugin:: Started web service";
	return a.exec();

}

RsmPlugin::~RsmPlugin()
{
	if (_signSimClient != NULL)
		delete _signSimClient;
}

void RsmPlugin::UpdateConfigSettings()
{
	// Configuration settings are retrieved from the API using the GetConfigValue template class.
	// This method does NOT execute in the main thread, so variables must be protected
	// (e.g. using atomic, mutex, etc.).

	lock_guard<mutex> lock(_cfgLock);

	GetConfigValue<string>("WebServiceIP", webip);
	GetConfigValue<uint16_t>("WebServicePort", webport);
}

void RsmPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void RsmPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
		// Start webservice needs to occur after the first updateConfigSettings call to acquire port and ip configurations.
		// Also needs to be called from Main thread to work.
		thread webthread(&RsmPlugin::StartWebService, this);
		webthread.detach(); // wait for the thread to finish 
	}
}

void RsmPlugin::HandleRoadSafetyMessage(RsmMessage &msg, routeable_message &routeableMsg)
{
	PLOG(logDEBUG)<<"HandleRoadSafetyMessage";
}

void RsmPlugin::BroadcastRsm(char * rsmJson) 
{

	RsmMessage rsmmessage;
	RsmEncodedMessage rsmENC;
	tmx::message_container_type container;
	unique_ptr<RsmEncodedMessage> msg;

	try
	{
		stringstream ss;
		ss << rsmJson;

		container.load<XML>(ss);
		rsmmessage.set_contents(container.get_storage().get_tree());

		const string rsmString(rsmJson);

		rsmENC.encode_j2735_message(rsmmessage);

		msg.reset();
		msg.reset(dynamic_cast<RsmEncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_ROADSAFETYMESSAGE_STRING)));

		string enc = rsmENC.get_encoding();
		msg->refresh_timestamp();
		msg->set_payload(rsmENC.get_payload_str());
		msg->set_encoding(enc);
		msg->set_flags(IvpMsgFlags_RouteDSRC);
		msg->addDsrcMetadata(0x8003);
		msg->refresh_timestamp();

		routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
		BroadcastMessage(*rMsg);

		PLOG(logINFO) << " RSM Plugin :: Broadcast RSM:: " << rsmENC.get_payload_str();	
	}
	catch(const exception& e)
	{
		PLOG(logWARNING) << "Error: " << e.what() << " broadcasting RSM for xml: " << rsmJson << endl;
	}
	
	

}

/**
 * Write HTTP response. 
 */
void RsmPlugin::writeResponse(int responseCode , QHttpEngine::Socket *socket) {
	socket->setStatusCode(responseCode);
    socket->writeHeaders();
    if(socket->isOpen()){
        socket->close();
    }

}


int RsmPlugin::Main()
{
	PLOG(logINFO) << "RsmPlugin:: Starting plugin.\n";

	uint msCount = 0;
	while (_plugin->state != IvpPluginState_error)
	{

		msCount += 10;

		if (_plugin->state == IvpPluginState_registered)
		{
			RoadSafetyMessage rsm_1;
			RoadSafetyMessage &rsm = rsm_1;

			this_thread::sleep_for(chrono::milliseconds(100));

			msCount = 0;
		}
	}

	PLOG(logINFO) << "Plugin terminating gracefully.";
	return EXIT_SUCCESS;
}

} /* namespace RsmPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<RsmPlugin::RsmPlugin>("RsmPlugin", argc, argv);
}
