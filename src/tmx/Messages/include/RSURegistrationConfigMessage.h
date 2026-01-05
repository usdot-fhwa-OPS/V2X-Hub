#pragma once


#include <tmx/messages/message.hpp>
#include "MessageTypes.h"
#include "TelematicUnit.h"
#include "RSUConfigList.h"


namespace tmx::messages {


class RSURegistrationConfigMessage : public tmx::message
{
	public:
		RSURegistrationConfigMessage()=default;
		explicit RSURegistrationConfigMessage(const tmx::message_container_type &contents);
		~RSURegistrationConfigMessage() override{};

		/// Message type for routing this message through TMX core.
		static constexpr const char* MessageType = MSGTYPE_APPLICATION_STRING;

		/// Message sub type for routing this message through TMX core.
		static constexpr const char* MessageSubType = MSGSUBTYPE_RSU_REGISTRATION_CONFIG_STRING;

		//Telematic unit identifier
		object_attribute(TelematicUnit, telematicUnit);

		//List of Registered RSUs
		object_attribute(RSUConfigList, rsuConfigList);

		// Epoch time in milliseconds
		std_attribute(this->msg, int64_t, timestamp, 0,);
	};

} /* namespace tmx::messages */
