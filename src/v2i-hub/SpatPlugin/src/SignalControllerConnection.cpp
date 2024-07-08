#include "SignalControllerConnection.h"

namespace SpatPlugin {

    SignalControllerConnection::SignalControllerConnection(const std::string &localIp, unsigned int localPort, const std::string &signalGroupMapping, const std::string &scIp, unsigned int scSNMPPort, const std::string &intersectionName, unsigned int intersectionId) : spatPacketReceiver(std::make_unique<tmx::utils::UdpServer>(localIp, localPort)) ,scSNMPClient(std::make_unique<tmx::utils::snmp_client>(scIp, scSNMPPort ,"administrator", "", "", "")), signalGroupMapping(signalGroupMapping), intersectionName(intersectionName), intersectionId(intersectionId) {

    };
    bool SignalControllerConnection::initializeSignalControllerConnection() {
        tmx::utils::snmp_response_obj resp;
        resp.val_int = 2;
        return scSNMPClient->process_snmp_request("1.3.6.1.4.1.1206.3.5.2.9.44.1.0", tmx::utils::request_type::SET, resp);
    };
    tmx::messages::SpatEncodedMessage SignalControllerConnection::receiveSPAT(SPAT *spat, uint64_t timeMs, const SPAT_MODE &spatMode)
    {
        if ( spatMode == SPAT_MODE::BINARY ) {
            FILE_LOG(tmx::utils::logDEBUG) << "Receiving binary SPAT ..." << std::endl;
            char buf[1000];
            auto numBytes = spatPacketReceiver->TimedReceive(buf, 1000, 1000);
            auto ntcip1202 = std::make_unique<Ntcip1202>();
            ntcip1202->setSignalGroupMappingList(this->signalGroupMapping);

            if ( numBytes > 0 ) {
                // TODO: Revist this implementation. See if we can make SPAT a shared pointer
                // and skipe the SPAT to SpatMessage conversion.
                FILE_LOG(tmx::utils::logDEBUG) << "Decoding binary SPAT from " << numBytes << " bytes ..." << std::endl;
                ntcip1202->copyBytesIntoNtcip1202(buf, numBytes);
                FILE_LOG(tmx::utils::logDEBUG) << "Read SPAT Bytes ..." << std::endl;

               
                ntcip1202->ToJ2735SPAT(spat,timeMs, intersectionName, intersectionId);
                FILE_LOG(tmx::utils::logDEBUG) << "Copied into SPAT object ..." << std::endl;
                if ( tmx::utils::FILELog::ReportingLevel() >= tmx::utils::logDEBUG) {
                    xer_fprint(stdout, &asn_DEF_SPAT, spat);
                }
                tmx::messages::SpatMessage _spatMessage(spat);
                tmx::messages::SpatEncodedMessage spatEncodedMsg;
                spatEncodedMsg.initialize(_spatMessage);
                // tmx::messages::MessageFrameMessage frame(_spatMessage.get_j2735_data());
                // FILE_LOG(tmx::utils::logDEBUG) << "Copied into  SPAT  Message Frame..." << std::endl;
                // spatEncodedMsg.set_data(tmx::messages::TmxJ2735EncodedMessage<SPAT>::encode_j2735_message<tmx::messages::codec::uper<tmx::messages::MessageFrameMessage>>(frame));
                FILE_LOG(tmx::utils::logDEBUG) << "Copied into encoded SPAT Message" << std::endl;
                return spatEncodedMsg;
            }
            else {
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
                 xer_fprint(stdout, &asn_DEF_SPAT, spatEncodedMsg.decode_j2735_message().get_j2735_data().get());

                return spatEncodedMsg;
            }
            else {
                throw std::runtime_error("Something went wrong");
            }
        }
    }
}