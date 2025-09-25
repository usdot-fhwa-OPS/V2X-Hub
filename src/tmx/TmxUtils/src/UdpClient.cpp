/*
 * UdpServer.cpp
 *
 *  Created on: Aug 27, 2015
 *      Author: ivp
 */

#include <string.h>
#include <unistd.h>
#include <cstdio>
#include <errno.h>
#include "UdpClient.h"

namespace tmx {
namespace utils {

/**
 * \brief Initialize a UDP client object.
 *
 * This function initializes the UDP client object using the address and the
 * port as specified.
 *
 * The port is expected to be a host side port number (i.e. 59200).
 *
 * The \p address parameter is a textual address. It may be an IPv4 or IPv6
 * address and it can represent a host name or an address defined with
 * just numbers. If the address cannot be resolved then an error occurs
 * and constructor throws.
 *
 * \note
 * The socket is open in this process. If you fork() or exec() then the
 * socket will be closed by the operating system.
 *
 * \warning
 * We only make use of the first address found by getaddrinfo(). All
 * the other addresses are ignored.
 *
 * \exception UdpClientRuntimeError
 * The server could not be initialized properly. Either the address cannot be
 * resolved, the port is incompatible or not available, or the socket could
 * not be created.
 *
 * \param[in] address  The address to send to.
 * \param[in] port  The port to send to.
 */
UdpClient::UdpClient(const std::string& address, int port)
    : _port(port)
    , _address(address)
{
	char decimalPort[16];
	snprintf(decimalPort, sizeof(decimalPort), "%d", _port);
	decimalPort[sizeof(decimalPort) / sizeof(decimalPort[0]) - 1] = '\0';

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	int r(getaddrinfo(address.c_str(), decimalPort, &hints, &_addrInfo));
	if (r != 0 || _addrInfo == NULL)
	{
		BOOST_THROW_EXCEPTION(UdpClientRuntimeError(("invalid address or port: \"" + address + ":" + decimalPort + "\"").c_str()));
	}

	_socket = socket(_addrInfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
	if (_socket == -1)
	{
		freeaddrinfo(_addrInfo);
		BOOST_THROW_EXCEPTION(UdpClientRuntimeError(("could not create UDP socket for: \"" + address + ":" + decimalPort + "\"").c_str()));
	}
}

/** \brief Clean up the UDP client.
 *
 * This function frees the address info structures and close the socket.
 */
UdpClient::~UdpClient()
{
    freeaddrinfo(_addrInfo);
    close(_socket);
}

/** \brief Retrieve a copy of the socket identifier.
 *
 * This function return the socket identifier as returned by the socket()
 * function. This can be used to change some flags.
 *
 * \return The socket used by this UDP client.
 */
int UdpClient::GetSocket() const
{
    return _socket;
}

/** \brief Retrieve the port used by this UDP client.
 *
 * This function returns the port used by this UDP client. The port is
 * defined as an integer, host side.
 *
 * \return The port as expected in a host integer.
 */
int UdpClient::GetPort() const
{
    return _port;
}

/** \brief Retrieve a copy of the address.
 *
 * This function returns a copy of the address as it was specified in the
 * constructor. This does not return a canonalized version of the address.
 *
 * The address cannot be modified. If you need to send data on a different
 * address, create a new UDP client.
 *
 * \return A string with a copy of the constructor input address.
 */
std::string UdpClient::GetAddress() const
{
    return _address;
}

/** \brief Send a message through this UDP client.
 *
 * This function sends the \p buffer data through the UDP client socket.
 * The function cannot be used to change the destination as it was defined
 * when creating the UdpClient object.
 *
 * The size must be small enough for the message to fit.
 *
 * \param[in] buffer  The buffer containing the data to send.
 * \param[in] size  The number of bytes contained within the buffer to send.
 *
 * \return -1 if an error occurs, otherwise the number of bytes sent. errno
 * is set accordingly on error.
 */
int UdpClient::Send(void *buffer, size_t size)
{
    return sendto(_socket, buffer, size, 0, _addrInfo->ai_addr, _addrInfo->ai_addrlen);
}

int UdpClient::Send(const std::string& message)
{
	//printf("UdpClient.Send: ");

	//printf("%s", _addrInfo->ai_canonname);

	//for (int i=0; i < _addrInfo->ai_addrlen; i++)
	//	printf("%d", _addrInfo->ai_addr->sa_data[i]);

	//printf("\n");

    return sendto(_socket, message.c_str(), message.length(), 0, _addrInfo->ai_addr, _addrInfo->ai_addrlen);
}

}} // namespace tmx::utils
