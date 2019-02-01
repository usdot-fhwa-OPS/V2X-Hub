/*
 * SystemShutdownMessage.h
 *
 */

#ifndef INCLUDE_SYSTEMSHUTDOWNMESSAGE_H_
#define INCLUDE_SYSTEMSHUTDOWNMESSAGE_H_

#include <tmx/messages/message.hpp>
#include "MessageTypes.h"
namespace tmx {
namespace messages {

/**
 * SystemStatusMessage .
 */
class SystemShutdownMessage: public tmx::message {
public:
	SystemShutdownMessage() {
	}
	SystemShutdownMessage(const tmx::message_container_type &contents) :
			tmx::message(contents) {
	}
	SystemShutdownMessage(std::string id,  std::string timestamp, int timerseconds, bool cancel) {
		set_Id(id);
		set_Timestamp(timestamp);
		set_TimerSeconds(timerseconds);
		set_Cancel(cancel);
	}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_SYSTEM_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_SHUTDOWN_STRING;

	//unique guid
	std_attribute(this->msg, std::string, Id, "", )

	//Timestamp of the event
	std_attribute(this->msg, std::string, Timestamp, "", )

	//Shutdown time in seconds of the event
	std_attribute(this->msg, int, TimerSeconds, 0, )

	//Cancel former shutdown message
	std_attribute(this->msg, bool, Cancel, 0, )

};

} /* namespace messages */
} /* namespace tmx */

#endif /* INCLUDE_SYSTEMSTATUSMESSAGE_H_ */
