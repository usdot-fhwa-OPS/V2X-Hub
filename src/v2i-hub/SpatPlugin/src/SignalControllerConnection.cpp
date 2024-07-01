#include "SignalControllerConnection.h"

namespace SpatPlugin {

    SignalControllerConnection::SignalControllerConnection(const std::string &localIp, unsigned int localPort, const std::string &signalGroupMapping, const std::string &scIp, unsigned int scSNMPPort, const std::string &intersectionName, unsigned int intersectionId) : spatPacketReceiver(std::make_unique<tmx::utils::UdpServer>(localIp, localPort)) ,scSNMPClient(std::make_unique<tmx::utils::snmp_client>(scIp, scSNMPPort ,"", "", "", "")), signalGroupMapping(signalGroupMapping), intersectionName(intersectionName), intersectionId(intersectionId) {

    };

    tmx::messages::SpatEncodedMessage SignalControllerConnection::receiveSPAT(uint64_t timeMs , const SPAT_MODE &spatMode = SPAT_MODE::BINARY) {
        if ( spatMode == SPAT_MODE::BINARY ) {
            char buf[1000];
            auto ntcip1202 = std::make_unique<Ntcip1202>(clock);
            auto numBytes = spatPacketReceiver->TimedReceive(buf, 1000, 3);
            if ( numBytes > 0 ) {
                // TODO: Revist this implementation. See if we can make SPAT a shared pointer
                // and skipe the SPAT to SpatMessage conversion.
                ntcip1202->copyBytesIntoNtcip1202(buf, numBytes);
                SPAT *_spat = (SPAT *) calloc(1, sizeof(SPAT));
                ntcip1202->ToJ2735r41SPAT(_spat, intersectionName, intersectionId);
                auto _spatMessage = std::make_unique<tmx::messages::SpatMessage>(_spat);
                tmx::messages::MessageFrameMessage frame(_spatMessage->get_j2735_data());
                tmx::messages::SpatEncodedMessage spatEncodedMsg;
                spatEncodedMsg.set_data(tmx::messages::TmxJ2735EncodedMessage<SPAT>::encode_j2735_message<tmx::messages::codec::uper<tmx::messages::MessageFrameMessage>>(frame));
                //Free the memory allocated for MessageFrame
                free(frame.get_j2735_data().get());

            }else {
                throw std::runtime_error("Something went wrong");
            }
        }
        else if ( spatMode == SPAT_MODE::J2735_HEX ) {

        }
    }

}