//==========================================================================
// Name        : PreemptionPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Preemption Plugin
//==========================================================================

#include "PreemptionPluginWorker.hpp"
#include <memory>
using namespace std;

namespace PreemptionPlugin {

	void PreemptionPluginWorker::ProcessMapMessageFile(const std::string &path){

        if(path != ""){
            try {
                boost::property_tree::read_json(path, geofence_data);
            
                BOOST_FOREACH( boost::property_tree::ptree::value_type const& v, geofence_data.get_child( "data" ) ) {
                    boost::property_tree::ptree subtree = v.second;
                    list <double> geox;
                    list <double> geoy;

                    BOOST_FOREACH( boost::property_tree::ptree::value_type const& u, subtree.get_child( "geox" ) ) {
                        double d =  u.second.get<double>("");
                        geox.push_back(d);
                    }

                    BOOST_FOREACH( boost::property_tree::ptree::value_type const& u, subtree.get_child( "geoy" ) ) {
                        double d =  u.second.get<double>("");
                        geoy.push_back(d);
                    }
                    
                    GeofenceObject geofenceObject(geox,geoy, static_cast<int>(subtree.get<double>("PreemptCall")),static_cast<int>(subtree.get<double>("HeadingMin")),static_cast<int>(subtree.get<double>("HeadingMax")));
                    
                    GeofenceSet.push_back(&geofenceObject);

                }
            }
            catch(...) { 
              	PLUGIN_LOG(logERROR, "Preemptionworker") << "Caught exception from reading a file"; 
            } 
        }
    }
    
    bool PreemptionPluginWorker::CarInGeofence(long double x,long  double y, std::vector<double> geox, std::vector<double>  geoy, long GeoCorners) const{
        long i = 0 ;
        long j = GeoCorners-1;
        bool  oddNodes = false;

        for (i=0; i<GeoCorners; i++) {
            if ((geoy.at(i)< y && geoy.at(j)>=y
            ||   geoy.at(j)< y && geoy.at(i)>=y)
            &&  (geox.at(i)<=x || geox.at(j)<=x) && (geox.at(i)+(y-geoy.at(i))/(geoy.at(j)-geoy.at(i))*(geox.at(j)-geox.at(i))<x)) {                
                    oddNodes = !oddNodes;                 
            }
            j=i; 
        }
        
        return oddNodes; 
    } 

    void PreemptionPluginWorker::VehicleLocatorWorker(BsmMessage* msg){

        double micro = 10000000.0;

        auto po = std::make_shared<PreemptionObject>();
        auto vehicle_coordinate = std::make_shared<VehicleCoordinate>();

        auto bsm = msg->get_j2735_data();
        int32_t bsmTmpID;
        GetInt32((unsigned char *)bsm->coreData.id.buf, &bsmTmpID);
        int buff_size = bsm->coreData.id.size;
        po->vehicle_id = bsmTmpID ;
        vehicle_coordinate->lat = bsm->coreData.lat / micro;
        vehicle_coordinate->lon = bsm->coreData.Long / micro;
        vehicle_coordinate->elevation = bsm->coreData.elev;
        vehicle_coordinate->heading = bsm->coreData.heading * 0.0125;

        for (auto const& it: GeofenceSet) {

            std::vector<double> geox;
            for (double const &i: it->geox) {
                geox.push_back(i);
            }

            std::vector<double> geoy;
            for (double const &i: it->geoy) {
                geoy.push_back(i);
            }

            bool in_geo =  CarInGeofence(vehicle_coordinate->lon, vehicle_coordinate->lat, geoy, geox, it->geox.size());

           if(in_geo){
               if(vehicle_coordinate->heading > it->minHeading && vehicle_coordinate->heading < it->maxHeading) {
                   po->approach = "1";
                   po->preemption_plan = std::to_string(it->PreemptCall);
                   PreemptionPlaner(po);
                   return;
               }
               else {
                    po->approach = "0";
               }
           }
           else {
                po ->approach = "0";
           }
        }

        PreemptionPlaner(po);
        return;

    };

    void PreemptionPluginWorker::PreemptionPlaner(std::shared_ptr<PreemptionObject> po){
 
        if(po->approach == "1") {

            if ( preemption_map.find(po->vehicle_id) == preemption_map.end() ) {
                TurnOnPreemption(po);
            } 
            else {
                std::cout << "Already sent the preemption plan.";
            } 
        }
        else if(po->approach == "0"){

            if (preemption_map.find(po->vehicle_id) == preemption_map.end() ) {
                std::cout << " vehicle id does not exist" << po->vehicle_id << std::endl;
            }
            else {
                TurnOffPreemption(po);
            }
        }
        else{
            std::cout << "approach is not 0 or 1" << po->approach << std::endl;
        }

        std::cout << " Finished PreemptionPlaner" << std::endl;
    };

    void PreemptionPluginWorker::TurnOnPreemption(std::shared_ptr<PreemptionObject> po){
        std::string preemption_plan_flag = "1";

        std::asctime(std::localtime(&(po->time)));

        preemption_map[po->vehicle_id] = *po;
        std::string PreemptionOid = base_preemption_oid + po->preemption_plan;

        int response = SendOid(PreemptionOid.c_str(), preemption_plan_flag.c_str());
        if(response != 0){
            std::cout << "Sending oid interupted with an error.";
        }
        else{
            std::cout << "Finished sending preemption plan.";
        }
    }

    void PreemptionPluginWorker::TurnOffPreemption(std::shared_ptr<PreemptionObject> po){
        std::string preemption_plan, preemption_plan_flag = "";
        preemption_plan = preemption_map[po ->vehicle_id].preemption_plan;
        preemption_plan_flag = "0";
        std::string PreemptionOid = base_preemption_oid + preemption_plan;
        int response = SendOid(PreemptionOid.c_str(), preemption_plan_flag.c_str());

        if(response != 0){
            std::cout << "Sending oid interupted with an error.";
        }
        else{
            std::cout << "Finished sending preemption plan.";
        }

        preemption_map.erase(po->vehicle_id);
    }

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

    void PreemptionPluginWorker::GetInt32(unsigned char *buf, int32_t *value)
{
	*value = (int32_t)((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
}

};