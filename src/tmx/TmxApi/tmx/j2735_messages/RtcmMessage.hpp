/*
 * @file RtcmMessage.hpp
 *
 *  Created on: May 8, 2016
 *      Author: Gregory M. Baumgardner
 */

#ifndef TMX_J2735_MESSAGES_RTCMMESSAGE_HPP_
#define TMX_J2735_MESSAGES_RTCMMESSAGE_HPP_

#include <RTCMcorrections.h>
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>
#include <tmx/messages/TmxJ2735.hpp>

TMX_J2735_DECLARE(Rtcm, RTCMcorrections, api::rtcmCorrections, api::MSGSUBTYPE_RTCMCORRECTIONS_STRING)

#endif /* TMX_J2735_MESSAGES_RTCMMESSAGE_HPP_ */
