/*
 * SunGuidePlugin.h
 *
 *  Created on: May 10, 2016
 *      Author: ivp
 */

#ifndef SRC_ODEPLUGIN_H_
#define SRC_ODEPLUGIN_H_

#include <FrequencyThrottle.h>
#include <PluginClient.h>
#include <UdpClient.h>
#include <boost/asio.hpp>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>

#define TCP "TCP"
#define UDP "UDP"


namespace ODE {

class ODEPlugin: public tmx::utils::PluginClient {
public:
	ODEPlugin(std::string);
	virtual ~ODEPlugin();
	void handleBsm(tmx::messages::BsmMessage &, tmx::routeable_message &);

	void sendBytes(const tmx::byte_stream &);
	void recvBytes(const tmx::byte_stream &);

	int Main();
protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);
private:
	std::string ip;
	unsigned short port = 0;

	tmx::utils::UdpClient *client = NULL;

	tmx::utils::FrequencyThrottle<int> _errThrottle;
	tmx::utils::FrequencyThrottle<int> _statThrottle;

};

} /* namespace ODEPlugin */

#endif /* SRC_ODEPLUGIN_H_ */
