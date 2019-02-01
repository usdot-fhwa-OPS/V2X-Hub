/*
 * PluginServer.cpp
 *
 *  Created on: Jul 16, 2014
 *      Author: ivp
 */

#ifndef __CYGWIN__
#include <sys/prctl.h>
#endif
#include "PluginServer.h"
#include <tmx/tmx.h>
#include <stdlib.h>
#include <assert.h>
using namespace std;

PluginServer::PluginServer(MessageRouter *router)
{
	assert(router != NULL);

	this->server_sockfd = 0;
	this->mRouter = router;
	boost::thread connectionThread(&PluginServer::startServer, this);
}

PluginServer::~PluginServer()
{

}

void PluginServer::startServer()
{
#ifndef __CYGWIN__
	prctl(PR_SET_NAME, "PluginServer", 0, 0, 0);
#endif

	int temp_fd;
	int server_len;
	int client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	int flags;
	int on = 1;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);

	//Create unnamed socket
	server_sockfd = socket(AF_INET, SOCK_STREAM,0);
	//Allows the socket to reuse the binding address so that on quick restarts we do not get an error
	//TODO check this return value
	/*retvalue = */setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);		//inet_addr("131.167.81.36"); // Use for static IP
	// TODO make port configurable
	server_address.sin_port = htons(IVP_DEFAULT_PORT);
	server_len = sizeof(server_address);
	if(bind(server_sockfd, (struct sockaddr *)&server_address, server_len))
		perror("BIND:");

	if(listen(server_sockfd, MAX_CONNECTIONS))
		perror("LISTEN:");
	client_len = sizeof(client_address);


	//Set socket as non blocking
	flags = fcntl(server_sockfd,F_GETFL,0);
	//TODO check this return value
	/*retvalue = */fcntl(server_sockfd,F_SETFL,O_NONBLOCK|flags);

	while(1)
	{

		temp_fd = accept(server_sockfd,(struct sockaddr *)&client_address, (socklen_t *) &client_len);


		if(temp_fd >= 0)
		{
			new PluginConnection(mRouter, temp_fd);
		}

		sleep(1);

		pthread_testcancel();
	}
}

