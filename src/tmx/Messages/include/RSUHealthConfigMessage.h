#pragma once


#include <tmx/messages/message.hpp>
#include "MessageTypes.h"


namespace tmx::messages {


class RSUHealthConfigMessage : public tmx::message
{
	public:
		RSUHealthConfigMessage() {}

		/// Message type for routing this message through TMX core.
		static constexpr const char* MessageType = MSGTYPE_APPLICATION_STRING;

		/// Message sub type for routing this message through TMX core.
		static constexpr const char* MessageSubType = MSGSUBTYPE_RSU_HEALTH_CONFIG_STRING;
	};

} /* namespace tmx::messages */
