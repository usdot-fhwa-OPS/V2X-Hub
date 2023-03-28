#pragma once


#include <tmx/messages/message.hpp>
#include "MessageTypes.h"


namespace tmx::messages {


class TimeSyncMessage : public tmx::message
{
	public:
		TimeSyncMessage() {}
		TimeSyncMessage(const tmx::message_container_type &contents): tmx::message(contents) {}
		TimeSyncMessage(uint64_t timestamp, uint64_t seq_number) {
			set_timestamp(timestamp);
			set_seq_num(seq_number);
		}

		/// Message type for routing this message through TMX core.
		static constexpr const char* MessageType = MSGTYPE_APPLICATION_STRING;

		/// Message sub type for routing this message through TMX core.
		static constexpr const char* MessageSubType = MSGSUBTYPE_TIMESYNC_STRING;

		std_attribute(this->msg, uint64_t, timestamp, 0, )
		std_attribute(this->msg, uint64_t, seq_num, 0, )
	};

} /* namespace tmx::messages */


