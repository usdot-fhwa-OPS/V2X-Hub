#include "IMFNTCIP1218Worker.h"

using namespace tmx::utils;

namespace ImmediateForward {

    std::string stripPsidPrefix(const std::string &psid) {
        size_t pos = psid.find("x");
        return pos == std::string::npos ? psid : psid.substr(pos + 1);
    }

    void clearImmediateForwardTable( tmx::utils::snmp_client* const client) {

        FILE_LOG(logDEBUG) << "Retrieving max Imf rows ..." ;
        snmp_response_obj maxImfsRep;
        maxImfsRep.type = snmp_response_obj::response_type::INTEGER;
        bool connected = client->process_snmp_request(rsu::mib::ntcip1218::maxRsuIFMs, request_type::GET, maxImfsRep);
        auto maxImfs = maxImfsRep.val_int;
        FILE_LOG(logDEBUG) << "Max Imf rows " << maxImfs ;
        auto curIndex = 1;
        if (connected) {
            while ( maxImfs >= curIndex ) {
                snmp_response_obj deleteRowRep;
                deleteRowRep.type = snmp_response_obj::response_type::INTEGER;
                deleteRowRep.val_int = 6;
                std::string oid = rsu::mib::ntcip1218::rsuIFMStatusOid +  "." + std::to_string(curIndex);
                connected = client->process_snmp_request(oid, request_type::SET, deleteRowRep);
                if (!connected) {
                    FILE_LOG(logWARNING) << "Cleared IMF Table Rows 0 to "<< curIndex ;
                    break;
                }
                curIndex++;

            }
        }
        
    }

    void setRSUMode(tmx::utils::snmp_client* const client, unsigned int mode) {
        snmp_response_obj obj;
        obj.type = snmp_response_obj::response_type::INTEGER;
        obj.val_int = mode;
        bool operational = client->process_snmp_request(rsu::mib::ntcip1218::rsuModeOid, request_type::SET, obj);
        if (!operational) {
            throw tmx::TmxException("Failed to set RSU to operational mode");
        }
    }

    void waitForRSUModeStandby(tmx::utils::snmp_client* const client, unsigned int retry, unsigned int interval) {
        snmp_response_obj obj;
        obj.type = snmp_response_obj::response_type::INTEGER;
        do {
            bool operational = client->process_snmp_request(rsu::mib::ntcip1218::rsuModeOid, request_type::GET, obj);
            if (!operational) {
                throw tmx::TmxException("Failed to get RSU to operational mode");
            }
            retry--;
            sleep(interval);
        }
        while ( retry > 0 && obj.val_int != 2);
        if ( obj.val_int != 2) {
            throw tmx::TmxException("Failed to set RSU Mode to Standby(2)");
        } 
    }

