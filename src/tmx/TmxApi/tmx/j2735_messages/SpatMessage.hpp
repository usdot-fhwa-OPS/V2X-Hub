/*
 * @file SpatMessage.hpp
 *
 *  Created on: Apr 28, 2016
 *      Author: Gregory M. Baumgardner
 */

#ifndef TMX_J2735_MESSAGES_SPATMESSAGE_HPP_
#define TMX_J2735_MESSAGES_SPATMESSAGE_HPP_

#include <SPAT.h>
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>
#include <tmx/messages/TmxJ2735.hpp>

TMX_J2735_DECLARE(Spat, SPAT, api::signalPhaseAndTimingMessage, api::MSGSUBTYPE_SIGNALPHASEANDTIMINGMESSAGE_STRING)

// Specialize the unique key function
TMX_J2735_NAMESPACE_START(tmx)
TMX_J2735_NAMESPACE_START(messages)
TMX_J2735_NAMESPACE_START(j2735)

template <>
inline int get_j2735_message_key<tmx::messages::SpatMessage>(std::shared_ptr<SPAT> message) {
	if (message && message->intersections.list.count > 0)
		return message->intersections.list.array[0]->id.id;

	return 0;
}

TMX_J2735_NAMESPACE_END(j2735)
TMX_J2735_NAMESPACE_END(messages)
TMX_J2735_NAMESPACE_END(tmx)


#endif /* TMX_J2735_MESSAGES_SPATMESSAGE_HPP_ */
