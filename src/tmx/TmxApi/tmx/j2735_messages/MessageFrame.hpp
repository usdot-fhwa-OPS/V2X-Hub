/*
 * MessageFrame.hpp
 *
 *  Created on: Aug 24, 2017
 *      Author: gmb
 */

#ifndef TMX_J2735_MESSAGES_MESSAGEFRAME_HPP_
#define TMX_J2735_MESSAGES_MESSAGEFRAME_HPP_

// #if SAEJ2735_SPEC < 63
// #include <UPERframe.h>
// #else
#include <MessageFrame.h>
// #endif
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>

// #if SAEJ2735_SPEC < 63
// TMX_J2735_DECLARE_MESSAGE(UperFrame, UPERframe, api::uperFrame_D, api::MSGSUBTYPE_UPERFRAME_D_STRING);

// typedef UperFrameTraits MessageFrameTraits;
// typedef UperFrameMessage MessageFrameMessage;

// #else
TMX_J2735_DECLARE_MESSAGE(MessageFrame, MessageFrame, api::uperFrame_D, "MessageFrame")
// #endif

TMX_J2735_DECLARE_END

#endif /* TMX_J2735_MESSAGES_MESSAGEFRAME_HPP_ */
