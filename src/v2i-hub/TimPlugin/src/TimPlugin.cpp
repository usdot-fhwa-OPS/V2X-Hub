#include "TimPlugin.h"
#include <WGS84Point.h>
#include "Clock.h"
#include "XmlCurveParser.h"
#include <tmx/messages/IvpDmsControlMsg.h>

using namespace std;
using namespace tmx::messages;
using namespace tmx::utils;
using namespace xercesc;
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
		tmx::message_container_type container;
		container.load<XML>(ss);		
		_timMsgPtr = std::make_shared<TimMessage>();
		_timMsgPtr->set_contents(container.get_storage().get_tree());
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
		if ( boost::filesystem::exists( _mapFile ) ){
            _isMapFileNew = true;
			PLOG(logINFO) << "Loading MapFile " << _mapFile << "." << std::endl;
        }
		else {
			PLOG(logWARNING) << "MapFile " << _mapFile << " does not exist!" << std::endl;
		}

	}
	
	GetConfigValue<string>("Start_Broadcast_Date", _startDate);
	GetConfigValue<string>("Stop_Broadcast_Date", _stopDate);
	GetConfigValue<string>("Start_Broadcast_Time", _startTime);
	GetConfigValue<string>("Stop_Broadcast_Time", _stopTime);
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

bool TimPlugin::TimDuration()
{
	PLOG(logDEBUG)<<"TimPlugin:: Reached in TimDuration";

	ostringstream firstTimTime_;
    firstTimTime_ << _startDate << " " << _startTime;
    auto firstTime = firstTimTime_.str();

	ostringstream lastTimTime_;
    lastTimTime_ << _stopDate << " " << _stopTime;
    auto lastTime = lastTimTime_.str();

	istringstream stopTimTime(lastTime);
	istringstream startTimTime(firstTime);
	
	// start time in seconds
	struct tm date_start;
	startTimTime >> get_time( &date_start, "%m-%d-%Y %H:%M:%S" );
	time_t secondsStart = mktime( & date_start );
	PLOG(logDEBUG) << "START : " << date_start.tm_mon << "-" << date_start.tm_mday << "-" 
		<< date_start.tm_year << " " << date_start.tm_hour << ":" << date_start.tm_min << ":" 
		<< date_start.tm_sec << std::endl;
	// stop time in seconds
	struct tm date_stop;
	stopTimTime >> get_time( &date_stop, "%m-%d-%Y %H:%M:%S" );
	PLOG(logDEBUG) << "STOP : " << date_stop.tm_mon << "-" << date_stop.tm_mday << "-" 
		<< date_stop.tm_year << " " << date_stop.tm_hour << ":" << date_stop.tm_min << ":" 
		<< date_stop.tm_sec << std::endl;
	time_t secondsStop = mktime( & date_stop );

	// Current Time in seconds
	auto t = time(nullptr);
	struct tm tm;
	localtime_r(&t, &tm);
	ostringstream oss1;
	oss1 << put_time(&tm, "%m-%d-%Y %H:%M:%S");
	auto _currentTimTime = oss1.str();
	istringstream currentTimTime(_currentTimTime);
	struct tm date_current;
	currentTimTime >> get_time( &date_current, "%m-%d-%Y %H:%M:%S" );
	PLOG(logDEBUG) << "CURRENT : " << date_current.tm_mon << "-" << date_current.tm_mday << "-" 
		<< date_current.tm_year << " " << date_current.tm_hour << ":" << date_current.tm_min << ":" 
		<< date_current.tm_sec << std::endl;
	time_t secondsCurrent = mktime( & date_current );

	PLOG(logDEBUG) << "Start : " << secondsStart << " Stop : " << secondsStop << 
		" Current : " << secondsCurrent << std::endl;
	if ( secondsStart <= secondsCurrent && secondsCurrent <= secondsStop) {
		return true;
	} else {
		return false;
	}
}

