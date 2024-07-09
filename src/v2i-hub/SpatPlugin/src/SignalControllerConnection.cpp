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

               
                ntcip1202->ToJ2735SPAT(spat,timeMs, intersectionName, intersectionId);
                FILE_LOG(tmx::utils::logDEBUG) << "Sending SPAT ..." << std::endl;
                if ( tmx::utils::FILELog::ReportingLevel() >= tmx::utils::logDEBUG) {
                    xer_fprint(stdout, &asn_DEF_SPAT, spat);
                }
                tmx::messages::SpatMessage _spatMessage(spat);
                tmx::messages::SpatEncodedMessage spatEncodedMsg;
                spatEncodedMsg.initialize(_spatMessage);
                return spatEncodedMsg;
            }
            else {
                throw std::runtime_error("Something went wrong");
            }
        }
        else {
            FILE_LOG(tmx::utils::logDEBUG) << "Receiving J2725 HEX SPAT ..." << std::endl;

            tmx::messages::SpatEncodedMessage spatEncodedMsg;
            auto payload = spatPacketReceiver->stringTimedReceive( 1000 );
            auto index = payload.find("Payload=");
            FILE_LOG(tmx::utils::logDEBUG) << "Found Payload at index " << index << std::endl;

            if ( index != std::string::npos ) {
                auto hex = payload.substr(index + 8);
                hex.erase(std::remove(hex.begin(), hex.end(), '\n'), hex.end());
                hex.erase(std::remove(hex.begin(), hex.end(), ' '), hex.end());

                FILE_LOG(tmx::utils::logDEBUG) << "Reading HEX String " << hex << std::endl;
                tmx::byte_stream bytes = tmx::byte_stream_decode(hex);

                FILE_LOG(tmx::utils::logDEBUG) << "Reading Bytes " << tmx::byte_stream_encode(bytes) ;
                tmx::messages::J2735MessageFactory myFactory;
                auto spatEncodedMsg = dynamic_cast<tmx::messages::SpatEncodedMessage*>(myFactory.NewMessage(bytes));
                if (tmx::utils::FILELog::ReportingLevel() >= tmx::utils::logDEBUG)
                {
                    xer_fprint(stdout, &asn_DEF_SPAT, spatEncodedMsg->decode_j2735_message().get_j2735_data().get());
                    PLOG(tmx::utils::logDEBUG) << "Message is "<< spatEncodedMsg->get_payload_str();
                }

                return *spatEncodedMsg;
            }
            else {
                throw std::runtime_error("Something went wrong");
            }
        }
    }
}