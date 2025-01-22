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
#include <PluginClient.h>
#include <UdpClient.h>
#include <SNMPClient.h>

#include <boost/chrono.hpp>
#include <FrequencyThrottle.h>
#include <curl/curl.h>
#include <tmx/Security/include/base64.h>
#include "ImmediateForwardConfiguration.h"

namespace ImmediateForward
{

class ImmediateForwardPlugin : public tmx::utils::PluginClient
{
	public:
		ImmediateForwardPlugin(const std::string &name);
	private:
		void UpdateConfigSettings();
		bool UpdateUdpClientFromConfigSettings(uint clientIndex);
		void SendMessageToRadio(IvpMessage *msg);	


		// Virtual method overrides.
		void OnConfigChanged(const char *key, const char *value);
		void OnMessageReceived(IvpMessage *msg);
		void OnStateChange(IvpPluginState state);


		// Mutex along with the data it protects.
		std::mutex _configMutex;
		// A map of UDP clients for sending V2X communication to different RSUs for broadcast (RSU Spec 4.1)
		std::unordered_map<std::string, std::unique_ptr<tmx::utils::UdpClient>> _udpClientMap;
		// A map of SNMP Clients for sending V2X communication to different RSUs for broadvast (RSU Spec NTCIP 1218)
		std::unordered_map<std::string, std::unique_ptr<tmx::utils::snmp_client>> _snmpClientMap;
		std::vector<ImfConfiguration> _imfConfigs;
		std::map<std::string, int> _messageCountMap;

		// Thread safe bool set to true the first time the configuration has been read.
		std::atomic<bool> _configRead;

		uint _skippedNoDsrcMetadata;
		uint _skippedNoMessageRoute;
		uint _skippedInvalidUdpClient;
		uint _skippedSignErrorResponse;



	};

} /* namespace ImmediateForward */

#endif /* IMMEDIATEFORWARDPLUGIN_H_ */
