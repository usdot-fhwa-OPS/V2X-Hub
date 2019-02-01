/*
 * ApplicationDataMessage.h
 *
 *  Created on: Jun 22, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_APPLICATIONDATAMESSAGE_H_
#define INCLUDE_APPLICATIONDATAMESSAGE_H_

#include <tmx/messages/message.hpp>
#include "MessageTypes.h"
#include "ApplicationMessageEnumTypes.h"
#include "ApplicationDataMessageEnumTypes.h"

namespace tmx {
namespace messages {

/**
 * ApplicationDataMessage is the message type used to send information messages about various application statuses.
 * It defines the message type and sub type and all data members.
 */
class ApplicationDataMessage : public tmx::message
{
public:
	ApplicationDataMessage() {}
	ApplicationDataMessage(const tmx::message_container_type &contents): tmx::message(contents) {}
	ApplicationDataMessage(std::string id, appmessage::ApplicationTypes appId, //std::string eventID,
			std::string timestamp, std::string interactionId, int intersectionId,
			appdatamessage::DataCodeId dataCode, std::string data ) {
		set_Id(id);
		set_AppId(appId);
		//set_EventID(eventID);
		set_Timestamp(timestamp);
		set_InteractionId(interactionId);
		set_IntersectionId(intersectionId);
		set_DataCode(dataCode);

		set_Data(data);
	}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_APPLICATION_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_DATA_STRING;

	///unique guid
	std_attribute(this->msg, std::string, Id, "", )

	///Id of the application unique string
	std_attribute(this->msg, appmessage::ApplicationTypes, AppId, appmessage::ApplicationTypes::NOAPPID, )

	///Unique message identifier for repeated notification
	//std_attribute(this->msg, std::string, EventID, "", )

	///Timestamp of the event
	std_attribute(this->msg, std::string, Timestamp, "", )

	//null/not present if not currently interacting with thing of interest.
	std_attribute(this->msg, std::string, InteractionId, "", )

	//Intersection Id of the current map.  null/not present if not on a map.
	std_attribute(this->msg, int, IntersectionId, -2, )

	///Code from master list of possible types
	std_attribute(this->msg, appdatamessage::DataCodeId, DataCode, appdatamessage::DataCodeId::NOEVENTID, )

	///Json payload of data fields. Fields vary depending on DataCodeId.
	std_attribute(this->msg, std::string, Data, "", )

};

} /* namespace messages */
} /* namespace tmx */



#endif /* INCLUDE_APPLICATIONDATAMESSAGE_H_ */
