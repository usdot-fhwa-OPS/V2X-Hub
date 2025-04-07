/*
 * @file TravelerInformationMessage.hpp
 *
 *  Created on: May 8, 2016
 *      Author: Gregory M. Baumgardner
 */

#ifndef TMX_J2735_MESSAGES_TRAVELERINFORMATIONMESSAGE_HPP_
#define TMX_J2735_MESSAGES_TRAVELERINFORMATIONMESSAGE_HPP_

#include <TravelerInformation.h>
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>
#include <tmx/messages/TmxJ2735.hpp>

#if SAEJ2735_SPEC < 2016
TMX_J2735_DECLARE(Tim, TravelerInformation, api::travelerInformation_D, api::MSGSUBTYPE_TRAVELERINFORMATION_STRING)
#else
TMX_J2735_DECLARE(Tim, TravelerInformation, api::travelerInformation, api::MSGSUBTYPE_TRAVELERINFORMATION_STRING)
#endif

// Specialize the unique key function
TMX_J2735_NAMESPACE_START(tmx)
TMX_J2735_NAMESPACE_START(messages)
TMX_J2735_NAMESPACE_START(j2735)

template <>
inline int get_j2735_message_key<tmx::messages::TimMessage>(std::shared_ptr<TravelerInformation> message) {
	if (message && message->packetID) {
		tmx::byte_stream bytes(fmax(message->packetID->size, sizeof(int)));
		::memcpy(bytes.data(), message->packetID->buf, bytes.size());
		return *((int *)bytes.data());
	}

	return 0;
}

TMX_J2735_NAMESPACE_END(j2735)
TMX_J2735_NAMESPACE_END(messages)
TMX_J2735_NAMESPACE_END(tmx)

#endif /* TMX_J2735_MESSAGES_TRAVELERINFORMATIONMESSAGE_HPP_ */
