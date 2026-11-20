/*
 * @file TollUsageAckMessage.hpp
 *
 *  J2735 2024 TUMack (Toll Usage Acknowledgment Message), also defined in SAE J3217.
 *  Created on: November 17, 2026
 */
#ifndef TMX_J2735_MESSAGES_TOLLUSAGEACKMESSAGE_HPP_
#define TMX_J2735_MESSAGES_TOLLUSAGEACKMESSAGE_HPP_

#include <TollUsageAckMessage.h>
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>
#include <tmx/messages/TmxJ2735.hpp>

TMX_J2735_DECLARE(TumAck, TollUsageAckMessage, api::tollUsageAckMessage, api::MSGSUBTYPE_TOLLUSAGEACKMESSAGE_STRING)

#endif /* TMX_J2735_MESSAGES_TOLLUSAGEACKMESSAGE_HPP_ */
