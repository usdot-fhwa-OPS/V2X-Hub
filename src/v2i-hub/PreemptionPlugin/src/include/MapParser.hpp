#include <iostream>
#include <fstream>
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include "PluginClient.h"
#include "PluginDataMonitor.h"
#include <sstream>
#include <math.h>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <BasicSafetyMessage.h>
#include <cmath>

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace PreemptionPlugin {

class MapParser {
	public:
		MapData* map = nullptr;
		std::string PreemptionPlan;
		std::string PreemptionPlan_flag;
		void ProcessMapMessageFile(std::string path);
		void VehicleLocatorWorker(BsmMessage* msg);
	};
};