/**
 * Copyright (C) 2025 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#include "ImmediateForwardPlugin.h"
#include <tmx/TmxException.hpp>

using namespace std;
using namespace tmx::utils;

namespace ImmediateForward
{	
	ImmediateForwardPlugin::ImmediateForwardPlugin(const std::string &name) : PluginClient(name)
	{
		AddMessageFilter("J2735", "*", IvpMsgFlags_RouteDSRC);
		AddMessageFilter("Battelle-DSRC", "*", IvpMsgFlags_RouteDSRC);
		AddMessageFilter(tmx::messages::RawSpdu::MessageType, "*", IvpMsgFlags_RouteDSRC);
		SubscribeToMessages();

	}

	void ImmediateForwardPlugin::OnConfigChanged(const char *key, const char *value)
	{
		PluginClient::OnConfigChanged(key, value);
		if (IsPluginState(IvpPluginState_registered)) {
			UpdateConfigSettings();
		}
	}

	void ImmediateForwardPlugin::OnMessageReceived(IvpMessage *msg)
	{
		// Uncomment this line to call the base method, which prints the message received to cout.
		PluginClient::OnMessageReceived(msg);
		if (!_configRead)
		{
			PLOG(logWARNING) << "Config not read yet.  Message Ignored: " <<"Type: " << msg->type << ", Subtype: " << msg->subtype;
		}
		else if (msg->dsrcMetadata == nullptr)
		{
			SetStatus<uint>(Key_SkippedNoDsrcMetadata, ++_skippedNoDsrcMetadata);
			PLOG(logWARNING) << "No DSRC metadata.  Message Ignored: " << "Type: " << msg->type << ", Subtype: " << msg->subtype;
		}
		else {
			SendMessageToRadio(msg);
		}


	}

	void ImmediateForwardPlugin::OnStateChange(IvpPluginState state)
	{
		PluginClient::OnStateChange(state);

		if (state == IvpPluginState_registered)
		{
			UpdateConfigSettings();
		}
	}

	void ImmediateForwardPlugin::UpdateConfigSettings()
	{
		PLOG(logDEBUG) << "Updating configuration settings.";
		
		// Update the configuration setting for all UDP clients.
		// This includes creation/update of _udpClientList and _imfConfigs
		_imfConfigs.clear();
		_skippedNoDsrcMetadata = 0;
		_skippedNoMessageRoute = 0;
		_skippedInvalidUdpClient = 0;
		_skippedSignErrorResponse = 0;
		_skippedInvalidSpdu = 0;
		SetStatus<uint>(Key_SkippedNoDsrcMetadata, _skippedNoDsrcMetadata);
		SetStatus<uint>(Key_SkippedNoMessageRoute, _skippedNoMessageRoute);
		SetStatus<uint>(Key_SkippedInvalidUdpClient, _skippedInvalidUdpClient);
		SetStatus<uint>(Key_SkippedSignError, _skippedSignErrorResponse);
		SetStatus<uint>(Key_SkippedInvalidSpdu, _skippedInvalidSpdu);
		std::string immediateForwardConfigurationsJson;
		GetConfigValue<string>("ImmediateForwardConfigurations", immediateForwardConfigurationsJson);
		_imfConfigs.clear();
		_imfConfigs =  parseImmediateForwardConfiguration(immediateForwardConfigurationsJson);
		// Setup UDP Clients
		_udpClientMap.clear();
		_snmpClientMap.clear();
		_imfNtcipMessageTypeIndex.clear();
		for (const auto &imfConfig: _imfConfigs) {
			if (imfConfig.spec == tmx::utils::rsu::RSU_SPEC::RSU_4_1) {
				_udpClientMap[imfConfig.name] = std::make_unique<tmx::utils::UdpClient>(imfConfig.address, imfConfig.port);
			}
			else if ( imfConfig.spec == tmx::utils::rsu::RSU_SPEC::NTCIP_1218) {
				_snmpClientMap[imfConfig.name] = std::make_unique<tmx::utils::snmp_client>(
						imfConfig.address, 
						imfConfig.port, 
						imfConfig.snmpAuth.value().community, 
						imfConfig.snmpAuth.value().user,
						securityLevelToString(imfConfig.snmpAuth.value().securityLevel),
						imfConfig.snmpAuth.value().authProtocol.value(),
						imfConfig.snmpAuth.value().authPassPhrase.value(),
						imfConfig.snmpAuth.value().privProtocol.value(),
						imfConfig.snmpAuth.value().privPassPhrase.value(),
						3,
						imfConfig.snmpAuth.value().snmpTimeout.value_or(1000000) // Defaults to 1000000 microseconds or 1 second.
					);
				// Set to standby mode
				setRSUMode(_snmpClientMap[imfConfig.name].get(), 2);
				// This will check RSU Mode for Standby status a maximum of 12 times waiting 5 seconds 
				// after each time before failing
				waitForRSUModeStandby(_snmpClientMap[imfConfig.name].get(), 12 ,5);
				clearImmediateForwardTable(_snmpClientMap[imfConfig.name].get());
				_imfNtcipMessageTypeIndex[imfConfig.name] = initializeImmediateForwardTable(
					_snmpClientMap[imfConfig.name].get(), 
					imfConfig.messageConfigs, 
					imfConfig.signMessage,
					imfConfig.payloadPlaceholder.value_or("FFFF") );
				// Set to operational mode
				setRSUMode(_snmpClientMap[imfConfig.name].get(), 3);

			}
		}
		// The same mutex is used that protects the UDP clients.
		_configRead = true;
		PLOG(logDEBUG) << "Configurations read sucessfully!";

	}


	inline void ImmediateForwardPlugin::SignWithHsm(const ImfConfiguration& imfConfig, const MessageConfig& messageConfig, IvpMessage* msg, string& payloadbyte)
	{
		std::string mType = messageConfig.sendType;

		std::for_each(mType.begin(), mType.end(), [](char & c){
			c = ::tolower(c);
		});
		/* convert to hex array */

		string msgString=msg->payload->valuestring;
		string base64str="";

		hex2base64(msgString,base64str);

		std::string req = "\'{\"type\":\""+mType+"\",\"message\":\""+base64str+"\"}\'";

		string cmd1="curl -X POST " + imfConfig.hsmUrl.value() + "sign" + " -H \'Content-Type: application/json\' -d "+req;
		const char *cmd=cmd1.c_str();
		char buffer[2048];
		std::string result="";
		FILE* pipe= popen(cmd,"r");

		if (pipe == NULL ){
			throw tmx::TmxException("popen() failed!");
		}

		while (fgets(buffer, sizeof(buffer), pipe) != NULL)
		{
			result+=buffer;
		}

		bool hasFerror = false;
		if (ferror(pipe)) {
			hasFerror = true;
		} else if (feof(pipe)) {
			PLOG(logDEBUG1) << "Successfully read all command output.";
		}
		
		// close pipe first before throwing any exception
		if(pclose(pipe)==-1){
			throw tmx::TmxException("pclose() failed!");
		};
		
		// catch stream error
		if(hasFerror){
			throw tmx::TmxException("A stream error occurred while reading from the pipe.");
		}

		PLOG(logDEBUG1) << "SCMS Contain response = " << result << std::endl;
		cJSON *root   = cJSON_Parse(result.c_str());
		// Check if status is 200 (successful)
		cJSON *status = cJSON_GetObjectItem(root, "code");
		if ( status ) {
			// IF status code exists this means the SCMS container returned an error response on attempting to sign
			// Set status will increment the count of message skipped due to signature error responses by one each
			// time this occurs. This count will be visible under the "State" tab of this plugin.
			cJSON *message = cJSON_GetObjectItem(root, "message");
			throw tmx::TmxException("Error response from SCMS container HTTP code " + std::to_string(status->valueint) + "!\n" + message->valuestring + "\n");
		}
		cJSON *sd = cJSON_GetObjectItem(root, "signedMessage");
		string signedMsg = sd->valuestring;
		base642hex(signedMsg, payloadbyte); // this allows sending hex of the signed message rather than base64
	}

	inline string ImmediateForwardPlugin::ConstructMessageRSU_4_1(const ImfConfiguration& imfConfig, const MessageConfig& messageConfig, IvpMessage* msg, const string& payloadbyte, bool signMessage){
		stringstream os;
		os << "Version=0.7" << "\n"
		   << "Type=" << messageConfig.sendType << "\n"
		   << "PSID=" << messageConfig.psid << "\n";
		if (!messageConfig.channel.has_value()) {
			os << "Priority=7" << "\n" 
			   << "TxMode=" << txModeToString(imfConfig.mode) << "\n"
			   << "TxChannel=" << msg->dsrcMetadata->channel << "\n";
		}
		else {
			os << "Priority=7" << "\n" 
			   << "TxMode=" << txModeToString(imfConfig.mode) << "\n" 
			   << "TxChannel=" << messageConfig.channel.value() << "\n";
		}
		os << "TxInterval=0" << "\n" 
		   << "DeliveryStart=\n" 
		   << "DeliveryStop=\n"
		   << "Signature=" << (signMessage ? "True" : "False") << "\n"
		   << "Encryption=False\n"
		   << "Payload=" << payloadbyte << "\n";

		return os.str();
	}

	void ImmediateForwardPlugin::SendMessageToRadio(IvpMessage *msg)
	{
		bool foundMessageType = false;
		static FrequencyThrottle<std::string> _statusThrottle(chrono::milliseconds(2000));

		const bool isSpdu = IsSPDU(msg);
		string tmxType;
		string payload;

		if (isSpdu)  // set tmxType and payload depending on whether it is spdu or not.
		{	
			tmx::messages::RawSpdu rawSpdu;
			try {
				rawSpdu = getRawSpdu(msg);
			}
			catch (const tmx::TmxException &ex) {
				SetStatus<uint>(Key_SkippedInvalidSpdu, ++_skippedInvalidSpdu);
				PLOG(logWARNING) << "Could not forward SPDU. Message Ignored: " << ex.what();
				return;
			}
			tmxType = rawSpdu.get_messageType();
			payload = toUpperHex(rawSpdu.get_fullByteData());

			PLOG(logDEBUG) << "Forwarding SPDU message with uuid: " << tmx::byte_stream_encode(rawSpdu.get_uuid())
						   << " and timestamp: " << msg->timestamp;
		}
		else
		{
			tmxType = msg->subtype;

			// Convert the payload to upper case.
			for (int i = 0; i < (int)(strlen(msg->payload->valuestring)); i++){
				msg->payload->valuestring[i] = toupper(msg->payload->valuestring[i]);
			}

			payload = msg->payload->valuestring;
		}

		int msgCount = 0;

		std::map<std::string, int>::iterator itMsgCount = _messageCountMap.find(tmxType);

		if(itMsgCount != _messageCountMap.end())
		{
			msgCount = (int)itMsgCount->second;
			msgCount ++;
		}

		_messageCountMap[tmxType] = msgCount;


		if (_statusThrottle.Monitor(tmxType)) {
			SetStatus<int>(tmxType.c_str(), msgCount);
		}

		//loop through all MessageConfig and send to each with the proper TmxType
		for (const auto &imfConfig: _imfConfigs)
		{
			for ( const auto &messageConfig: imfConfig.messageConfigs )
			{
				if (messageConfig.tmxType != tmxType){continue;}

				foundMessageType = true;
				string payloadbyte="";
				bool signMessage = imfConfig.signMessage;

				// Format the message using the protocol defined in the
				// USDOT ROadside Unit Specifications Document v 4.0 Appendix C.

				if (isSpdu)
				{
					// A raw SPDU is already signed so the RSU must not sign again. signMessages is thus set to false.
					signMessage = false;
					payloadbyte = payload;
				}
				/// if signing is Enabled, request signing with HSM
				else if (imfConfig.enableHsm == 1)
				{
					try {
						SignWithHsm(imfConfig, messageConfig, msg, payloadbyte);
					}
					catch (const tmx::TmxException &ex) {
						SetStatus<uint>(Key_SkippedSignError, ++_skippedSignErrorResponse);
						PLOG(logERROR) << "Signing with HSM failed: " << ex.what();

						// if signing failed, return immediately
						return;
					}
				}
				else
				{
					payloadbyte=payload;
				}

				if (imfConfig.spec == tmx::utils::rsu::RSU_SPEC::RSU_4_1) {
					string message = ConstructMessageRSU_4_1(imfConfig, messageConfig, msg, payloadbyte, signMessage);

					auto &client = _udpClientMap.at(imfConfig.name);
					client->Send(message);

					PLOG(logDEBUG1) << _logPrefix
									<< "Sending - TmxType: " << messageConfig.tmxType
									<< ", SendType: " << messageConfig.sendType
									<< ", PSID: " << messageConfig.psid
									<< ", Client: " << client->GetAddress()
									<< ", Channel: " << (messageConfig.channel.has_value() ? ::to_string( msg->dsrcMetadata->channel) : ::to_string(messageConfig.channel.value()))
									<< ", Port: " << client->GetAddress()
									<< ", SignMessage: " << signMessage;
				}
				else {
					const auto &client = _snmpClientMap.at(imfConfig.name);
					sendNTCIP1218ImfMessage(client.get(), payloadbyte, _imfNtcipMessageTypeIndex[imfConfig.name][messageConfig.sendType], messageConfig.psid, signMessage);

					PLOG(logDEBUG1) << "Sending - TmxType: " << messageConfig.tmxType
									<< ", SendType: " << messageConfig.sendType
									<< ", PSID: " << messageConfig.psid
									<< ", Client: " << client->get_port()
									<< ", Channel: " << (messageConfig.channel.has_value() ? ::to_string( msg->dsrcMetadata->channel) : ::to_string(messageConfig.channel.value()))
									<< ", Port: " << client->get_port()
									<< ", SignMessage: " << signMessage;
				}
		
			}
		}
		if (!foundMessageType)
		{
			SetStatus<uint>(Key_SkippedNoMessageRoute, ++_skippedNoMessageRoute);
			PLOG(logWARNING)<<" WARNING TMX Subtype not found in configuration. Message Ignored: " <<
					"Type: " << msg->type << ", Subtype: " << tmxType;
			return;
		}


	}



} /* namespace ImmediateForward */
// The main entry point for this application.
int main(int argc, char *argv[])
{
	return run_plugin<ImmediateForward::ImmediateForwardPlugin>("Immediate Forward", argc, argv);
}
