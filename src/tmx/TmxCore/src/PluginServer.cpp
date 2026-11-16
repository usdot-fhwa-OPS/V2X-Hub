/*
 * PluginServer.cpp
 *
 *  Created on: Jul 16, 2014
 *      Author: ivp
 */

#include "PluginServer.h"

using namespace std;

PluginServer::PluginServer(MessageRouter *router)
{
	assert(router != nullptr);

	this->server_sockfd = 0;
	this->mRouter = router;
	boost::thread connectionThread(&PluginServer::startServer, this);
}



void PluginServer::startServer()
{
	prctl(PR_SET_NAME, "PluginServer", 0, 0, 0);
	int temp_fd;
	int server_len;
	int client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	int flags;
	int on = 1;
	
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,nullptr);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,nullptr);

	//Create unnamed socket
	server_sockfd = socket(AF_INET, SOCK_STREAM,0);
	if ( server_sockfd == -1 ) {
		perror("SOCKET");
		exit(1);
	}
	//Allows the socket to reuse the binding address so that on quick restarts we do not get an error
 	if (setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) == -1) {
        perror("SETSOCKOPT:");
        close(server_sockfd);
        exit(1);
    }
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
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
	if (flags == -1) {
		perror("fcntl F_GETFL");
		exit(1);
	}

	if ( fcntl(server_sockfd,F_SETFL,O_NONBLOCK|flags) == -1 ) {
		perror("fcntl F_SETFL");
		exit(1);
	}

	while(true)
	{

		temp_fd = accept(server_sockfd,(struct sockaddr *)&client_address, (socklen_t *) &client_len);


		if(temp_fd >= 0)
		{
			pluginConnections.push_back(std::make_unique<PluginConnection>(mRouter,temp_fd));
		}

		sleep(1);

		pthread_testcancel();
	}
}

