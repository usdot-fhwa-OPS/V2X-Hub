#include "MapParser.hpp"

using namespace std;

namespace PreemptionPlugin {

	void MapParser::ProcessMapMessageFile(std::string path){

        ifstream message_file;
        std::string map_message;
        message_file.open(path);
        if (message_file.is_open()) {
            while (!message_file.eof()) {
                message_file >> map_message;
                std::getline(message_file,map_message);
            }
        }
        message_file.close();

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
        }
    }
    
    void MapParser::VehicleLocatorWorker(BsmMessage* msg){

        auto bsm = msg->get_j2735_data();

        int32_t latitude = bsm->coreData.lat;
        int32_t longitude = bsm->coreData.Long;
	    int32_t longAcceleration = bsm->coreData.accelSet.Long;

        if(this->map == nullptr){
            std::cout << "loading map ... " << PreemptionPlan << std::endl;
            ProcessMapMessageFile("/home/V2X-Hub/src/v2i-hub/PreemptionPlugin/src/include/sample_map.txt");
        }

        std::cout << "refrence point lat , long " << this->map->intersections[0].list.array[0]->refPoint.lat << "," << this->map->intersections[0].list.array[0]->refPoint.Long;

        int32_t lat_offset = latitude - this->map->intersections[0].list.array[0]->refPoint.lat;
        int32_t long_offset = longitude - this->map->intersections[0].list.array[0]->refPoint.Long;

        std::cout << "offset lat , long " << lat_offset << "," << long_offset << std::endl;

        int min_distance = 999999;
        int lane_id = -1;

        for(int i = 0; i< this->map->intersections[0].list.array[0]->laneSet.list.count; i++) {
            for(int j = 0; j< this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.count; j++) {

                int min_x = lat_offset - this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.array[j]->delta.choice.node_XY3.x;
                int min_y = long_offset - this->map->intersections[0].list.array[0]->laneSet.list.array[i]->nodeList.choice.nodes.list.array[j]->delta.choice.node_XY3.y;
                int temp_min_distance = sqrt(min_x * min_x + min_y * min_y);

                if( temp_min_distance < min_distance) {
                    min_distance = temp_min_distance;

                    lane_id = this->map->intersections[0].list.array[0]->laneSet.list.array[i]->laneID;

                    if(this->map->intersections[0].list.array[0]->laneSet.list.array[i]->ingressApproach){
                        PreemptionPlan_flag = "1";
                    }
                    else {
                        PreemptionPlan_flag = "0";
                    }
                }
            }
        }

        PreemptionPlan = std::to_string(lane_id);
        std::cout << "PreemptionPlan " << PreemptionPlan << std::endl;

    };

};