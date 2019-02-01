/*
 * @file RoadSideAlertMessage.hpp
 *
 *  Created on: May 8, 2016
 *      Author: Gregory M. Baumgardner
 */

#ifndef TMX_J2735_MESSAGES_ROADSIDEALERTMESSAGE_HPP_
#define TMX_J2735_MESSAGES_ROADSIDEALERTMESSAGE_HPP_

#include <RoadSideAlert.h>
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>
#include <tmx/messages/TmxJ2735.hpp>

#if SAEJ2735_SPEC < 63
TMX_J2735_DECLARE(Rsa, RoadSideAlert, api::roadSideAlert_D, api::MSGSUBTYPE_ROADSIDEALERT_STRING)
#else
TMX_J2735_DECLARE(Rsa, RoadSideAlert, api::roadSideAlert, api::MSGSUBTYPE_ROADSIDEALERT_STRING)
#endif

#endif /* TMX_J2735_MESSAGES_ROADSIDEALERTMESSAGE_HPP_ */
