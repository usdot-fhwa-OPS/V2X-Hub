/*
 * @file TargetAcquisitionMessage.h
 *
 *  Created on: May 26, 2016
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_REMOTEVEHICLEMESSAGE_H_
#define INCLUDE_REMOTEVEHICLEMESSAGE_H_

#include <tmx/messages/message.hpp>
#include "MessageTypes.h"
#include "Units.h"

namespace tmx {
namespace messages {

/**
 * The remote vehicle message is used for notifications of other vehicles being tracked in the vicinity.
 */
class RemoteVehicleMessage: public tmx::message
{
public:
	RemoteVehicleMessage(): tmx::message() {}
	RemoteVehicleMessage(const tmx::message &contents): tmx::message(contents) {}
	virtual ~RemoteVehicleMessage() {}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_VEHICLE_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_REMOTE_STRING;


	/**
	 * A unique identifier for the vehicle
	 */
	std_attribute(this->msg, uint64_t, VehicleID, -1, )

	/**
	 * The compass heading of the remote vehicle
	 */
	std_attribute(this->msg, double, Heading, 0, );

	/**
	 * The lateral distance to the remote vehicle in millimeters
	 */
	std_attribute(this->msg, double, LateralSeparation, 0, );

	/**
	 * The longitudinal distance to the remote vehicle in millimeters
	 */
	std_attribute(this->msg, double, LongitudinalSeparation, 0, )

	/**
	 * Number of messages received for this vehicle
	 */
	std_attribute(this->msg, unsigned short, RxCount, 0, );

	/**
	 * The sequence number for the last message
	 */
	std_attribute(this->msg, unsigned short, SeqNumber, 0, );

	/**
	 * The relative signal strength indicator
	 */
	std_attribute(this->msg, int, RSSI, 0, );

	/**
	 * The timestamp (ms since Epoch) when the remote vehicle last changed
	 */
	std_attribute(this->msg, uint64_t, LastChanged, 0, if (value > 0));

	/**
	 * The timestamp (ms since Epoch) when remote vehicle data was last received
	 */
	std_attribute(this->msg, uint64_t, LastReceived, 0, if (value > 0));

	/**
	 * The timestamp (ms since Epoch) after which this remote vehicle information should not be used
	 */
	std_attribute(this->msg, uint64_t, ExpiryTime, 0, if (value > 0));

	/**
	 * Is the target remote vehicle at the same elevation
	 */
	std_attribute(this->msg, bool, IsSameElevation, false, );

	/**
	 * Is the target remote vehicle at a different elevation
	 */
	std_attribute(this->msg, bool, IsDifferentElevation, false, );

	/**
	 * Is the target remote vehicle moving in a similar direction
	 */
	std_attribute(this->msg, bool, IsSimilarDirection, false, );

	/**
	 * Is the target remote vehicle moving in the opposite direction
	 */
	std_attribute(this->msg, bool, IsOppositeDirection, false, );

	/**
	 * Is the target remote vehicle moving toward
	 */
	std_attribute(this->msg, bool, IsClosing, false, );

	/**
	 * Is the target remote vehicle moving away
	 */
	std_attribute(this->msg, bool, IsReceding, false, );

	/**
	 * Is the target remote vehicle ahead
	 */
	std_attribute(this->msg, bool, IsAhead, false, );

	/**
	 * Is the target remote vehicle behind
	 */
	std_attribute(this->msg, bool, IsBehind, false, );

	/**
	 * Is the target remote vehicle in the same lane
	 */
	std_attribute(this->msg, bool, IsSameLane, false, );

	/**
	 * Is the target remote vehicle in a near lane
	 */
	std_attribute(this->msg, bool, IsNearLane, false, );

	/**
	 * Is the target remote vehicle in a far lane
	 */
	std_attribute(this->msg, bool, IsFarLane, false, );

	/**
	 * Is the target remote vehicle to the left
	 */
	std_attribute(this->msg, bool, IsLeft, false, );

	/**
	 * Is the target remote vehicle to the right
	 */
	std_attribute(this->msg, bool, IsRight, false, );

	/**
	 * Is the target remote vehicle on course to cross
	 */
	std_attribute(this->msg, bool, IsXing, false, );

	/**
	 * Remote vehicle latitude
	 */
	std_attribute(this->msg, double, Latitude, 0.0, );

	/**
	 * Remote vehicle longitude
	 */
	std_attribute(this->msg, double, Longitude, 0.0, );

	/**
	 * Remote vehicle speed in meters per second
	 */
	std_attribute(this->msg, double, Speed_mps, 0.0, );

	/**
	 * Remote vehicle acceleration along the direction of travel
	 */
	std_attribute(this->msg, double, LongitudinalAcceleration, 0.0, );

	/**
	 * Remote vehicle acceleration orthogonal to direction of travel
	 */
	std_attribute(this->msg, double, LateralAcceleration, 0.0, );

	// Converter methods for MPH and KPH
	inline double get_Speed_mph()
	{
		return this->get_Speed_mps() * Units::MPH_PER_MPS;
	}

	inline void set_Speed_mph(double mph)
	{
		this->set_Speed_mps(mph * Units::MPS_PER_MPH);
	}

	inline double get_Speed_kph()
	{
		return this->get_Speed_mps() * Units::KPH_PER_MPS;
	}

	inline void set_Speed_kph(double kph)
	{
		this->set_Speed_mps(kph * Units::MPS_PER_KPH);
	}
};

} /* End namespace messages */
} /* End namespace tmx */

#endif /* INCLUDE_REMOTEVEHICLEMESSAGE_H_ */
