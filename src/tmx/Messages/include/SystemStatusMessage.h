/*
 * SystemStatusMessage.h
 *
 */

#ifndef INCLUDE_SYSTEMSTATUSMESSAGE_H_
#define INCLUDE_SYSTEMSTATUSMESSAGE_H_

#include <tmx/messages/message.hpp>
#include "MessageTypes.h"
#include "SystemStatusMessageEnumTypes.h"
//#include "ApplicationMessageTypes.h"
namespace tmx {
namespace messages { 

/**
 * SystemStatusMessage .
 */
class SystemStatusMessage: public tmx::message {
public:
	SystemStatusMessage() {
	}
	SystemStatusMessage(const tmx::message_container_type &contents) :
			tmx::message(contents) {
	}
	SystemStatusMessage(std::string id,  std::string timestamp, sysstatus::ModuleTypes moduleType,
			sysstatus::OperationModeTypes opModeType,
			 std::string message) {
		set_Id(id);
		set_Timestamp(timestamp);
		set_ModuleType(moduleType);
		set_OpModeType(opModeType);
		set_Message(message);
	}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_SYSTEM_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_MODULESTATUS_STRING;

	//unique guid
std_attribute(this->msg, std::string, Id, "", )

	//Timestamp of the event
std_attribute(this->msg, std::string, Timestamp, "", )

//Type of module reporting status.
std_attribute(this->msg, sysstatus::ModuleTypes, ModuleType,
		sysstatus::ModuleTypes::NoModuleType, )

	//Operation Mode of the module reporting
std_attribute(this->msg, sysstatus::OperationModeTypes, OpModeType,
		sysstatus::OperationModeTypes::NoOpModeType, )

		//Open text field for specific messages related to this event
		std_attribute(this->msg, std::string, Message, "", )

		//TBD LogLevel type

};

} /* namespace messages */
} /* namespace tmx */

#endif /* INCLUDE_SYSTEMSTATUSMESSAGE_H_ */