    std::unordered_map<std::string, unsigned int> initializeImmediateForwardTable( snmp_client* const client, const std::vector<MessageConfig> &messageConfigs, bool signMessages, const std::string &payloadPlaceholder) {
        std::unordered_map<std::string, unsigned int> tmxMessageTypeToIMFTableIndex;
        // Immediate Forward Messages Table index starts with 1
        auto curIndex = 1;
        FILE_LOG(logDEBUG1) << "Initializing RSU IMF Table" ;
        for (const auto &message : messageConfigs)
        {
            //create new row entry
            FILE_LOG(logDEBUG1) << "Creating IMF row " + std::to_string(curIndex) ;
            std::vector<snmp_request> requests;
           
            if (message.psid.find("x") == std::string::npos) {
                throw tmx::TmxException("Message PSID " + message.psid + " is malformed and should be formated 0x<PSID HEX>");
            }
            std::string messagePsidwithoutPrefix = stripPsidPrefix(message.psid);
            snmp_request psid{
                rsu::mib::ntcip1218::rsuIFMPsidOid + "." + std::to_string(curIndex),
                'x',
                messagePsidwithoutPrefix
            };
            snmp_request channel{
                rsu::mib::ntcip1218::rsuIFMTxChannelOid + "." + std::to_string(curIndex),
                'i',
                std::to_string(message.channel.value())
            };
            snmp_request payload{
                rsu::mib::ntcip1218::rsuIFMPayloadOid + "." + std::to_string(curIndex),
                'x',
                payloadPlaceholder
            };
            // Set enable to false for placeholder payload while rsuMode is standby to prevent transmission and Xmit errors
            snmp_request enable{
                rsu::mib::ntcip1218::rsuIFMEnableOid + "." + std::to_string(curIndex),
                'i',
                "0"
            };
            snmp_request creatRow{
                rsu::mib::ntcip1218::rsuIFMStatusOid + "." + std::to_string(curIndex),
                'i',
                "4"
            };
            snmp_request priority{
                rsu::mib::ntcip1218::rsuIFMPriorityOid + "." + std::to_string(curIndex),
                'i',
                "6"
            };
            // Yunex value for signed messages
            // binary 10000000 to hexidecimal 80 ( see rsuIFMOptionsOid for bit values )
            snmp_request options;
            if (signMessages ) {
                options = {
                    rsu::mib::ntcip1218::rsuIFMOptionsOid + "." + std::to_string(curIndex),
                    'x',
                    "80"
                };
            }
            else {
                options = {
                    rsu::mib::ntcip1218::rsuIFMOptionsOid + "." + std::to_string(curIndex),
                    'x',
                    "00"
                };
            }
            requests.assign({psid, channel,payload, enable, creatRow, priority, options});
            bool success = client->process_snmp_set_requests(requests);
            if ( !success) {
                throw tmx::TmxException("Failed to create IMF row " + std::to_string(curIndex));
            }
            // Add message to table with index
            tmxMessageTypeToIMFTableIndex[message.sendType] = curIndex;

            // Increment index
            curIndex++;
        }

        
        return tmxMessageTypeToIMFTableIndex;
    }

    void sendNTCIP1218ImfMessage( snmp_client* const client, const std::string &message, unsigned int index, const std::string &psid, bool signMessage){

        // A row is shared by every message of one send type, and a forwarded raw SPDU broadcasts
        // under the PSID it arrived with rather than the configured one, and must not be signed
        // again. Writing both on every send keeps the row from carrying over what the previous
        // message left behind. All requests go out in a single SET PDU, so this costs no additional
        // round trip.
        snmp_request psidRequest{
            rsu::mib::ntcip1218::rsuIFMPsidOid + "." + std::to_string(index),
            'x',
            stripPsidPrefix(psid)
        };
        // 80 HEX is binary 10000000, setting Bit 0 = Process1609.2, which asks the RSU to secure the
        // message itself. 00 HEX leaves Bit 0 = Bypass1609.2, which per NTCIP 1218 5.5.2.7 "allows
        // the RSU to send the message that has been signed and/or encrypted by the TMC", wrapping it
        // in a WSMP header and nothing more. That is what a forwarded raw SPDU needs. Bit 1
        // (Secure/Unsecure) is ignored when Bit 0 = 0.
        // Note Bit 0 is the most significant bit, which is what a Yunex RSU expects.
        snmp_request options{
            rsu::mib::ntcip1218::rsuIFMOptionsOid + "." + std::to_string(index),
            'x',
            signMessage ? "80" : "00"
        };
        snmp_request payload {
            rsu::mib::ntcip1218::rsuIFMPayloadOid + "." + std::to_string(index),
            'x',
            message
        };
        // Enable the message for transmission
        snmp_request enable{
                rsu::mib::ntcip1218::rsuIFMEnableOid + "." + std::to_string(index),
                'i',
                "1"
        };
        std::vector reqs {psidRequest, options, payload, enable};
        client->process_snmp_set_requests(reqs);
    }


}