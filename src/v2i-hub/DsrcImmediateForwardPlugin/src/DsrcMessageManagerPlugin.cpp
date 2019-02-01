/*
 * DsrcMessageManagerPlugin.cpp
 *
 *  Created on: Feb 26, 2016
 *      Author: ivp
 */

#include "DsrcMessageManagerPlugin.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "UdpClient.h"


using namespace boost::property_tree;
using namespace std;
using namespace tmx;
using namespace tmx::utils;

namespace DsrcMessageManager
{

const char* Key_SkippedNoDsrcMetadata = "Messages Skipped (No DSRC metadata)";
const char* Key_SkippedNoMessageRoute = "Messages Skipped (No route)";
const char* Key_SkippedInvalidUdpClient = "Messages Skipped (Invalid UDP Client)";

DsrcMessageManagerPlugin::DsrcMessageManagerPlugin(std::string name) : PluginClient(name),
	_configRead(false),
	_skippedNoDsrcMetadata(0),
	_skippedNoMessageRoute(0),
	_skippedInvalidUdpClient(0)
{
	AddMessageFilter("J2735", "*", IvpMsgFlags_RouteDSRC);
	AddMessageFilter("Battelle-DSRC", "*", IvpMsgFlags_RouteDSRC);
	SubscribeToMessages();

	_muteDsrc = false;
	SetSystemConfigValue("MuteDsrcRadio", _muteDsrc, false);
}

DsrcMessageManagerPlugin::~DsrcMessageManagerPlugin()
{
	lock_guard<mutex> lock(_mutexUdpClient);

	for (uint i = 0; i < _udpClientList.size(); i++)
	{
		for (uint j = 0; j < _udpClientList[i].size(); j++)
		{
			if (_udpClientList[i][i] != NULL)
				delete _udpClientList[i][j];
		}
	}
}

void DsrcMessageManagerPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);

	UpdateConfigSettings();
}

void DsrcMessageManagerPlugin::OnMessageReceived(IvpMessage *msg)
{
	// Uncomment this line to call the base method, which prints the message received to cout.
	//PluginClient::OnMessageReceived(msg);

	if (!_configRead)
	{
		PLOG(logWARNING) << "Config not read yet.  Message Ignored: " <<
				"Type: " << msg->type << ", Subtype: " << msg->subtype;
		return;
	}

	if (msg->dsrcMetadata == NULL)
	{
		SetStatus<uint>(Key_SkippedNoDsrcMetadata, ++_skippedNoDsrcMetadata);
		PLOG(logWARNING) << "No DSRC metadata.  Message Ignored: " <<
				"Type: " << msg->type << ", Subtype: " << msg->subtype;
		return;
	}

	if(!_muteDsrc)
	{
		SendMessageToRadio(msg);
	}
}

void DsrcMessageManagerPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
	}
}

void DsrcMessageManagerPlugin::UpdateConfigSettings()
{
	PLOG(logINFO) << "Updating configuration settings.";

	// Update the configuration setting for all UDP clients.
	// This includes creation/update of _udpClientList and _messageConfigMap.
	{
		lock_guard<mutex> lock(_mutexUdpClient);
		_messageConfigMap.clear();

		_skippedNoDsrcMetadata = 0;
		_skippedNoMessageRoute = 0;
		_skippedInvalidUdpClient = 0;
		SetStatus<uint>(Key_SkippedNoDsrcMetadata, _skippedNoDsrcMetadata);
		SetStatus<uint>(Key_SkippedNoMessageRoute, _skippedNoMessageRoute);
		SetStatus<uint>(Key_SkippedInvalidUdpClient, _skippedInvalidUdpClient);
	}
	for (uint i = 0; i < _udpClientList.size(); i++)
	{
		UpdateUdpClientFromConfigSettings(i);
	}

	// Get the signature setting.
	// The same mutex is used that protects the UDP clients.
	GetConfigValue("Signature", _signature, &_mutexUdpClient);

	GetConfigValue("MuteDsrcRadio", _muteDsrc);
	SetStatus("MuteDsrc", _muteDsrc);
	_configRead = true;
}

