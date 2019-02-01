/*
 * PedestrianLogMessage.h
 *
 *  Created on: Jul 25, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_PEDESTRIANLOGMESSAGE_H_
#define INCLUDE_PEDESTRIANLOGMESSAGE_H_


#include <tmx/messages/message.hpp>
#include "MessageTypes.h"

namespace tmx {
namespace messages {

/**
 * PedestrianLogMessage captures the camera,zone and detection information of pedestrians at a roadside.
 * This data is logged directly to the database and this message type is really just for consistency and
 * type/subtype data fields.
 */
class PedestrianLogMessage : public tmx::message
{
public:
	PedestrianLogMessage() {}
	PedestrianLogMessage(const tmx::message_container_type &contents): tmx::message(contents) {}
	PedestrianLogMessage(std::string id, int64_t detectionTime, int type, int zoneID, int eventNumber,
			std::string ipAddress, int laneID) {
		set_Id(id);
		set_DetectionTime(detectionTime);
		set_Type(type);
		set_ZoneID(zoneID);
		set_EventNumber(eventNumber);
		set_IpAddress(ipAddress);
		set_LaneID(laneID);
	}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_PEDESTRIAN_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_DETECTIONLOG_STRING;

	std_attribute(this->msg, std::string, Id, "", )
	std_attribute(this->msg, int64_t, DetectionTime, 0, )
	std_attribute(this->msg, int, Type, 0, )
	std_attribute(this->msg, int, ZoneID, 0, )
	std_attribute(this->msg, int, EventNumber, 0, )
	std_attribute(this->msg, std::string, IpAddress, "", )
	std_attribute(this->msg, int, LaneID, 0, )
};

} /* namespace messages */
} /* namespace tmx */



#endif /* INCLUDE_PEDESTRIANLOGMESSAGE_H_ */
