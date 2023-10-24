/*
 * @file SignalRequestMessage.hpp
 *
 *  Created on: May 8, 2016
 *      Author: Gregory M. Baumgardner
 */

#ifndef TMX_J2735_MESSAGES_SIGNALREQUESTMESSAGE_HPP_
#define TMX_J2735_MESSAGES_SIGNALREQUESTMESSAGE_HPP_

// #if SAEJ2735_SPEC < 63
// #include <SignalRequestMsg.h>
// #else
#include <SignalRequestMessage.h>
// #endif
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>
#include <tmx/messages/TmxJ2735.hpp>

// #if SAEJ2735_SPEC < 63
// TMX_J2735_DECLARE(Srm, SignalRequestMsg, api::signalRequestMessage_D, api::MSGSUBTYPE_SIGNALREQUESTMESSAGE_STRING)
// #else
TMX_J2735_DECLARE(Srm, SignalRequestMessage, api::signalRequestMessage, api::MSGSUBTYPE_SIGNALREQUESTMESSAGE_STRING)
// #endif

#endif /* TMX_J2735_MESSAGES_SIGNALREQUESTMESSAGE_HPP_ */
