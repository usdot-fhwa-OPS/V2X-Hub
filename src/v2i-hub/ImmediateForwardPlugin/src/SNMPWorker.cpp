#include "SNMPWorker.h"

using namespace tmx::utils;

namespace ImmediateForward {
    void clearImmediateForwardTable(const std::unique_ptr<tmx::utils::snmp_client> &client) {

        // FILE_LOG(logDEBUG) << "Retrieving max Imf rows ..." ;
        // snmp_response_obj maxImfsRep;
        // maxImfsRep.type = snmp_response_obj::response_type::INTEGER;
        // bool connected = client->process_snmp_request(rsu::mib::ntcip1218::maxRsuIFMs, request_type::GET, maxImfsRep);
        // auto maxImfs = maxImfsRep.val_int;
        // FILE_LOG(logDEBUG) << "Max Imf rows " << maxImfs ;
        // auto curIndex = 0;
        // if (connected) {
        //     // while ( maxImfs > curIndex || 16 > curIndex) {
        //     //     snmp_response_obj deleteRowRep;
        //     //     deleteRowRep.type = snmp_response_obj::response_type::INTEGER;
        //     //     deleteRowRep.val_int = 6;
        //     //     std::string oid = rsu::mib::ntcip1218::rsuImfRowStatus +  "." + std::to_string(curIndex);
        //     //     connected = client->process_snmp_request(oid, request_type::SET, deleteRowRep);
        //     //     curIndex++;
        //     //     if (!connected) {
        //     //         break;
        //     //     }

        //     // }
        // }
        
    }

    std::unordered_map<std::string, unsigned int> initializeImmediateForwardTable(const std::unique_ptr<snmp_client> &client, const std::vector<Message> &messages){
        std::unordered_map<std::string, unsigned int> tmxMessageTypeToIMFTableIndex;
        auto curIndex = 1;
        FILE_LOG(logDEBUG1) << "Setting RSU Mode to 2" ;
        snmpRequest psid{
            rsu::mib::ntcip1218::rsuModeOid,
            'i',
            "2"
        };
        std::vector<snmpRequest> requests;
        requests.push_back(psid);
        client->process_snmp_set_requests(requests);
        sleep(1);
        for (const auto &message : messages)
        {
            //create new row entry
            FILE_LOG(logDEBUG1) << "Creating row " + curIndex ;
            std::vector<snmpRequest> requests;
           
            size_t pos = message.psid.find("x");
            if (pos == std::string::npos) {
                throw tmx::TmxException("Message PSID " + message.psid + " is malformed and should be formated 0x<PSID HEX>");
            }
            std::string messagePsidwithoutPrefix = message.psid.substr(pos+1);
            snmpRequest psid{
                rsu::mib::ntcip1218::rsuIFMPsidOid + "." + std::to_string(curIndex),
                'x',
                messagePsidwithoutPrefix
            };

            snmpRequest channel{
                rsu::mib::ntcip1218::rsuIFMTxChannelOid + "." + std::to_string(curIndex),
                'i',
                std::to_string(message.channel.value())
            };
            snmpRequest payload{
                rsu::mib::ntcip1218::rsuIFMPayloadOid + "." + std::to_string(curIndex),
                'x',
                "00133500100f368500001e0f702001043f8000464fc64f8000780000010237c0002327e327c0003c000000c123e0001193f193e0001e0000"
            };

            snmpRequest enable{
                rsu::mib::ntcip1218::rsuIFMEnableOid + "." + std::to_string(curIndex),
                'i',
                "1"
            };
            snmpRequest creatRow{
                rsu::mib::ntcip1218::rsuIFMStatusOid + "." + std::to_string(curIndex),
                'i',
                "4"
            };
            snmpRequest priority{
                rsu::mib::ntcip1218::rsuIFMPriorityOid + "." + std::to_string(curIndex),
                'i',
                "6"
            };

            snmpRequest options{
                rsu::mib::ntcip1218::rsuIFMOptionsOid + "." + std::to_string(curIndex),
                'x',
                "01"
            };
            
            requests.assign({psid, channel,payload, enable, creatRow, priority, options});
            client->process_snmp_set_requests(requests);
            
            // Add message to table with index
            tmxMessageTypeToIMFTableIndex[message.sendType] = curIndex;
            // Increment index
            curIndex++;
        }
        return tmxMessageTypeToIMFTableIndex;
    }

    void sendNTCIP1218ImfMessage(const std::unique_ptr<snmp_client> &client, const std::string &message, unsigned int index){
        snmp_response_obj resp;
        resp.type = snmp_response_obj::response_type::STRING;
        resp.val_string = std::vector<char>(message.begin(), message.end());
        std::string oid = rsu::mib::ntcip1218::rsuIFMPayloadOid +  "." + std::to_string(index);
        client->process_snmp_request(oid, request_type::SET, resp);
    }


}