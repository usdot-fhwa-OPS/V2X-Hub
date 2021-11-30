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
#include <boost/algorithm/hex.hpp>
#include "UdpClient.h"

using namespace boost::algorithm; 
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
	UpdateConfigSettings();

	//txSign.loginSession();

	GetConfigValue<string>("HSMurl",baseurl);
	GetConfigValue<unsigned int>("signMessage",signState);
	std::string request="sign";


	url=baseurl+request;
}

string hex2bin(char c)
{
	switch(toupper(c))
    {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'A': return "1010";
        case 'B': return "1011";
        case 'C': return "1100";
        case 'D': return "1101";
        case 'E': return "1110";
        case 'F': return "1111";
    }
}

char bin2hex(string b)
{
	const char *c=b.c_str(); 
	int dec = strtol(c,nullptr,2);

	if (dec >=0 & dec <=9)
		return dec+'0'; 
	else
		return dec+'A'-10;  
}

char bin2base64(string s)
{


	const char *c= s.c_str(); 

	int dec = strtol(c,nullptr,2); 

	if ( dec >= 0 && dec <= 25 )
		return 'A'+dec;
	else if ( dec >= 26 && dec <= 51 )
		return 'a'+dec-26; 
	else if ( dec >= 52 && dec <= 61 )
		return '0'+dec-52; 
	else if (dec == 62)
		return '+';
	else 
		return '/';

	return '-'; // this would be error but is a failsafe check, shouldnt happen.

}

string dec2bin(int a)
{
	string out="000000";
	int i=0; 
	while(a)
	{
		out[out.length()-1-i]=char('0'+(a%2)); 
		a=a/2; 
		i++; 
	}

	return out; 

}

string base642bin(char b64)
{
	if (b64 >='A' & b64 <='Z')
		 return dec2bin(b64-'A'); 
	else if (b64 >='a' & b64 <='z')
		return dec2bin(b64-'a'+26); 
	else if (b64 >='0' & b64 <='9') 
		return dec2bin(b64-'0'+52); 
	else if (b64 == '+')
		return dec2bin(62);
	else 
		return dec2bin(63);
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

void DsrcMessageManagerPlugin:: hex2base64(string hexstr, string& base64str)
{

	// convert hex string to binary, 8 bits 
	// take 6 bits chops to convert to base64

	string hexbin=""; 
	int i=0; 

	while(i<hexstr.length())
		hexbin+=hex2bin(hexstr[i++]);
	
	int padcount= (int)(hexbin.length()%24)/6; 
	i=0; 
	while(i<hexbin.length())
	{
		string s = hexbin.substr(i,6); 

		if(s.length()<6)
		{
			for(int j=0;j<6-s.length();j++)
			s+="0";
		}

		base64str+=bin2base64(s);

		i+=6; 
	} 

	for (i=0;i<padcount;i++)
	base64str+='='; 

}



void DsrcMessageManagerPlugin::base642hex(string base64str, string& hexstr)
{ // this function decodes base64 stream and returns a hex stream 


	int i =0;
	string binstr=""; 
	while (i++<base64str.length())
	{	
		if(base64str[i-1]=='=')
			continue; 
		else 
			binstr+=base642bin(base64str[i-1]);
	}
	int padcount=base64str.length()-base64str.find("=");
	binstr=binstr.substr(0,binstr.length()-padcount*2); //remove twice the number of padcount bits from the end 
	i=0;
	while(i<binstr.length())
	{
		hexstr+=bin2hex(binstr.substr(i,4)); //take 4 bits for hex  
		i+=4; 
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
	GetConfigValue<unsigned int>("signMessage", signState, &_mutexUdpClient);

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
		//string json2 = "{ "Messages": [ ] }";
		//string json2 = "{ "Messages": [ { "TmxType": "MAP-P", "SendType": "MAP", "PSID": "0x8002" }, { "TmxType": "SPAT-P", "SendType": "SPAT", "PSID": "0x8002" } ] }";

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

	PLOG(logWARNING)<<_messageConfigMap.size();
	//loop through all MessageConfig and send to each with the proper TmxType
	for (int configIndex = 0;configIndex < _messageConfigMap.size();configIndex++)
	{	PLOG(logWARNING)<<_messageConfigMap[configIndex].TmxType;
		if (_messageConfigMap[configIndex].TmxType == msg->subtype)
		{
			foundMessageType = true;
			string payloadbyte="";


			// Format the message using the protocol defined in the
			// USDOT ROadside Unit Specifications Document v 4.0 Appendix C.

			stringstream os;

			/// if signing is Enabled, request signing with HSM 

			if (signState == 1)
			{
				std::string mType = _messageConfigMap[configIndex].SendType; 

				std::for_each(mType.begin(), mType.end(), [](char & c){
					c = ::tolower(c);
				});
				/* convert to hex array */

				string msgString=msg->payload->valuestring;
				string base64str=""; 

				hex2base64(msgString,base64str);  

				//string _base2hex="";

				//base642hex(base64str,_base2hex);
				//std::string req = "\'{\"type\":\""+mType+"\",\"message\":\""+msg->payload->valuestring+"\"}\'"; //old version 
				std::string req = "\'{\"type\":\""+mType+"\",\"message\":\""+base64str+"\"}\'";



				string cmd1="curl -X POST "+url+" -H \'Content-Type: application/json\' -d "+req; 
				const char *cmd=cmd1.c_str();  
				char buffer[2048];
				std::string result="";
				FILE* pipe= popen(cmd,"r"); 

				if (!pipe) throw std::runtime_error("popen() failed!");
				try{
					while (fgets(buffer, sizeof(buffer),pipe) != NULL)
					{
						result+=buffer; 
					}
				} catch (...) {
					pclose(pipe); 
					throw; 
				}

				cJSON *root   = cJSON_Parse(result.c_str());
				cJSON *sd = cJSON_GetObjectItem(root, "signedMessage");

				base642hex(sd->valuestring,payloadbyte); // this allows sending hex of the signed message rather than base64
			}
			else 
			{
				payloadbyte=msg->payload->valuestring; 
			}

			os << "Version=0.7" << "\n";
			os << "Type=" << _messageConfigMap[configIndex].SendType << "\n" << "PSID=" << _messageConfigMap[configIndex].Psid << "\n";
			if (_messageConfigMap[configIndex].Channel.empty())
				os << "Priority=7" << "\n" << "TxMode=CONT" << "\n" << "TxChannel=" << msg->dsrcMetadata->channel << "\n";
			else
				os << "Priority=7" << "\n" << "TxMode=CONT" << "\n" << "TxChannel=" << _messageConfigMap[configIndex].Channel << "\n";
			os << "TxInterval=0" << "\n" << "DeliveryStart=\n" << "DeliveryStop=\n";
			os << "Signature= "<< _signature << "\n" << "Encryption=False\n";
			os << "Payload=" << payloadbyte << "\n";

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
		PLOG(logWARNING)<<" WARNINNGGG TMX Subtype not found in configuration.  Message Ignored: " <<
				"Type: " << msg->type << ", Subtype: " << msg->subtype;
		return;
	}


}


} /* namespace DsrcMessageManager */
