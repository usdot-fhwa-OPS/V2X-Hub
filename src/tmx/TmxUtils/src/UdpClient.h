/*
 * UdpClient.h
 *
 *  Created on: Aug 27, 2015
 *      Author: ivp
 */

#ifndef UDPCLIENT_H_
#define UDPCLIENT_H_

#include <netdb.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <tmx/TmxException.hpp>

namespace tmx {
namespace utils {

class UdpClientRuntimeError : public tmx::TmxException
{
public:
	explicit UdpClientRuntimeError(const char *w) : tmx::TmxException(w) {}

};

class UdpClient 
{
public:
	UdpClient(const std::string& address, int port);
	~UdpClient();

	int GetSocket() const;
	int GetPort() const;
	std::string GetAddress() const;

	int Send(std::string& message);
	int Send(void *buffer, size_t size);

private:
	int _socket;
	int _port;
	std::string _address;
	struct addrinfo *_addrInfo;
};

}} // namespace tmx::utils

#endif /* UDPCLIENT_H_ */

