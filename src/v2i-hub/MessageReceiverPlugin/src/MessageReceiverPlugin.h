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
#include <Clock.h>
#include <FrequencyThrottle.h>
#include <mutex>
#include <stdexcept>
#include <thread>

#include <tmx/apimessages/TmxEventLog.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <BsmConverter.h>
#include <LocationMessage.h>


#include <asn_application.h>
#include <boost/any.hpp>
#include <tmx/TmxApiMessages.h>
#include <tmx/messages/J2735Exception.hpp>
#include <tmx/messages/SaeJ2735Traits.hpp>
#include <tmx/messages/routeable_message.hpp>


#define UDP "UDP"

//using namespace Botan; 
namespace MessageReceiver {

class MessageReceiverPlugin: public tmx::utils::TmxMessageManager {
public:
	MessageReceiverPlugin(std::string);
	virtual ~MessageReceiverPlugin();
	int Main();
	void OnMessageReceived(tmx::routeable_message &msg);
	void getmessageid();
protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);
private:
	tmx::messages::BsmMessage* DecodeBsm(uint32_t vehicleId, uint32_t heading, uint32_t speed, uint32_t latitude,
			   uint32_t longitude, uint32_t elevation, tmx::messages::DecodedBsmMessage &decodedBsm);
	tmx::messages::SrmMessage* DecodeSrm(uint32_t vehicleId, uint32_t heading, uint32_t speed, uint32_t latitude,
		uint32_t longitude, uint32_t role);
	std::atomic<bool> cfgChanged { false };
	std::string ip;
	unsigned short port = 0;

	std::atomic<bool> routeDsrc { false };
	std::atomic<bool> simBSM { true };
	std::atomic<bool> simSRM { true };
	std::atomic<bool> simLoc { true };
	unsigned int verState;
	std::string url; 
	std::string baseurl;
	std::vector<string> messageid;
	std::string messageidstr; 
	std::mutex syncLock;
	tmx::utils::FrequencyThrottle<int> errThrottle;
	tmx::utils::FrequencyThrottle<int> statThrottle;

	

};

} /* namespace MessageReceiver */

#endif /* SRC_MESSAGERECEIVERPLUGIN_H_ */
