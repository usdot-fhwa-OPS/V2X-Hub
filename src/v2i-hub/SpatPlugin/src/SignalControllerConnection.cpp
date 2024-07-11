#include "SignalControllerConnection.h"

namespace SpatPlugin {

    SignalControllerConnection::SignalControllerConnection(const std::string &localIp, unsigned int localPort, const std::string &signalGroupMapping, const std::string &scIp, unsigned int scSNMPPort, const std::string &intersectionName, unsigned int intersectionId) : spatPacketReceiver(std::make_unique<tmx::utils::UdpServer>(localIp, localPort)) ,scSNMPClient(std::make_unique<tmx::utils::snmp_client>(scIp, scSNMPPort ,"administrator", "", "", "")), signalGroupMapping(signalGroupMapping), intersectionName(intersectionName), intersectionId(intersectionId) {

    };
    bool SignalControllerConnection::initializeSignalControllerConnection() {
        tmx::utils::snmp_response_obj resp;
        resp.val_int = 2;
        resp.type = tmx::utils::snmp_response_obj::response_type::INTEGER;
        return scSNMPClient->process_snmp_request("1.3.6.1.4.1.1206.3.5.2.9.44.1.0", tmx::utils::request_type::SET, resp);
    };

    void SignalControllerConnection::receiveBinarySPAT(std::shared_ptr<SPAT> spat, uint64_t timeMs ) {
        FILE_LOG(tmx::utils::logDEBUG) << "Receiving binary SPAT ..." << std::endl;
        char buf[1000];
        auto numBytes = spatPacketReceiver->TimedReceive(buf, 1000, 1000);
        
        if ( numBytes > 0 ) {
            // Convert Binary  buffer to SPAT pointer 
            Ntcip1202 ntcip1202;
            ntcip1202.setSignalGroupMappingList(this->signalGroupMapping);
            ntcip1202.copyBytesIntoNtcip1202(buf, numBytes);
            ntcip1202.ToJ2735SPAT(spat.get(),timeMs, intersectionName, intersectionId);
           
        }
        else {
            throw tmx::utils::UdpServerRuntimeError("UDP Server error occured or socket time out.");
        }
    }
    
    void SignalControllerConnection::receiveUPERSPAT(std::shared_ptr<tmx::messages::SpatEncodedMessage> spatEncoded_ptr) {
        FILE_LOG(tmx::utils::logDEBUG) << "Receiving J2725 HEX SPAT ..." << std::endl;
        auto payload = spatPacketReceiver->stringTimedReceive( 1000 );
        auto index = payload.find("Payload=");
        if ( index != std::string::npos ) {
            // Retreive hex string payload
            auto hex = payload.substr(index + 8);
            // Remove new lines and empty space
            hex.erase(std::remove(hex.begin(), hex.end(), '\n'), hex.end());
            hex.erase(std::remove(hex.begin(), hex.end(), ' '), hex.end());
            FILE_LOG(tmx::utils::logDEBUG) << "Reading HEX String " << hex << std::endl;
            // Convert to byte stream
            tmx::byte_stream bytes = tmx::byte_stream_decode(hex);
            // Read SpateEncodedMessage from bytes
            tmx::messages::J2735MessageFactory myFactory;
            spatEncoded_ptr.reset(dynamic_cast<tmx::messages::SpatEncodedMessage*>(myFactory.NewMessage(bytes)));
            if (tmx::utils::FILELog::ReportingLevel() >= tmx::utils::logDEBUG)
            {
                xer_fprint(stdout, &asn_DEF_SPAT, spatEncoded_ptr->decode_j2735_message().get_j2735_data().get());
            }
        }
        else {
            throw tmx::TmxException("Could not find UPER Payload in received SPAT UDP Packet!");
        }
    }

}