// Retrieve all settings for a UDP client, then create a UDP client using those settings.
// Other settings related to the UDP client are also updated (i.e. msg id list, psid list).
bool DsrcMessageManagerPlugin::UpdateUdpClientFromConfigSettings(uint clientIndex)
{
	if (_udpClientList.size() <= clientIndex)
	{
		PLOG(logWARNING) << "Invalid client number. Only " << _udpClientList.size() << " clients available.";
		return false;
	}

	int clientNum = clientIndex + 1;
	string messagesSetting((boost::format("Messages_Destination_%d") % clientNum).str());
	string udpPortSetting((boost::format("Destination_%d") % clientNum).str());

	try
	{
		string destinations;
		GetConfigValue(udpPortSetting, destinations);

		string messages;
		GetConfigValue(messagesSetting, messages);

		// Take the lock while shared data is accessed.
		// A lock_guard will unlock when it goes out of scope (even if an exception occurs).
		lock_guard<mutex> lock(_mutexUdpClient);

		ParseJsonMessageConfig(messages, clientIndex);

		for (uint i = 0; i < _udpClientList[clientIndex].size(); i++)
		{
			if (_udpClientList[clientIndex][i] != NULL)
				delete _udpClientList[clientIndex][i];
		}

		_udpClientList[clientIndex].clear();

		if (destinations.length() > 0)
		{
			vector<string> srvs;
			boost::split(srvs, destinations, boost::is_any_of(" \t,;"));

			for (uint i = 0; i < srvs.size(); i++)
			{
				vector<string> addr;
				boost::split(addr, srvs[i], boost::is_any_of(":"));
				if (addr.size() != 2)
					continue;

				PLOG(logINFO) << "Creating UDP Client " << (clientIndex + 1) <<
						" - Radio IP: " << addr[0] << ", Port: " << addr[1];
				_udpClientList[clientIndex].push_back(new UdpClient(addr[0], ::atoi(addr[1].c_str())));
			}
		}
	}
	catch(std::exception const & ex)
	{
		PLOG(logERROR) << "Error getting config settings: " << ex.what();
		return false;
	}

	return true;
}

bool DsrcMessageManagerPlugin::ParseJsonMessageConfig(const std::string& json, uint clientIndex)
{
	if (json.length() == 0)
		return true;

	try
	{
		//delete all MessageConfig for this client
		for (auto it = _messageConfigMap.begin(); it != _messageConfigMap.end(); )
		{
			 if (it->ClientIndex == clientIndex)
				 it = _messageConfigMap.erase(it);
			 else
				 ++it;

		}

		// Example JSON parsed:
		// { "Messages": [ { "TmxType": "MAP-P", "SendType": "MAP", "PSID": "0x8002", "Channel": "172" }, { "TmxType": "SPAT-P", "SendType": "SPAT", "PSID": "0x8002" } ] }
		// The strings below (with extra quotes escaped) can be used for testing.
		//string json2 = "{ \"Messages\": [ ] }";
		//string json2 = "{ \"Messages\": [ { \"TmxType\": \"MAP-P\", \"SendType\": \"MAP\", \"PSID\": \"0x8002\" }, { \"TmxType\": \"SPAT-P\", \"SendType\": \"SPAT\", \"PSID\": \"0x8002\" } ] }";

		// Read the JSON into a boost property tree.
		ptree pt;
		istringstream is(json);
		read_json(is, pt);

		// Iterate over the Messages section of the property tree.
		// Note that Messages is at the root of the property tree, otherwise the entire
		// path to the child would have to be specified to get_child.
		BOOST_FOREACH(ptree::value_type &child, pt.get_child("Messages"))
		{
			// Array elements have no names.
			assert(child.first.empty());

			MessageConfig config;
			config.ClientIndex = clientIndex;
			config.TmxType = child.second.get<string>("TmxType");
			config.SendType = child.second.get<string>("SendType");
			config.Psid = child.second.get<string>("PSID");
			try
			{
				config.Channel = child.second.get<string>("Channel");
			}
			catch(std::exception const & exChannel)
			{
				config.Channel.clear();
			}

			PLOG(logINFO) << "Message Config - Client: " << (config.ClientIndex + 1) <<
					", TmxType: " << config.TmxType << ", SendType: " << config.SendType << ", PSID: " << config.Psid <<
					", Channel: " << config.Channel;

			// Add the message configuration to the map.
			_messageConfigMap.push_back(config);
		}
	}
	catch(std::exception const & ex)
	{
		PLOG(logERROR) << "Error parsing Messages: " << ex.what();
		return false;
	}

	return true;
}

