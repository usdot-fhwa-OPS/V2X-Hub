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
#include <UdpServer.h>
#include <PluginClientClockAware.h>
#include <ThreadTimer.h>
#include <SNMPClient.h>
#include "SignalControllerConnection.h"

namespace SpatPlugin {

class SpatPlugin: public tmx::utils::PluginClientClockAware {

public:

	SpatPlugin(std::string name);
	virtual ~SpatPlugin();


protected:

	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);

private:

	std::mutex data_lock;

    std::unique_ptr<tmx::utils::ThreadTimer> spatReceiverThread;

	std::unique_ptr<SignalControllerConnection> scConnection;

	std::string spatMode = "";

	const char* keyConnectionStatus = "Connection Status";

	const char* keySkippedMessages = "Skipped Messages";

	bool isConnected = false;

	void processSpat();
};
} /* namespace SpatPlugin */

#endif /* SPATPLUGIN_H_ */
