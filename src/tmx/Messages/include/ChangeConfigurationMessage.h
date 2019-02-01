/*
 * ChangeConfigurationMessage.h
 *
 *  Created on: Aug 3, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_CHANGECONFIGURATIONMESSAGE_H_
#define INCLUDE_CHANGECONFIGURATIONMESSAGE_H_


#include <tmx/messages/message.hpp>
#include "MessageTypes.h"

namespace tmx {
namespace messages {

/**
* The ChangeConfigurationMessage is sent from the Cloud to the device to set system configuration values.
*/
class ChangeConfigurationMessage : public tmx::message
{
public:
	ChangeConfigurationMessage() {}
	ChangeConfigurationMessage(const tmx::message_container_type &contents): tmx::message(contents) {}
	ChangeConfigurationMessage(std::string key, std::string value)
	{
		set_Key(key);
		set_Value(value);
	}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_SYSTEM_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_CHANGECONFIGURATION_STRING;

	std_attribute(this->msg, std::string, Key, "", )
	std_attribute(this->msg, std::string, Value, "", )
};

} /* namespace messages */
} /* namespace tmx */


#endif /* INCLUDE_CHANGECONFIGURATIONMESSAGE_H_ */
