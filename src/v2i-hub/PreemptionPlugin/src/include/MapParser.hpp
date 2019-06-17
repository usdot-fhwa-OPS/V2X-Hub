#include <iostream>
#include <fstream>
#include <tmx/j2735_messages/MapDataMessage.hpp>

#include "PluginClient.h"
#include "PluginDataMonitor.h"


using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace PreemptionPlugin {

class MapParser {
	public:
		std::string map_message;
		void ProcessMapMessageFile(std::string path);
		void test();

};


};