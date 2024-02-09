//==========================================================================
// Name        : PhantomTrafficPlugin.cpp
// Author      : Battelle Memorial Institute
// Version     :
// Copyright   : Copyright (c) 2014 Battelle Memorial Institute. All rights reserved.
// Description : Example Plugin
//==========================================================================

#include "PluginClient.h"
#include "PluginDataMonitor.h"

#include <atomic>
#include <thread>
#include <DecodedBsmMessage.h>
#include <tmx/j2735_messages/MapDataMessage.hpp>

using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;

namespace PhantomTrafficPlugin
{

/**
 * This plugin observes vehicle counts in a slowdown region and slows down vehicles prior to it.
 */
class PhantomTrafficPlugin: public PluginClient
{
public:
	PhantomTrafficPlugin(std::string);
	virtual ~PhantomTrafficPlugin();
	int Main();
protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);

	void HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg);
	void HandleDecodedBsmMessage(DecodedBsmMessage &msg, routeable_message &routeableMsg);
	void HandleDataChangeMessage(DataChangeMessage &msg, routeable_message &routeableMsg);
private:
	std::atomic<uint64_t> _frequency{0};
	DATA_MONITOR(_frequency);   // Declares the
	std::atomic<uint64_t> vehicle_count; // vehicle count in the slowdown region
	vector<int32_t> vehicle_ids; // vehicle IDs in the slowdown region 
	std::mutex vehicle_ids_mutex; // mutex for vehicle IDs
};

/**
 * Construct a new PhantomTrafficPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes.
 */
PhantomTrafficPlugin::PhantomTrafficPlugin(string name): PluginClient(name)
{
	// The log level can be changed from the default here.
	FILELog::ReportingLevel() = FILELog::FromString("DEBUG");

	// Add a message filter and handler for each message this plugin wants to receive.
	AddMessageFilter<DecodedBsmMessage>(this, &PhantomTrafficPlugin::HandleDecodedBsmMessage);

	// This is an internal message type that is used to track some plugin data that changes
	AddMessageFilter<DataChangeMessage>(this, &PhantomTrafficPlugin::HandleDataChangeMessage);

	AddMessageFilter<MapDataMessage>(this, &PhantomTrafficPlugin::HandleMapDataMessage);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();

	vehicle_count = 0; // Set initial vehicle count to 0 upon creation of plugin.

	bool vehicle_count_status = SetStatus("VehicleCountInSlowdown", vehicle_count); // Initial vehicle count in slowdown region is 0
	bool speed_limit_status = SetStatus("SpeedLimit", 50.0); // Initial speed limit is 50km/h
}

PhantomTrafficPlugin::~PhantomTrafficPlugin()
{
}

void PhantomTrafficPlugin::UpdateConfigSettings()
{
	// Configuration settings are retrieved from the API using the GetConfigValue template class.
	// This method does NOT execute in the main thread, so variables must be protected
	// (e.g. using std::atomic, std::mutex, etc.).

	int instance;
	GetConfigValue("Instance", instance);

	GetConfigValue("Frequency", __frequency_mon.get());
	__frequency_mon.check();
}


void PhantomTrafficPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void PhantomTrafficPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
		SetStatus("ReceivedMaps", 0);
	}
}

void PhantomTrafficPlugin::HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg)
{
	static std::atomic<int> count {0};

	PLOG(logINFO) << "New MAP: " << msg;

	int mapCount = count;
	SetStatus("ReceivedMaps", mapCount);
}

