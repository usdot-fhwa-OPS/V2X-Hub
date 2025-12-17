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
            tmx::utils::snmp_response_obj enable_spat_resp;
            enable_spat_resp.val_int = 2;
            enable_spat_resp.type = tmx::utils::snmp_response_obj::response_type::INTEGER;
            status = status && scSNMPClient->process_snmp_request(NTCIP1202V2::ENABLE_SPAT_OID, tmx::utils::request_type::SET, enable_spat_resp);
        }
    
        return status;
    };

    void SignalControllerConnection::receiveBinarySPAT( SPAT * const spat, const std::shared_ptr<fwha_stol::lib::time::CarmaClock> &clock) {
        std::vector<char> buf(SPAT_BINARY_BUFFER_SIZE);
        auto numBytes = spatPacketReceiver->TimedReceive(buf.data(), SPAT_BINARY_BUFFER_SIZE, UDP_SERVER_TIMEOUT_MS);
        if (numBytes > 0)
        {
            // Convert Binary  buffer to SPAT pointer 
            Ntcip1202 ntcip1202;
            ntcip1202.setSignalGroupMappingList(this->signalGroupMapping);
            ntcip1202.copyBytesIntoNtcip1202(buf.data(), numBytes);
            ntcip1202.ToJ2735SPAT(spat,clock->nowInMilliseconds(), intersectionName, intersectionId);
            // Update status map with intersection status information
            updateIntersectionStatus(spat->intersections.list.array[0]->status);
            if (tmx::utils::FILELog::ReportingLevel() >= tmx::utils::logDEBUG)
            {
                asn_fprint(stdout, &asn_DEF_SPAT, spat);
            }
        }
        else {
            throw tmx::utils::UdpServerRuntimeError("UDP Server error occured or socket time out.");
        }
    }
    
    void SignalControllerConnection::receiveUPERSPAT(std::shared_ptr<tmx::messages::SpatEncodedMessage> &spatEncoded_ptr) {
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
            // Update status map with intersection status information
            updateIntersectionStatus(spatEncoded_ptr->decode_j2735_message().get_j2735_data()->intersections.list.array[0]->status);
            if (tmx::utils::FILELog::ReportingLevel() >= tmx::utils::logDEBUG)
            {
                asn_fprint(stdout, &asn_DEF_SPAT, spatEncoded_ptr->decode_j2735_message().get_j2735_data().get());
            }
        }
        else {
            throw tmx::TmxException("Could not find UPER Payload in received SPAT UDP Packet!");
        }
    }

    uint SignalControllerConnection::calculateSPaTInterval(uint64_t lastSpatMessage, uint64_t currentSpatMessage) {
        // Measure interval between SPAT messages
		if ( lastSpatMessage != 0 ) {
			uint64_t intervalMs = currentSpatMessage - lastSpatMessage;
            if ( intervalMs > SPAT_INTERVAL_MAX_THRESHOLD_MS ) {
                throw tmx::TmxException("Interval " + std::to_string(intervalMs) +
                     " ms between received SPAT information from TSC exceeded CTI 4501 maximum described limit of " 
                     + std::to_string(SPAT_INTERVAL_MAX_THRESHOLD_MS) +" ms");
            }
			return static_cast<uint>(intervalMs);
		}
		else {
			return static_cast<uint>(currentSpatMessage);
		}
    }

    void SignalControllerConnection::updateIntersectionStatus(const IntersectionStatusObject_t &status) {
        uint16_t statusVal = static_cast<uint16_t>(status.buf[0]) | (static_cast<uint16_t>(status.buf[1]) << 8);
        std::bitset<16> bitSet(statusVal);
        intersectionStatus["Manual Control Is Enabled"] = bitSet[IntersectionStatusObject::IntersectionStatusObject_manualControlIsEnabled];
        intersectionStatus["Stop Time Is Activated"] = bitSet[IntersectionStatusObject::IntersectionStatusObject_stopTimeIsActivated];
        intersectionStatus["Failure Flash"] = bitSet[IntersectionStatusObject::IntersectionStatusObject_failureFlash];
        intersectionStatus["Preemption Is Active"] = bitSet[IntersectionStatusObject::IntersectionStatusObject_preemptIsActive];
        intersectionStatus["Signal Priority Is Active"] = bitSet[IntersectionStatusObject::IntersectionStatusObject_signalPriorityIsActive];
        intersectionStatus["Fixed Time Operation"] = bitSet[IntersectionStatusObject::IntersectionStatusObject_fixedTimeOperation];
        intersectionStatus["Traffic Dependent Operation"] = bitSet[IntersectionStatusObject::IntersectionStatusObject_trafficDependentOperation];
        intersectionStatus["Standby Operation"] = bitSet[IntersectionStatusObject::IntersectionStatusObject_standbyOperation];
        intersectionStatus["Failure Mode"] = bitSet[IntersectionStatusObject::IntersectionStatusObject_failureMode];
        intersectionStatus["Off"] = bitSet[IntersectionStatusObject::IntersectionStatusObject_off];
        intersectionStatus["Recent MAP Message Update"] = bitSet[IntersectionStatusObject::IntersectionStatusObject_recentMAPmessageUpdate];
        intersectionStatus["Recent Change in MAP Assigned Lane Ids Used"] = bitSet[IntersectionStatusObject::IntersectionStatusObject_recentChangeInMAPassignedLanesIDsUsed];
        intersectionStatus["No Valid SPAT is Available At This Time"] = bitSet[IntersectionStatusObject::IntersectionStatusObject_noValidSPATisAvailableAtThisTime];
    }

    std::map<std::string, bool> SignalControllerConnection::getIntersectionStatus() const{
        return intersectionStatus;
    }

}