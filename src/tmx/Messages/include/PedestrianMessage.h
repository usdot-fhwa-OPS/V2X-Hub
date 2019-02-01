/*
 * PedestrianMessage.h
 *
 *  Created on: May 25, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_PEDESTRIANMESSAGE_H_
#define INCLUDE_PEDESTRIANMESSAGE_H_


#include <tmx/messages/message.hpp>
#include "MessageTypes.h"

namespace tmx {
namespace messages {


/**
 * PedestrianMessage passes a comma separated list of detection zones that have triggered pedestrians
 * within the pedestrian detection plugin.
 */
class PedestrianMessage : public tmx::message {
public:
	PedestrianMessage() {}
	PedestrianMessage(const tmx::message_container_type &contents): tmx::message(contents) {}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_PEDESTRIAN_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_DETECTIONZONES_STRING;

	/**
	 * DetectionZones.
	 */
	std_attribute(this->msg, std::string, DetectionZones, "", )
};

} /* namespace messages */
} /* namespace tmx */


#endif /* INCLUDE_PEDESTRIANMESSAGE_H_ */

