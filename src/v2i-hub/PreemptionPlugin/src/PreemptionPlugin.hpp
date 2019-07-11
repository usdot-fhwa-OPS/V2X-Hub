//==========================================================================
// Name        : PreemptionPlugin.cpp
// Author      : Leidos Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 Leidos Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Preemption Plugin
//==========================================================================

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>

#include "PluginClient.h"
#include "PluginDataMonitor.h"

#include <atomic>
#include <thread>
#include <DecodedBsmMessage.h>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <BasicSafetyMessage.h>
#include <tmx/j2735_messages/MapDataMessage.hpp>

#include <UdpClient.h>
#include <tmx/messages/auto_message.hpp>
#include "include/PreemptionPluginWorker.hpp"

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;


namespace PreemptionPlugin
{

/**
 * This plugin is an example to demonstrate the capabilities of a TMX plugin.
 */
class PreemptionPlugin: public PluginClient
{
public:
	PreemptionPlugin(std::string);
	virtual ~PreemptionPlugin();
	int Main();

protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);

	void HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg);
	void HandleDecodedBsmMessage(DecodedBsmMessage &msg, routeable_message &routeableMsg);
	void HandleDataChangeMessage(DataChangeMessage &msg, routeable_message &routeableMsg);
	void HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg);

	void GetInt32(unsigned char *buf, int32_t *value);

private:
	std::atomic<uint64_t> _frequency{0};
	DATA_MONITOR(_frequency);

	std::string ipwithport;
	int snmp_version = SNMP_VERSION_1;
	std::string snmp_community;
	std::string BasePreemptionOid;
	std::string map_path;
	PreemptionPluginWorker *mp = new PreemptionPluginWorker;
	// sends oid to controler
	int SendOid(const char *PreemptionOid, const char *value);
	tmx::utils::UdpClient *_signSimClient = NULL;
};
};