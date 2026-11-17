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
#include <tmx/tmx.h>
#include <assert.h>
#include <boost/thread.hpp>
#include <sys/prctl.h>
#include <memory>
#include <vector>
#include "MessageRouter.h"
#include "PluginConnection.h"

inline constexpr int MAX_CONNECTIONS = 5;

/**
 * PluginServer
 * \ingroup IVPCore
 */
class PluginServer
{
public:
	explicit PluginServer(MessageRouter *router);
	~PluginServer() = default;


private:
	MessageRouter *mRouter;
	int server_sockfd;
    std::vector<std::unique_ptr<PluginConnection>> pluginConnections;

	void startServer(void);
};

#endif /* PLUGINSERVER_H_ */
