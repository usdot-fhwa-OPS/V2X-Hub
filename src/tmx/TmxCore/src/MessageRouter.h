/*
 * MessageRouter.h
 *
 *  Created on: Jul 15, 2014
 *      Author: ivp
 */

#ifndef MESSAGEROUTER_H_
#define MESSAGEROUTER_H_

#include <string>
#include <vector>
#include "MessageReceiver.h"

/*!
 * Filter notes:
 * 		- A '*' value is wildcard.
 * 		- A '*' value for the 'type' in the filter entry subscribes to
 * 			all messages (the subtype field in the filter entry is ignored)
 * 		- A flagmask of 0 allows all messages through, otherwise a message is allowed through if the resultant of the flagmask or'ed with
 * 			the message's flag field is not zero.
 *
 */
struct MessageFilterEntry
{
	std::string type;
	std::string subtype;
	IvpMsgFlags flagmask;

	MessageFilterEntry() {
		this->flagmask = 0;
	}
};

/**
 * Interface for a message router.
 * \ingroup IVPCore
 */
class MessageRouter
{
	public:
		virtual ~MessageRouter() {};

		/*!
		 * Allows a client to broadcast a messages.  Can pass in itself as 'sender' so no messages loop back to itself.
		 *
		 * @param sender
		 * 		The MessageReceiver that sent the message.  Prevents loopbacks.  NULL is allowed.
		 *
		 * @param msg
		 * 		The message to send
		 *
		 * @requires
		 * 		msg != NULL
		 */
		virtual void broadcastMessage(MessageReceiver *sender, IvpMessage *msg) = 0;
		/*!
		 *
		 * @requires
		 * 		receiver != NULL
		 */
		virtual void registerReceiver(MessageReceiver *receiver, const std::vector<MessageFilterEntry> &filters) = 0;
		/*!
		 *
		 * @requires
		 * 		receiver != NULL
		 */
		virtual void unregisterReceiver(MessageReceiver *receiver) = 0;
};

#endif /* MESSAGEROUTER_H_ */
