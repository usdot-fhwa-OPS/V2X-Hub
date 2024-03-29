/*
 * MessageRouterBasic.h
 *
 *  Created on: Jul 18, 2014
 *      Author: ivp
 */

#ifndef MESSAGEROUTERBASIC_H_
#define MESSAGEROUTERBASIC_H_

#include "MessageRouter.h"
#include <map>

/**
 * \ingroup IVPCore
 *
 * -Basic implementation of searching through filters to send messages.
 * -Single threaded, the sender is blocked until the receivers are completed.
 * -The message should be destroyed by the sender after it is sent to the router.
 * 		-Implies that the message receivers can't hold onto the message, they either need to copy it or
 * 		 be done with it by the time they return from onMessageReceived()
 * -Allows concurrent broadcast to happen at the same time.
 */
class MessageRouterBasic : public MessageRouter
{
public:
	MessageRouterBasic();
	virtual ~MessageRouterBasic();

	virtual void broadcastMessage(MessageReceiver *sender, IvpMessage *msg);
	virtual void registerReceiver(MessageReceiver *receiver, const std::vector<MessageFilterEntry> &filters);
	virtual void unregisterReceiver(MessageReceiver *receiver);

private:

	uint64_t GetMsTimeSinceEpoch();
	/*!
	 * Map stores each receiver's list of message filters that it's last subscribed with.
	 */
	std::map<MessageReceiver *, std::vector<MessageFilterEntry> > mReceiverMessageFilterEntryMap;

	/*!
	 * Lock for the map.  Map can not have keys (MessageReceivers) added or removed while inside broadcasting.
	 *
	 * Map can not have values(list of MessageFilterEntries) changed while inside broadcasting except for on the same thread.
	 * This is because by the time that the broadcast function run's any callbacks, it's already completed with that list of MessageFilterEntries but has not yet
	 * started using the next list of entries.
	 */
	pthread_mutex_t mMapLock;

	/*!
	 * Keeps track of how many threads are actively inside the broadcast method in a thread safe manner (using mActiveBroadcastsLock).
	 * This is to allow concurrent execution of the broadcast, but give the register and unregister methods a way to tell when broadcasts are inactive.
	 */
	int mActiveBroadcasts;

	/*!
	 * Used to keep the mActiveBroadcast count accurate since multiple thread's may be accessing.
	 */
	pthread_mutex_t mActiveBroadcastsLock;
};

#endif /* MESSAGEROUTERBASIC_H_ */
