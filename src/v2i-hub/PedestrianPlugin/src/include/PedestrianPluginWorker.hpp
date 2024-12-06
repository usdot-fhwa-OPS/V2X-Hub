//==========================================================================
// Name        : PedestrianPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2024 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Pedestrian Plugin
//==========================================================================
#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <cmath>
#include <map>
#include <chrono>
#include <PersonalSafetyMessage.h>
#include <tmx/j2735_messages/PersonalSafetyMessage.hpp>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "PluginClient.h"
#include "PluginDataMonitor.h"

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace PedestrianPlugin {

	class PedestrianPluginWorker {
		public:
			// struct PSM {
			// 	PersonalSafetyMessage psm;
			// 	PSM(int anInt, double aDouble) : psm.TemporaryID_t(anInt), number(aDouble) { }

			// };
			
	};



};
