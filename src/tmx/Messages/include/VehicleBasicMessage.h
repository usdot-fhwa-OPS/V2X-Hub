/*
 * VehicleBasicMessage.h
 *
 *  Created on: Apr 8, 2016
 *      Author: ivp
 */

#ifndef INCLUDE__VEHICLEBASICMESSAGE_H_
#define INCLUDE__VEHICLEBASICMESSAGE_H_

#include <tmx/messages/message.hpp>
#include "MessageTypes.h"
#include "Units.h"
#include "VehicleParameterEnumTypes.h"

namespace tmx {
namespace messages {

/**
 * VehicleBasicMessage is the message type used to transmit information from the vehicle through TMX core.
 * It defines the message type and sub type and all data members.
 */
class VehicleBasicMessage : public tmx::message
{
public:
	VehicleBasicMessage() {}
	VehicleBasicMessage(const tmx::message_container_type &contents): tmx::message(contents) {}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_VEHICLE_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_BASIC_STRING;

	/// The gear shift position.
	std_attribute(this->msg, vehicleparam::GearState, GearPosition, vehicleparam::GearState::GearUnknown, )

	/// Indicates whether the brake is currently applied.
	std_attribute(this->msg, bool, BrakeApplied, false, )

	/// The speed of the vehicle in meters per second
	std_attribute(this->msg, double, Speed_mps, 0, )

	/// The turn signal position.
	std_attribute(this->msg, vehicleparam::TurnSignalState, TurnSignalPosition, vehicleparam::TurnSignalState::SignalUnknown, )

	/// The front door status
	std_attribute(this->msg, bool, FrontDoorsOpen, false, );

	/// The rear door status
	std_attribute(this->msg, bool, RearDoorsOpen, false, );

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

} /* namespace messages */
} /* namespace tmx */

#endif /* INCLUDE__VEHICLEBASICMESSAGE_H_ */
