#include "TimPlugin.h"
#include <WGS84Point.h>
#include "TimeHelper.h"
#include "XmlCurveParser.h"
#include <tmx/messages/IvpDmsControlMsg.h>
#include <tmx/j2735_messages/TravelerInformationMessage.hpp>




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

		// xml parser setup 
	    std::lock_guard<mutex> lock(_cfgLock);

		if (GetConfigValue<string>("MapFile", _mapFile))
			_isMapFileNew = true;
	
        GetConfigValue<string>("Start_Broadcast_Date", _startDate);
        GetConfigValue<string>("Stop_Broadcast_Date", _stopDate);
        GetConfigValue<string>("Start_Broadcast_Time", _startTime);
        GetConfigValue<string>("Stop_Broadcast_Time", _stopTime);
		GetConfigValue<string>("WebServiceIP",webip);
		GetConfigValue<uint16_t>("WebServicePort",webport);

		tmpTIM.open("/tmp/tmpTIM.xml", std::ofstream::out);	

		std::thread webthread(&TimPlugin::StartWebService,this);
		webthread.detach(); // wait for the thread to finish 
}

TimPlugin::~TimPlugin() {

	tmpTIM.close();
}

void TimPlugin::TimRequestHandler(QHttpEngine::Socket *socket)
{

	// should read from the websocket and parse 
	auto router = QSharedPointer<OpenAPI::OAIApiRouter>::create();
	QString st; 
	while(socket->bytesAvailable()>0)
	{	
		st.append(socket->readAll());
	}
	QByteArray array = st.toLocal8Bit();

	char* _cloudUpdate = array.data(); // would be the cloud update packet, needs parsing
 

	std::stringstream ss;
	ss << _cloudUpdate;

	ptree ptr;

	// Catch XML parse exceptions 
	try { 
		read_xml(ss,ptr);

		lock_guard<mutex> lock(_cfgLock);
		BOOST_FOREACH(auto &n, ptr.get_child("timdata"))
		{
			std::string labeltext = n.first;

			if(labeltext == "starttime")
				_startTime = n.second.get_value<std::string>();
			
			if(labeltext == "stoptime")
				_stopTime = n.second.get_value<std::string>();

			if(labeltext == "startdate")
				_startDate = n.second.get_value<std::string>();

			if(labeltext == "stopdate")
				_stopDate = n.second.get_value<std::string>();

			if(labeltext == "timupdate"){
				tmpTIM<<n.second.get_value<std::string>();
				_mapFile = "/tmp/tmpTIM.xml";
				_isMapFileNew = true;
			}

		}
		writeResponse(QHttpEngine::Socket::Created, socket);
	}
	catch( const ptree_error &e ) {
		PLOG(logERROR) << "Error parsing file: " << e.what() << std::endl;
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

    QObject::connect(handler.data(), &OpenAPI::OAIApiRequestHandler::requestReceived, [&](QHttpEngine::Socket *socket) {

		this->TimRequestHandler(socket);
    });

    QHttpEngine::Server server(handler.data());

    if (!server.listen(address, port)) {
        qCritical("TimPlugin:: Unable to listen on the specified port.");
        return 1;
    }
	PLOG(logDEBUG4)<<"TimPlugin:: Started web service";
	return a.exec();

}


void TimPlugin::UpdateConfigSettings() {


	lock_guard<mutex> lock(_cfgLock);
	
	GetConfigValue<uint64_t>("Frequency", _frequency);
	
	
	if (GetConfigValue<string>("MapFile", _mapFile))
		_isMapFileNew = true;
	
	
	GetConfigValue<string>("Start_Broadcast_Date", _startDate);
	GetConfigValue<string>("Stop_Broadcast_Date", _stopDate);
	GetConfigValue<string>("Start_Broadcast_Time", _startTime);
	GetConfigValue<string>("Stop_Broadcast_Time", _stopTime);
	GetConfigValue<string>("WebServiceIP",webip);
	GetConfigValue<uint16_t>("WebServicePort",webport);
	
}

void TimPlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void TimPlugin::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);


	if (state == IvpPluginState_registered) {
		UpdateConfigSettings();
	}
}



