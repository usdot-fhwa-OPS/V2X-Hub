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

	const char* Key_SkippedNoDsrcMetadata = "Messages Skipped (No DSRC metadata)";
	const char* Key_SkippedNoMessageRoute = "Messages Skipped (No route)";
	const char* Key_SkippedSignError = "Message Skipped (Signature Error Response)";
	const char* Key_SkippedInvalidUdpClient = "Messages Skipped (Invalid UDP Client)";
	const char* Key_SkippedInvalidSpdu = "Messages Skipped (Invalid SPDU)";

	ImmediateForwardPlugin::ImmediateForwardPlugin(const std::string &name) : PluginClient(name),
		_configRead(false),
		_skippedNoDsrcMetadata(0),
		_skippedNoMessageRoute(0),
		_skippedInvalidUdpClient(0),
		_skippedInvalidSpdu(0)
	{
		AddMessageFilter("J2735", "*", IvpMsgFlags_RouteDSRC);
		AddMessageFilter("Battelle-DSRC", "*", IvpMsgFlags_RouteDSRC);
		// Raw 1609.2 SPDUs, published by MessageReceiverPlugin in FullSPDUMode, are forwarded to the
		// radio exactly as received rather than being re-encoded or re-signed.
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

	inline string ImmediateForwardPlugin::ConstructMessageRSU_4_1(const ImfConfiguration& imfConfig, const MessageConfig& messageConfig, IvpMessage* msg, const string& payloadbyte, const string& psid){
		stringstream os;
		os << "Version=0.7" << "\n"
		   << "Type=" << messageConfig.sendType << "\n"
		   << "PSID=" << psid << "\n";
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
		   << "Signature=" << (imfConfig.signMessage ? "True" : "False") << "\n" 
		   << "Encryption=False\n"
		   << "Payload=" << payloadbyte << "\n";

		return os.str();
	}

	void ImmediateForwardPlugin::SendMessageToRadio(IvpMessage *msg)
	{
		bool foundMessageType = false;
		static FrequencyThrottle<std::string> _statusThrottle(chrono::milliseconds(2000));

		// A raw SPDU carries its payload as a JSON object rather than a hex string, and routes on the
		// J2735 type of the message inside the SPDU rather than on the TMX subtype, which is always
		// "Basic" for a RawSpdu.
		const bool isSpdu = (strcmp(msg->type, tmx::messages::RawSpdu::MessageType) == 0);
		tmx::messages::RawSpdu rawSpdu;
		string tmxType;
		string spduPayload;

		if (isSpdu)
		{
			try {
				rawSpdu = getRawSpdu(msg);
			}
			catch (const std::exception &ex) {
				SetStatus<uint>(Key_SkippedInvalidSpdu, ++_skippedInvalidSpdu);
				PLOG(logWARNING) << "Could not forward SPDU. Message Ignored: " << ex.what();
				return;
			}
			tmxType = rawSpdu.get_messageType();
			spduPayload = toUpperHex(rawSpdu.get_fullByteData());
		}
		else
		{
			tmxType = msg->subtype;

			// Convert the payload to upper case.
			for (int i = 0; i < (int)(strlen(msg->payload->valuestring)); i++){
				msg->payload->valuestring[i] = toupper(msg->payload->valuestring[i]);
			}
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
				// A forwarded SPDU broadcasts under the PSID it arrived with, so that what goes out
				// over the air matches the message that came in.
				string psid = messageConfig.psid;

				// Format the message using the protocol defined in the
				// USDOT ROadside Unit Specifications Document v 4.0 Appendix C.

				if (isSpdu)
				{
					// NTCIP 1218 rows are initialized once with rsuIFMOptions 0x80 when signMessages
					// is set, which is what tells the RSU the payload already carries its own
					// signature. RSU4.1 likewise sends Signature=True from this flag. Forwarding an
					// already signed SPDU without it would broadcast incorrectly.
					if (!imfConfig.signMessage)
					{
						SetStatus<uint>(Key_SkippedInvalidSpdu, ++_skippedInvalidSpdu);
						PLOG(logERROR) << "Cannot forward SPDU to " << imfConfig.name
									   << ": set signMessages to true to forward already signed messages.";
						continue;
					}
					// The SPDU is forwarded byte for byte, under its own PSID.
					payloadbyte = spduPayload;
					psid = toPsidHex(rawSpdu.get_psid());
					PLOG(logDEBUG3) << "Forwarding SPDU with PSID " << psid
									<< " (configured PSID for " << messageConfig.tmxType << " is "
									<< messageConfig.psid << ")";
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
					payloadbyte=msg->payload->valuestring;
				}

				if (imfConfig.spec == tmx::utils::rsu::RSU_SPEC::RSU_4_1) {
					string message = ConstructMessageRSU_4_1(imfConfig, messageConfig, msg, payloadbyte, psid);

					auto &client = _udpClientMap.at(imfConfig.name);
					client->Send(message);

					PLOG(logDEBUG1) << _logPrefix
									<< "Sending - TmxType: " << messageConfig.tmxType
									<< ", SendType: " << messageConfig.sendType
									<< ", PSID: " << psid
									<< ", Client: " << client->GetAddress()
									<< ", Channel: " << (messageConfig.channel.has_value() ? ::to_string( msg->dsrcMetadata->channel) : ::to_string(messageConfig.channel.value()))
									<< ", Port: " << client->GetAddress();
				}
				else {
					const auto &client = _snmpClientMap.at(imfConfig.name);
					sendNTCIP1218ImfMessage(client.get(), payloadbyte, _imfNtcipMessageTypeIndex[imfConfig.name][messageConfig.sendType], psid);

					PLOG(logDEBUG2) << "Sending - TmxType: " << messageConfig.tmxType
									<< ", SendType: " << messageConfig.sendType
									<< ", PSID: " << psid
									<< ", Client: " << client->get_port()
									<< ", Channel: " << (messageConfig.channel.has_value() ? ::to_string( msg->dsrcMetadata->channel) : ::to_string(messageConfig.channel.value()))
									<< ", Port: " << client->get_port();
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
