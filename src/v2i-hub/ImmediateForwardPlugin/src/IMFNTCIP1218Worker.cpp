#include "IMFNTCIP1218Worker.h"

using namespace tmx::utils;

namespace ImmediateForward {
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
           
            size_t pos = message.psid.find("x");
            if (pos == std::string::npos) {
                throw tmx::TmxException("Message PSID " + message.psid + " is malformed and should be formated 0x<PSID HEX>");
            }
            std::string messagePsidwithoutPrefix = message.psid.substr(pos+1);
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
            snmp_request enable{
                rsu::mib::ntcip1218::rsuIFMEnableOid + "." + std::to_string(curIndex),
                'i',
                "1"
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

    void sendNTCIP1218ImfMessage( snmp_client* const client, const std::string &message, unsigned int index){
        
        snmp_request payload {
            rsu::mib::ntcip1218::rsuIFMPayloadOid + "." + std::to_string(index),
            'x',
            message
        };
        std::vector reqs {payload};
        client->process_snmp_set_requests(reqs);
    }


}