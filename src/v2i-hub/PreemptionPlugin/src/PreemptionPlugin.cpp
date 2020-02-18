//==========================================================================
// Name        : PreemptionPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     : 1.0
// Copyright   : Copyright (c) 2019 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Preemption Plugin
//==========================================================================

#include "PreemptionPlugin.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;
using namespace boost::property_tree;

namespace PreemptionPlugin
{

/**
 * Construct a new PreemptionPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes.
 */
PreemptionPlugin::PreemptionPlugin(string name): PluginClient(name)
{
	// The log level can be changed from the default here.
	FILELog::ReportingLevel() = FILELog::FromString("DEBUG");

	AddMessageFilter <BsmMessage> (this, &PreemptionPlugin::HandleBasicSafetyMessage);

	SubscribeToMessages();
}

PreemptionPlugin::~PreemptionPlugin()
{
	if (_signSimClient != NULL)
		delete _signSimClient;
}

void PreemptionPlugin::UpdateConfigSettings()
{
	GetConfigValue("BasePreemptionOid", BasePreemptionOid);
	GetConfigValue("ipwithport", ipwithport);
	GetConfigValue("snmp_community", snmp_community);
	GetConfigValue("map_path", map_path);
	mp->ProcessMapMessageFile(map_path);
	GetConfigValue("allowedList",allowedListjson);

	std::stringstream ss; 
	ptree pr; 
	ss<<allowedListjson; 
	read_json(ss,pr);

	BOOST_FOREACH(auto &validveh, pr.get_child("validVehicles"))
	{
		int val = validveh.second.get_value<int>(); 
		std::vector<int>::iterator it = std::find(allowedList.begin(),allowedList.end(),val);
		
		if(it == allowedList.end())
			allowedList.push_back(val);
	}

	for(int i = 0;i<allowedList.size() ;i++)
	{
		std::cout<<"Preemption Allowed List "<<allowedList[i]<<std::endl;
	}


}


void PreemptionPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void PreemptionPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
		SetStatus("ReceivedMaps", 0);
	}
}

void PreemptionPlugin::HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg) {

	PLOG(logDEBUG)<<"HandleBasicSafetyMessage";
	if(mp->GeofenceSet.size() < 0) {
		mp->ProcessMapMessageFile(map_path);
	}
	int32_t bsmTmpID;
	
	auto bsm=msg.get_j2735_data();
	unsigned char* buf = bsm->coreData.id.buf; 

	bsmTmpID =  (int32_t)((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
	

	std::vector<int>::iterator it = std::find(allowedList.begin(),allowedList.end(),bsmTmpID);
	
	if( it != allowedList.end())
	{	
		if (bsm->partII != NULL) {
			if (bsm->partII[0].list.count >= partII_Value_PR_SpecialVehicleExtensions ) {
				try {
						if(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts != NULL){
							if(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->lightsUse == SirenInUse_inUse ) // if lights on 
								mp->VehicleLocatorWorker(&msg);
							else if (bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->sirenUse == SirenInUse_inUse && 
									bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->lightsUse == LightbarInUse_inUse)
								mp->VehicleLocatorWorker(&msg);
						}
				}
				catch(exception &e)
				{
					PLOG(logDEBUG)<<"Standard Exception:; Vehicle alerts Unavailable";
				}
			}
		}
	}
	
}

int PreemptionPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	uint msCount = 0;
	while (_plugin->state != IvpPluginState_error)
	{

		if(mp->GeofenceSet.size() < 1 && map_path != "") {
			mp->ProcessMapMessageFile(map_path);
		}

		mp->ip_with_port = ipwithport;
		mp->snmp_version = SNMP_VERSION_1;
		mp->snmp_community = snmp_community;
		mp->base_preemption_oid = BasePreemptionOid;

		msCount += 10;

		if (_plugin->state == IvpPluginState_registered)
		{

			this_thread::sleep_for(chrono::milliseconds(100));

			msCount = 0;
		}
	}

	PLOG(logINFO) << "Plugin terminating gracefully.";
	return EXIT_SUCCESS;
}

} /* namespace PreemptionPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<PreemptionPlugin::PreemptionPlugin>("PreemptionPlugin", argc, argv);
}