bool TimPlugin::TimDuration(std::shared_ptr<TimMessage> TimMsg)
{
	PLOG(logINFO)<<"TimPlugin:: Reached in TimDuration upcon receiving TIM message.";
	lock_guard<mutex> lock(_cfgLock);
	auto timPtr = TimMsg->get_j2735_data();
	//startTime unit of minute
	auto startTime = timPtr->dataFrames.list.array[0]->startTime;
	if(startTime >= 527040)
	{
		PLOG(logERROR) << "Invalid startTime." << std::endl;
		return false;
	}
	//Duration is unit of minute
	auto duration = timPtr->dataFrames.list.array[0]->duratonTime; 
	bool isPersist = false;
	if(duration >= 32000)
	{
		PLOG(logERROR) << "Duration = 32000, ignore stop time." << std::endl;
		isPersist = true;
	}

	//Get year start UTC in seconds
	auto t = time(nullptr);
	struct tm* timeInfo = gmtime(&t);
	ostringstream currentYearStartOS;
	currentYearStartOS << (timeInfo->tm_year+1900) <<"-01-01T00:00:00.000Z";
	struct tm currentYearStartTimeInfo;
	istringstream currentYearStartIS(currentYearStartOS.str());
	currentYearStartIS >> get_time( &currentYearStartTimeInfo, "%Y-%m-%dT%H:%M:%S" );
	PLOG(logINFO) << "Year Start : " << (currentYearStartTimeInfo.tm_mon + 1) << "-" << currentYearStartTimeInfo.tm_mday << "-" 
		<< (currentYearStartTimeInfo.tm_year + 1900) << " " << currentYearStartTimeInfo.tm_hour << ":" << currentYearStartTimeInfo.tm_min << ":" 
		<< currentYearStartTimeInfo.tm_sec << std::endl;
	time_t secondsYearStart = mktime( &currentYearStartTimeInfo );
	
	//Start Time in seconds
	time_t secondsStart = secondsYearStart + startTime * 60;
	//Stop Time in seconds
	time_t secondsStop = secondsStart + duration * 60;

	// Current UTC Time in seconds;
	auto secondsCurrent = time(nullptr);

	//Comparing current time with start and end time 
	PLOG(logINFO) << "Year Start(s): " << secondsYearStart << " Broadcast Start(s):" << secondsStart << " Broadcast Stop(s): " << secondsStop << 
		" Current(s):" << secondsCurrent << " Elpased minutes since year start(min):" << ((secondsCurrent-secondsYearStart)/60.0)<< std::endl;
	if ( secondsStart <= secondsCurrent && (secondsCurrent <= secondsStop || isPersist)) {
		return true;
	} else {
		return false;
	}
}

bool TimPlugin::LoadTim(TravelerInformation *tim, const char *mapFile)
{
	memset(tim, 0, sizeof(TravelerInformation));
	// J2735 packet header.
	//tim->msgID = DSRCmsgID_travelerInformation;

	DsrcBuilder::SetPacketId(tim);
	// Data Frame (1 of 1).
	XmlCurveParser curveParser;

	// Read the curve file, which creates and populates the data frame of the TIM.
	if (!curveParser.ReadCurveFile(mapFile, tim))
		return false;

	_speedLimit = curveParser.SpeedLimit;

	PluginUtil::SetStatus<unsigned int>(_plugin, "Speed Limit", _speedLimit);

	return true;
}

bool TimPlugin::LoadTim(std::shared_ptr<TimMessage> TimMsg, const char *mapFile)
{
	std::ifstream in = std::ifstream(mapFile, ios_base::in);
	if(in && in.is_open())
	{
		try
		{
			std::stringstream ss;
			ss << in.rdbuf();
			in.close();

			tmx::message_container_type container;
			container.load<XML>(ss);
			TimMsg->set_contents(container.get_storage().get_tree());
			PLOG(logINFO) << "Loaded MapFile and updated TIM." << std::endl;
			return true;
		}
		catch(const std::exception& e)
		{
			PLOG(logERROR)<<"Cannot read file " << mapFile<<std::endl;
		}		
	}else{
		PLOG(logERROR)<<"Cannot find file " << mapFile<<std::endl;
	}
	return false;
}

int TimPlugin::Main() {
	FILE_LOG(logINFO) << "TimPlugin:: Starting plugin.\n";

	while (_plugin->state != IvpPluginState_error) {
		if (IsPluginState(IvpPluginState_registered))
		{
			while (_timMsgPtr && TimDuration(_timMsgPtr)) 
			{ 
				lock_guard<mutex> lock(_cfgLock);
				uint64_t sendFrequency = _frequency;

				// Load the TIM from the map file if it is new.
				if (_isMapFileNew)
				{					
					PLOG(logINFO)<<"TimPlugin:: isMAPfileNEW  "<<_isMapFileNew<<endl;
					//reset map update indicator
					_isMapFileNew = false;
					//Update the TIM message with XML from map file
					_timMsgPtr = std::make_shared<TimMessage>();
					_isTimLoaded = LoadTim(_timMsgPtr, _mapFile.c_str());
					if(!_isTimLoaded)
					{
						_timMsgPtr = nullptr;
					}
				}		

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
