#pragma once


#include <tmx/messages/message.hpp>
#include "MessageTypes.h"


namespace tmx::messages {


class TimeSyncMessage : public tmx::message
{
	public:
		TimeSyncMessage() {}
		TimeSyncMessage(const tmx::message_container_type &contents): tmx::message(contents) {}
		TimeSyncMessage(uint64_t timestep, uint64_t seq) {
			set_timestep(timestep);
			set_seq(seq);
		}

		/// Message type for routing this message through TMX core.
		static constexpr const char* MessageType = MSGTYPE_APPLICATION_STRING;

		/// Message sub type for routing this message through TMX core.
		static constexpr const char* MessageSubType = MSGSUBTYPE_TIMESYNC_STRING;

		std_attribute(this->msg, uint64_t, timestep, 0, )
		std_attribute(this->msg, uint64_t, seq, 0, )
	};

} /* namespace tmx::messages */


