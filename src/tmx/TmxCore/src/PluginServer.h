/*
 * PluginServer.h
 *
 *  Created on: Jul 16, 2014
 *      Author: ivp
 */

#ifndef PLUGINSERVER_H_
#define PLUGINSERVER_H_

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

#include <boost/thread.hpp>

#include "MessageRouter.h"
#include "PluginConnection.h"

#define NET_DELIM ";"
#define MAX_CONNECTIONS	5
#define DISCONNECTED	0
#define CONNECTED		1

/**
 * PluginServer
 * \ingroup IVPCore
 */
class PluginServer
{
public:
	explicit PluginServer(MessageRouter *router);
	~PluginServer();


private:
	MessageRouter *mRouter;
	int server_sockfd;
//	boost::thread routerThread;

	void startServer(void);
};

#endif /* PLUGINSERVER_H_ */
