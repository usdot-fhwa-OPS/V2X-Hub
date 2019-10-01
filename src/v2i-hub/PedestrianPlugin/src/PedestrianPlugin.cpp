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

	std::thread webthread(&PedestrianPlugin::StartWebService,this);
	webthread.join(); // wait for the thread to finish 

}

void PedestrianPlugin::PedestrianRequestHandler(QHttpEngine::Socket *socket)
{
	auto router = QSharedPointer<OpenAPI::OAIApiRouter>::create();
	QString st; 
	while(socket->bytesAvailable()>0)
	{	
		st.append(socket->readAll());
	}
	qDebug()<< "Read from socket : "<<st;
	QByteArray array = st.toLocal8Bit();

	char* psmMsgdef = array.data();	

	//sprintf(psmMsgdef,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<PersonalSafetyMessage>\n<basicType>\n<aPEDESTRIAN/>\n</basicType>\n<secMark>0</secMark>\n<msgCnt>0</msgCnt>\n<id>87654321</id>\n<position>\n<lat>406680509</lat>\n<long>-738318466</long>\n<elevation>40</elevation>\n</position>\n<accuracy>\n<semiMajor>255</semiMajor>\n<semiMinor>255</semiMinor>\n<orientation>65535</orientation>\n</accuracy>\n<speed>75</speed>\n<heading>3672</heading>\n<crossState>\n<true/>\n</crossState>\n<clusterSize>\n<medium/>\n</clusterSize>\n<clusterRadius>6</clusterRadius>\n</PersonalSafetyMessage>");
	//PersonalSafetyMessage psm1;
	//PersonalSafetyMessage &psm=psm1; 
	BroadcastPsm(psmMsgdef);
	//BroadcastPsm(psm);
}


int PedestrianPlugin::StartWebService()
{
	//Web services 
	char *placeholderX[1]={0};
	int placeholderC=1;
	QCoreApplication a(placeholderC,placeholderX);

 	QHostAddress address = QHostAddress(WEBSERVADDR);
    quint16 port = static_cast<quint16>(WEBSERVPORT);

	QSharedPointer<OpenAPI::OAIApiRequestHandler> handler(new OpenAPI::OAIApiRequestHandler());
	handler = QSharedPointer<OpenAPI::OAIApiRequestHandler> (new OpenAPI::OAIApiRequestHandler());

    QObject::connect(handler.data(), &OpenAPI::OAIApiRequestHandler::requestReceived, [&](QHttpEngine::Socket *socket) {

		this->PedestrianRequestHandler(socket);
    });

    QHttpEngine::Server server(handler.data());


    qDebug() << "Pedestrian Plugin Web service::: Serving on " << address.toString() << ":" << port;

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

	int instance;PersonalSafetyMessage psm_1;
			PersonalSafetyMessage &psm = psm_1;
			//BroadcastPsm(psm);
	GetConfigValue("Instance", instance);

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

	PLOG(logDEBUG)<<"Pedestrian Plugin < Broadcast PSM> char* :: Broadcasting PSM";

	PsmMessage psmmessage;
	PsmEncodedMessage psmENC;
	tmx::message_container_type container;
	std::unique_ptr<PsmEncodedMessage> msg;


	std::stringstream ss;
	ss << psmJson;
	

	qDebug()<<psmJson; 

	//container.load<XML>("/home/saxtonlab/V2X-Hub/src/v2i-hub/PedestrianPlugin/PSM.xml");
	container.load<XML>(ss);
	psmmessage.set_contents(container.get_storage().get_tree());


	const std::string psmString(psmJson);

	PLOG(logINFO) << "Pedestrian Plugin:: loaded data";

	PLOG(logDEBUG) << "Pedestrian Plugin :: Encoding " << psmmessage;
	PLOG(logDEBUG)<< " Ready to encode PSM";
	psmENC.encode_j2735_message(psmmessage);
	PLOG(logDEBUG)<<" Done Encoding PSM ";

	msg.reset();
	msg.reset(dynamic_cast<PsmEncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_PERSONALSAFETYMESSAGE_STRING)));
	//msg.reset(dynamic_cast<PsmEncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_SIGNALPHASEANDTIMINGMESSAGE_STRING)));

	//MSGSUBTYPE_PERSONALSAFETYMESSAGE_D_STRING

	string enc = psmENC.get_encoding();
	msg->refresh_timestamp();
	msg->set_payload(psmENC.get_payload_str());
	msg->set_encoding(enc);
	msg->set_flags(IvpMsgFlags_RouteDSRC);
	msg->addDsrcMetadata(172, 0x8002);
	msg->refresh_timestamp();



	routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
	BroadcastMessage(*rMsg);



	PLOG(logINFO) << " Pedestrian Plugin :: sending PSM -----  using structure sent from pedestrian " << psmENC.get_payload_str();

}

void PedestrianPlugin::BroadcastPsm(PersonalSafetyMessage &psm) {
	PLOG(logDEBUG)<<"Pedestrian Plugin :: Broadcasting PSM";

	PsmMessage psmmessage;
	PsmEncodedMessage psmENC;
	tmx::message_container_type container;
	std::unique_ptr<PsmEncodedMessage> msg;

	container.load<XML>("/home/saxtonlab/V2X-Hub/src/v2i-hub/PedestrianPlugin/PSM.xml");
	PLOG(logINFO) << "Pedestrian Plugin:: loaded data";
	psmmessage.set_contents(container.get_storage().get_tree());
	PLOG(logDEBUG) << "Pedestrian Plugin :: Encoding " << psmmessage;
	psmENC.encode_j2735_message(psmmessage);

	msg.reset();
	msg.reset(dynamic_cast<PsmEncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_TRAVELERINFORMATION_STRING)));

	string enc = psmENC.get_encoding();
	msg->set_payload(psmENC.get_payload_str());
	msg->set_encoding(enc);
	msg->set_flags(IvpMsgFlags_RouteDSRC);
	msg->addDsrcMetadata(172, 0x8002);
	msg->refresh_timestamp();

	routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
	BroadcastMessage(*rMsg);

	PLOG(logINFO) << " Pedestrian Plugin :: sending PSM " << psmENC.get_payload_str();

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
