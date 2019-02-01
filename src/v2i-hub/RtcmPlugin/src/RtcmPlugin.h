/*
 * RtcmPlugin.h
 *
 *  Created on: Mar 22, 2018
 *      Author: gmb
 */

#ifndef SRC_RTCMPLUGIN_H_
#define SRC_RTCMPLUGIN_H_

#include <Base64.h>
#include <PluginClient.h>
#include <rtcm/RtcmMessage.h>
#include <tmx/j2735_messages/RtcmMessage.hpp>
#include <tmx/messages/TmxNmea.hpp>

namespace RtcmPlugin {

class RtcmPlugin: public tmx::utils::PluginClient {
public:
	RtcmPlugin(std::string name);
	virtual ~RtcmPlugin();

	int Main();
	void BroadcastRTCMMessage(tmx::messages::TmxRtcmMessage &msg, tmx::routeable_message &routeableMsg);
protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);
	void OnMessageReceived(IvpMessage *ivpMsg);
private:
	std::atomic<bool> _connected { false };
	std::atomic<int> _socket { 0 };
	std::string _username;
	std::string _password;
	std::string _mount;
	std::string _nmea;
	std::string _version;
	std::atomic<bool> _routeRTCM { false };
};

} /* namespace RtcmPlugin */

#endif /* SRC_RTCMPLUGIN_H_ */
