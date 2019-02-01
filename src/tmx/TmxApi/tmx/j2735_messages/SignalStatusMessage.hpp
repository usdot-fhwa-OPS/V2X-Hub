/*
 * @file SignalStatusMessage.hpp
 *
 *  Created on: May 8, 2016
 *      Author: Gregory M. Baumgardner
 */

#ifndef TMX_J2735_MESSAGES_SIGNALSTATUSMESSAGE_HPP_
#define TMX_J2735_MESSAGES_SIGNALSTATUSMESSAGE_HPP_

#include <SignalStatusMessage.h>
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>
#include <tmx/messages/TmxJ2735.hpp>

#if SAEJ2735_SPEC < 63
TMX_J2735_DECLARE(Ssm, SignalStatusMessage, api::signalStatusMessage_D, api::MSGSUBTYPE_SIGNALSTATUSMESSAGE_STRING)
#else
TMX_J2735_DECLARE(Ssm, SignalStatusMessage, api::signalStatusMessage, api::MSGSUBTYPE_SIGNALSTATUSMESSAGE_STRING)
#endif

#endif /* TMX_J2735_MESSAGES_SIGNALSTATUSMESSAGE_HPP_ */
