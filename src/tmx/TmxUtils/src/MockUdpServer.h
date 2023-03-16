#pragma once
#include "UdpServer.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
namespace tmx {
    namespace utils {
        /**
         * @brief Mock UDP Server used for unit testing using gmock. For documentation using gmock mocks 
         * (https://google.github.io/googletest/gmock_for_dummies.html).
         * 
         * @author Paul Bourelly
         */
        class MockUpdServer : public UdpServer {
            public:
                /**
                 * @brief Mock constructor with all default parameters. Can be used as an default constructor.
                 * 
                 * @param address 
                 * @param port 
                 */
                MockUpdServer(const std::string& address = "127.0.0.1", int port = 4567) : UdpServer(address, port) {};
                ~MockUpdServer() = default;
                MOCK_METHOD(int, GetPort,(),(override, const));
                MOCK_METHOD(int, TimedReceive, (char *msg, size_t maxSize, int maxWait_ms), (override));
                MOCK_METHOD(std::string, GetAddress, (), (const, override));
                MOCK_METHOD(int, Receive, (char *msg, size_t maxSize), (override));
                MOCK_METHOD(int, GetSocket, (), (override, const));
        };
    }
}