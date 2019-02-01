/*
 * MessageProfiler.cpp
 *
 * Internal ivpcore plugin that monitors all messages routed through the system.
 * It computes and updates the average interval for each message (per plugin id and message type)
 * along with the timestamp of when the last message was received.
 * This plugin maintains the database tables: messageType and messageActivity.
 */

#ifndef __CYGWIN__
#include <sys/prctl.h>
#endif
#include "MessageProfiler.h"
#include <iostream>
#include <assert.h>
#include <sstream>
#include <string.h>
#include <time.h>
#include "version.h"
#include "logger.h"
#include "database/PluginContext.h"
#include "utils/TimeUtils.h"

#define MSGPROFILER_CONFIGKEY_DB_RFRSH_INTERVAL "Database Refresh Interval (ms)"
#define MSGPROFILER_CONFIGKEY_AVERAGINGWINDOW "Message Averaging Window (ms)"

using namespace std;

MessageProfiler::MessageProfiler(MessageRouter *messageRouter) : Plugin(messageRouter)
{
	RegistrationInformation info;
	info.pluginInfo.name = "ivpcore.MessageProfiler";
	info.pluginInfo.description = "Core element that is responsible for profiling the statistics of received messages";
	info.pluginInfo.version = IVPCORE_VERSION;

	info.configDefaultEntries.push_back(PluginConfigurationParameterEntry(MSGPROFILER_CONFIGKEY_DB_RFRSH_INTERVAL, "2000", "The interval (in milliseconds) between uploads of message statistics to the database."));
	info.configDefaultEntries.push_back(PluginConfigurationParameterEntry(MSGPROFILER_CONFIGKEY_AVERAGINGWINDOW, "20000", "The averaging window (in milliseconds) that the profiler measures average interval."));

	try
	{
		this->registerPlugin(info);
	}
	catch (PluginException &e)
	{
		LOG_FATAL(e.what());
		throw e;
	}

	// Register for all messages.
	vector<MessageFilterEntry> entries;
	MessageFilterEntry entry;
	entry.type = "*";
	entries.push_back(entry);
	this->subscribeForMessages(entries);

	// Retrieve all known message types from the database.
	// This class will later add new ones as needed when a message with an unknown message type is received.
	try
	{
		MessageContext context;
		this->mMessageTypes = context.getAllMessageTypes();
	}
	catch (const DbException &e)
	{
		LOG_WARN(e.what());
	}

	pthread_mutex_init(&this->mLock, NULL);

	this->mDbRefreshThread = boost::thread(&MessageProfiler::dbRefreshThreadEntry, this);
}

MessageProfiler::~MessageProfiler()
{

}

void MessageProfiler::onConfigChanged(std::string key, std::string value)
{
	cout << "Configuration Changed: " << key << " = " << this->getConfigValue(key) << endl;
}

// Since MessageProfiler is subscribed to all messages, this method will be called for every message.
// This method is executed on whatever thread the message was received on.
// For messages from external plugins, this will be called from the processor thread
// of PluginConnection.
void MessageProfiler::onMessageReceived(IvpMessage *msg)
{
	assert(msg != NULL);

	// These are heartbeats. So don't profile them.
	if (msg->type == NULL)// && msg->subtype == NULL)
		return;

	if (msg->source == NULL || msg->source[0] == '\0' || msg->sourceId == 0)
	{
		LOG_WARN("Message does not contain source information... Dropping message.");
		return;
	}

	uint64_t rxTime = TimeUtils::getSystemMillis();

	pthread_mutex_lock(&this->mLock);

	// Determine if this message type is already in the mMessageTypes set.  If not, then add it.
	// This is done here and not deferred to the other thread.
	// Since it only writes to the database when a new message type is encountered, it will not
	// cause latency for subsequent messages of the same type.

	MessageTypeEntry messageType(string(msg->type), string(msg->subtype != NULL ? msg->subtype : ""));
	set<MessageTypeEntry>::iterator localMsgType = this->mMessageTypes.find(messageType);

	if (localMsgType == this->mMessageTypes.end())
	{
		try
		{
			MessageContext context;
			context.insertMessageType(messageType);
			assert(messageType.id != 0);
			if (messageType.id != 0)
				this->mMessageTypes.insert(messageType);
		}
		catch (const DbException &e)
		{
			LOG_WARN(e.what());
		}
	}
	else
	{
		messageType = *localMsgType;
	}

	assert(messageType.id != 0);
	if (messageType.id == 0)
		LOG_ERROR("Message type id is zero.  Something when wrong.");

	// Form a key from the source plugin id and the message type.
	// Doing it this way instead of using [] operator because it should be much faster.
	PluginIdMessageTypeKey pluginIdMsgType(msg->sourceId, messageType);

	// Add the data from this message to the corresponding ProfileData entry in the mProfileData map.
	map<PluginIdMessageTypeKey, ProfileData>::iterator entry = this->mProfileData.find(pluginIdMsgType);
	if (entry != this->mProfileData.end())
	{
		entry->second.lastReceivedTime = rxTime;
		entry->second.messageCounts++;
		entry->second.receiveTimeForAveraging.push_back(rxTime);
	}
	else
	{
		// This is a new entry, add it to the map.
		ProfileData profileData;
		profileData.lastReceivedTime = rxTime;
		profileData.messageCounts = 1;
		profileData.receiveTimeForAveraging.push_back(rxTime);
		this->mProfileData[pluginIdMsgType] = profileData;
	}

	//cout << this->mProfileData.size() << endl;

	pthread_mutex_unlock(&this->mLock);
}

