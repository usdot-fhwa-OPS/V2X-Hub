/*
 * HisStateChangeMessage.h
 *
 *  Created on: Aug 18, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_HISSTATECHANGEMESSAGE_H_
#define INCLUDE_HISSTATECHANGEMESSAGE_H_

#include <tmx/messages/message.hpp>
#include "MessageTypes.h"
#include "HisStateChangeMessageEnumTypes.h"
#include "ApplicationMessageEnumTypes.h"

namespace tmx {
namespace messages {

/**
 * HisStateChangeMessage is the message type used to send information messages about the state of the His Display.
 * It defines the message type and sub type and all data members.
 */
class HisStateChangeMessage : public tmx::message
{
public:
	HisStateChangeMessage() {}
	HisStateChangeMessage(const tmx::message_container_type &contents): tmx::message(contents) {}
	HisStateChangeMessage(std::string id, std::string stateChangeTimestamp, hismessage::HISType hISType, appmessage::ApplicationTypes triggeringAlertType,
			std::string triggeringAlertId, std::string hISPreState, std::string hISPostState, appmessage::Severity severity) {
		set_Id(id);
		set_StateChangeTimestamp(stateChangeTimestamp);
		set_HISType(hISType);
		set_TriggeringAlertType(triggeringAlertType);
		set_TriggeringAlertId(triggeringAlertId);
		set_HISPreState(hISPreState);
		set_HISPostState(hISPostState);
		set_Severity(severity);
	}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_INTERFACE_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_STATECHANGE_STRING;

	///unique guid
	std_attribute(this->msg, std::string, Id, "", )

	//(local system time which should be synced to GPS time)
	std_attribute(this->msg, std::string, StateChangeTimestamp, "", )

	//- HISType (Display or Audible)
	std_attribute(this->msg, hismessage::HISType, HISType, hismessage::HISType::HISUnknown, )

	//- TriggeringAlertType same as the Application Message types
	std_attribute(this->msg, appmessage::ApplicationTypes, TriggeringAlertType, appmessage::ApplicationTypes::NOAPPID, )

	//- TriggeringAlertId (AlertId of the Application Message triggering the alert)
	std_attribute(this->msg, std::string, TriggeringAlertId, "", )

	//- HISPreState (What was displayed or playing before the change)
	std_attribute(this->msg, std::string, HISPreState, "", )

	//- HISPostState (What was displayed or playing after the change)
	std_attribute(this->msg, std::string, HISPostState, "", )

	//Severity is the of the event
	std_attribute(this->msg, appmessage::Severity, Severity, appmessage::Severity::Info, )

};

} /* namespace messages */
} /* namespace tmx */



#endif /* INCLUDE_HISSTATECHANGEMESSAGE_H_ */
