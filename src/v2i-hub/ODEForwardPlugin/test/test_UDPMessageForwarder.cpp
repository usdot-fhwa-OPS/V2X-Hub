#include <gtest/gtest.h>
#include <UDPMessageForwarder.h>

namespace ODEForwardPlugin{
    class test_UDPMessageForwarder: public ::testing::Test{
        protected:
            std::shared_ptr<ODEForwardPlugin::UDPMessageForwarder>  _udpMessageForwarder = std::make_shared<UDPMessageForwarder>();
    };

    TEST_F(test_UDPMessageForwarder, test_sendMessage_BEFORE_ATTACH_UDP_CLIENT){
        EXPECT_THROW(_udpMessageForwarder->sendMessage(UDPMessageType::TIM, "fffe4386ba00078005a53373"), TmxException);
        EXPECT_THROW(_udpMessageForwarder->sendMessage(UDPMessageType::BSM, "fffe4386ba00078005a53373"), TmxException);
        EXPECT_THROW(_udpMessageForwarder->sendMessage(UDPMessageType::SPAT, "fffe4386ba00078005a53373"), TmxException);
        EXPECT_THROW(_udpMessageForwarder->sendMessage(UDPMessageType::MAP, "fffe4386ba00078005a53373"), TmxException);
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
        _udpMessageForwarder->sendMessage(UDPMessageType::TIM, "fffe4386ba00078005a53373");
        _udpMessageForwarder->sendMessage(UDPMessageType::BSM, "fffe4386ba00078005a53373");
        _udpMessageForwarder->sendMessage(UDPMessageType::SPAT, "fffe4386ba00078005a53373");
        _udpMessageForwarder->sendMessage(UDPMessageType::MAP, "fffe4386ba00078005a53373");
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

    TEST_F(test_UDPMessageForwarder, test_getUdpClient_BEFORE_ATTACH_UDP_CLIENT){
        EXPECT_THROW(_udpMessageForwarder->getUdpClient(UDPMessageType::TIM), TmxException);
        EXPECT_THROW(_udpMessageForwarder->getUdpClient(UDPMessageType::BSM), TmxException);
        EXPECT_THROW(_udpMessageForwarder->getUdpClient(UDPMessageType::SPAT), TmxException);
        EXPECT_THROW(_udpMessageForwarder->getUdpClient(UDPMessageType::MAP), TmxException);
    }
    
    TEST_F(test_UDPMessageForwarder, test_getUdpClient_AFTER_ATTACH_UDP_CLIENT){
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

    TEST_F(test_UDPMessageForwarder, test_hexToBytes){
        //TIM Hex to bytes
        std::string timHex = "001f526011c35d000000000023667bac0407299b9ef9e7a9b9408230dfffe4386ba00078005a53373df3cf5372810461b90ffff53373df3cf53728104618129800010704a04c7d7976ca3501872e1bb66ad19b2620";
        std::string expectedBytes="031829617195930000035102123172474115515824923116918564130482232552285610716001200908355612432078311412949718515255245511152236024555401670241815201741607612512111820253113546271821062091553832";
        std::string bytesResult = _udpMessageForwarder->hexToBytes(timHex);
        std::stringstream bytesToIntResult;
        for(unsigned char byte: bytesResult){
            bytesToIntResult << (int)byte;
        }
       EXPECT_EQ(bytesToIntResult.str(), expectedBytes);
    }
}