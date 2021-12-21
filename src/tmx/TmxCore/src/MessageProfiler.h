/*
 * MessageProfiler.h
 *
 *  Created on: Jul 15, 2014
 *      Author: ivp
 */

#ifndef MESSAGEPROFILER_H_
#define MESSAGEPROFILER_H_

#include "Plugin.h"
#include <map>
#include <sstream>
#include <boost/thread.hpp>
#include <time.h>
#include "database/MessageContext.h"

/**
 * A plugin that receives all the messages sent in the IVP System and keeps statistics for messasges sent by each plugin.
 * \ingroup IVPCore
 */
class MessageProfiler : public Plugin
{
public:
	MessageProfiler(MessageRouter *messageRouter);
	~MessageProfiler();

	virtual void onConfigChanged(std::string key, std::string value);
	virtual void onMessageReceived(IvpMessage *msg);

private:
	struct PluginIdMessageTypeKey {
		unsigned int pluginId;
		MessageTypeEntry messageType;

		PluginIdMessageTypeKey(unsigned int pluginId, MessageTypeEntry messageTypeId)
		{
			this->pluginId = pluginId;
			this->messageType = messageTypeId;
		}

		bool operator== (const PluginIdMessageTypeKey &cmp) const
		{
			return this->pluginId == cmp.pluginId && this->messageType == cmp.messageType;
		}

		bool operator< (const PluginIdMessageTypeKey &cmp) const
		{
			std::stringstream ss1, ss2;
			ss1 << this->pluginId << "||" << this->messageType.type << "||" << this->messageType.subtype;
			ss2 << cmp.pluginId << "||" << cmp.messageType.type << "||" << cmp.messageType.subtype;
			return ss1.str() < ss2.str();
		}
	};

	struct ProfileData {
		unsigned int messageCounts;
		uint64_t lastReceivedTime;
		std::vector<uint64_t> receiveTimeForAveraging;

		ProfileData()
		{
			messageCounts = 0;
			lastReceivedTime = 0;
		}
	};

	std::set<MessageTypeEntry> mMessageTypes;
	std::map<PluginIdMessageTypeKey, ProfileData> mProfileData;
	pthread_mutex_t mLock;
	boost::thread mDbRefreshThread;

	void dbRefreshThreadEntry();
};

#endif /* MESSAGEPROFILER_H_ */
