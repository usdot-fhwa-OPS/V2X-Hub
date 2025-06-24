/*
 * @file NmeaMessage.hpp
 *
 *  Created on: May 8, 2016
 *      Author: Gregory M. Baumgardner
 */

#ifndef TMX_J2735_MESSAGES_NMEAMESSAGE_HPP_
#define TMX_J2735_MESSAGES_NMEAMESSAGE_HPP_

#if SAEJ2735_SPEC < 2016
#include <NMEA-Corrections.h>
#else
#include <NMEAcorrections.h>
#endif
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>
#include <tmx/messages/TmxJ2735.hpp>

#if SAEJ2735_SPEC < 2016
TMX_J2735_DECLARE(Nmea, NMEA_Corrections, api::nmeaCorrections_D, api::MSGSUBTYPE_NMEACORRECTIONS_STRING)
#else
TMX_J2735_DECLARE(Nmea, NMEAcorrections, api::nmeaCorrections, api::MSGSUBTYPE_NMEACORRECTIONS_STRING)
#endif

#endif /* TMX_J2735_MESSAGES_NMEAMESSAGE_HPP_ */
