/*
 * MessageRouterBasic.cpp
 *
 *  Created on: Jul 18, 2014
 *      Author: ivp
 */

#include "MessageRouterBasic.h"
#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include "logger.h"
#include "utils/PerformanceTimer.h"

using namespace std;

MessageRouterBasic::MessageRouterBasic()
{
	pthread_mutexattr_t lockAttr;
	pthread_mutexattr_init(&lockAttr);
	pthread_mutexattr_settype(&lockAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&this->mMapLock, &lockAttr);
	pthread_mutex_init(&this->mActiveBroadcastsLock, &lockAttr);

	mActiveBroadcasts = 0;
	/*
	 * Nothing to do.  Just has to wait for clients to register.
	 */
}

MessageRouterBasic::~MessageRouterBasic()
{
	/*
	 * Honestly this should never happen... not for this project.
	 * The Message router should outlive all of it's clients, and that's the
	 * only time it can really can be destroyed.
	 */
}

uint64_t MessageRouterBasic::GetMsTimeSinceEpoch()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t)((double)(tv.tv_sec) * 1000 + (double)(tv.tv_usec) / 1000);
}

void MessageRouterBasic::broadcastMessage(MessageReceiver *sender, IvpMessage *msg)
{
	assert(msg != NULL);

	//PerformanceTimer timer;

	pthread_mutex_lock(&this->mMapLock);
	pthread_mutex_lock(&this->mActiveBroadcastsLock);
	this->mActiveBroadcasts++;
	pthread_mutex_unlock(&this->mActiveBroadcastsLock);
	pthread_mutex_unlock(&this->mMapLock);

	int broadcastCount = 0;

	for(std::map<MessageReceiver *, std::vector<MessageFilterEntry> >::iterator mapIter = this->mReceiverMessageFilterEntryMap.begin(); mapIter != this->mReceiverMessageFilterEntryMap.end(); mapIter++)
	{
		// Do not send to self.
		if (sender == mapIter->first)
			continue;

		for(std::vector<MessageFilterEntry>::iterator filterIter = mapIter->second.begin(); filterIter != mapIter->second.end(); filterIter++)
		{
			//std::string pluginName = mapIter->first->pluginName.empty() ? "unknown plugin" : mapIter->first->pluginName;

			if (filterIter->type.compare("*") == 0)
			{
				if (!filterIter->flagmask || (filterIter->flagmask & msg->flags) > 0)
				{

					//if (timer.Elapsed_Ms() > 0.2)
					//	cout << "MessageRouterBasic::broadcastMessage, " << pluginName << ", 1: " << timer.Elapsed_Ms() << " ms" << endl;

					mapIter->first->receiveMessage(msg);

					//if (timer.Elapsed_Ms() > 0.2)
					//	cout << "MessageRouterBasic::broadcastMessage, " << pluginName << ", 2: " << timer.Elapsed_Ms() << " ms" << endl;

					broadcastCount++;
					break;
				}
			}
			else if(msg->type != NULL && filterIter->type.compare(msg->type) == 0)
			{
				if (filterIter->subtype.compare("*") == 0
						|| (msg->subtype != NULL && filterIter->subtype.compare(msg->subtype) == 0))
				{
					if (!filterIter->flagmask || (filterIter->flagmask & msg->flags) > 0)
					{
						//if (timer.Elapsed_Ms() > 0.2)
						//	cout << "MessageRouterBasic::broadcastMessage, " << pluginName << ", 3: " << timer.Elapsed_Ms() << " ms" << endl;

						mapIter->first->receiveMessage(msg);

						//if (timer.Elapsed_Ms() > 0.2)
						//	cout << "MessageRouterBasic::broadcastMessage, " << pluginName << ", 4: " << timer.Elapsed_Ms() << " ms" << endl;

						broadcastCount++;
						break;
					}
				}
			}
		}
	}

	pthread_mutex_lock(&this->mActiveBroadcastsLock);
	this->mActiveBroadcasts--;
	pthread_mutex_unlock(&this->mActiveBroadcastsLock);

	//LOG_DEBUG("MessageRouterBasic::broadcastMessage for " << msg->subtype << " from "<< sender->pluginName << " Time (ms) "<<timer_ms);

	//char *jsonmsg = ivpMsg_createJsonString(msg, IvpMsg_FormatOptions_none);
	//LOG_DEBUG("Sent to " << broadcastCount << " receivers: " << jsonmsg);
	//LOG_DEBUG("Sent to " << broadcastCount << " receivers. type: " << msg->type<< ", subtype: " <<msg->subtype);
	//free(jsonmsg);
}

void MessageRouterBasic::registerReceiver(MessageReceiver *receiver, const std::vector<MessageFilterEntry> &filters)
{
	assert(receiver != NULL);

	pthread_mutex_lock(&this->mMapLock);
	while(this->mActiveBroadcasts > 0) { sleep(0); }

	mReceiverMessageFilterEntryMap[receiver] = filters;

	pthread_mutex_unlock(&this->mMapLock);
}

void MessageRouterBasic::unregisterReceiver(MessageReceiver *receiver)
{
	assert(receiver != NULL);

	pthread_mutex_lock(&this->mMapLock);
	while(this->mActiveBroadcasts > 0) { sleep(0); }

	mReceiverMessageFilterEntryMap.erase(receiver);

	pthread_mutex_unlock(&this->mMapLock);
}
