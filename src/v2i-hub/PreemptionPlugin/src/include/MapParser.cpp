#include "MapParser.hpp"

using namespace std;

namespace PreemptionPlugin {

	void MapParser::ProcessMapMessageFile(std::string path){
        ifstream message_file;
        message_file.open(path);
        if (message_file.is_open()) {
            while (!message_file.eof()) {
                message_file >> map_message;
                std::getline(message_file,map_message);
            }
        }
        message_file.close();
    }

    void MapParser::test(){
				std::cout << "teeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeest.";
    }

};