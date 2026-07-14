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
#include <tmx/json/cJSON.h>
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
#include <RawSpdu.h>
#include <stol-1609dot2-2022/Ieee1609Dot2Data.h>
#include <stol-1609dot2-2022/Ieee1609Dot2Content.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <environment/EnvUtils.h>



namespace MessageReceiver {

class MessageReceiverPlugin: public tmx::utils::TmxMessageManager {
public:
	explicit MessageReceiverPlugin(const std::string &name);
	~MessageReceiverPlugin() override = default;
	int Main() override;
	using tmx::utils::TmxMessageManager::OnMessageReceived;
	void OnMessageReceived(tmx::routeable_message &msg) override;
	void getmessageid();
protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value) override;
	void OnStateChange(IvpPluginState state) override;
private:
	std::atomic<bool> cfgChanged { false };
	std::string ip;
	unsigned short port = 0;
	std::atomic<bool> routeDsrc { false };
	unsigned int verState;
	std::string url;
	std::string baseurl;
	std::vector<string> messageid;
	std::string messageidstr;
	bool fullSPDUMode = false;
	boost::uuids::random_generator _uuidGen;
	std::mutex syncLock;
	tmx::utils::FrequencyThrottle<int> errThrottle;
	tmx::utils::FrequencyThrottle<int> statThrottle;
	uint _skippedSignVerifyErrorResponse;
	uint _processedSPDU = 0;
	const char* Key_SkippedSignVerifyError = "Message Skipped (Signature Verification Error Response)";
	const char* Key_ProcessedSPDU = "Message Processed (SPDU)";
	long identifyJ2735Type(const std::vector<uint8_t>& payload);
	bool findMessageId(const std::string& hex, size_t& idloc, int& hexLen, long& dsrcMsgId);
	bool buildRawSpduMessage(const byte_stream& incoming, int len, uint64_t rxTime, tmx::messages::RawSpdu& out, std::vector<uint8_t>& payloadOut);
	bool unwrapSpdu(const Ieee1609Dot2Data_t* d, std::vector<uint8_t>& payloadOut, uint32_t& psidOut, bool& psidSet);



};

} /* namespace MessageReceiver */

#endif /* SRC_MESSAGERECEIVERPLUGIN_H_ */
