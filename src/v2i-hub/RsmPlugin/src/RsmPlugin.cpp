//==========================================================================
// Name        : RsmPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2024 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
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
}

void RsmPlugin::RsmRequestHandler(QHttpEngine::Socket *socket)
{
	if(socket->bytesAvailable() == 0)
	{
		PLOG(logERROR) << "RSM Plugin does not receive web service request content!";
		writeResponse(QHttpEngine::Socket::BadRequest, socket);
		return;
	}

	// Read from the websocket and parse
	QString st; 
	while(socket->bytesAvailable()>0)
	{	
		st.append(socket->readAll());
	}
	QByteArray array = st.toLocal8Bit();

	char* rsmMsgdef = array.data();	

	stringstream ss;
	ss << rsmMsgdef;
	PLOG(logDEBUG) << "Received from webservice:\n" << ss.str();
	
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
	// Web services 
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
	PLOG(logINFO) << "RsmPlugin:: Started web service";
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

	lock_guard<mutex> lock(cfgLock);
	GetConfigValue<string>("WebServiceIP", webip);
	GetConfigValue<uint16_t>("WebServicePort", webport);
	GetConfigValue<uint64_t>("Interval", interval);
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
		
		while (RsmDuration(rsmENC))
		{
			lock_guard<mutex> lock(cfgLock);
			uint64_t sendInterval = interval;

			msg.reset();
			msg.reset(dynamic_cast<RsmEncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_ROADSAFETYMESSAGE_STRING)));

			string enc = rsmENC.get_encoding();
			msg->refresh_timestamp();
			msg->set_payload(rsmENC.get_payload_str());
			msg->set_encoding(enc);
			msg->set_flags(IvpMsgFlags_RouteDSRC);
			msg->addDsrcMetadata(tmx::messages::api::roadSafetyMessage_PSID);
			msg->refresh_timestamp();

			routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
			BroadcastMessage(*rMsg);
			PLOG(logINFO) << "RSM Plugin :: Broadcast RSM:: " << rsmENC.get_payload_str();

			// Make sure Interval configuration is positive. Otherwise, set to default 1000 milliseconds.
			sendInterval = sendInterval > 0 ? sendInterval: 1000;
			// Sleep sendInterval for every attempt to broadcast RSM.
			this_thread::sleep_for(chrono::milliseconds(sendInterval));
		}	
	}
	catch(const exception& e)
	{
		PLOG(logWARNING) << "Error: " << e.what() << " broadcasting RSM for xml: " << rsmJson;
	}
}

bool RsmPlugin::RsmDuration(RsmEncodedMessage RsmEncMsg)
{
	auto rsm_ptr = RsmEncMsg.decode_j2735_message().get_j2735_data();
	auto startYear = rsm_ptr->commonContainer.eventInfo.startDateTime.year;
	auto startMonth = rsm_ptr->commonContainer.eventInfo.startDateTime.month;
	auto startDay = rsm_ptr->commonContainer.eventInfo.startDateTime.day;
	auto startHour = rsm_ptr->commonContainer.eventInfo.startDateTime.hour;
	auto startMin = rsm_ptr->commonContainer.eventInfo.startDateTime.minute;
	auto startSec = rsm_ptr->commonContainer.eventInfo.startDateTime.second;

	PLOG(logINFO) << "Start Date yy/mm/dd : " << *startYear << "/" << *startMonth << "/" << *startDay;
	PLOG(logINFO) << "Start Time hh:mm:ss : " << *startHour << ":" << *startMin << ":" << *startSec;

	if (!rsm_ptr->commonContainer.eventInfo.endDateTime)
	{
		PLOG(logINFO) << "No End Date/Time in message.";
		return true;
	}
	else
	{
		auto endYear = rsm_ptr->commonContainer.eventInfo.endDateTime->year;
		auto endMonth = rsm_ptr->commonContainer.eventInfo.endDateTime->month;
		auto endDay = rsm_ptr->commonContainer.eventInfo.endDateTime->day;
		auto endHour = rsm_ptr->commonContainer.eventInfo.endDateTime->hour;
		auto endMin = rsm_ptr->commonContainer.eventInfo.endDateTime->minute;
		auto endSec = rsm_ptr->commonContainer.eventInfo.endDateTime->second;

		PLOG(logINFO) << "End Date yy/mm/dd : " << *endYear << "/" << *endMonth << "/" << *endDay;
		PLOG(logINFO) << "End Time hh:mm:ss : " << *endHour << ":" << *endMin << ":" << *endSec;

		time_t now = time(0);
		// Current UTC datetime
		tm *utcTM = gmtime(&now);
		auto currentYear = 1900 + utcTM->tm_year;
		auto currentMonth = 1 + utcTM->tm_mon;
		auto currentDay = utcTM->tm_mday;
		auto currentHour = utcTM->tm_hour;
		auto currentMin = utcTM->tm_min;
		auto currentSec = utcTM->tm_sec;
		PLOG(logDEBUG) << "Current UTC DateTime yy/mm/dd hh:mm:ss : " << currentYear << "/" << currentMonth << "/" << currentDay << " " << currentHour << ":" << currentMin << ":" << currentSec;

		auto diffYear = *endYear - currentYear;
		auto diffMonth = *endMonth - currentMonth;
		auto diffDay = (*endDay - currentDay);
		PLOG(logDEBUG4) << "Diff Date yy/mm/dd: " << diffYear << "/" << diffMonth << "/" << diffDay;

		if (diffYear > 0)
		{
			return true;
		}
		else if (diffMonth > 0)
		{
			return true;
		}
		else if (diffDay > 0)
		{
			return true;
		}
		else
		{
			auto diffHour = (*endHour - currentHour)*3600;
			auto diffMin = (*endMin - currentMin)*60;
			auto diffSec = *endSec - currentSec;
			PLOG(logDEBUG4) << "Time difference h:m:s " << diffHour << ":" << diffMin << ":" << diffSec;

			// Message duration in seconds
			auto rsmDuration = diffHour + diffMin + diffSec;
			
			if (rsmDuration > 0)
			{
				PLOG(logDEBUG) << "RSM will transmit for: " << rsmDuration << " seconds";
				return true;
			}
			else 
			{
				PLOG(logINFO) << "RSM endDateTime has passed.";
				return false;
			}
		}
	}
}

/**
 * Write HTTP response. 
 */
void RsmPlugin::writeResponse(int responseCode , QHttpEngine::Socket *socket)
{
	socket->setStatusCode(responseCode);
    socket->writeHeaders();
    if(socket->isOpen()){
        socket->close();
    }
}


int RsmPlugin::Main()
{
	PLOG(logINFO) << "RsmPlugin:: Starting plugin.";

	while (_plugin->state != IvpPluginState_error)
	{
		if (IsPluginState(IvpPluginState_registered))
		{
			// this_thread::sleep_for(chrono::milliseconds(100));
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}

	PLOG(logINFO) << "Plugin terminating gracefully.";
	return EXIT_SUCCESS;
}

} /* namespace RsmPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<RsmPlugin::RsmPlugin>("RsmPlugin", argc, argv);
}
