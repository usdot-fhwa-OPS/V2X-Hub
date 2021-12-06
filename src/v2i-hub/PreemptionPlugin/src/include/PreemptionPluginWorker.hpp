//==========================================================================
// Name        : PreemptionPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Preemption Plugin
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

namespace PreemptionPlugin {

	class PreemptionPluginWorker {

		public:
			struct PreemptionObject {
				std::string approach; // 0: egress 1: ingress
				std::string preemption_plan;
				int vehicle_id;
				std::time_t time = std::time(nullptr);
			};

			struct GeofenceObject {
				list <double> geox;
				list <double> geoy;
				int PreemptCall;
				int minHeading;
				int maxHeading;

				GeofenceObject(list <double> geox, list <double> geoy, int PreemptCall, int minHeading, int maxHeading) {
					this->geox.assign (geox.begin(),geox.end());
					this->geoy.assign (geoy.begin(),geoy.end());
					this->PreemptCall = PreemptCall;
					this->minHeading = minHeading;
					this->maxHeading = maxHeading;
				};
			};

			struct VehicleCoordinate
			{
				long double lat, lon, heading, elevation;
			};

			std::string preemption_plan;
			std::string preemption_plan_flag;

			std::map <int,PreemptionObject> preemption_map;

			void ProcessMapMessageFile(std::string path);
			void VehicleLocatorWorker(BsmMessage* msg);
			void PreemptionPlaner(PreemptionObject* po);
			void TurnOnPreemption(PreemptionObject* po);
			void TurnOffPreemption(PreemptionObject* po);
			bool CarInGeofence(double x, double y, double geox[], double geoy[], int GeoCorners);

			std::string ip_with_port;
			int snmp_version = SNMP_VERSION_1;
			std::string snmp_community;
			std::string base_preemption_oid;

			int SendOid(const char *PreemptionOid, const char *value);
			void GetInt32(unsigned char *buf, int32_t *value);

			boost::property_tree::ptree geofence_data;
			std::list<GeofenceObject*> GeofenceSet;
	};


};

