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
         std::cout << "openning map file" << std::endl;

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

        std::cout << "decoding map file" << std::endl;

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
            } else {
                std::cout << "rval.code is " << rval.code;
            }
        }
        else {
            std::cout << " map_message is empty";
        }

        boost::property_tree::read_json("/geo.json", geofence_data  );
    

        BOOST_FOREACH( boost::property_tree::ptree::value_type const& v, geofence_data.get_child( "data" ) ) {
            assert(v.first.empty()); // array elements have no names
            boost::property_tree::ptree subtree = v.second;
            list <double> geox;
			list <double> geoy;

            BOOST_FOREACH( boost::property_tree::ptree::value_type const& u, subtree.get_child( "geox" ) ) {
                assert(u.first.empty()); // array elements have no names
                // std::cout << u.second.get<double>("") << std::endl;
                double d =  u.second.get<double>("");
                geox.push_back(d);
            }

            BOOST_FOREACH( boost::property_tree::ptree::value_type const& u, subtree.get_child( "geoy" ) ) {
                assert(u.first.empty()); // array elements have no names
                double d =  u.second.get<double>("");
                geoy.push_back(d);

                // std::cout << u.second.get<double>("") << std::endl;
            }
            
            // std::cout << subtree.get<double>("PreemptCall");
            // std::cout << subtree.get<double>("HeadingMin");
            // std::cout << subtree.get<double>("HeadingMax");

            GeofenceObject* geofenceObject = new GeofenceObject(geox,geoy,subtree.get<double>("PreemptCall"),subtree.get<double>("HeadingMin"),subtree.get<double>("HeadingMax"));
            
            // std::cout << geofenceObject->maxHeading;

            GeofenceSet.push_back(geofenceObject);
        }
    }
    
    bool CarInGeofence(double x, double y, double geox[], double geoy[], int GeoCorners) {
        int   i, j=GeoCorners-1 ;
        bool  oddNodes      ;

        for (i=0; i<GeoCorners; i++) {
            if ((geoy[i]< y && geoy[j]>=y
            ||   geoy[j]< y && geoy[i]>=y)
            &&  (geox[i]<=x || geox[j]<=x)) {
            oddNodes^=(geox[i]+(y-geoy[i])/(geoy[j]-geoy[i])*(geox[j]-geox[i])<x); }
            j=i; }

        return oddNodes; 
    } 

    double distance(long x1, long y1, long x2, long y2) { 

        return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.000000); 
    } 

    void PreemptionPluginWorker::VehicleLocatorWorker(BsmMessage* msg){

        double micro = 10000000.0;

        PreemptionObject* po = new PreemptionObject;

        wgs84_coordinate* vehicle_coordinate = new wgs84_coordinate;
        wgs84_coordinate* ref_coordinate = new wgs84_coordinate;

        auto bsm = msg->get_j2735_data();

        int buff_size = bsm->coreData.id.size;
        po->vehicle_id = (int)*(bsm->coreData.id.buf);
        vehicle_coordinate->lat = bsm->coreData.lat / micro;
        vehicle_coordinate->lon = bsm->coreData.Long / micro;
        vehicle_coordinate->elevation = bsm->coreData.elev;
        vehicle_coordinate->heading = bsm->coreData.heading * 0.00125;

        // po ->vehicle_id = 10;
        // vehicle_coordinate->lat = 38.9549306;
        // vehicle_coordinate->lon = -77.1482428;
        // vehicle_coordinate->heading = 85.225;

        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;

		std::cout << "vehicle id is" << po->vehicle_id << std::endl;
		std::cout << "vehicle heading is" << std::setprecision(10) << vehicle_coordinate->heading << std::endl;

        for (auto const& it: GeofenceSet) {

            double geox[it->geox.size()];
            int k = 0;
            for (double const &i: it->geox) {
                geox[k++] = i;
            }

            double geoy[it->geoy.size()];
            k = 0;
            for (double const &i: it->geoy) {
                geoy[k++] = i;
            }

            bool in_geo =  CarInGeofence(vehicle_coordinate->lon, vehicle_coordinate->lat, geoy, geox, it->geox.size());
            // std::cout << "geox,geoy" << geox[0] << "," << geoy[0] << std::endl;
            std::cout << "x,y" <<  vehicle_coordinate->lat << "," << vehicle_coordinate->lon << std::endl;
            // std::cout << "it->PreemptCall outside" << it->PreemptCall << std::endl;
            // std::cout << "in_geo" << in_geo << std::endl;

           if(in_geo){
               std::cout << "in the geo" << in_geo << std::endl;
               if(vehicle_coordinate->heading > it->minHeading && vehicle_coordinate->heading < it->maxHeading) {
                   std::cout << "in the heading" << in_geo << std::endl;
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

        // // vehicle_coordinate->lat = 389548501/micro;
        // // vehicle_coordinate->lon = -771494139/micro;
        // // vehicle_coordinate->elevation = 0.0;
        // // vehicle_coordinate->heading = 165840;

        // // po ->vehicle_id = 10;


        // if(this->map != nullptr){

        //     ref_coordinate->lat = this->map->intersections[0].list.array[0]->refPoint.lat / micro;
        //     ref_coordinate->lon = this->map->intersections[0].list.array[0]->refPoint.Long / micro;
        //     ref_coordinate->elevation = 0.0;

        //     double lat_diff = ((double)(vehicle_coordinate->lat - ref_coordinate->lat)) * 3.14159265358979323846/180.00000000;
        //     double lon_diff = ((double)(vehicle_coordinate->lon - ref_coordinate->lon)) * 3.14159265358979323846/180.00000000;

        //     double lat_offset = -1 * (double)wgs84_utils::calcMetersPerRadLat(*ref_coordinate) * lat_diff;
        //     double long_offset = (double)wgs84_utils::calcMetersPerRadLon(*ref_coordinate) * lon_diff;

        //     std::cout << std::endl << std::endl << std::endl << std::endl;
        //     std::cout << "ref_coordinate->lat,lon " << std::setprecision(10) << ref_coordinate->lat << std::setprecision(10) << ref_coordinate->lon << std::endl;

        //     std::cout << "vehicle_coordinate->lat,lon " << std::setprecision(10) << vehicle_coordinate->lat << "," << vehicle_coordinate->lon << std::endl;

        //     // std::cout << "calcMetersPerRadLat, calcMetersPerRadLon" << std::setprecision(10) << wgs84_utils::calcMetersPerRadLat(*ref_coordinate) << " , " << std::setprecision(10) << wgs84_utils::calcMetersPerRadLon(*ref_coordinate) << std::endl;

        //     // std::cout << "lat difference lon difference" << std::setprecision(10) << lat_diff << " , " << lon_diff << std::endl;

        //     std::cout << "vehicle long_offset lat_offset " << std::setprecision(10) << long_offset * 100 << " , " << -1 * lat_offset * 100 << std::endl;

        //     double distance_from_ref = distance(lat_offset, long_offset, 0, 0);

        //     double min_distance = 300.0;
        //     double temp_min_distance = 0.0;
        //     for(int i = 0; i< this->map->intersections[0].list.array[0]->laneSet.list.count; i++) {
        //         double x = 0;
        //         double y = 0;

        //         for(int j = 0; j < this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.count - 1; j++) {

        //             x += this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.array[j]->delta.choice.node_XY3.x;
        //             y += this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.array[j]->delta.choice.node_XY3.y;

        //             temp_min_distance = distance(long_offset * 100, -1 *lat_offset * 100, x, y);
        //             // std::cout << "map point offset distance from car" << temp_min_distance/100.0 << std::endl;
        //             std::cout << "x,y " << x << " ," << y << std::endl;

        //             if( temp_min_distance < min_distance) {

        //                 // std::cout << std::endl << std::endl << "map point offset distance from refrence" << distance(x/100.0, y/100.0, 0, 0) << std::endl;
        //                 // std::cout << "map point offset distance from car" << temp_min_distance/100.0 << std::endl;

        //                 min_distance = temp_min_distance;

        //                 po ->lane_id = this->map->intersections[0].list.array[0]->laneSet.list.array[i]->laneID;

        //                 if(this->map->intersections[0].list.array[0]->laneSet.list.array[i]->ingressApproach){
        //                     po ->approach = "1";
        //                 }
        //                 else {
        //                     po ->approach = "0";
        //                 }
        //                 // std::cout << "closest map point offset cmeter,cmeter" << this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.array[j]->delta.choice.node_XY3.x << " , " << this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.array[j]->delta.choice.node_XY3.y << std::endl;
        //             }
        //             // std::cout << "degree = " << std::setprecision(10) << 180.00000000/3.14159265358979323846 * atan2(this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.array[j]->delta.choice.node_XY3.y - this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.array[j+1]->delta.choice.node_XY3.y, this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.array[j]->delta.choice.node_XY3.x - this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.array[j+1]->delta.choice.node_XY3.x) << std::endl;
        //             // std::cout << "vehicle_coordinate->heading " << std::setprecision(10) << vehicle_coordinate->heading * 0.00125 << std::endl;

        //             // std::cout << "map point offset distance from car" << min_distance/100.0 << std::endl;

        //             // std::cout << "car distance from the refrence point in m " << std::setprecision(10) << distance_from_ref << std::endl;
                    
        //             // std::cout << "min distance in m " << std::setprecision(10) << min_distance/100 << std::endl << std::endl;
        //             // std::cout << "finished locating the vehicle approach " << po ->approach << " ,vid " << po ->vehicle_id << " ,lainid " << po ->lane_id << std::endl;

        //         }
        //         std::cout << std::endl << std::endl << std::endl;
        //     }

        //     std::cout << "distance from the refrence point in m " << std::setprecision(10) << distance_from_ref << std::endl;

        //     if(distance_from_ref < 9.0){
        //         po ->approach = "0";
        //     }
        //     else if(distance_from_ref < 200.0){
        //         po ->approach = "1";
		//     }
            
            // PreemptionPlaner(po);

        // }

    };

    void PreemptionPluginWorker::PreemptionPlaner(PreemptionObject* po){
 
        if(po->approach == "1") {
            std::cout << "Approach == 1 the vehicle id is " << po ->vehicle_id << std::endl;

            if ( preemption_map.find(po->vehicle_id) == preemption_map.end() ) {
                TurnOnPreemption(po);
            } 
            else {
                std::cout << "Already sent the preemption plan.";
            } 
        }
        else if(po->approach == "0"){

            if (preemption_map.find(po->vehicle_id) == preemption_map.end() ) {
                std::cout << " vehicle id does not exitst" << po->vehicle_id << std::endl;
            }
            else {
                TurnOffPreemption(po);
            }
        }
        else{
            std::cout << " po approach is not 0 or 1" << po->approach << std::endl;
        }

        std::cout << " finished PreemptionPlaner" << std::endl;
    };

    void PreemptionPluginWorker::TurnOnPreemption(PreemptionObject* po){
        std::string preemption_plan_flag = "1";

        std::asctime(std::localtime(&(po->time)));

        preemption_map[po->vehicle_id] = *po;
        std::string PreemptionOid = base_preemption_oid + po->preemption_plan;
        std::cout << "PreemptionOid " << PreemptionOid << std::endl;

        int response = SendOid(PreemptionOid.c_str(), preemption_plan_flag.c_str());
        if(response != 0){
            std::cout << "Sending oid intrupted with an error.";
        }
        else{
            std::cout << "Finished sending preemption plan.";
        }
    }

    void PreemptionPluginWorker::TurnOffPreemption(PreemptionObject* po){
        std::string preemption_plan, preemption_plan_flag = "";
        preemption_plan = preemption_map[po ->vehicle_id].preemption_plan;
        preemption_plan_flag = "0";
        std::string PreemptionOid = base_preemption_oid + preemption_plan;
        int response = SendOid(PreemptionOid.c_str(), preemption_plan_flag.c_str());
        std::cout << "PreemptionOid " << PreemptionOid << std::endl;

        if(response != 0){
            std::cout << "Sending oid intrupted with an error.";
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




};