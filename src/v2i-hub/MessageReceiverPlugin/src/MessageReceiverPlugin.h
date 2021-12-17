/*
 * BsmReceiver.h
 *
 *  Created on: May 10, 2016
 *      Author: ivp
 */

#ifndef SRC_MESSAGERECEIVERPLUGIN_H_
#define SRC_MESSAGERECEIVERPLUGIN_H_

#include <atomic>
#include <PluginClient.h>
#include <TmxMessageManager.h>
#include <UdpClient.h>
#include <UdpServer.h>

#include <boost/asio.hpp>
#include <memory>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <../../../tmx/TmxApi/tmx/json/cJSON.h>
#include <tmx/Security/include/base64.h>
//#include <tmx/Security/include/softhsm2.h>

#define UDP "UDP"

//using namespace Botan; 
namespace MessageReceiver {

class MessageReceiverPlugin: public tmx::utils::TmxMessageManager {
public:
	MessageReceiverPlugin(std::string);
	virtual ~MessageReceiverPlugin();
// @SONAR_STOP@
	int Main();
	void OnMessageReceived(tmx::routeable_message &msg);
	void getmessageid();
protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);
private:
	std::atomic<bool> cfgChanged { false };
	std::string ip;
	unsigned short port = 0;

	std::atomic<bool> routeDsrc { false };
	std::atomic<bool> simBSM { true };
	std::atomic<bool> simSRM { true };
	std::atomic<bool> simLoc { true };
	unsigned int verState;
	std::string liblocation; 
	std::string url; 
	std::string baseurl;
	std::vector<string> messageid;
	std::string messageidstr; 

	//softhsm st; 
// @SONAR_START@

};

} /* namespace MessageReceiver */

#endif /* SRC_MESSAGERECEIVERPLUGIN_H_ */
