/*
 * EventLogMessage.h
 *
 *  Created on: Jun 13, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_EVENTLOGMESSAGE_H_
#define INCLUDE_EVENTLOGMESSAGE_H_


#include <tmx/messages/message.hpp>
#include "MessageTypes.h"
#include "VehicleParameterEnumTypes.h"

namespace tmx {
namespace messages {

/**
 * ApplicationEventMessage is the message type used to send information messages about plugin status/activities.
 * It defines the message type and sub type and all data members.
 */
class EventLogMessage : public tmx::message
{
public:
	EventLogMessage() {}
	EventLogMessage(const tmx::message_container_type &contents): tmx::message(contents) {}
	EventLogMessage(std::string id, std::string description, std::string source, std::string timestamp, std::string logLevel) {
		set_Id(id);
		set_Description(description);
		set_Source(source);
		set_Timestamp(timestamp);
		set_LogLevel(logLevel);
	}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_APPLICATION_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_TMXEVENTLOG_STRING;

	std_attribute(this->msg, std::string, Id, "", )
	std_attribute(this->msg, std::string, Description, "", )
	std_attribute(this->msg, std::string, Source, "", )
	//Timestamp of the data change,
	std_attribute(this->msg, std::string, Timestamp, "", )
	std_attribute(this->msg, std::string, LogLevel, "", )
};

} /* namespace messages */
} /* namespace tmx */


#endif /* INCLUDE_EVENTLOGMESSAGE_H_ */
