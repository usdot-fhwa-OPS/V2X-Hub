/*
 * MessageRouterClient.h
 *
 *  Created on: Jul 16, 2014
 *      Author: ivp
 */

#ifndef MESSAGEROUTERCLIENT_H_
#define MESSAGEROUTERCLIENT_H_

#include <string>
#include <vector>
#include "MessageRouter.h"
#include "MessageReceiver.h"

/**
 * \ingroup IVPCore
 * Parent class for any client that will be register to
 * receive messages from the router.
 *
 * This class MUST outlive the MessageRouter that it was given to work with.
 */
class MessageRouterClient : public MessageReceiver
{
public:
	~MessageRouterClient();

protected:
	/*!
	 *
	 * @requires
	 * 		messageRouter != NULL
	 */
	explicit MessageRouterClient(MessageRouter *messageRouter);

	/*!
	 * Subscribes with the router to receive messages that match ANY of the filters in the provided list.
	 *
	 * An empty list subscribes to no messages.
	 */
	virtual void subscribeForMessages(const std::vector<MessageFilterEntry> &filter);

	/*!
	 * Sends a message to the router to be broadcasted to all other clients that are subscribed with filters that
	 * match the message
	 *
	 * @requires
	 * 		msg != NULL
	 */
	virtual void sendMessageToRouter(IvpMessage *msg);

private:

private:
	MessageRouter *mMessageRouter;
};



#endif /* MESSAGEROUTERCLIENT_H_ */
