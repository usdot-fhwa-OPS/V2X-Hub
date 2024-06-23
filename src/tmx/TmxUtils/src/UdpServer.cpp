/*
 * UdpServer.cpp
 *
 *  Created on: Aug 27, 2015
 *      Author: ivp
 */

#include "UdpServer.h"

#include <string.h>
#include <unistd.h>
#include <cstdio>
#include <errno.h>

namespace tmx::utils {

    /**
     * \brief Initialize a UDP server object.
     *
     * This function initializes a UDP server object making it ready to
     * receive messages.
     *
     * The server address and port are specified in the constructor so
     * if you need to receive messages from several different addresses
     * and/or port, you'll have to create a server for each.
     *
     * The address is a string and it can represent an IPv4 or IPv6
     * address.
     *
     * Note that this function calls connect() to connect the socket
     * to the specified address. To accept data on different UDP addresses
     * and ports, multiple UDP servers must be created.
     *
     * \note
     * The socket is open in this process. If you fork() or exec() then the
     * socket will be closed by the operating system.
     *
     * \warning
     * We only make use of the first address found by getaddrinfo(). All
     * the other addresses are ignored.
     *
     * \exception udp_client_server_runtime_error
     * The udp_client_server_runtime_error exception is raised when the address
     * and port combination cannot be resolved or if the socket cannot be
     * opened.
     *
     * \param[in] address  The address we receive on.
     * \param[in] port  The port we receive from.
     */
    UdpServer::UdpServer(const std::string& address, int port)
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
            throw UdpServerRuntimeError(("invalid address or port for UDP socket: \"" + address + ":" + decimalPort + "\"").c_str());
        }

        _socket = socket(_addrInfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
        if (_socket == -1)
        {
            freeaddrinfo(_addrInfo);
            throw UdpServerRuntimeError(("could not create UDP socket for: \"" + address + ":" + decimalPort + "\"").c_str());
        }

        r = bind(_socket, _addrInfo->ai_addr, _addrInfo->ai_addrlen);
        if (r != 0)
        {
            freeaddrinfo(_addrInfo);
            close(_socket);
            throw UdpServerRuntimeError(("could not bind UDP socket with: \"" + address + ":" + decimalPort + "\"").c_str());
        }
    }

    /** \brief Clean up the UDP server.
     *
     * This function frees the address info structures and close the socket.
     */
    UdpServer::~UdpServer()
    {
        freeaddrinfo(_addrInfo);
        close(_socket);
    }

    /** \brief The socket used by this UDP server.
     *
     * This function returns the socket identifier. It can be useful if you are
     * doing a select() on many sockets.
     *
     * \return The socket of this UDP server.
     */
    int UdpServer::GetSocket() const
    {
        return _socket;
    }

    /** \brief The port used by this UDP server.
     *
     * This function returns the port attached to the UDP server. It is a copy
     * of the port specified in the constructor.
     *
     * \return The port of the UDP server.
     */
    int UdpServer::GetPort() const
    {
        return _port;
    }

    /** \brief Return the address of this UDP server.
     *
     * This function returns a verbatim copy of the address as passed to the
     * constructor of the UDP server (i.e. it does not return the canonalized
     * version of the address.)
     *
     * \return The address as passed to the constructor.
     */
    std::string UdpServer::GetAddress() const
    {
        return _address;
    }

    /** \brief Wait on a message.
     *
     * This function waits until a message is received on this UDP server.
     * There are no means to return from this function except by receiving
     * a message. Remember that UDP does not have a connect state so whether
     * another process quits does not change the status of this UDP server
     * and thus it continues to wait forever.
     *
     * Note that you may change the type of socket by making it non-blocking
     * (use the get_socket() to retrieve the socket identifier) in which
     * case this function will not block if no message is available. Instead
     * it returns immediately.
     *
     * \param[in] msg  The buffer where the message is saved.
     * \param[in] maxSize  The maximum size the message (i.e. size of the \p msg buffer.)
     *
     * \return The number of bytes read or -1 if an error occurs.
     */
    int UdpServer::Receive(char *msg, size_t maxSize)
    {
        return ::recv(_socket, msg, maxSize, 0);
    }

    /** \brief Wait for data to come in.
     *
     * This function waits for a given amount of time for data to come in. If
     * no data comes in after max_wait_ms, the function returns with -1 and
     * errno set to EAGAIN.
     *
     * The socket is expected to be a blocking socket (the default,) although
     * it is possible to setup the socket as non-blocking if necessary for
     * some other reason.
     *
     * This function blocks for a maximum amount of time as defined by
     * max_wait_ms. It may return sooner with an error or a message.
     *
     * \param[in] msg  The buffer where the message will be saved.
     * \param[in] maxSize  The size of the \p msg buffer in bytes.
     * \param[in] maxWait_ms  The maximum number of milliseconds to wait for a message.
     *
     * \return -1 if an error occurs or the function timed out, the number of bytes received otherwise.
     */
    int UdpServer::TimedReceive(char *msg, size_t maxSize, int maxWait_ms)
    {
        fd_set s;
        FD_ZERO(&s);
        FD_SET(_socket, &s);
        struct timeval timeout;
        timeout.tv_sec = maxWait_ms / 1000;
        timeout.tv_usec = (maxWait_ms % 1000) * 1000;

        int retval = select(_socket + 1, &s, &s, &s, &timeout);
        if (retval == -1)
        {
            // select() set errno accordingly
            return -1;
        }
        if (retval > 0)
        {
            // The socket has data.
            return ::recv(_socket, msg, maxSize, 0);
        }

        // The socket has no data.
        errno = EAGAIN;
        return -1;
    }

    std::string UdpServer::stringTimedReceive(int maxWait_ms) {
        std::vector<char> msg(4000);
        int num_of_bytes = this->TimedReceive(msg.data(),4000, maxWait_ms);
        if (num_of_bytes > 0 ) {
            msg.resize(num_of_bytes);
            std::string ret(msg.data());
            FILE_LOG(logDEBUG) << "UDP Server message received : " << ret << " of size " << num_of_bytes << std::endl;
            return ret;
        }
        else if ( num_of_bytes == 0 ) {
            throw UdpServerRuntimeError("Received empty message!");
        }
        else {
            throw UdpServerRuntimeError("Listen timed out after 5 ms!");
        }
        return "";
    }

} // namespace tmx::utils
