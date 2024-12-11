#include <gtest/gtest.h>
#include "UDPMessageForwarder.h"

namespace ODEForwardPlugin{
    class test_UDPMessageForwarder: public ::testing::Test{
        protected:
            std::shared_ptr<ODEForwardPlugin::UDPMessageForwarder>  _udpMessageForwarder = std::make_shared<UDPMessageForwarder>();
    };

    TEST_F(test_UDPMessageForwarder, test_sendMessage_BEFORE_ATTACH_UDP_CLIENT){
        EXPECT_THROW(_udpMessageForwarder->sendMessage(UDPMessageType::TIM, "test"), TmxException);
        EXPECT_THROW(_udpMessageForwarder->sendMessage(UDPMessageType::BSM, "test"), TmxException);
        EXPECT_THROW(_udpMessageForwarder->sendMessage(UDPMessageType::SPAT, "test"), TmxException);
        EXPECT_THROW(_udpMessageForwarder->sendMessage(UDPMessageType::MAP, "test"), TmxException);
    }

    TEST_F(test_UDPMessageForwarder, test_attachUdpClient){
        _udpMessageForwarder->attachUdpClient(UDPMessageType::TIM, std::make_shared<UdpClient>("127.0.0.1", 1232));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::BSM, std::make_shared<UdpClient>("127.0.0.1", 1233));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::MAP, std::make_shared<UdpClient>("127.0.0.1", 1234));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::SPAT, std::make_shared<UdpClient>("127.0.0.1", 1235));
    }

    TEST_F(test_UDPMessageForwarder, test_sendMessage_AFTER_ATTACH_UDP_CLIENT){
        _udpMessageForwarder->attachUdpClient(UDPMessageType::TIM, std::make_shared<UdpClient>("127.0.0.1", 1232));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::BSM, std::make_shared<UdpClient>("127.0.0.1", 1233));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::MAP, std::make_shared<UdpClient>("127.0.0.1", 1234));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::SPAT, std::make_shared<UdpClient>("127.0.0.1", 1235));
        _udpMessageForwarder->sendMessage(UDPMessageType::TIM, "test");
        _udpMessageForwarder->sendMessage(UDPMessageType::BSM, "test");
        _udpMessageForwarder->sendMessage(UDPMessageType::SPAT, "test");
        _udpMessageForwarder->sendMessage(UDPMessageType::MAP, "test");
    }

    TEST_F(test_UDPMessageForwarder, test_getAllUdpClientMessageTypes){
        _udpMessageForwarder->attachUdpClient(UDPMessageType::TIM, std::make_shared<UdpClient>("127.0.0.1", 1232));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::BSM, std::make_shared<UdpClient>("127.0.0.1", 1233));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::MAP, std::make_shared<UdpClient>("127.0.0.1", 1234));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::SPAT, std::make_shared<UdpClient>("127.0.0.1", 1235));
        auto allUdpClientMessageTypes = _udpMessageForwarder->getAllUdpClientMessageTypes();
        EXPECT_EQ(allUdpClientMessageTypes.size(), 4);
        EXPECT_TRUE(std::find(allUdpClientMessageTypes.begin(), allUdpClientMessageTypes.end(), UDPMessageType::TIM) != allUdpClientMessageTypes.end());
        EXPECT_TRUE(std::find(allUdpClientMessageTypes.begin(), allUdpClientMessageTypes.end(), UDPMessageType::BSM) != allUdpClientMessageTypes.end());
        EXPECT_TRUE(std::find(allUdpClientMessageTypes.begin(), allUdpClientMessageTypes.end(), UDPMessageType::SPAT) != allUdpClientMessageTypes.end());
        EXPECT_TRUE(std::find(allUdpClientMessageTypes.begin(), allUdpClientMessageTypes.end(), UDPMessageType::MAP) != allUdpClientMessageTypes.end());
    }

    TEST_F(test_UDPMessageForwarder, test_getUdpClient){
        _udpMessageForwarder->attachUdpClient(UDPMessageType::TIM, std::make_shared<UdpClient>("127.0.0.1", 1232));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::BSM, std::make_shared<UdpClient>("127.0.0.1", 1233));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::MAP, std::make_shared<UdpClient>("127.0.0.1", 1234));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::SPAT, std::make_shared<UdpClient>("127.0.0.1", 1235));
        EXPECT_EQ(_udpMessageForwarder->getUdpClient(UDPMessageType::TIM)->GetPort(), 1232);
        EXPECT_EQ(_udpMessageForwarder->getUdpClient(UDPMessageType::BSM)->GetPort(), 1233);
        EXPECT_EQ(_udpMessageForwarder->getUdpClient(UDPMessageType::MAP)->GetPort(), 1234);
        EXPECT_EQ(_udpMessageForwarder->getUdpClient(UDPMessageType::SPAT)->GetPort(), 1235);
        _udpMessageForwarder->attachUdpClient(UDPMessageType::TIM, std::make_shared<UdpClient>("127.0.0.1", 2232));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::BSM, std::make_shared<UdpClient>("127.0.0.1", 2233));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::MAP, std::make_shared<UdpClient>("127.0.0.1", 2234));
        _udpMessageForwarder->attachUdpClient(UDPMessageType::SPAT, std::make_shared<UdpClient>("127.0.0.1", 2235));
        EXPECT_EQ(_udpMessageForwarder->getUdpClient(UDPMessageType::TIM)->GetPort(), 2232);
        EXPECT_EQ(_udpMessageForwarder->getUdpClient(UDPMessageType::BSM)->GetPort(), 2233);
        EXPECT_EQ(_udpMessageForwarder->getUdpClient(UDPMessageType::MAP)->GetPort(), 2234);
        EXPECT_EQ(_udpMessageForwarder->getUdpClient(UDPMessageType::SPAT)->GetPort(), 2235);
    }
}