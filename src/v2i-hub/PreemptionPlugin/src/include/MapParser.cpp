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

        std::cout << std::endl << std::endl;
        std::cout << std::endl << std::endl;
        
        for (int i = 0; i < map_message.length()/2; i++) 
            std::cout << buf[i] << " , "; 

        std::cout << std::endl << std::endl;
        std::cout << std::endl << std::endl;

        asn_dec_rval_t rval;
        MessageFrame_t *message = 0;
        rval = uper_decode(0, &asn_DEF_MessageFrame, (void **) &message, buf, map_message.length()/2, 0, 0);

        if(rval.code == RC_OK) {
            map = message -> value.choice.MapData;
            std::cout << "i diddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddid it.";
            std::cout << message->value.choice.MapData.intersections[0].list.array[0].Reg_Intersection;
        }

        std::cout << "rval code reutrn" << rval.code << std::endl  << std::endl;
    }

    void MapParser::test(){
				std::cout << "teeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeest.";
    }
}