void PhantomTrafficPlugin::HandleDecodedBsmMessage(DecodedBsmMessage &msg, routeable_message &routeableMsg)
{
	//PLOG(logDEBUG) << "Received Decoded BSM: " << msg;

	// Determine if location, speed, and heading are valid.
	bool isValid = msg.get_IsLocationValid() && msg.get_IsSpeedValid() && msg.get_IsHeadingValid();

	if (!isValid) 
	{
		PLOG(logDEBUG) << "Received BSM with invalid location, speed, or heading.";
		return;
	}

	// Print some of the BSM values.
	PLOG(logDEBUG) << "ID: " << msg.get_TemporaryId()
		<< ", Location: (" <<  msg.get_Latitude() << ", " <<  msg.get_Longitude() << ")"
		<< ", Speed: " << msg.get_Speed_kph() << "kph"
		<< ", Heading: " << msg.get_Heading() << "°";

	// Coordinates of slowdown region
	// Longitude = east-west (increases towards east,more negative towards west)
	// Latitude = south-north (increases north, more negative towards south)
	double top_left_long; // top left corner 
	double top_left_lat; // top left corner
	double top_right_long; // top right corner
	double top_right_lat; // top right corner
	double bottom_left_long; // bottom left corner
	double bottom_left_lat; // bottom left corner
	double bottom_right_long; // bottom right corner
	double bottom_right_lat; // bottom right corner


	// Coordinates of the vehicle
	double vehicle_long = msg.get_Longitude();
	double vehicle_lat = msg.get_Latitude();

	// Vehicle ID
	int32_t vehicle_id = msg.get_TemporaryId();

	// Lock the mutex
	std::lock_guard<std::mutex> lock(vehicle_ids_mutex);
	

	// Check if the vehicle is in the slowdown region.
	if (vehicle_long >= top_left_long && vehicle_long <= top_right_long && vehicle_lat >= bottom_left_lat && vehicle_lat <= top_left_lat)
	{
		// Add the vehicle to the list of vehicles being tracked if it's not already tracked
		if (!find(vehicle_ids.begin(), vehicle_ids.end(), vehicle_id) != vehicle_ids.end())
		{
			vehicle_ids.push_back(vehicle_id);
			PLOG(logDEBUG) << "Vehicle ID " << vehicle_id << " is now being tracked.";
			vehicle_count += 1;
			PLOG(logDEBUG) << "Vehicle count in slowdown region: " << vehicle_count;
		}
		// If the vehicle is already being tracked, do nothing
	}
	else // Vehicle is not in the slowdown region
	{
		// Remove the vehicle from the list of vehicles being tracked if it's being tracked
		if (find(vehicle_ids.begin(), vehicle_ids.end(), vehicle_id) != vehicle_ids.end())
		{
			vehicle_ids.erase(remove(vehicle_ids.begin(), vehicle_ids.end(), vehicle_id), vehicle_ids.end());
			PLOG(logDEBUG) << "Vehicle ID " << vehicle_id << " is no longer being tracked as it left the slowdown region.";
			vehicle_count -= 1;
			PLOG(logDEBUG) << "Vehicle count in slowdown region: " << vehicle_count;
		}
		// If the vehicle is not being tracked, do nothing
	}

	// The lock_guard automatically unlocks the mutex when it goes out of scope
}

// Example of handling
void PhantomTrafficPlugin::HandleDataChangeMessage(DataChangeMessage &msg, routeable_message &routeableMsg)
{
	PLOG(logINFO) << "Received a data change message: " << msg;

	PLOG(logINFO) << "Data field " << msg.get_untyped(msg.Name, "?") <<
			" has changed from " << msg.get_untyped(msg.OldValue, "?") <<
			" to " << msg.get_untyped(msg.NewValue, to_string(_frequency));
}

// Override of main method of the plugin that should not return until the plugin exits.
// This method does not need to be overridden if the plugin does not want to use the main thread.
int PhantomTrafficPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	double original_speed = 50.0; // km/h

	while (_plugin->state != IvpPluginState_error)
	{
		PLOG(logDEBUG4) << "Sleeping for 5 seconds" << endl;

		long long ms_to_sleep = 5000; // 5 seconds

		this_thread::sleep_for(chrono::milliseconds(ms_to_sleep));

		// Only do work if the plugin is registered
		if (_plugin->state == IvpPluginState_registered)
		{
			// Reduce speed to a minimum of 10km/h if 7 vehicles are in the slowdown region
			double new_speed = original_speed - (vehicle_count * (40. / 7.));  // 40km/h reduction for 7 vehicles

			// Ensure speed does not go below 10km/h
			if (new_speed < 10.0)
			{
				new_speed = 10.0;
			}

			// Print the new speed
			PLOG(logDEBUG) << "New speed: " << new_speed << "km/h";

			// Output new speed
			int msgPSID = api::msgPSID::basicSafetyMessage_PSID; // 0x20
			TimMessage timMsg(_tim);
			TimEncodedMessage timEncMsg;
			timEncMsg.initialize(timMsg);
			timEncMsg.set_flags(IvpMsgFlags_RouteDSRC);
			timEncMsg.addDsrcMetadata(msgPSID);

			routeable_message *rMsg = dynamic_cast<routeable_message *>(&timEncMsg);
			if (rMsg) BroadcastMessage(*rMsg);

			// Set status information for monitoring in the admin portal
			bool vehicle_count_status = SetStatus("VehicleCountInSlowdown", vehicle_count); // Vehicle count in slowdown region
			bool speed_limit_status = SetStatus("SpeedLimit", new_speed); 					// New speed limit
		}
	}

	PLOG(logINFO) << "Plugin terminating gracefully.";
	return EXIT_SUCCESS;
}

} /* namespace PhantomTrafficPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<PhantomTrafficPlugin::PhantomTrafficPlugin>("PhantomTrafficPlugin", argc, argv);
}
