/*
 * MessageRouterClient.cpp
 *
 *  Created on: Jul 16, 2014
 *      Author: ivp
 */

#include "MessageRouterClient.h"
#include "tmx/tmx.h"
#include <assert.h>
#include <string.h>
#include <sstream>
#include <iostream>
using namespace std;

MessageRouterClient::MessageRouterClient(MessageRouter *router)
{
	assert(router != NULL);

	this->mMessageRouter = router;
}

MessageRouterClient::~MessageRouterClient()
{
	this->mMessageRouter->unregisterReceiver(this);
}

void MessageRouterClient::subscribeForMessages( const std::vector<MessageFilterEntry> &filter)
{
	mMessageRouter->registerReceiver(this, filter);
}

void MessageRouterClient::sendMessageToRouter(IvpMessage *msg)
{
	assert(msg != NULL);

	mMessageRouter->broadcastMessage(this, msg);
}
