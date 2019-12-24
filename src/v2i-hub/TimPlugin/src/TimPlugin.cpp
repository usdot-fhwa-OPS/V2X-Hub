#include "TimPlugin.h"
#include <WGS84Point.h>
#include "TimeHelper.h"
#include "XmlCurveParser.h"
#include <tmx/messages/IvpDmsControlMsg.h>
#include <tmx/j2735_messages/TravelerInformationMessage.hpp>

using namespace std;
using namespace tmx::messages;
using namespace tmx::utils;

namespace TimPlugin {




/**
 * Construct a new TimPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes
 */
TimPlugin::TimPlugin(string name) :
		PluginClient(name) {

        std::lock_guard<mutex> lock(_cfgLock);
        GetConfigValue("Start_Broadcast_Date", _startDate);
        GetConfigValue("Stop_Broadcast_Date", _stopDate);
        GetConfigValue("Start_Broadcast_Time", _startTime);
        GetConfigValue("Stop_Broadcast_Time", _stopTime);

}

TimPlugin::~TimPlugin() {
}

void TimPlugin::UpdateConfigSettings() {

	GetConfigValue<uint64_t>("Frequency", _frequency);

	{
		lock_guard<mutex> lock(_mapFileLock);
		if (GetConfigValue<string>("MapFile", _mapFile))
			_isMapFileNew = true;
	}
	
        std::lock_guard<mutex> lock(_cfgLock);
	GetConfigValue("Start_Broadcast_Date", _startDate);
	GetConfigValue("Stop_Broadcast_Date", _stopDate);
	GetConfigValue("Start_Broadcast_Time", _startTime);
	GetConfigValue("Stop_Broadcast_Time", _stopTime);
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
	PLOG(logDEBUG)<<"Reached in TimDuration";

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
		PLOG(logERROR)<<"TimDuration is True";
		return true;
	} else {
		PLOG(logdEBUG)<<"TimDuration is False";
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

	std::cout << "Loading curve file: " << mapFile << std::endl;

	// Read the curve file, which creates and populates the data frame of the TIM.
	if (!curveParser.ReadCurveFile(mapFile, tim))
		return false;

	// Verify that a single data frame was added by the parser.
	//if (tim->dataFrameCount == NULL || *tim->dataFrameCount != 1)
	//	return false;

	std::cout << "TIM was created." << std::endl;

	_speedLimit = curveParser.SpeedLimit;

	PluginUtil::SetStatus<unsigned int>(_plugin, "Speed Limit", _speedLimit);

	return true;
}

int TimPlugin::Main() {
	FILE_LOG(logINFO) << "Starting plugin.";

	uint64_t updateFrequency = 24 * 60 * 60 * 1000;
	uint64_t lastUpdateTime = 0;

	uint64_t lastSendTime = 0;
	string mapFileCopy;

	while (_plugin->state != IvpPluginState_error) {

		while (TimDuration()) {

			PLOG(logDEBUG)<<"Reached TimPlugin::main";

			if (IsPluginState(IvpPluginState_registered))
			{
				pthread_mutex_lock(&_settingsMutex);
				uint64_t sendFrequency = _frequency;
				pthread_mutex_unlock(&_settingsMutex);

				// Load the TIM from the map file if it is new.

				if (_isMapFileNew)
				{
					{
						lock_guard<mutex> lock(_mapFileLock);
						mapFileCopy = _mapFile;
						_isMapFileNew = false;
					}

					pthread_mutex_lock(&_timMutex);
					if (_isTimLoaded)
						ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_TravelerInformation, &_tim);
					_isTimLoaded = LoadTim(&_tim, mapFileCopy.c_str());
					//xer_fprint(stdout, &asn_DEF_TravelerInformation, &_tim);
					//TestFindRegion();
					pthread_mutex_unlock(&_timMutex);
				}
				// Get system time in milliseconds.
				uint64_t time = TimeHelper::GetMsTimeSinceEpoch();

				// Update the start time of the TIM message if it is time.
				// Since the contents of the TIM change, the packet ID is also updated.
				// If the packet ID is not changed, a recipient may choose to ignore it.
				if (_isTimLoaded && (time - lastUpdateTime) > updateFrequency)
				{
					lastUpdateTime = time;
					pthread_mutex_lock(&_timMutex);
					if (_isTimLoaded)
					{
						std::cout << "Updating TIM start time." << std::endl;
						DsrcBuilder::SetPacketId(&_tim);
						DsrcBuilder::SetStartTimeToYesterday(_tim.dataFrames.list.array[0]);
					}
					pthread_mutex_unlock(&_timMutex);
				}

				// Send out the TIM at the frequency read from the configuration.
				if (_isTimLoaded && sendFrequency > 0 && (time - lastSendTime) > sendFrequency)
				{

					PLOG(logERROR)<<"Send TIM";
					lastSendTime = time;
					TimMessage timMsg(_tim);

					PLOG(logERROR)<<"Send TIM 2";

					PLOG(logERROR)<<timMsg;
					TimEncodedMessage timEncMsg;
					timEncMsg.initialize(timMsg);

					timEncMsg.set_flags(IvpMsgFlags_RouteDSRC);
					timEncMsg.addDsrcMetadata(172, 0x8003);

					routeable_message *rMsg = dynamic_cast<routeable_message *>(&timEncMsg);
					if (rMsg) BroadcastMessage(*rMsg);
				}
			}

		}
		usleep(50000);

	}

	return (EXIT_SUCCESS);
}
} /* namespace */

int main(int argc, char *argv[]) {
	return run_plugin < TimPlugin::TimPlugin > ("TimPlugin", argc, argv);
}
