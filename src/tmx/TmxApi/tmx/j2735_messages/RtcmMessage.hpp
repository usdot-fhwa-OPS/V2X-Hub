/*
 * @file RtcmMessage.hpp
 *
 *  Created on: May 8, 2016
 *      Author: Gregory M. Baumgardner
 */

#ifndef TMX_J2735_MESSAGES_RTCMMESSAGE_HPP_
#define TMX_J2735_MESSAGES_RTCMMESSAGE_HPP_

#if SAEJ2735_SPEC < 2016
#include <RTCM-Corrections.h>
#else
#include <RTCMcorrections.h>
#endif
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>
#include <tmx/messages/TmxJ2735.hpp>

#if SAEJ2735_SPEC < 2016
TMX_J2735_DECLARE(Rtcm, RTCM_Corrections, api::rtcmCorrections_D, api::MSGSUBTYPE_RTCMCORRECTIONS_STRING)
#else
TMX_J2735_DECLARE(Rtcm, RTCMcorrections, api::rtcmCorrections, api::MSGSUBTYPE_RTCMCORRECTIONS_STRING)
#endif

#endif /* TMX_J2735_MESSAGES_RTCMMESSAGE_HPP_ */
