/*
 * @file TollUsageMessage.hpp
 *
 *  J2735 2024 TUM (Toll Usage Message), also defined in SAE J3217.
 *  Created on: November 17, 2026
 */
#ifndef TMX_J2735_MESSAGES_TOLLUSAGEMESSAGE_HPP_
#define TMX_J2735_MESSAGES_TOLLUSAGEMESSAGE_HPP_

#include <TollUsageMessage.h>
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>
#include <tmx/messages/TmxJ2735.hpp>

TMX_J2735_DECLARE(Tum, TollUsageMessage, api::tollUsageMessage, api::MSGSUBTYPE_TOLLUSAGEMESSAGE_STRING)

#endif /* TMX_J2735_MESSAGES_TOLLUSAGEMESSAGE_HPP_ */
