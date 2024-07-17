#pragma once

#include <UdpServer.h>
#include <SNMPClient.h>
#include <tmx/j2735_messages/SpatMessage.hpp>
#include "NTCIP1202.h"
#include <PluginLog.h>
#include <tmx/messages/byte_stream.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <tmx/TmxException.hpp>
#include <gtest/gtest_prod.h>  



namespace SpatPlugin {

    class SignalControllerConnection
    {
        private:
            // UDP Server socket listening for SPAT
            std::shared_ptr<tmx::utils::UdpServer> spatPacketReceiver;

            std::shared_ptr<tmx::utils::snmp_client> scSNMPClient;

            std::string signalGroupMapping;
            std::string intersectionName;
            unsigned int intersectionId;
            friend class TestSignalControllerConnection;

        public:
            SignalControllerConnection(const std::string &localIp, unsigned int localPort, const std::string &signalGroupMapping, const std::string &scIp, unsigned int scSNMPPort, const std::string &intersectionName, unsigned int intersectionID);
            
            bool initializeSignalControllerConnection();

            void receiveBinarySPAT(std::shared_ptr<SPAT> &spat, uint64_t timeMs);

            void receiveUPERSPAT(std::shared_ptr<tmx::messages::SpatEncodedMessage> &spatEncoded_ptr);
    };
}