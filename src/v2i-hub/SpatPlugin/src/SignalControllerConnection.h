#pragma once

#include <UdpServer.h>
#include <SNMPClient.h>
#include <tmx/j2735_messages/SpatMessage.hpp>
#include "NTCIP1202.h"
#include <PluginLog.h>

namespace SpatPlugin {
    enum class SPAT_MODE
    {
        J2735_HEX,
        BINARY
    };

    class SignalControllerConnection
    {
        private:
            // UDP Server socket listening for SPAT
            std::unique_ptr<tmx::utils::UdpServer> spatPacketReceiver;

            std::unique_ptr<tmx::utils::snmp_client> scSNMPClient;

            std::string signalGroupMapping;
            std::string intersectionName;
            unsigned int intersectionId;

        public:
            SignalControllerConnection(const std::string &localIp, unsigned int localPort, const std::string &signalGroupMapping, const std::string &scIp, unsigned int scSNMPPort, const std::string &intersectionName, unsigned int intersectionID);
            bool initializeSignalControllerConnection();
            tmx::messages::SpatEncodedMessage receiveSPAT(SPAT *spat, uint64_t timeMs , const SPAT_MODE &spat_mode = SPAT_MODE::BINARY);

    };
}