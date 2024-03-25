//==========================================================================
// Name        : RsmPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2024 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : RSM Plugin
//==========================================================================
#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <cmath>
#include <map>
#include <chrono>
#include <RoadSafetyMessage.h>
#include <tmx/j2735_messages/RoadSafetyMessage.hpp>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "PluginClient.h"
#include "PluginDataMonitor.h"

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace RsmPlugin {

	class RsmPluginWorker {
		public:
			// struct RSM {
			// 	RoadSafetyMessage rsm;
			// 	RSM(int anInt, double aDouble) : rsm.TemporaryID_t(anInt), number(aDouble) { }

			// };
			
	};



};
