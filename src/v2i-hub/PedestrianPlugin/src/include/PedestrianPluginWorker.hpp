//==========================================================================
// Name        : PedestrianPlugin.cpp
// Author      : Leidos Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 Leidos Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Pedestrian Plugin
//==========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <cmath>
#include <map>
#include <chrono>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <ctime>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <BasicSafetyMessage.h>

#include "PluginClient.h"
#include "PluginDataMonitor.h"
#include <list> 

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace PedestrianPlugin {

	class PedestrianPluginWorker {
		public:
	};

};

