#pragma once
#include "UdpClient.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
namespace tmx {
    namespace utils {
        /**
         * @brief Mock Udp Client  used for unit testing using gmock. For documentation using gmock mocks 
         * (https://google.github.io/googletest/gmock_for_dummies.html).
         * 
         * @author Paul Bourelly
         */
        class MockUpdClient : public UdpClient {
            public:
                /**
                 * @brief Mock constructor with all default parameters. Can be used as an default constructor.
                 * 
                 * @param address 
                 * @param port 
                 */
                MockUpdClient(const std::string& address = "127.0.0.1", int port = 4567) : UdpClient(address, port) {};
                ~MockUpdClient() = default;
                MOCK_METHOD(int, GetPort,(),(const, override));
                MOCK_METHOD(int, Send, (const std::string &msg), (override));
                MOCK_METHOD(std::string, GetAddress, (), (const, override));
                MOCK_METHOD(int, Send, (void *buffer, size_t size), (override));
                MOCK_METHOD(int, GetSocket, (), (const , override));
        };
    }
}