bool TimPlugin::TimDuration()
{
	PLOG(logDEBUG)<<"TimPlugin:: Reached in TimDuration";

	string _endTime = ("23:59:59");

	istringstream startTimDate(_startDate);
        istringstream startTimTime(_startTime);
        istringstream stopTimTime(_stopTime);

	struct tm date_start;
	startTimDate >> get_time( &date_start, "%m-%d-%Y" );
	time_t secondsStartDate = mktime( & date_start );

	ostringstream lastTimTime_;
        lastTimTime_ << _stopDate << " " << _endTime;
        auto lastTime = lastTimTime_.str();

        istringstream stopTimDate(lastTime);

	struct tm date_stop;
	stopTimDate >> get_time( &date_stop, "%m-%d-%Y %H:%M:%S" );
	time_t secondsStopDate = mktime( & date_stop );

        auto t = time(nullptr);
        auto tm = *localtime(&t);

        ostringstream oss1;
        oss1 << put_time(&tm, "%m-%d-%Y");
        auto _currentTimDate = oss1.str();

	istringstream currentTimDate(_currentTimDate);

	struct tm date_current;
	currentTimDate >> get_time( &date_current, "%m-%d-%Y" );
	time_t secondsCurrentDate = mktime( & date_current );

        ostringstream oss2;
        oss2 << put_time(&tm, "%H:%M:%S");
        auto _currentTimTime = oss2.str();

	ostringstream currentTimTime_;
        currentTimTime_ << _currentTimDate << " " << _currentTimTime;
        auto currentTime = currentTimTime_.str();

        ostringstream startTimTime_;
        startTimTime_ << _currentTimDate << " " << _startTime;
        auto StartTime = startTimTime_.str();

        ostringstream stopTimTime_;
        stopTimTime_ << _currentTimDate << " " << _stopTime;
        auto StopTime = stopTimTime_.str();

        istringstream currentTimTime(currentTime);
        istringstream StartTimTime(StartTime);
        istringstream StopTimTime(StopTime);

	struct tm time_current;
	currentTimTime >> get_time( &time_current, "%m-%d-%Y %H:%M:%S" );
	time_t secondsCurrentTime = mktime( & time_current );

	struct tm time_start;
	StartTimTime >> get_time( &time_start, "%m-%d-%Y %H:%M:%S" );
	time_t secondsStartTime = mktime( & time_start );

	struct tm time_stop;
	StopTimTime >> get_time( &time_stop, "%m-%d-%Y %H:%M:%S" );
	time_t secondsStopTime = mktime( & time_stop );

	if ((secondsStartDate <= secondsCurrentDate) && (secondsCurrentDate <= secondsStopDate) && (secondsStartTime <= secondsCurrentTime) && (secondsCurrentTime <= secondsStopTime)) {
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

int TimPlugin::Main() {
	FILE_LOG(logINFO) << "TimPlugin:: Starting plugin.\n";

	uint64_t updateFrequency = 24 * 60 * 60 * 1000;
	uint64_t lastUpdateTime = 0;

	uint64_t lastSendTime = 0;
	string mapFileCopy;
	while (_plugin->state != IvpPluginState_error) {

		while (TimDuration()) {

			if (IsPluginState(IvpPluginState_registered))
			{
				uint64_t sendFrequency = _frequency;

				// Load the TIM from the map file if it is new.
				//cout<<"TimPlugin:: isMAPfileNEW  "<<_isMapFileNew<<endl;
				if (_isMapFileNew)
				{
					{
						//lock_guard<mutex> lock(_cfgLock);
						//mapFileCopy = _mapFile;
						_isMapFileNew = false;
					}
					if (_isTimLoaded)
						ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_TravelerInformation, &_tim);
					_isTimLoaded = LoadTim(&_tim, _mapFile.c_str());
				}

				uint64_t time = TimeHelper::GetMsTimeSinceEpoch();

				if (_isTimLoaded && (time - lastUpdateTime) > updateFrequency)
				{
					lastUpdateTime = time;
					if (_isTimLoaded)
					{
						DsrcBuilder::SetPacketId(&_tim);
						DsrcBuilder::SetStartTimeToYesterday(_tim.dataFrames.list.array[0]);
					}
				}

				// Send out the TIM at the frequency read from the configuration.
				if (_isTimLoaded && sendFrequency > 0 && (time - lastSendTime) > sendFrequency)
				{

					lastSendTime = time;
					TimMessage timMsg(_tim);

					TimEncodedMessage timEncMsg;
					timEncMsg.initialize(timMsg);

					timEncMsg.set_flags(IvpMsgFlags_RouteDSRC);
					timEncMsg.addDsrcMetadata(172, 0x8003);

					routeable_message *rMsg = dynamic_cast<routeable_message *>(&timEncMsg);
					if (rMsg) BroadcastMessage(*rMsg);
				}
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
