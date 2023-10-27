/*
 * @file ProbeDataManagementMessage.hpp
 *
 *  Created on: May 8, 2016
 *      Author: Gregory M. Baumgardner
 */

#ifndef TMX_J2735_MESSAGES_PROBEDATAMANAGEMENTMESSAGE_HPP_
#define TMX_J2735_MESSAGES_PROBEDATAMANAGEMENTMESSAGE_HPP_

#include <ProbeDataManagement.h>
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>
#include <tmx/messages/TmxJ2735.hpp>

#if SAEJ2735_SPEC < 63
TMX_J2735_DECLARE(Pdm, ProbeDataManagement, api::probeDataManagement_D, api::MSGSUBTYPE_PROBEDATAMANAGEMENT_STRING)
#else
TMX_J2735_DECLARE(Pdm, ProbeDataManagement, api::probeDataManagement, api::MSGSUBTYPE_PROBEDATAMANAGEMENT_STRING)
#endif

// Specialize the unique key function
TMX_J2735_NAMESPACE_START(tmx)
TMX_J2735_NAMESPACE_START(messages)
TMX_J2735_NAMESPACE_START(j2735)

template <>
inline int get_j2735_message_key<tmx::messages::PdmMessage>(std::shared_ptr<ProbeDataManagement> message) {
	if (message)
		return (int)(message->sample.sampleStart ^ message->sample.sampleEnd);

	return 0;
}

TMX_J2735_NAMESPACE_END(j2735)
TMX_J2735_NAMESPACE_END(messages)
TMX_J2735_NAMESPACE_END(tmx)

#endif /* TMX_J2735_MESSAGES_PROBEDATAMANAGEMENTMESSAGE_HPP_ */
