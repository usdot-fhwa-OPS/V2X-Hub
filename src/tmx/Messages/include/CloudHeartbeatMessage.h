/*
 * HeartbeatMessage.h
 *
 *  Created on: Aug 2, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_CLOUDHEARTBEATMESSAGE_H_
#define INCLUDE_CLOUDHEARTBEATMESSAGE_H_

#include <functional>

#include <tmx/messages/message.hpp>
#include "MessageTypes.h"

namespace tmx {
namespace messages {

/**
 * This heartbeat message is sent from the CloudInterfacePlugin to the cloud.
 */
class CloudHeartbeatMessage : public tmx::message
{
public:
	CloudHeartbeatMessage() {}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_SYSTEM_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_DEVICEHEARTBEAT_STRING;

	std_attribute(this->msg, std::string, DeviceType, "", )

	std_attribute(this->msg, std::string, VehicleId, "", )

	std_attribute(this->msg, std::string, RouteNumber, "", )

	std_attribute(this->msg, std::string, BusId, "", )

	std_attribute(this->msg, std::string, OperationMode, "", )

	struct UpTime
	{
		std::string Name;
		int UpTimeSeconds = 0;
		std::string Version;

		UpTime() {}
		UpTime(std::string name, int upTimeSeconds, std::string version) : Name(name), UpTimeSeconds(upTimeSeconds), Version(version) {}

		static message_tree_type to_tree(UpTime element)
		{
			message_tree_type treeElement;
			treeElement.put("Name", element.Name);
			treeElement.put("UpTimeSeconds", element.UpTimeSeconds);
			treeElement.put("Version", element.Version);
			return treeElement;
		}

		static UpTime from_tree(message_tree_type& treeElement)
		{
			UpTime element;
			element.Name = treeElement.get<std::string>("Name");
			element.UpTimeSeconds = treeElement.get<int>("UpTimeSeconds");
			element.Version = treeElement.get<std::string>("Version");
			return element;
		}
	};

	array_attribute(UpTime, UpTimes)
};

} /* namespace messages */
} /* namespace tmx */


#endif /* INCLUDE_CLOUDHEARTBEATMESSAGE_H_ */
