#include "CswPlugin.h"
#include <WGS84Point.h>
#include "Clock.h"
#include "XmlCurveParser.h"
#include "VehicleLocate.h"
#include <tmx/messages/IvpDmsControlMsg.h>
#include <tmx/j2735_messages/TravelerInformationMessage.hpp>

using namespace std;
using namespace tmx::messages;
using namespace tmx::utils;

namespace CswPlugin {




/**
 * Construct a new EpcwPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes
 */
CswPlugin::CswPlugin(string name) :
		PluginClient(name) {

	AddMessageFilter < BsmMessage > (this, &CswPlugin::HandleBasicSafetyMessage);

	SubscribeToMessages();

}

CswPlugin::~CswPlugin() {
}

void CswPlugin::UpdateConfigSettings() {

	GetConfigValue<uint64_t>("Frequency", _frequency);

	{
		lock_guard<mutex> lock(_mapFileLock);
		if (GetConfigValue<string>("MapFile", _mapFile))
			_isMapFileNew = true;
	}

	GetConfigValue<uint64_t>("Snap Interval", _snapInterval);
	GetConfigValue<uint64_t>("Vehicle Timeout", _vehicleTimeout);

}

void CswPlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}


void CswPlugin::HandleBasicSafetyMessage(BsmMessage &msg,
		routeable_message &routeableMsg) {

PLOG(logDEBUG)<<"HandleBasicSafetyMessage";
	auto bsm = msg.get_j2735_data();

	int regionNumber;
	float speed_mph;
	int32_t vehicleId;

	if (VehicleLocate::ProcessBsmMessage(*bsm, &_tim, &regionNumber, &speed_mph, &vehicleId))
	{
		// Store the ID in a map, where the vehicle ID is the key, and the value is the speed
		// and zone/region where the vehicle is currently at.
		UpdateVehicleInZone(vehicleId, regionNumber, speed_mph);

		// Get the highest priority id for all vehicles that are speeding.
		int zoneId = GetHighestPriorityZoneId(_speedLimit);

		PLOG(logDEBUG)<<"Highest Priority Zone: " << zoneId;

		// Message ID is the ID of the message to display on the sign.
		// An ID of 0 blanks the sign.
		int msgId = 0;

		// Only show a message on the sign when in zones 1, 2, or 3.
		// Always show the same message (ID 3).  Otherwise the sign is blank.
		if (zoneId >= 1 && zoneId <= 3)
			msgId = 3;

		if (_lastMsgIdSent != msgId)
		{
			PLOG(logDEBUG)<<"Sending DMS Message ID: " <<msgId;
			_lastMsgIdSent = msgId;

			IvpMessage *actionMsg = ivpDmsCont_createMsg(msgId);
			if (actionMsg!=NULL)
			{
				ivp_broadcastMessage(_plugin, actionMsg);
				ivpMsg_destroy(actionMsg);

				PluginUtil::SetStatus<int>(_plugin, "DMS Message ID Sent", msgId);
			}
		}

		SetStatusForVehiclesInZones();
	}
}

void CswPlugin::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered) {
		UpdateConfigSettings();
	}
}


bool CswPlugin::LoadTim(TravelerInformation *tim, const char *mapFile)
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

void CswPlugin::TestFindRegion()
{
	if (_isTimLoaded)
	{
		WGS84Point point;
		point.Latitude = 42.2891;
		point.Longitude = -83.71933195;
		int regionNumber = VehicleLocate::FindRegion(&_tim, point, 0);
		std::cout << "In Region: " << regionNumber << std::endl;
	}
}

