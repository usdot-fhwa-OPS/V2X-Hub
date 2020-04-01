//==========================================================================
// Name        : PreemptionPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory  
// Version     : 1.0
// Copyright   : Copyright (c) 2019 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Preemption Plugin
//==========================================================================

#include <string.h>
#include <atomic>
#include <thread>
#include <vector>
#include <DecodedBsmMessage.h>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <BasicSafetyMessage.h>
#include <UdpClient.h>
#include <tmx/messages/auto_message.hpp>

#include "PluginClient.h"
#include "PluginDataMonitor.h"
#include "include/PreemptionPluginWorker.hpp"

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;


namespace PreemptionPlugin
{

class PreemptionPlugin: public PluginClient
{
	public:
		PreemptionPlugin(std::string);
		PreemptionPlugin();
		// PreemptionPlugin(const PreemptionPlugin& mn)
		// {
		
		// }

  		PreemptionPlugin& operator=(const PreemptionPlugin& other) {
  		}
  		// PreemptionPlugin(PreemptionPlugin &&fp) noexcept {
  		// }
  		PreemptionPlugin const & operator=(PreemptionPlugin &&fp) {
  
  		}



		virtual ~PreemptionPlugin();
		int Main();

	protected:
		void UpdateConfigSettings();

		// Virtual method overrides.
		void OnConfigChanged(const char *key, const char *value);
		void OnStateChange(IvpPluginState state);
		void HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg);

	private:

		std::string ipwithport;
		int snmp_version = SNMP_VERSION_1;
		std::string snmp_community;
		std::string BasePreemptionOid;
		std::string map_path;
		PreemptionPluginWorker *mp = new PreemptionPluginWorker;
		// sends oid to controler
		int SendOid(const char *PreemptionOid, const char *value);
		tmx::utils::UdpClient *_signSimClient = NULL;
		std::string allowedListjson; 
		std::vector<int> allowedList; 
	};
};
