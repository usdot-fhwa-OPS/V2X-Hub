/**
 * Copyright (C) 2024 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
#include "SignalControllerConnection.h"

namespace SpatPlugin {

    SignalControllerConnection::SignalControllerConnection(const std::string &localIp, unsigned int localPort, const std::string &signalGroupMapping, const std::string &scIp, unsigned int scSNMPPort, const std::string &scSNMPCommunity, const std::string &intersectionName, unsigned int intersectionId) : spatPacketReceiver(std::make_shared<tmx::utils::UdpServer>(localIp, localPort)) ,scSNMPClient(std::make_shared<tmx::utils::snmp_client>(scIp, scSNMPPort ,scSNMPCommunity, "", "", "")), signalGroupMapping(signalGroupMapping), intersectionName(intersectionName), intersectionId(intersectionId) {

    };
    bool SignalControllerConnection::initializeSignalControllerConnection(bool enable_spat) const {
        // TODO : Update to more generic TSC Initialization process that simply follows NTCIP 1202 version guidelines. Also
        // set intersection ID in J2735_HEX SPAT Mode. The HEX payload should include a intersection ID.  
        bool status = true;
        if (enable_spat)
        {   
            // For binary SPAT a value of 2 enables original SPAT binary broadcast on the TSC and a value of 6 enables original SPAT plugin additional Pedestrian Information.
            // NOTE: Pedestrian information is untested.
            tmx::utils::snmp_response_obj enable_spat;
            enable_spat.val_int = 2;
            enable_spat.type = tmx::utils::snmp_response_obj::response_type::INTEGER;
            status = status && scSNMPClient->process_snmp_request(NTCIP1202V2::ENABLE_SPAT_OID, tmx::utils::request_type::SET, enable_spat);
        }
    
        return status;
    };

    void SignalControllerConnection::receiveBinarySPAT(const std::shared_ptr<SPAT> &spat, uint64_t timeMs ) const {
        FILE_LOG(tmx::utils::logDEBUG) << "Receiving binary SPAT ..." << std::endl;
        char buf[SPAT_BINARY_BUFFER_SIZE];
        auto numBytes = spatPacketReceiver->TimedReceive(buf, SPAT_BINARY_BUFFER_SIZE, UDP_SERVER_TIMEOUT_MS);
        if (numBytes > 0)
        {
            // Convert Binary  buffer to SPAT pointer 
            Ntcip1202 ntcip1202;
            ntcip1202.setSignalGroupMappingList(this->signalGroupMapping);
            ntcip1202.copyBytesIntoNtcip1202(buf, numBytes);
            ntcip1202.ToJ2735SPAT(spat.get(),timeMs, intersectionName, intersectionId);
            if (tmx::utils::FILELog::ReportingLevel() >= tmx::utils::logDEBUG)
            {
                xer_fprint(stdout, &asn_DEF_SPAT, spat.get());
            }
        }
        else {
            throw tmx::utils::UdpServerRuntimeError("UDP Server error occured or socket time out.");
        }
    }
    
    void SignalControllerConnection::receiveUPERSPAT(std::shared_ptr<tmx::messages::SpatEncodedMessage> &spatEncoded_ptr) const {
        FILE_LOG(tmx::utils::logDEBUG1) << "Receiving J2725 HEX SPAT ..." << std::endl;
        auto payload = spatPacketReceiver->stringTimedReceive( UDP_SERVER_TIMEOUT_MS );
        auto index = payload.find("Payload=");
        if ( index != std::string::npos ) {
            // Retreive hex string payload
            auto hex = payload.substr(index + 8);
            // Remove new lines and empty space
            hex.erase(std::remove(hex.begin(), hex.end(), '\n'), hex.end());
            hex.erase(std::remove(hex.begin(), hex.end(), ' '), hex.end());
            FILE_LOG(tmx::utils::logDEBUG1) << "Reading HEX String " << hex << std::endl;
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