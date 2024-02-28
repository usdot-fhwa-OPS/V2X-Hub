/*
 * DatabaseMessage.h
 *
 *  Created on: February 27, 2024
 *      Author: Ishan Joshi
 */

#ifndef INCLUDE_DATABASEMESSAGE_H_
#define INCLUDE_DATABASEMESSAGE_H_

#if __cplusplus > 199711L
	#include <tmx/messages/message.hpp>
	#include <tmx/TmxApiMessages.h>
	#include "MessageTypes.h"
#else
	#include <tmx/messages/faux_message.hpp>
#endif

#include "Units.h"

namespace tmx {
namespace messages {

/**
 * A message to send aggregated data from the Phantom Traffic Plugin to the Database Plugin.
 */
class DatabaseMessage : public tmx::message
{
public:
	DatabaseMessage() {} // Required no-argument constructor.

#if __cplusplus > 199711L
	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_DECODED_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_BASIC_STRING;
#endif

	/* Constructor for message */
	DatabaseMessage(uint64_t timestamp, 
					int number_of_vehicles_in_road_segment,
					double average_speed_of_vehicles_in_road_segment,
					double speed_limit_of_road_segment,
					double throughput_of_road_segment)
	{
		set_Timestamp(timestamp);
		set_NumberOfVehiclesInRoadSegment(number_of_vehicles_in_road_segment);
		set_AverageSpeedOfVehiclesInRoadSegment(average_speed_of_vehicles_in_road_segment);
		set_SpeedLimitOfRoadSegment(speed_limit_of_road_segment);
		set_ThroughputOfRoadSegment(throughput_of_road_segment);
	}

	/* The std_attribute macro genreates get_ and set_ functions for each attribute 
	 * Looking at the macro in faux_message.hpp, the last value "0" isn't used.
	 */

	// Std Attribute for timestamp
	std_attribute(this->msg, uint64_t, Timestamp, 0, )

	// Std Attribute for number of vehicles in road segment
	std_attribute(this->msg, int, NumberOfVehiclesInRoadSegment, 0, )

	// Std Attribute for average speed of vehicles in road segment
	std_attribute(this->msg, double, AverageSpeedOfVehiclesInRoadSegment, 0.0, )

	// Std Attribute for speed limit of road segment
	std_attribute(this->msg, double, SpeedLimitOfRoadSegment, 0.0, )

	// Std Attribute for throughput of road segment
	std_attribute(this->msg, double, ThroughputOfRoadSegment, 0.0, )

};

} /* namespace messages */
} /* namespace tmx */


#endif /* INCLUDE_DATABASEMESSAGE_H_ */
