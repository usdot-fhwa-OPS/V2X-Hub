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
PedestrianPlugin::PedestrianPlugin(const std::string &name) : PluginClient(name), runningWebSocket(false), runningWebService(false)
{	
	if (_signSimClient != nullptr)
		_signSimClient.reset();
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

	if (psmMsgdef == nullptr || strlen(psmMsgdef) == 0) 
	{
        PLOG(logWARNING) << "Received empty PSM message.";
        writeResponse(QHttpEngine::Socket::BadRequest, socket);
        return;
    }

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

	flirSession = std::make_shared<FLIRWebSockAsyncClnSession>(ioc);

    // Launch the asynchronous operation
	flirSession->run(webSocketIP.c_str(), webSocketURLExt.c_str(), cameraRotation, hostString.c_str());	

	PLOG(logDEBUG) << "Successfully running the I/O service";	
    runningWebSocket = true;

    std::thread([this]
	{
        this->ioc.run();
        runningWebSocket = false;
    }).detach();

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
			std::queue<std::string> currentPSMQueue = flirSession->getPSMQueue();

			while(!currentPSMQueue.empty())
			{		
				char* char_arr = &currentPSMQueue.front()[0];

				BroadcastPsm(char_arr);
				currentPSMQueue.pop();
			}			 
		}
	}	
}

int PedestrianPlugin::StartWebService()
{
	// Web services 
	std::array<char*, 1> placeholderX = {nullptr};
	int placeholderC = 1;
	QCoreApplication a(placeholderC, placeholderX.data());

 	auto address = QHostAddress(QString::fromStdString (webip));

	QSharedPointer<OpenAPI::OAIApiRequestHandler> handler(new OpenAPI::OAIApiRequestHandler());
	handler = QSharedPointer<OpenAPI::OAIApiRequestHandler> (new OpenAPI::OAIApiRequestHandler());

    QObject::connect(handler.data(), &OpenAPI::OAIApiRequestHandler::requestReceived, [this](QHttpEngine::Socket *socket) 
	{
		this->PedestrianRequestHandler(socket);
	});

    QHttpEngine::Server server(handler.data());

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
        if (webServiceThread.joinable())
        {
            webServiceThread.join();
        }
    }
    runningWebService = false;
}

void PedestrianPlugin::UpdateConfigSettings()
{
	// Configuration settings are retrieved from the API using the GetConfigValue template class.
	// This method does NOT execute in the main thread, so variables must be protected
	// (e.g. using std::atomic, std::mutex, etc.).

	std::lock_guard<mutex> lock(_cfgLock);

	GetConfigValue<std::string>("WebServiceIP",webip);
	GetConfigValue<uint16_t>("WebServicePort",webport);
	GetConfigValue<std::string>("WebSocketHost",webSocketIP);
	GetConfigValue<std::string>("WebSocketPort",webSocketURLExt);
	GetConfigValue<int>("Instance", instance);
	GetConfigValue<std::string>("DataProvider", dataprovider);
	GetConfigValue<float>("FLIRCameraRotation",cameraRotation);
	GetConfigValue<std::string>("HostString",hostString);

	PLOG(logDEBUG) << "Pedestrian data provider: "<< dataprovider.c_str();

	// std::vector<std::thread> threads;

	if (dataprovider.compare("FLIR") == 0)
    {
		PLOG(logDEBUG) << "Before creating websocket to: " << webSocketIP.c_str() <<  " on port: " << webSocketURLExt.c_str();
		
        StopWebService();
        if (!runningWebSocket)
        {
            StartWebSocket();
            PLOG(logDEBUG) << "WebSocket service started";
        }
    }

	else  // default if PSM XML data consumed using the webservice implementation
	{
        StopWebSocket();
        if (!runningWebService)
        {
            StartWebService();
            PLOG(logDEBUG) << "WebService started";
        }
    }

	// for (auto &thread : threads)
    // {
    //     if (thread.joinable())
    //     {
    //         thread.join();
    //     }
    // }
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

void PedestrianPlugin::BroadcastPsm(const char * psmJson) {  //overloaded 

	if (psmJson == nullptr || strlen(psmJson) == 0) 
	{
        PLOG(logWARNING) << "Attempted to broadcast an empty PSM message.";
        return;
    }

	PsmMessage psmmessage;
	PsmEncodedMessage psmENC;
	tmx::message_container_type container;
	std::unique_ptr<PsmEncodedMessage> msg;	

	try
	{
		std::stringstream ss;
		ss << psmJson;

		container.load<XML>(ss);
		psmmessage.set_contents(container.get_storage().get_tree());

		const std::string psmString(psmJson);

		psmENC.encode_j2735_message(psmmessage);

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
    catch (const std::invalid_argument &e)
    {
        PLOG(logWARNING) << "Invalid argument error: " << e.what() << " broadcasting PSM for xml: " << psmJson;
    }
    catch (const std::out_of_range &e)
    {
        PLOG(logWARNING) << "Out of range error: " << e.what() << " broadcasting PSM for xml: " << psmJson;
    }
    catch (const std::exception &e)
    {
        PLOG(logWARNING) << "General error: " << e.what() << " broadcasting PSM for xml: " << psmJson;
    }
}

/**
 * Write HTTP response. 
 */
void PedestrianPlugin::writeResponse(int responseCode, QHttpEngine::Socket *socket) const
{
	socket->setStatusCode(responseCode);
    socket->writeHeaders();
    if(socket->isOpen())
	{
        socket->close();
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
