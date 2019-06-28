//==========================================================================
// Name        : PreemptionPlugin.cpp
// Author      : Leidos Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 Leidos Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Preemption Plugin
//==========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <cmath>
#include <map>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>


#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <BasicSafetyMessage.h>

#include "PluginClient.h"
#include "PluginDataMonitor.h"

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace PreemptionPlugin {

	class PreemptionPluginWorker {

		struct PreemptionObject {
			int lane_id;
			std::string approach; // 0: egress 1: ingress
			uint8_t* vehicle_id;
		};

		public:
			MapData* map = nullptr;
			std::string preemption_plan;
			std::string preemption_plan_flag;

			std::map <uint8_t*,std::string> preemption_map;

			void ProcessMapMessageFile(std::string path);
			void VehicleLocatorWorker(BsmMessage* msg);
			void PreemptionPlaner(PreemptionObject* po);

			std::string ip_with_port;
			int snmp_version = SNMP_VERSION_1;
			std::string snmp_community;
			std::string base_preemption_oid;

			int SendOid(const char *PreemptionOid, const char *value);

	};


};

