/*
 * @file testmessage00.hpp
 *
 *  Created on: May 8, 2016
 *      Author: Gregory M. Baumgardner
 */

#ifndef TMX_J2735_MESSAGES_TESTMESSAGE00_HPP_
#define TMX_J2735_MESSAGES_TESTMESSAGE00_HPP_

#include <cmath>
#include <TestMessage00.h>
#include <tmx/messages/TmxJ2735.hpp>
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>

#if SAEJ2735_SPEC >= 2016
TMX_J2735_DECLARE(tsm0, TestMessage00, api::testMessage00, api::MSGSUBTYPE_TESTMESSAGE00_STRING)

// Specialize the unique key function
TMX_J2735_NAMESPACE_START(tmx)
TMX_J2735_NAMESPACE_START(messages)
TMX_J2735_NAMESPACE_START(j2735)

template <>
inline int get_j2735_message_key<tmx::messages::tsm0Message>(std::shared_ptr<TestMessage00> message) {
	// if (message && message->coreData.id.buf) {
	// 	tmx::byte_stream bytes(fmax(message->coreData.id.size, sizeof(int)));
	// 	::memcpy(bytes.data(), message->coreData.id.buf, bytes.size());
	// 	return *((int *)bytes.data());
	// }

	return 1;
}

TMX_J2735_NAMESPACE_END(j2735)
TMX_J2735_NAMESPACE_END(messages)
TMX_J2735_NAMESPACE_END(tmx)

#endif


#endif /* TMX_J2735_MESSAGES_TESTMESSAGE00_HPP_ */
