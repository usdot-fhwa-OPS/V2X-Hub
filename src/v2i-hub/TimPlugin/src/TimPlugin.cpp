#include "TimPlugin.hpp"

using namespace std;
using namespace tmx::messages;
using namespace tmx::utils;
using namespace boost::property_tree;

namespace TimPlugin {
/**
 * Construct a new TimPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes
 */
TimPlugin::TimPlugin(string name) :
		PluginClient(name) {
					
}

TimPlugin::~TimPlugin() {

}

void TimPlugin::TimRequestHandler(QHttpEngine::Socket *socket)
{
	if(socket->bytesAvailable() == 0)
	{
		PLOG(logERROR) << "TimPlugin does not receive web service request content!" <<std::endl;
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

	char* _cloudUpdate = array.data(); // would be the cloud update packet, needs parsing
 
	std::stringstream ss;
	ss << _cloudUpdate;
	PLOG(logDEBUG) << "Received from webservice: " << ss.str() << std::endl;
	try { 
		lock_guard<mutex> lock(_cfgLock);
		_timMsgPtr = readTimXml(ss.str());
		_isTimUpdated = true;
		writeResponse(QHttpEngine::Socket::Created, socket);
	}
	catch (TmxException &ex) {
		PLOG(logERROR) << "Failed to encode message : " << ex.what();
		writeResponse(QHttpEngine::Socket::BadRequest, socket);
	}

}
/**
 * Write HTTP response. 
 */
void TimPlugin::writeResponse(int responseCode , QHttpEngine::Socket *socket) {
	socket->setStatusCode(responseCode);
    socket->writeHeaders();
    if(socket->isOpen()){
        socket->close();
    }

}

int TimPlugin::StartWebService()
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

		TimRequestHandler(socket);
    });

    QObject::connect(handler.data(), &OpenAPI::OAIApiRequestHandler::requestReceived, [&](QHttpEngine::Socket *socket) {
		router->processRequest(socket);
    });

    QHttpEngine::Server server(handler.data());

    if (!server.listen(address, port)) {
        qCritical("TimPlugin::Unable to listen on the specified port.");
        return 1;
    }
	PLOG(logINFO)<<"TimPlugin:: Started web service";
	return a.exec();

}

void TimPlugin::UpdateConfigSettings() {

	lock_guard<mutex> lock(_cfgLock);
	
	GetConfigValue<uint64_t>("Frequency", _frequency);
	
	if (GetConfigValue<string>("MapFile", _mapFile)) {
		if ( std::filesystem::exists( _mapFile ) ){
            _isTimFileNew = true;
			PLOG(logINFO) << "Loading MapFile " << _mapFile << "." << std::endl;
        }
		else {
			PLOG(logWARNING) << "MapFile " << _mapFile << " does not exist!" << std::endl;
		}

	}
	
	GetConfigValue<string>("WebServiceIP", webip);
	GetConfigValue<uint16_t>("WebServicePort", webport);

}

void TimPlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void TimPlugin::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);


	if (state == IvpPluginState_registered) {
		UpdateConfigSettings();

		// Start webservice needs to occur after the first updateConfigSettings call
		// to acquire port and ip configurations.
		// Also needs to be called from Main thread to work.
		std::thread webthread(&TimPlugin::StartWebService,this);
		webthread.detach(); // wait for the thread to finish 
	}
}

int TimPlugin::Main() {
	FILE_LOG(logINFO) << "TimPlugin:: Starting plugin.\n";

	while (_plugin->state != IvpPluginState_error) {
		if (IsPluginState(IvpPluginState_registered))
		{

			// Load the TIM from the map file if it is new.
			if (_isTimFileNew)
			{				
				lock_guard<mutex> lock(_cfgLock);	
				PLOG(logINFO)<<"Reading new TIM file ...";
				//reset map update indicator
				_isTimFileNew = false;
				//Update the TIM message with XML from map file
				_timMsgPtr = readTimFile( _mapFile);
			}	
			while (_timMsgPtr && isTimActive(_timMsgPtr)) 
			{ 
				lock_guard<mutex> lock(_cfgLock);
				uint64_t sendFrequency = _frequency;	

				if(_isTimUpdated){
					PLOG(logINFO) <<"TimPlugin:: _isTimUpdated via Post request: "<< _isTimUpdated<<endl;
					//reset TIM update indicator
					_isTimUpdated = false;
				}

				if (_timMsgPtr)
				{
					PLOG(logINFO) << "timMsg XML to send: " << *_timMsgPtr << std::endl;
					TimEncodedMessage timEncMsg;
					timEncMsg.initialize(*_timMsgPtr);
					PLOG(logINFO) << "encoded timEncMsg: " << timEncMsg << std::endl;
					timEncMsg.set_flags(IvpMsgFlags_RouteDSRC);
					timEncMsg.addDsrcMetadata(0x8003);
					routeable_message *rMsg = dynamic_cast<routeable_message *>(&timEncMsg);
					if (rMsg) BroadcastMessage(*rMsg);						
				}

				//Make sure send frequency configuration is positive, otherwise set to default 1000 milliseconds
				sendFrequency = sendFrequency > 0 ? sendFrequency: 1000;
				//Sleep sendFrequency for every attempt to broadcast TIM
				this_thread::sleep_for(chrono::milliseconds(sendFrequency));
			}
		}
		this_thread::sleep_for(chrono::milliseconds(500));
	}

	return (EXIT_SUCCESS);
}

	


} /* namespace */

int main(int argc, char *argv[]) {
	return run_plugin < TimPlugin::TimPlugin > ("TimPlugin", argc, argv);
}
