//==========================================================================
// Name        : PreemptionPlugin.cpp
// Author      : Leidos Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 Leidos Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Preemption Plugin
//==========================================================================

#include "PreemptionPluginWorker.hpp"

using namespace std;

namespace PreemptionPlugin {

	void PreemptionPluginWorker::ProcessMapMessageFile(std::string path){

        std::cout << "Processing map file" << std::endl;
        ifstream message_file;
        std::string map_message;

        try {
            message_file.open(path);
            if (message_file.is_open()) {
                while (!message_file.eof()) {
                    message_file >> map_message;
                    std::getline(message_file,map_message);
                }
            }
            message_file.close();
        }
        catch(...) { 
            std::cout << "Caught exception from reading a file"; 
        } 

        if(map_message != ""){
            std::cout << map_message; 

            int len = map_message.length();
            char buf[map_message.length()/2];

            int j = 0;
            
            for(int i = 0; i < len ; i+=2){
                char temp_hex[2];
                strcpy(temp_hex, map_message.substr(i, 2).c_str());
                long int li1 = strtol(temp_hex, nullptr,16);
                buf[j] = li1;
                j = j + 1;
            }

            asn_dec_rval_t rval;
            MessageFrame_t *message = 0;
            rval = uper_decode(0, &asn_DEF_MessageFrame, (void **) &message, buf, map_message.length()/2, 0, 0);

            if(rval.code == RC_OK) {
                map = &(message -> value.choice.MapData);
                std::cout << "finished processing map";
            }
        }
    }
    
    void PreemptionPluginWorker::VehicleLocatorWorker(BsmMessage* msg){

        PreemptionObject* po = new PreemptionObject;

        auto bsm = msg->get_j2735_data();

        int buff_size = bsm->coreData.id.size;
        po ->vehicle_id = bsm->coreData.id.buf;

        std::cout << " Vehicle id is " << po ->vehicle_id << std::endl;

        int32_t latitude = bsm->coreData.lat;
        int32_t longitude = bsm->coreData.Long;

        int32_t lat_offset = latitude - this->map->intersections[0].list.array[0]->refPoint.lat;
        int32_t long_offset = longitude - this->map->intersections[0].list.array[0]->refPoint.Long;

        std::cout << " Offset lat , long " << lat_offset << " , " << long_offset << std::endl;

        int min_distance = 999999;
        int lane_id = -1;

        for(int i = 0; i< this->map->intersections[0].list.array[0]->laneSet.list.count; i++) {
            for(int j = 0; j< this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.count; j++) {

                int min_x = lat_offset - this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.array[j]->delta.choice.node_XY3.x;
                int min_y = long_offset - this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.array[j]->delta.choice.node_XY3.y;
                int temp_min_distance = sqrt(min_x * min_x + min_y * min_y);

                if( temp_min_distance < min_distance) {
                    min_distance = temp_min_distance;

                    po ->lane_id = this->map->intersections[0].list.array[0]->laneSet.list.array[i]->laneID;

                    if(this->map->intersections[0].list.array[0]->laneSet.list.array[i]->ingressApproach){
                        po ->approach = "1";
                    }
                    else {
                        po ->approach = "0";
                    }
                }
            }
        }

        std::cout << " finished locating the vehicle " << po ->approach << po ->vehicle_id << po ->lane_id << std::endl;

        PreemptionPlaner(po);
    };

    void PreemptionPluginWorker::PreemptionPlaner(PreemptionObject* po){
        std::string preemption_plan = "";
        std::string preemption_plan_flag = "";

        if(po->approach == "1"){
            switch(po->lane_id) {
                case 1 : preemption_plan == "2";
                case 2 : preemption_plan == "2";
                case 3 : preemption_plan == "2";
                case 4 : preemption_plan == "2";
                case 5 : preemption_plan == "2";
                case 6 : preemption_plan == "2";
                case 7 : preemption_plan == "2";
                case 8 : preemption_plan == "2";
                case 9 : preemption_plan == "2";
                case 10 : preemption_plan == "2";
                case 11 : preemption_plan == "2";
                case 12 : preemption_plan == "2";
            }

            preemption_map[po->vehicle_id] = preemption_plan;
            preemption_plan_flag = "0";

            std::string PreemptionOid = base_preemption_oid + preemption_plan;
            int response = SendOid(PreemptionOid.c_str(), preemption_plan_flag.c_str());
            if(response != 0){
                std::cout << "Sending oid intrupted with an error.";
            }
            else{
                std::cout << "Finished sending preemption plan.";
            }
        }
        else if(po->approach == "0"){
            if ( preemption_map.find(po ->vehicle_id) == preemption_map.end() ) {
                std::cout << " vehicle id does not exitst" << po->vehicle_id << std::endl;
            }
            else {
                preemption_plan = preemption_map[po ->vehicle_id];
                preemption_plan_flag = "0";
                std::string PreemptionOid = base_preemption_oid + preemption_plan;
                int response = SendOid(PreemptionOid.c_str(), preemption_plan_flag.c_str());
                if(response != 0){
                    std::cout << "Sending oid intrupted with an error.";
                }
                else{
                    std::cout << "Finished sending preemption plan.";
                }
            }
        }
        else{
            std::cout << " po approach is not 0 or 1" << po->approach << std::endl;
        }

        std::cout << " finished PreemptionPlaner" << std::endl;
    };

    int PreemptionPluginWorker::SendOid(const char *PreemptionOid, const char *value) {
        netsnmp_session session, *ss;
        netsnmp_pdu    *pdu, *response = NULL;
        netsnmp_variable_list *vars;
        oid             name[MAX_OID_LEN];
        size_t          name_length;
        int             status;
        int             failures = 0;
        int             exitval = 0;

        init_snmp("snmpset");
        snmp_sess_init(&session);
        session.peername = strdup(ip_with_port.c_str());
        session.version = snmp_version;
        session.community = (u_char *)snmp_community.c_str();
        session.community_len = strlen((const char*) session.community);
        session.timeout = 1000000;

        SOCK_STARTUP;

        ss = snmp_open(&session);

        if (ss == NULL) {
            snmp_sess_perror("snmpset", &session);
            SOCK_CLEANUP;
            exit(1);
        }

        // create PDU for SET request and add object names and values to request 
        
        pdu = snmp_pdu_create(SNMP_MSG_SET);
        
        name_length = MAX_OID_LEN;
        if (snmp_parse_oid(PreemptionOid, name, &name_length) == NULL) {
            snmp_perror(PreemptionOid);
            failures++;
        } else {
            if (snmp_add_var
                (pdu, name, name_length, 'i', value)) {
                snmp_perror(PreemptionOid);
                failures++;
            }
        }

        if (failures) {
            snmp_close(ss);
            SOCK_CLEANUP;
            exit(1);
        }

        //send the request 
        
        status = snmp_synch_response(ss, pdu, &response);
        if (status == STAT_SUCCESS) {
            if (response->errstat == SNMP_ERR_NOERROR) {
                if (1) {
                    print_variable(response->variables->name, response->variables->name_length, response->variables);
                }
            } else {
                fprintf(stderr, "Error in packet.\nReason: %s\n", snmp_errstring(response->errstat));
                exitval = 2;
            }
        } else if (status == STAT_TIMEOUT) {
            fprintf(stderr, "Timeout: No Response from %s\n", session.peername);
            exitval = 1;
        } else {                    /* status == STAT_ERROR */
            snmp_sess_perror("snmpset", ss);
            exitval = 1;
        }

        if (response)
            snmp_free_pdu(response);
        snmp_close(ss);
        SOCK_CLEANUP;

        return exitval;
    };


};