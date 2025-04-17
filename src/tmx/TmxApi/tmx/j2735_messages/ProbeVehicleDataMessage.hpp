/*
 * @file ProbeVehicleDataMessage.hpp
 *
 *  Created on: May 8, 2016
 *      Author: Gregory M. Baumgardner
 */

#ifndef TMX_J2735_MESSAGES_PROBEVEHICLEDATAMESSAGE_HPP_
#define TMX_J2735_MESSAGES_PROBEVEHICLEDATAMESSAGE_HPP_

#include <ProbeVehicleData.h>
#include <tmx/j2735_messages/J2735MessageTemplate.hpp>
#include <tmx/messages/TmxJ2735.hpp>

#if SAEJ2735_SPEC < 2016
TMX_J2735_DECLARE(Pvd, ProbeVehicleData, api::probeVehicleData_D, api::MSGSUBTYPE_PROBEVEHICLEDATA_STRING)
#else
TMX_J2735_DECLARE(Pvd, ProbeVehicleData, api::probeVehicleData, api::MSGSUBTYPE_PROBEVEHICLEDATA_STRING)
#endif

// Specialize the unique key function
TMX_J2735_NAMESPACE_START(tmx)
TMX_J2735_NAMESPACE_START(messages)
TMX_J2735_NAMESPACE_START(j2735)

template <>
inline int get_j2735_message_key<tmx::messages::PvdMessage>(std::shared_ptr<ProbeVehicleData> message) {
	if (message && message->probeID && message->probeID->id) {
#if SAEJ2735_SPEC < 2016
		{
			TemporaryID_t &id = *message->probeID->id;
#else
		if (message->probeID->id->present == VehicleID_PR_stationID) {
			return (int)message->probeID->id->choice.stationID;
		} else if (message->probeID->id->present == VehicleID_PR_entityID) {
			TemporaryID_t &id = message->probeID->id->choice.entityID;
#endif
			tmx::byte_stream bytes(fmax(id.size, sizeof(int)));
			::memcpy(bytes.data(), id.buf, bytes.size());
			return *((int *)bytes.data());
		}
	}

	if (message && message->segNum) {
		return (int)*(message->segNum);
	}

	return 0;
}

TMX_J2735_NAMESPACE_END(j2735)
TMX_J2735_NAMESPACE_END(messages)
TMX_J2735_NAMESPACE_END(tmx)

#endif /* TMX_J2735_MESSAGES_PROBEVEHICLEDATAMESSAGE_HPP_ */
