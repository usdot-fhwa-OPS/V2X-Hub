/*
 * CswPlugin.h
 *
 *  Created on: October 25, 2017
 *      Author: zinkg
 */

#ifndef CSWPLUGIN_H_
#define CSWPLUGIN_H_

#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <time.h>
#include <sys/time.h>
#include <atomic>
#include <tmx/tmx.h>
#include <tmx/IvpPlugin.h>
#include <tmx/messages/IvpJ2735.h>
#include <GeoVector.h>



#include "PluginUtil.h"


#include "PluginClient.h"

#include <ApplicationMessage.h>
#include <ApplicationDataMessage.h>

#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <tmx/messages/auto_message.hpp>

using boost::property_tree::ptree;

using namespace std;

using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;


#define INPUTSREADY "Have Map/Spat/Veh"
#define INGRESSREGION "Ingress Region"
#define EGRESSREGION "Egress Region"



struct ZoneInfo
{
	// ID of the zone the vehicle is in.  0 indicates no zone.
	int ZoneId;

	// The current speed of the vehicle in MPH.
	float Speed_mph;

	// The time the ZoneId was last updated.
	// Note that if a 0 is received for ZoneId, this time will not be set unless
	// it has been longer than a configurable duration.
	uint64_t LastUpdateTime;
};

namespace CswPlugin {


class CswPlugin: public PluginClient {
public:
	CswPlugin(std::string);
	virtual ~CswPlugin();
	int Main();
protected:

	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	//void OnMessageReceived(IvpMessage *msg);
	void OnStateChange(IvpPluginState state);

	// Message handler functions
	void HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg);

	void UpdateVehicleInZone(int32_t vehicleId, int zoneId, float speed_mph);
	void RemoveOldVehicles();
	int GetHighestPriorityZoneId(unsigned int speedLimit_mph);
	void SetStatusForVehiclesInZones();


	bool LoadTim(TravelerInformation *tim, const char *mapFile);
	void TestFindRegion();

private:

	// For each vehicle ID, store what zone they are in.
	map<int32_t, ZoneInfo> _vehicleInZone;

	pthread_mutex_t _settingsMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t _timMutex = PTHREAD_MUTEX_INITIALIZER;

	uint64_t _frequency = 0;
	uint64_t _snapInterval = 0;
	uint64_t _vehicleTimeout = 1000;

	TravelerInformation _tim;

	mutex _mapFileLock;
	string _mapFile;
	atomic<bool> _isMapFileNew{false};
	bool _isTimLoaded = false;
	unsigned int _speedLimit = 0;
	int _lastMsgIdSent = -1;

	

};
}
#endif
