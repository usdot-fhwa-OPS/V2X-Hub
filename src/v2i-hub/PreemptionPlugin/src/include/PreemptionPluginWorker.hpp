//==========================================================================
// Name        : PreemptionPlugin.cpp
// Author      : Leidos Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 Leidos Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Preemption Plugin
//==========================================================================

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

class PreemptionPluginWorker {
	public:
		MapData* map = nullptr;
		std::string PreemptionPlan;
		std::string PreemptionPlan_flag;
		void ProcessMapMessageFile(std::string path);
		void VehicleLocatorWorker(BsmMessage* msg);
	};
};