void MessageProfiler::dbRefreshThreadEntry()
{
#ifndef __CYGWIN__
	prctl(PR_SET_NAME, "MsgProfiler", 0, 0, 0);
#endif

	vector<MessageActivityEntry> activityEntries;

	boost::this_thread::disable_interruption di;

	while (!boost::this_thread::interruption_requested())
	{
		// Sleep for the interval specified by the corresponding configuration value.
		{
			uint64_t startTime = TimeUtils::getSystemMillis();
			uint64_t sleepTime;
			do
			{
				usleep(100000);
				char *tailptr;
				sleepTime = strtoimax(this->getConfigValue(MSGPROFILER_CONFIGKEY_DB_RFRSH_INTERVAL).c_str(), &tailptr, 10);

			} while(TimeUtils::getSystemMillis() < startTime + (sleepTime == 0 ? 3000 : sleepTime));
		}

		// Get the purge window configuration value.
		char *tailptr;
		unsigned int purgeWindow = strtoul(this->getConfigValue(MSGPROFILER_CONFIGKEY_AVERAGINGWINDOW).c_str(), &tailptr, 10);
		if (purgeWindow == 0)
			purgeWindow = 6000;

		uint64_t currentTime = TimeUtils::getSystemMillis();

		// Clear any old activty entries from last time.
		activityEntries.clear();

		pthread_mutex_lock(&this->mLock);

		// Iterate over all ProfileData (one per plugin id / message type) and store the message activity in the activityEntries array.
		for (map<PluginIdMessageTypeKey, ProfileData>::iterator itr = this->mProfileData.begin(); itr != this->mProfileData.end(); itr++)
		{
			// Iterate over the array of receive times.
			// Purge any data that is outside the time window used for calculating the average.
			for (std::vector<uint64_t>::iterator rxTimeItr = itr->second.receiveTimeForAveraging.begin(); rxTimeItr != itr->second.receiveTimeForAveraging.end(); )
			{
				if (purgeWindow + *rxTimeItr < currentTime)
					rxTimeItr = itr->second.receiveTimeForAveraging.erase(rxTimeItr);
				else
					rxTimeItr++;
			}

			assert(itr->first.messageType.id != 0);
			assert(itr->first.pluginId != 0);

			MessageActivityEntry entry;
			entry.messageTypeId = itr->first.messageType.id;
			entry.pluginId = itr->first.pluginId;
			entry.count = itr->second.messageCounts;
			entry.lastReceivedTimestamp = itr->second.lastReceivedTime / 1000;
			if (itr->second.receiveTimeForAveraging.size() == 0)
				entry.averageInterval = 0;
			else
				entry.averageInterval = (uint64_t)((double)purgeWindow / itr->second.receiveTimeForAveraging.size());

			// The activity is stored in an array for now.
			// This allows the mutex to be unlocked while the database is updated.
			// Otherwise the onMessageReceived method must wait for the database writes to complete,
			// which introduces latency into the message routing functionality of the system.
			activityEntries.push_back(entry);
		}

		pthread_mutex_unlock(&this->mLock);

		// Update the database.

		MessageContext context;

		for (std::vector<MessageActivityEntry>::iterator entryItr = activityEntries.begin(); entryItr != activityEntries.end(); entryItr++)
		{
			try
			{
				context.insertOrUpdateMessageActivity(*entryItr);
			}
			catch (const DbException &e)
			{
				LOG_WARN(e.what());
			}
		}
	}
}
