/*
 * @file testmessage01.hpp
 *
 *  Created on: May 8, 2016
 *      Author: Gregory M. Baumgardner
 */

#ifndef TMX_J2735_MESSAGES_TESTMESSAGE01_HPP_
#define TMX_J2735_MESSAGES_TESTMESSAGE01_HPP_

#include <cmath>
#include <TestMessage05.h>
#include <tmx/messages/TmxJ2735.hpp>
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>


TMX_J2735_DECLARE(tsm1, TestMessage01, api::testMessage01, api::MSGSUBTYPE_TESTMESSAGE01_STRING)

// Specialize the unique key function
TMX_J2735_NAMESPACE_START(tmx)
TMX_J2735_NAMESPACE_START(messages)
TMX_J2735_NAMESPACE_START(j2735)

template <>
inline int get_j2735_message_key<tmx::messages::tsm1Message>(std::shared_ptr<TestMessage01> message) {
	return 1;
}

TMX_J2735_NAMESPACE_END(j2735)
TMX_J2735_NAMESPACE_END(messages)
TMX_J2735_NAMESPACE_END(tmx)



#endif /* TMX_J2735_MESSAGES_TESTMESSAGE05_HPP_ */
