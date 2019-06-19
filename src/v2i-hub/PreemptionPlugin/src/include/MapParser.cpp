#include "MapParser.hpp"

using namespace std;

namespace PreemptionPlugin {

	void MapParser::ProcessMapMessageFile(std::string path){
        std::cout << " ProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFileProcessMapMessageFile.";

        ifstream message_file;
        message_file.open(path);
        if (message_file.is_open()) {
            while (!message_file.eof()) {
                message_file >> map_message;
                std::getline(message_file,map_message);
            }
        }
        message_file.close();

        char buf[map_message.length()/2];

        int j = 0;

        for(int i = 0; i < map_message.length(); i+2) {
            std::cout << " in the for loop";
            char temp[2];
            temp[0] = map_message[i];
            temp[1] = map_message[i + 1];

            std::ostringstream ostr;
            ostr << hexadecimalToDecimal(temp);
            std::string s = ostr.str();
            buf[j] = s[0];
            j = j + 1;
        }
        
        std::cout << " what what what what what what  it.";

        std::cout << std::endl << std::endl << map_message << std::endl;

        std::cout << std::endl << std::endl << buf << std::endl;



        // uper is saved in message
        asn_dec_rval_t rval;
        MessageFrame_t *message = 0;
        rval = uper_decode(0, &asn_DEF_MessageFrame, (void **) &message, buf, map_message.length()/2, 0, 0);

        if(rval.code == RC_OK) {
            map = message -> value.choice.MapData;
            std::cout << "i diddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddiddid it.";
        }

        std::cout << "i not not not not it."  << std::endl  << std::endl;
        std::cout << rval.code << std::endl  << std::endl;

    }

    void MapParser::test(){
				std::cout << "teeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeest.";
    }

    int MapParser::hexadecimalToDecimal(char hexVal[]) 
    {    
        int len = strlen(hexVal); 
        
        // Initializing base value to 1, i.e 16^0 
        int base = 1; 
        
        int dec_val = 0; 
        
        // Extracting characters as digits from last character 
        for (int i=len-1; i>=0; i--) 
        {    
            // if character lies in '0'-'9', converting  
            // it to integral 0-9 by subtracting 48 from 
            // ASCII value. 
            if (hexVal[i]>='0' && hexVal[i]<='9') 
            { 
                dec_val += (hexVal[i] - 48)*base; 
                    
                // incrementing base by power 
                base = base * 16; 
            } 
    
            // if character lies in 'A'-'F' , converting  
            // it to integral 10 - 15 by subtracting 55  
            // from ASCII value 
            else if (hexVal[i]>='A' && hexVal[i]<='F') 
            { 
                dec_val += (hexVal[i] - 55)*base; 
            
                // incrementing base by power 
                base = base*16; 
            } 
        } 
        
        return dec_val; 
    } 
}