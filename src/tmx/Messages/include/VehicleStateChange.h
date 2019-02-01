/*
 * VehicleStateChange.h
 *
 *  Created on: Jun 10, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_VEHICLESTATECHANGE_H_
#define INCLUDE_VEHICLESTATECHANGE_H_


#include <tmx/messages/message.hpp>
#include "MessageTypes.h"
#include "VehicleParameterEnumTypes.h"

namespace tmx {
namespace messages {

/**
 * ApplicationEventMessage is the message type used to send information messages about plugin status/activities.
 * It defines the message type and sub type and all data members.
 */
class VehicleStateChange : public tmx::message
{
public:
	VehicleStateChange() {}
	VehicleStateChange(const tmx::message_container_type &contents): tmx::message(contents) {}
	VehicleStateChange(std::string id, std::string timestamp, vehicleparam::VehicleParameter vehicleParamId,
			vehicleparam::VehicleParameterState preState, vehicleparam::VehicleParameterState postState) {
		set_Id(id);
		set_Timestamp(timestamp);
		set_VehicleParamId(vehicleParamId);
		set_PreState(preState);
		set_PostState(postState);
	}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_VEHICLE_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_STATECHANGE_STRING;

	std_attribute(this->msg, std::string, Id, "", )
	//Timestamp of the data change,
	std_attribute(this->msg, std::string, Timestamp, "", )
	//Id of the Vehicle Parameter being reported
    std_attribute(this->msg, vehicleparam::VehicleParameter, VehicleParamId, vehicleparam::VehicleParameter::ParameterNA, )
	//State of the parameter before the change,
    std_attribute(this->msg, vehicleparam::VehicleParameterState, PreState, vehicleparam::VehicleParameterState::StateNA, )
	//State of the parameter after the change,
    std_attribute(this->msg, vehicleparam::VehicleParameterState, PostState, vehicleparam::VehicleParameterState::StateNA, )
};

} /* namespace messages */
} /* namespace tmx */


#endif /* INCLUDE_VEHICLESTATECHANGE_H_ */
