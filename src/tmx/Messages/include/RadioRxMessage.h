/*
 * @file: RadioRxMessage.h
 *
 *  Created on: Feb 14, 2017
 *      @author: gmb
 */

#ifndef INCLUDE_RADIORXMESSAGE_H_
#define INCLUDE_RADIORXMESSAGE_H_

#include <tmx/messages/message.hpp>
#include "MessageTypes.h"

namespace tmx {
namespace messages {

class RadioRxMessage: public tmx::message
{
public:
	RadioRxMessage() {}
	RadioRxMessage(std::string radio, std::string channel, std::string source) {
		set_RadioID(radio);
		set_ChannelID(channel);
		set_Source(source);
	}

	static constexpr const char *MessageType = MSGTYPE_RADIO_STRING;

	static constexpr const char *MessageSubType = MSGSUBTYPE_INCOMING_STRING;

	/**
	 * The radio identifier.  For Cohda this may be A or B
	 */
	std_attribute(this->msg, std::string, RadioID, "Unknown", )

	/**
	 * The logical channel identifier.  For Cohda this may be CCH or SCH
	 */
	std_attribute(this->msg, std::string, ChannelID, "Unknown", )

	/**
	 * The source address, if known.
	 */
	std_attribute(this->msg, std::string, Source, "Unknown", )

	/**
	 * The number of radio packets received from this source
	 */
	std_attribute(this->msg, uint64_t, Count, 0, )

	/**
	 * The smallest received RSSI value from this source, in dB.
	 */
	std_attribute(this->msg, double, RSSI_Min, -16384.0, )

	/**
	 * The largest received RSSI value from this source, in dB.
	 */
	std_attribute(this->msg, double, RSSI_Max, -16384.0, )

	/**
	 * The average received RSSI value from this source, in dB.
	 */
	std_attribute(this->msg, double, RSSI_Avg, -16384.0, )

	/**
	 * The smallest received noise value from this source, in dB.
	 */
	std_attribute(this->msg, double, Noise_Min, -16384.0, )

	/**
	 * The largest received noise value from this source, in dB.
	 */
	std_attribute(this->msg, double, Noise_Max, -16384.0, )

	/**
	 * The average received noise value from this source, in dB.
	 */
	std_attribute(this->msg, double, Noise_Avg, -16384.0, )

};

} /* End namespace messages */
} /* End namespace tmx */


#endif /* INCLUDE_RADIORXMESSAGE_H_ */
