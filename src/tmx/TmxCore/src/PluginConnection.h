/*
 * PluginConnection.h
 *
 *  Created on: Jul 16, 2014
 *      Author: ivp
 */

#ifndef PLUGINCONNECTION_H_
#define PLUGINCONNECTION_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <pthread.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <iostream>
#include <queue>

#include <boost/thread.hpp>
#include "utils/AutoResetEvent.h"

#include "Plugin.h"
#include <set>

class PluginConnection : public Plugin
{
public:
	PluginConnection(MessageRouter *router, int socket);
	~PluginConnection();

protected:
	virtual void onConfigChanged(std::string key, std::string value);
	virtual void onMessageReceived(IvpMessage *msg);

private:
	void receiverThread(void);
	void fastProcessorThread(void);
	void slowProcessorThread(void);

	void processRegistrationMessage(IvpMessage *msg);
	void processSubscribeMessage(IvpMessage *msg);
	void processConfigMessage(IvpMessage *msg);
	void processStatusMessage(IvpMessage *msg);
	void processEventLogMessage(IvpMessage *msg);

	boost::thread mReceiverThread;
	boost::thread mFastProcessorThread;
	boost::thread mSlowProcessorThread;
	int mSocket;

	AutoResetEvent mEventContinueFastProcessor;
	boost::mutex mMutexFastMessageQueue;
	std::queue<IvpMessage*> mFastMessageQueue;

	boost::mutex mMutexSlowMessageQueue;
	std::queue<IvpMessage*> mSlowMessageQueue;

	AutoResetEvent mEventContinueSlowProcessor;
};

#endif /* PLUGINCONNECTION_H_ */
