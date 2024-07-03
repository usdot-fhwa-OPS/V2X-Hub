#include "SignalControllerConnection.h"

namespace SpatPlugin {

    SignalControllerConnection::SignalControllerConnection(const std::string &localIp, unsigned int localPort, const std::string &signalGroupMapping, const std::string &scIp, unsigned int scSNMPPort, const std::string &intersectionName, unsigned int intersectionId) : spatPacketReceiver(std::make_unique<tmx::utils::UdpServer>(localIp, localPort)) ,scSNMPClient(std::make_unique<tmx::utils::snmp_client>(scIp, scSNMPPort ,"administrator", "", "", "")), signalGroupMapping(signalGroupMapping), intersectionName(intersectionName), intersectionId(intersectionId) {

    };
    bool SignalControllerConnection::initializeSignalControllerConnection() {
        tmx::utils::snmp_response_obj resp;
        resp.val_int = 2;
        return scSNMPClient->process_snmp_request("1.3.6.1.4.1.1206.3.5.2.9.44.1.0", tmx::utils::request_type::SET, resp);
    };
    tmx::messages::SpatEncodedMessage SignalControllerConnection::receiveSPAT(uint64_t timeMs, const SPAT_MODE &spatMode)
    {
        if ( spatMode == SPAT_MODE::BINARY ) {
            FILE_LOG(tmx::utils::logDEBUG) << "Receiving binary SPAT..." << std::endl;
            char buf[1000];
            auto numBytes = spatPacketReceiver->TimedReceive(buf, 1000, 1000);
            auto ntcip1202 = std::make_unique<Ntcip1202>();
           
            if ( numBytes > 0 ) {
                // TODO: Revist this implementation. See if we can make SPAT a shared pointer
                // and skipe the SPAT to SpatMessage conversion.
                ntcip1202->copyBytesIntoNtcip1202(buf, numBytes);
                SPAT *_spat = (SPAT *) calloc(1, sizeof(SPAT));
                if ( tmx::utils::FILELog::ReportingLevel() == tmx::utils::logDEBUG) {
                    xer_fprint(stdout, &asn_DEF_SPAT, _spat);
                }
                ntcip1202->ToJ2735SPAT(_spat,timeMs, intersectionName, intersectionId);
                auto _spatMessage = std::make_unique<tmx::messages::SpatMessage>(_spat);
                tmx::messages::MessageFrameMessage frame(_spatMessage->get_j2735_data());
                tmx::messages::SpatEncodedMessage spatEncodedMsg;
                spatEncodedMsg.set_data(tmx::messages::TmxJ2735EncodedMessage<SPAT>::encode_j2735_message<tmx::messages::codec::uper<tmx::messages::MessageFrameMessage>>(frame));
                //Free the memory allocated for MessageFrame
                free(_spat);
                return spatEncodedMsg;
            }else {
                throw std::runtime_error("Something went wrong");
            }
        }
        else {
            FILE_LOG(tmx::utils::logDEBUG) << "Receiving J2725 HEX SPAT ..." << std::endl;

            tmx::messages::SpatEncodedMessage spatEncodedMsg;
            tmx::byte_stream buf(4000);
            int numBytes = spatPacketReceiver->TimedReceive((char *)buf.data(), buf.size(), 3);

            if ( numBytes > 0 ) {
                spatEncodedMsg.set_payload_bytes(buf);
                return spatEncodedMsg;
            }
            else {
                throw std::runtime_error("Something went wrong");
            }
        }
    }
}