#include <iostream>
#include <fstream>
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include "PluginClient.h"
#include "PluginDataMonitor.h"
#include <sstream>
#include <math.h>

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace PreemptionPlugin {

class VehicleLocator {
	public:
        void VehicleLocatorWorker(MapData* map, BsmMessage* msg);
};
};