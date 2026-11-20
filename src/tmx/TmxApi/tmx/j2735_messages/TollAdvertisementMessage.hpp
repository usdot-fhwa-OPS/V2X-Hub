/*
 * @file TollAdvertisementMessage.hpp
 *
 *  J2735 2024 TAM (Toll Advertisement Message), also defined in SAE J3217.
 *  Created on: November 17, 2026
 */
#ifndef TMX_J2735_MESSAGES_TOLLADVERTISEMENTMESSAGE_HPP_
#define TMX_J2735_MESSAGES_TOLLADVERTISEMENTMESSAGE_HPP_

#include <TollAdvertisementMessage.h>
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>
#include <tmx/messages/TmxJ2735.hpp>

TMX_J2735_DECLARE(Tam, TollAdvertisementMessage, api::tollAdvertisementMessage, api::MSGSUBTYPE_TOLLADVERTISEMENTMESSAGE_STRING)

#endif /* TMX_J2735_MESSAGES_TOLLADVERTISEMENTMESSAGE_HPP_ */