void DsrcMessageManagerPlugin::SendMessageToRadio(IvpMessage *msg)
{
	bool foundMessageType = false;
	static FrequencyThrottle<std::string> _statusThrottle(chrono::milliseconds(2000));

	lock_guard<mutex> lock(_mutexUdpClient);

	int msgCount = 0;

	std::map<std::string, int>::iterator itMsgCount = _messageCountMap.find(msg->subtype);

	if(itMsgCount != _messageCountMap.end())
	{
		msgCount = (int)itMsgCount->second;
		msgCount ++;
	}

	_messageCountMap[msg->subtype] = msgCount;


	if (_statusThrottle.Monitor(msg->subtype)) {
		SetStatus<int>(msg->subtype, msgCount);
	}

	// Convert the payload to upper case.
	for (int i = 0; i < (int)(strlen(msg->payload->valuestring)); i++)
		msg->payload->valuestring[i] = toupper(msg->payload->valuestring[i]);


	//loop through all MessageConfig and send to each with the proper TmxType
	for (int configIndex = 0;configIndex < _messageConfigMap.size();configIndex++)
	{
		if (_messageConfigMap[configIndex].TmxType == msg->subtype)
		{
			foundMessageType = true;

			// Format the message using the protocol defined in the
			// USDOT ROadside Unit Specifications Document v 4.0 Appendix C.

			stringstream os;

			os << "Version=0.7" << "\n";
			os << "Type=" << _messageConfigMap[configIndex].SendType << "\n" << "PSID=" << _messageConfigMap[configIndex].Psid << "\n";
			if (_messageConfigMap[configIndex].Channel.empty())
				os << "Priority=7" << "\n" << "TxMode=CONT" << "\n" << "TxChannel=" << msg->dsrcMetadata->channel << "\n";
			else
				os << "Priority=7" << "\n" << "TxMode=CONT" << "\n" << "TxChannel=" << _messageConfigMap[configIndex].Channel << "\n";
			os << "TxInterval=0" << "\n" << "DeliveryStart=\n" << "DeliveryStop=\n";
			os << "Signature= "<< _signature << "\n" << "Encryption=False\n";
			os << "Payload=" << msg->payload->valuestring << "\n";

			string message = os.str();

			// Send the message using the configured UDP client.

			for (uint i = 0; i < _udpClientList[_messageConfigMap[configIndex].ClientIndex].size(); i++)
			{
				//cout << message << endl;

				if (_udpClientList[_messageConfigMap[configIndex].ClientIndex][i] != NULL)
				{
					PLOG(logDEBUG2) << _logPrefix << "Sending - TmxType: " << _messageConfigMap[configIndex].TmxType << ", SendType: " << _messageConfigMap[configIndex].SendType
						<< ", PSID: " << _messageConfigMap[configIndex].Psid << ", Client: " << _messageConfigMap[configIndex].ClientIndex
						<< ", Channel: " << (_messageConfigMap[configIndex].Channel.empty() ? ::to_string( msg->dsrcMetadata->channel) : _messageConfigMap[configIndex].Channel)
						<< ", Port: " << _udpClientList[_messageConfigMap[configIndex].ClientIndex][i]->GetPort();

					_udpClientList[_messageConfigMap[configIndex].ClientIndex][i]->Send(message);
				}
				else
				{
					SetStatus<uint>(Key_SkippedInvalidUdpClient, ++_skippedInvalidUdpClient);
					PLOG(logWARNING) << "UDP Client Invalid. Cannot send message. TmxType: " << _messageConfigMap[configIndex].TmxType;
				}
			}
		}
	}

	if (!foundMessageType)
	{
		SetStatus<uint>(Key_SkippedNoMessageRoute, ++_skippedNoMessageRoute);
		PLOG(logWARNING) << "TMX Subtype not found in configuration.  Message Ignored: " <<
				"Type: " << msg->type << ", Subtype: " << msg->subtype;
		return;
	}


}


} /* namespace DsrcMessageManager */
