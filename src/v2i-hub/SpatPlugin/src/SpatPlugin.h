/*
 * SpatPlugin.h
 *
 *  Created on: April 20, 2017
 *      Author: zinkg
 */

#ifndef SPATPLUGIN_H_
#define SPATPLUGIN_H_

#include <atomic>
#include <array>
#include <map>
#include <mutex>
#include <vector>
#include "PluginClientClockAware.h"
#include "UdpClient.h"
#include "signalController.h"

#include <tmx/j2735_messages/SpatMessage.hpp>
#include <tmx/messages/IvpSignalControllerStatus.h>
#include <tmx/messages/IvpJ2735.h>
#include <boost/chrono.hpp>
#include <FrequencyThrottle.h>
#include <PedestrianMessage.h>

namespace SpatPlugin {

class SpatPlugin: public tmx::utils::PluginClientClockAware {

public:

	SpatPlugin(std::string name);
	virtual ~SpatPlugin();
	virtual int Main();

	void HandlePedestrianDetection(tmx::messages::PedestrianMessage &pedMsg, tmx::routeable_message &routeableMsg);
protected:

	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);

private:


	unsigned char derEncoded[4000];
	unsigned int derEncodedBytes = 0;

	SignalController sc;
	int _actionNumber = -1;

	std::mutex data_lock;
	std::string localIp;
	std::string localUdpPort;
	std::string tscIp;
	std::string tscRemoteSnmpPort;
	std::string signalGroupMappingJson;

	std::string intersectionName;

	int intersectionId;

	bool isConfigurationLoaded = false;
	bool isConfigured = false;

	bool encodeSpat();
	bool createUPERframe_DERencoded_msg();

	tmx::messages::PedestrianMessage _pedMessage;
};
} /* namespace SpatPlugin */

#endif /* SPATPLUGIN_H_ */
