/*
 * UdpServer.h
 *
 *  Created on: Aug 27, 2015
 *      Author: ivp
 */

#ifndef SRC_UDPSERVER_H_
#define SRC_UDPSERVER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdexcept>
#include <tmx/TmxException.hpp>
#include <PluginLog.h>

namespace tmx {
namespace utils {

class UdpServerRuntimeError : public tmx::TmxException
{
public:
	UdpServerRuntimeError(const std::string &w) : tmx::TmxException(w) {}
};

class UdpServer
{
public:
	UdpServer(const std::string& address, int port);
	virtual ~UdpServer();

	virtual int GetSocket() const;
	virtual int GetPort() const;
	virtual std::string GetAddress() const;

	virtual int Receive(char *msg, size_t maxSize);
	virtual int TimedReceive(char *msg, size_t maxSize, int maxWait_ms);
	
	/**
	 * @brief Timed receive with sender information
	 * @param msg Buffer to store received message
	 * @param maxSize Maximum size of buffer
	 * @param maxWait_ms Maximum wait time in milliseconds
	 * @param senderIp Output parameter for sender IP address
	 * @param senderPort Output parameter for sender port
	 * @return Number of bytes received, or -1 on error
	 */
	virtual int TimedReceiveWithSender(char *msg, size_t maxSize, int maxWait_ms, std::string &senderIp, int &senderPort);

	virtual std::string stringTimedReceive(int maxWait_ms=5);

private:
	int _socket;
	int _port;
	std::string _address;
	struct addrinfo *_addrInfo;
};

}} // namespace tmx::utils

#endif /* SRC_UDPSERVER_H_ */