void CswPlugin::UpdateVehicleInZone(int32_t vehicleId, int zoneId, float speed_mph)
{
	uint64_t now = Clock::GetMillisecondsSinceEpoch();

	// If msgId is 0, the vehicle is removed from the map.
	// If msgId is not 0, the vehicle is either added or updated in the map.

	std::map<int32_t, ZoneInfo>::iterator it = _vehicleInZone.find(vehicleId);
	if (it == _vehicleInZone.end())
	{
		if (zoneId != 0)
		{
			PLOG(logDEBUG) << "Adding   Vehicle ID: " << vehicleId << ", zone: " << zoneId << ", speed: " << speed_mph;

			ZoneInfo zoneInfo;
			zoneInfo.ZoneId = zoneId;
			zoneInfo.Speed_mph = speed_mph;
			zoneInfo.LastUpdateTime = now;

			_vehicleInZone.insert(std::pair<int32_t, ZoneInfo>(vehicleId, zoneInfo));
		}
	}
	else
	{
		pthread_mutex_lock(&_settingsMutex);
		uint64_t snapInterval = _snapInterval;
		pthread_mutex_unlock(&_settingsMutex);

		if (zoneId == 0 && (now - it->second.LastUpdateTime) < snapInterval)
		{
			PLOG(logDEBUG) << "Removing Vehicle ID: " << vehicleId << ", zone: " << zoneId << ", speed: " << speed_mph;
			_vehicleInZone.erase(vehicleId);
		}
		else
		{
			if (it->second.ZoneId != zoneId || it->second.Speed_mph != speed_mph)
			{
				PLOG(logDEBUG) << "Updating Vehicle ID: " << vehicleId << ", zone: " << zoneId << ", speed: " << speed_mph;
				it->second.ZoneId = zoneId;
				it->second.Speed_mph = speed_mph;
			}
			it->second.LastUpdateTime = now;
		}
	}
}

// Remove any vehicles where a BSM has not been received within the timeout period.
void CswPlugin::RemoveOldVehicles()
{
	uint64_t now = Clock::GetMillisecondsSinceEpoch();

	pthread_mutex_lock(&_settingsMutex);
	uint64_t vehicleTimeout = _vehicleTimeout;
	pthread_mutex_unlock(&_settingsMutex);

	std::map<int32_t, ZoneInfo>::iterator it = _vehicleInZone.begin();

	while (it != _vehicleInZone.end())
	{
	    if (now - it->second.LastUpdateTime > vehicleTimeout)
	    {
	    	std::map<int32_t, ZoneInfo>::iterator toErase = it;
	       it++;
	       _vehicleInZone.erase(toErase);
	    }
	    else
	    {
	       it++;
	    }
	}
}

// Get the zone id with the highest priority of all vehicles that are speeding.
int CswPlugin::GetHighestPriorityZoneId(unsigned int speedLimit_mph)
{
	// Remove any old vehicles that have timed out.
	RemoveOldVehicles();

	// The lower the zone id, the higher the priority.

	int id = 9999;

	for (auto& kv : _vehicleInZone)
	{
		if (kv.second.ZoneId != 0 && id > kv.second.ZoneId && kv.second.Speed_mph > speedLimit_mph)
			id = kv.second.ZoneId;
	}

	return id == 9999 ? 0 : id;
}

void CswPlugin::SetStatusForVehiclesInZones()
{
	ostringstream ss;
	ss.precision(1);

	bool first = true;
	for (auto& kv : _vehicleInZone)
	{
		if (first)
			first = false;
		else
			ss << ", ";

		ss << "[ID: " << kv.first << ", Zone: " << kv.second.ZoneId << ", Speed: " << fixed << kv.second.Speed_mph << "]";
	}

	if (first)
		ss << "None";

	SetStatus<string>("Vehicles In Zones", ss.str());
}

int CswPlugin::Main() {
	FILE_LOG(logINFO) << "Starting plugin.";

	uint64_t updateFrequency = 24 * 60 * 60 * 1000;
	uint64_t lastUpdateTime = 0;

	uint64_t lastSendTime = 0;
	string mapFileCopy;

	while (_plugin->state != IvpPluginState_error) {
		
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
			uint64_t time = Clock::GetMillisecondsSinceEpoch();

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

				PLOG(logDEBUG)<<"Send TIM";
				lastSendTime = time;
				TimMessage timMsg(_tim);

				PLOG(logDEBUG)<<"Send TIM 2";

				PLOG(logDEBUG)<<timMsg;
				TimEncodedMessage timEncMsg;
				timEncMsg.initialize(timMsg);

				timEncMsg.set_flags(IvpMsgFlags_RouteDSRC);
				timEncMsg.addDsrcMetadata(0x8003);

				routeable_message *rMsg = dynamic_cast<routeable_message *>(&timEncMsg);
				if (rMsg) BroadcastMessage(*rMsg);
			}
		}

		usleep(50000);
	}

	return (EXIT_SUCCESS);
}
} /* namespace */

int main(int argc, char *argv[]) {
	return run_plugin < CswPlugin::CswPlugin > ("CswPlugin", argc, argv);
}
