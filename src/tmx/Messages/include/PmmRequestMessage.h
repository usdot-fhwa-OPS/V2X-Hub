/*
 * PmmRequestMessage.h
 *
 *  Created on: Oct 20, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_PMMREQUESTMESSAGE_H_
#define INCLUDE_PMMREQUESTMESSAGE_H_


#include <tmx/messages/message.hpp>
#include "MessageTypes.h"
#include "PmmRequestMessageEnumTypes.h"
#include <functional>

namespace tmx {
namespace messages {
namespace pmmrequestmessage {

/**
 * PmmRequest is the message is a close duplicate to the DSRC message type of PmmRequest. However, this is an internal type used to send the next selected trip information to the driver.
 */
class PmmRequestMessage : public tmx::message
{
public:
	PmmRequestMessage() {}
	PmmRequestMessage(const tmx::message_container_type &contents): tmx::message(contents) {}
	PmmRequestMessage(std::string id, std::string groupId, int requestId, std::string requestDate, std::string pickupDate,
			StatusTypes status, ModeOfTransportTypes modeOfTransport, double pickupLatitude, double pickupLongitude,
			double pickupElevation, double destLatitude, double destLongitude,
			double destElevation ) {
		set_Id(id);
		set_GroupId(groupId);
		set_RequestId(requestId);
		set_RequestDate(requestDate);
		set_PickupDate(pickupDate);
		set_Status(status);
		set_ModeOfTransport(modeOfTransport);
		set_PickupLatitude(pickupLatitude);
		set_PickupLongitude(pickupLongitude);
		set_PickupElevation(pickupElevation);
		set_DestLatitude(destLatitude);
		set_DestLongitude(destLongitude);
		set_DestElevation(destElevation);
	}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_PMM_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_NEXTREQUEST_STRING;

	//unique guid
	std_attribute(this->msg, std::string, Id, "", )

	std_attribute(this->msg, std::string, GroupId, "", )

	std_attribute(this->msg, int ,RequestId, 0, )

	std_attribute(this->msg, std::string, RequestDate, "", )

    std_attribute(this->msg, std::string, PickupDate, "", )

	std_attribute(this->msg, StatusTypes, Status, StatusTypes::New, )

	std_attribute(this->msg, ModeOfTransportTypes, ModeOfTransport, ModeOfTransportTypes::noPreference, )

	std_attribute(this->msg, double ,PickupLatitude, 0, )

	std_attribute(this->msg, double ,PickupLongitude, 0, )

	std_attribute(this->msg, double ,PickupElevation, 0, )

	std_attribute(this->msg, double ,DestLatitude, 0, )

	std_attribute(this->msg, double ,DestLongitude, 0, )

	std_attribute(this->msg,double ,DestElevation, 0, )

	//This maps to mobility needs
	/*
	 * note: the following enums map to the Type (int) field in the SeatsByType struct
	enum MobilityNeedTypes
	{
		NoSpecialNeeds = 0,
		Wheelchair = 1,
		NeedsSeat = 2
	};
	*/
	struct SeatsByType
	{
		int Count = 0;
		int Type = 0;

		SeatsByType() {}
		SeatsByType(int count, int type) : Count(count), Type(type) {}

		static message_tree_type to_tree(SeatsByType element)
		{
			message_tree_type treeElement;
			treeElement.put("Count", element.Count);
			treeElement.put("Type", element.Type);
			return treeElement;
		}

		static SeatsByType from_tree(message_tree_type& treeElement)
		{
			SeatsByType element;
			element.Count = treeElement.get<int>("Count");
			element.Type = treeElement.get<int>("Type");
			return element;
		}
	};

	array_attribute(SeatsByType, SeatsByTypes)
};

} /* namespace pmmrequestmessage */
} /* namespace messages */
} /* namespace tmx */


#endif /* INCLUDE_PMMREQUESTMESSAGE_H_ */
