/*
 * MessageReceiver.h
 *
 *  Created on: Jul 16, 2014
 *      Author: ivp
 */

#ifndef MESSAGERECEIVER_H_
#define MESSAGERECEIVER_H_

#include <stdlib.h>
#include "tmx/IvpMessage.h"

/*!
 * An interface for receiving messages.
 */
class MessageReceiver
{
public:
	virtual ~MessageReceiver() { };
	/*!
	 * An 'abstract' method for receiving messages.
	 */
	virtual void receiveMessage(IvpMessage *msg) = 0;

	std::string pluginName;
};



#endif /* MESSAGERECEIVER_H_ */
