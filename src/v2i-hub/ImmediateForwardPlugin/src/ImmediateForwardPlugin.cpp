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
		else if (strcmp(msg->type, tmx::messages::RawSpdu::MessageType) == 0)
		{
			SendSpduToRadio(msg);
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


	void ImmediateForwardPlugin::SendMessageToRadio(IvpMessage *msg)
	{
		bool foundMessageType = false;
		static FrequencyThrottle<std::string> _statusThrottle(chrono::milliseconds(2000));

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
			for (const auto &imfConfig: _imfConfigs)
			{
				for ( const auto &messageConfig: imfConfig.messageConfigs ) {

					if (messageConfig.tmxType == msg->subtype)
					{
						foundMessageType = true;
						string payloadbyte="";


						// Format the message using the protocol defined in the
						// USDOT ROadside Unit Specifications Document v 4.0 Appendix C.

						stringstream os;

						/// if signing is Enabled, request signing with HSM


						if (imfConfig.enableHsm == 1)
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

							if (pipe == NULL )
								throw std::runtime_error("popen() failed!");
							try{
								while (fgets(buffer, sizeof(buffer),pipe) != NULL)
								{
									result+=buffer;
								}
							} catch (std::exception const & ex) {

								pclose(pipe);
								SetStatus<uint>(Key_SkippedSignError, ++_skippedSignErrorResponse);
								PLOG(logERROR) << "Error parsing Messages: " << ex.what();
								return;
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
								SetStatus<uint>(Key_SkippedSignError, ++_skippedSignErrorResponse);
								PLOG(logERROR) << "Error response from SCMS container HTTP code " << status->valueint << "!\n" << message->valuestring << std::endl;
								return;
							}
							cJSON *sd = cJSON_GetObjectItem(root, "signedMessage");
							string signedMsg = sd->valuestring;
							base642hex(signedMsg,payloadbyte); // this allows sending hex of the signed message rather than base64

						}
						else
						{
							payloadbyte=msg->payload->valuestring;
						}
						if (imfConfig.spec == tmx::utils::rsu::RSU_SPEC::RSU_4_1) {
							os << "Version=0.7" << "\n";
							os << "Type=" << messageConfig.sendType << "\n" << "PSID=" << messageConfig.psid << "\n";
							if (!messageConfig.channel.has_value()) {
								os << "Priority=7" << "\n" << "TxMode=" << txModeToString(imfConfig.mode) << "\n" << "TxChannel=" << msg->dsrcMetadata->channel << "\n";
							}
							else {
								os << "Priority=7" << "\n" << "TxMode=" << txModeToString(imfConfig.mode) << "\n" << "TxChannel=" << messageConfig.channel.value() << "\n";
							}
							os << "TxInterval=0" << "\n" << "DeliveryStart=\n" << "DeliveryStop=\n";
							os << "Signature=" << (imfConfig.signMessage ? "True" : "False") << "\n" << "Encryption=False\n";
							os << "Payload=" << payloadbyte << "\n";

							string message = os.str();



						auto &client = _udpClientMap.at(imfConfig.name);
						client->Send(message);
						PLOG(logDEBUG1) << _logPrefix << "Sending - TmxType: " << messageConfig.tmxType << ", SendType: " << messageConfig.sendType
									<< ", PSID: " << messageConfig.psid << ", Client: " << client->GetAddress()
									<< ", Channel: " << (messageConfig.channel.has_value() ? ::to_string(messageConfig.channel.value()) : ::to_string(msg->dsrcMetadata->channel))
									<< ", Port: " << client->GetAddress();
					}
					else {
						const auto &client = _snmpClientMap.at(imfConfig.name);
						PLOG(logDEBUG2) << "Sending - TmxType: " << messageConfig.tmxType << ", SendType: " << messageConfig.sendType
									<< ", PSID: " << messageConfig.psid << ", Client: " << client->get_port()
									<< ", Channel: " << (messageConfig.channel.has_value() ? ::to_string(messageConfig.channel.value()) : ::to_string(msg->dsrcMetadata->channel))
									<< ", Port: " << client->get_port();
						sendNTCIP1218ImfMessage(client.get(), payloadbyte, _imfNtcipMessageTypeIndex[imfConfig.name][messageConfig.sendType]);
					}
				}
			}
		}
		if (!foundMessageType)
		{
			SetStatus<uint>(Key_SkippedNoMessageRoute, ++_skippedNoMessageRoute);
			PLOG(logWARNING)<<" WARNING TMX Subtype not found in configuration. Message Ignored: " <<
					"Type: " << msg->type << ", Subtype: " << msg->subtype;
			return;
		}


	}

	void ImmediateForwardPlugin::SendSpduToRadio(IvpMessage *msg)
	{
		static FrequencyThrottle<std::string> _spduStatusThrottle(chrono::milliseconds(2000));

		// A RawSpdu payload is a JSON object rather than a hex string, so it cannot be read the way
		// SendMessageToRadio reads msg->payload->valuestring.
		SpduForwardData spduData;
		try
		{
			spduData = extractSpduForwardData(msg);
		}
		catch (const tmx::TmxException &ex)
		{
			SetStatus<uint>(Key_SkippedInvalidSpdu, ++_skippedInvalidSpdu);
			PLOG(logWARNING) << "Could not forward SPDU. Message Ignored: " << ex.what();
			return;
		}

		// The TMX subtype of a RawSpdu is always "Basic", so count against the J2735 type of the
		// message carried inside the SPDU instead.
		int msgCount = 0;
		std::map<std::string, int>::iterator itMsgCount = _messageCountMap.find(spduData.messageType);

		if (itMsgCount != _messageCountMap.end())
		{
			msgCount = (int)itMsgCount->second;
			msgCount ++;
		}

		_messageCountMap[spduData.messageType] = msgCount;

		if (_spduStatusThrottle.Monitor(spduData.messageType)) {
			SetStatus<int>(spduData.messageType, msgCount);
		}

		bool foundMessageType = false;

		for (const auto &imfConfig: _imfConfigs)
		{
			for (const auto &messageConfig: imfConfig.messageConfigs)
			{
				if (messageConfig.tmxType != spduData.messageType)
				{
					continue;
				}
				foundMessageType = true;

				// The SPDU carries its own PSID, which is what is actually broadcast. A disagreement
				// with the configuration is not fatal, but it does mean the configuration is stale.
				if (!psidMatches(messageConfig.psid, spduData.psidHex))
				{
					PLOG(logWARNING) << "PSID in configuration (" << messageConfig.psid << ") does not match PSID in received SPDU ("
								<< spduData.psidHex << ") for " << spduData.messageType << ". Forwarding with the SPDU PSID.";
				}

				int channel = messageConfig.channel.has_value() ? messageConfig.channel.value() : msg->dsrcMetadata->channel;

				if (imfConfig.spec == tmx::utils::rsu::RSU_SPEC::RSU_4_1)
				{
					// Format the message using the protocol defined in the
					// USDOT Roadside Unit Specifications Document v 4.0 Appendix C.
					stringstream os;
					os << "Version=0.7" << "\n";
					os << "Type=" << messageConfig.sendType << "\n" << "PSID=" << spduData.psidHex << "\n";
					os << "Priority=7" << "\n" << "TxMode=" << txModeToString(imfConfig.mode) << "\n" << "TxChannel=" << channel << "\n";
					os << "TxInterval=0" << "\n" << "DeliveryStart=\n" << "DeliveryStop=\n";
					// The payload is already a signed 1609.2 SPDU, so the RSU must not sign it again.
					os << "Signature=True" << "\n" << "Encryption=False\n";
					os << "Payload=" << spduData.payloadHex << "\n";

					auto &client = _udpClientMap.at(imfConfig.name);
					client->Send(os.str());
					PLOG(logDEBUG1) << _logPrefix << "Sending SPDU - TmxType: " << messageConfig.tmxType << ", SendType: " << messageConfig.sendType
								<< ", PSID: " << spduData.psidHex << ", Client: " << client->GetAddress()
								<< ", Channel: " << channel;
				}
				else
				{
					// NTCIP 1218 rows are initialized once with rsuIFMOptions 0x80 when signMessages is
					// set, which is what tells the RSU the payload already carries its own signature.
					// Forwarding an SPDU into a row initialized with 0x00 would broadcast incorrectly.
					if (!imfConfig.signMessage)
					{
						SetStatus<uint>(Key_SkippedInvalidSpdu, ++_skippedInvalidSpdu);
						PLOG(logERROR) << "Cannot forward SPDU to " << imfConfig.name
									<< " because signMessages is false. Set signMessages to true to forward already signed messages.";
						continue;
					}
					const auto &client = _snmpClientMap.at(imfConfig.name);
					PLOG(logDEBUG2) << "Sending SPDU - TmxType: " << messageConfig.tmxType << ", SendType: " << messageConfig.sendType
								<< ", PSID: " << spduData.psidHex << ", Port: " << client->get_port()
								<< ", Channel: " << channel;
					sendNTCIP1218ImfMessage(client.get(), spduData.payloadHex,
							_imfNtcipMessageTypeIndex[imfConfig.name][messageConfig.sendType], spduData.psidHex);
				}
			}
		}

		if (!foundMessageType)
		{
			SetStatus<uint>(Key_SkippedNoMessageRoute, ++_skippedNoMessageRoute);
			PLOG(logWARNING) << " WARNING SPDU message type not found in configuration. Message Ignored: " <<
					"Type: " << msg->type << ", MessageType: " << spduData.messageType;
		}
	}



} /* namespace ImmediateForward */
// The main entry point for this application.
int main(int argc, char *argv[])
{
	return run_plugin<ImmediateForward::ImmediateForwardPlugin>("Immediate Forward", argc, argv);
}
