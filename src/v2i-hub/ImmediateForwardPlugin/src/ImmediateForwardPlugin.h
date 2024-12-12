/*
 * ImmediateForwardPlugin.h
 *
 *  Created on: Feb 26, 2016
 *      Author: ivp
 */

#ifndef IMMEDIATEFORWARDPLUGIN_H_
#define IMMEDIATEFORWARDPLUGIN_H_

#include <atomic>
#include <array>
#include <map>
#include <mutex>
#include <vector>
#include "PluginClient.h"
#include "UdpClient.h"

#include <boost/chrono.hpp>
#include <FrequencyThrottle.h>
#include <curl/curl.h>
#include <tmx/json/cJSON.h>
#include <tmx/Security/include/base64.h>


namespace ImmediateForward
{

	struct MessageConfig
	{
		uint ClientIndex;
		std::string TmxType;
		std::string SendType;
		std::string Psid;
		std::string Channel;
	};

class ImmediateForwardPlugin : public tmx::utils::PluginClient
{
	public:
		ImmediateForwardPlugin(std::string name);
	private:
		void UpdateConfigSettings();
		bool UpdateUdpClientFromConfigSettings(uint clientIndex);
		bool ParseJsonMessageConfig(const std::string& json, uint clientIndex);
		int GetUdpClientIndexForMessage(std::string subtype);
		void SendMessageToRadio(IvpMessage *msg);	


		// Virtual method overrides.
		void OnConfigChanged(const char *key, const char *value);
		void OnMessageReceived(IvpMessage *msg);
		void OnStateChange(IvpPluginState state);


		// Mutex along with the data it protects.
		std::mutex _mutexUdpClient;
		// A vector of UDP clients for recieving V2X communication from different RSUs
		using svr_list = std::vector<std::unique_ptr<tmx::utils::UdpClient>>;
		std::array<svr_list, 4> _udpClientList;
		std::vector<MessageConfig> _messageConfigMap;
		std::map<std::string, int> _messageCountMap;
		std::string signatureData;
		std::string url;
		std::string baseurl;
		std::string txMode;
		unsigned int signState;
		unsigned int enableHSM;

		// Thread safe bool set to true the first time the configuration has been read.
		std::atomic<bool> _configRead;

		uint _skippedNoDsrcMetadata;
		uint _skippedNoMessageRoute;
		uint _skippedInvalidUdpClient;
		uint _skippedSignErrorResponse;



	};

} /* namespace ImmediateForward */

#endif /* IMMEDIATEFORWARDPLUGIN_H_ */
