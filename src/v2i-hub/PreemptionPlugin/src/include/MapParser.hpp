#include <iostream>
#include <fstream>
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include "PluginClient.h"
#include "PluginDataMonitor.h"
#include <sstream>

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace PreemptionPlugin {

class MapParser {
	public:
		std::string map_message;
		MapData map;
		void ProcessMapMessageFile(std::string path);
		int hexadecimalToDecimal(char hexVal[]);
		void test();

};


};