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

    SignalControllerConnection::SignalControllerConnection(const std::string &localIp, unsigned int localPort, const std::string &signalGroupMapping, const std::string &scIp, unsigned int scSNMPPort, const std::string &intersectionName, unsigned int intersectionId) : spatPacketReceiver(std::make_shared<tmx::utils::UdpServer>(localIp, localPort)) ,scSNMPClient(std::make_shared<tmx::utils::snmp_client>(scIp, scSNMPPort ,"administrator", "", "", "")), signalGroupMapping(signalGroupMapping), intersectionName(intersectionName), intersectionId(intersectionId) {

    };
    bool SignalControllerConnection::initializeSignalControllerConnection(bool enable_spat, bool set_intersection_id) const {
        // TODO : Update to more generic TSC Initialization process that simply follows NTCIP 1202 version guidelines.
        bool status = true;
        if (enable_spat)
        {
            tmx::utils::snmp_response_obj enable_spat;
            enable_spat.val_int = 2;
            enable_spat.type = tmx::utils::snmp_response_obj::response_type::INTEGER;
            status = status && scSNMPClient->process_snmp_request(NTCIP1202V2::ENABLE_SPAT_OID, tmx::utils::request_type::SET, enable_spat);
        }
        if ( set_intersection_id ) {
            tmx::utils::snmp_response_obj intersection_id;
            intersection_id.val_int = intersectionId;
            intersection_id.type = tmx::utils::snmp_response_obj::response_type::INTEGER;
            status = status && scSNMPClient->process_snmp_request(NTCIP1202V3::INTERSECTION_ID, tmx::utils::request_type::SET, intersection_id);
        }
        return status;
    };

    void SignalControllerConnection::receiveBinarySPAT(const std::shared_ptr<SPAT> &spat, uint64_t timeMs ) const {
        FILE_LOG(tmx::utils::logDEBUG) << "Receiving binary SPAT ..." << std::endl;
        char buf[1000];
        auto numBytes = spatPacketReceiver->TimedReceive(buf, 1000, 1000);
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
        auto payload = spatPacketReceiver->stringTimedReceive( 1000 );
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