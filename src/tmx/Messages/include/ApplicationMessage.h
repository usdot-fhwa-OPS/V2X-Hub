/*
 * ApplicationMessage.h
 *
 *  Created on: Jun 7, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_APPLICATIONMESSAGE_H_
#define INCLUDE_APPLICATIONMESSAGE_H_

#include <tmx/messages/message.hpp>
#include "ApplicationMessageEnumTypes.h"
#include "MessageTypes.h"

namespace tmx {
namespace messages {

/**
 * ApplicationMessage is the message type used to send information messages about various application statuses.
 * It defines the message type and sub type and all data members.
 */
class ApplicationMessage : public tmx::message
{
public:
	ApplicationMessage() {}
	ApplicationMessage(const tmx::message_container_type &contents): tmx::message(contents) {}
	ApplicationMessage(std::string id, appmessage::ApplicationTypes appId, std::string eventID,
			std::string timestamp, std::string displayDuration, appmessage::Severity severity,
			appmessage::EventCodeTypes eventCode, std::string interactionId, std::string customText ) {
		set_Id(id);
		set_AppId(appId);
		set_EventID(eventID);
		set_Timestamp(timestamp);
		set_DisplayDuration(displayDuration);
		set_Severity(severity);
		set_EventCode(eventCode);
		set_InteractionId(interactionId);
		set_CustomText(customText);
	}
	ApplicationMessage(std::string id, appmessage::ApplicationTypes appId, std::string eventID,
			std::string timestamp, std::string displayDuration, appmessage::Severity severity,
			appmessage::EventCodeTypes eventCode, std::string interactionId, std::string customText, double distanceToRefPoint, double angleToRefPoint ) {
		set_Id(id);
		set_AppId(appId);
		set_EventID(eventID);
		set_Timestamp(timestamp);
		set_DisplayDuration(displayDuration);
		set_Severity(severity);
		set_EventCode(eventCode);
		set_InteractionId(interactionId);
		set_CustomText(customText);
		set_DistanceToRefPoint(distanceToRefPoint);
		set_AngleToRefPoint(angleToRefPoint);
	}	

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_APPLICATION_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_BASIC_STRING;

	//unique guid
	std_attribute(this->msg, std::string, Id, "", )

	//Id of the application unique string
	std_attribute(this->msg, appmessage::ApplicationTypes, AppId, appmessage::ApplicationTypes::NOAPPID, )

	//Unique message identifier for repeated notification
	std_attribute(this->msg, std::string, EventID, "", )

	//Timestamp of the event
	std_attribute(this->msg, std::string, Timestamp, "", )

	//How long to display in milliseconds
	std_attribute(this->msg, std::string, DisplayDuration, "", )

	//Info or InformAlert or WarnAler
	std_attribute(this->msg, appmessage::Severity, Severity, appmessage::Severity::Info, )

	//Code from master list of possible events
	std_attribute(this->msg, appmessage::EventCodeTypes, EventCode, appmessage::EventCodeTypes::NOEVENTID, )

	//null/not present if not currently interacting with thing of interest.
	std_attribute(this->msg, std::string, InteractionId, "", )

	//Open text field for specific messages related to this event
	std_attribute(this->msg, std::string, CustomText, "", )

	// Distance to the Reference point of the Map
	std_attribute(this->msg, double, DistanceToRefPoint, 0, )

	//Open text field for specific messages related to this event
	std_attribute(this->msg, double, AngleToRefPoint, 0, )
};

} /* namespace messages */
} /* namespace tmx */



#endif /* INCLUDE_APPLICATIONMESSAGE_H_ */
