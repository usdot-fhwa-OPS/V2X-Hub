//==========================================================================
// Name        : PedestrianPlugin.cpp
// Author      : Leidos Saxton Transportation Operations Laboratory  
// Version     :
// Copyright   : Copyright (c) 2019 Leidos Saxton Transportation Operations Laboratory. All rights reserved.
// Description : Pedestrian Plugin
//==========================================================================

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
#include "include/PedestrianPluginWorker.hpp"

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;


namespace PedestrianPlugin
{

/**
 * This plugin is an example to demonstrate the capabilities of a TMX plugin.
 */
class PedestrianPlugin: public PluginClient
{
public:
	PedestrianPlugin(std::string);
	virtual ~PedestrianPlugin();
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
	tmx::utils::UdpClient *_signSimClient = NULL;
};
};