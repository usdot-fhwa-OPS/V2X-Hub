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

	ImmediateForwardPlugin::ImmediateForwardPlugin(const std::string &name) : PluginClient(name),
		_configRead(false),
		_skippedNoDsrcMetadata(0),
		_skippedNoMessageRoute(0),
		_skippedInvalidUdpClient(0)
	{
		AddMessageFilter("J2735", "*", IvpMsgFlags_RouteDSRC);
		AddMessageFilter("Battelle-DSRC", "*", IvpMsgFlags_RouteDSRC);
		SubscribeToMessages();

	}

	void ImmediateForwardPlugin::OnConfigChanged(const char *key, const char *value)
	{
		PluginClient::OnConfigChanged(key, value);
		UpdateConfigSettings();
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
		SetStatus<uint>(Key_SkippedNoDsrcMetadata, _skippedNoDsrcMetadata);
		SetStatus<uint>(Key_SkippedNoMessageRoute, _skippedNoMessageRoute);
		SetStatus<uint>(Key_SkippedInvalidUdpClient, _skippedInvalidUdpClient);
		SetStatus<uint>(Key_SkippedSignError, _skippedSignErrorResponse);
		std::string immediateForwardConfigurationsJson;
		GetConfigValue<string>("ImmediateForwardConfigurations", immediateForwardConfigurationsJson, &_configMutex);
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
						100
					);
				clearImmediateForwardTable(_snmpClientMap[imfConfig.name]);
				_imfNtcipMessageTypeIndex[imfConfig.name] = initializeImmediateForwardTable(_snmpClientMap[imfConfig.name], imfConfig.messages);
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
				for ( const auto &messageConfig: imfConfig.messages ) {

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
									<< ", Channel: " << (messageConfig.channel.has_value() ? ::to_string( msg->dsrcMetadata->channel) : ::to_string(messageConfig.channel.value()))
									<< ", Port: " << client->GetAddress();
					}
					else {
						auto &client = _snmpClientMap.at(imfConfig.name);
						sendNTCIP1218ImfMessage(client, payloadbyte, _imfNtcipMessageTypeIndex[imfConfig.name][messageConfig.sendType]);
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
	}



} /* namespace ImmediateForward */
// The main entry point for this application.
int main(int argc, char *argv[])
{
	return run_plugin<ImmediateForward::ImmediateForwardPlugin>("Immediate Forward", argc, argv);
}
