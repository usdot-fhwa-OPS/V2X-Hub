/*
 * BsmReceiver.cpp
 *
 *  Created on: May 10, 2016
 *      Author: ivp
 */

#include "MessageReceiverPlugin.h"


#define ABBR_BSM 1000
#define ABBR_SRM 2000

#define IDCHECKLIMIT 60
#if SAEJ2735_SPEC >= 2024
typedef Common_Longitude_t Longitude_t;
typedef Common_Latitude_t Latitude_t;
typedef Common_Elevation_t Elevation_t;
#endif
using namespace std;
using namespace boost::asio;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

// BSMs may be 10 times a second, so only send errors at most every 2 minutes
#define ERROR_WAIT_MS 120000
#define STATUS_WAIT_MS 2000

namespace MessageReceiver {

	static std::atomic<uint64_t> totalBytes {0};
	static std::map<std::string, std::atomic<uint32_t> > totalCount;

MessageReceiverPlugin::MessageReceiverPlugin(std::string name): TmxMessageManager(name)
{
	errThrottle.set_Frequency(std::chrono::milliseconds(ERROR_WAIT_MS));
	statThrottle.set_Frequency(std::chrono::milliseconds(STATUS_WAIT_MS));
}

void MessageReceiverPlugin::getmessageid()
{

	stringstream ss(messageidstr);

	messageid.clear(); // better to clear out the vector

	while(ss.good())
	{
		string tmp;
		getline(ss, tmp, ',' );
		messageid.push_back(tmp);
	}

}


MessageReceiverPlugin::~MessageReceiverPlugin() { }

template <typename T>
TmxJ2735EncodedMessage<T> *encode(TmxJ2735EncodedMessage<T> &encMsg, T *msg) {
	encMsg.clear();

	if (msg)
		encMsg.initialize(*msg);

	// Clean up the TMX message pointer and thus the J2735 structure pointer
	delete msg;
	return &encMsg;
}

BsmMessage* MessageReceiverPlugin::DecodeBsm(uint32_t vehicleId, uint32_t heading, uint32_t speed, uint32_t latitude,
			   uint32_t longitude, uint32_t elevation, DecodedBsmMessage &decodedBsm)
{
	PLOG(logDEBUG4) << "BSM vehicleId: " << vehicleId
			<< ", heading: " << heading
			<< ", speed: " << speed
			<< ", latitude: " << latitude
			<< ", longitude: " << longitude
			<< ", elevation: " << elevation<< " \n";

	//send BSM

	// Set the temp ID
	decodedBsm.set_TemporaryId(vehicleId);

	// Heading
	decodedBsm.set_Heading((float)(heading / 1000000.0));
	decodedBsm.set_IsHeadingValid(true);

	// Speed
	decodedBsm.set_Speed_mps((double)(speed / 1000.0));
	decodedBsm.set_IsSpeedValid(true);

	// Latitude and Longitude
	decodedBsm.set_Latitude((double)(latitude / 1000000.0 - 180));
	decodedBsm.set_Longitude((double)(longitude / 1000000.0 - 180));
	decodedBsm.set_IsLocationValid(true);

	// Altitude
	decodedBsm.set_Elevation_m((float)(elevation / 1000.0 - 500));
	decodedBsm.set_IsElevationValid(true);



	BasicSafetyMessage *bsm = (BasicSafetyMessage *)calloc(1, sizeof(BasicSafetyMessage));
	if (bsm)
		BsmConverter::ToBasicSafetyMessage(decodedBsm, *bsm);

	PLOG(logDEBUG4) << " Decoded BSM: " << decodedBsm;
	// Note that this constructor assumes control of cleaning up the J2735 structure pointer
	return new BsmMessage(bsm);
}

SrmMessage* MessageReceiverPlugin::DecodeSrm(uint32_t vehicleId, uint32_t heading, uint32_t speed, uint32_t latitude,
		uint32_t longitude, uint32_t role)
{
	PLOG(logDEBUG4) << "SRM vehicleId: " << vehicleId
			<< ", heading: " << heading
			<< ", speed: " << speed
			<< ", latitude: " << latitude
			<< ", longitude: " << longitude
			<< ", role: " << role;

	// send SRM
	size_t s = sizeof(vehicleId);
	SignalRequestMessage *srm = (SignalRequestMessage *)calloc(1, sizeof(SignalRequestMessage));
	if (srm) {
		srm->requestor.id.present = VehicleID_PR_entityID;
		srm->requestor.id.choice.entityID.size = s;
		srm->requestor.id.choice.entityID.buf = (uint8_t *)calloc(s, sizeof(uint8_t));
		if (srm->requestor.id.choice.entityID.buf)
			memcpy(srm->requestor.id.choice.entityID.buf, &vehicleId, s);

		srm->requestor.type =
				(struct RequestorType *)calloc(1, sizeof(struct RequestorType));
		if (srm->requestor.type)
		{
			srm->requestor.type->role = (BasicVehicleRole_t)role;
			srm->requestor.position =
				(struct RequestorPositionVector *)calloc(1, sizeof(struct RequestorPositionVector));
			if (srm->requestor.position)
			{
				srm->requestor.position->position.lat = (Latitude_t)(10.0 * latitude - 1800000000);
				srm->requestor.position->position.Long = (Longitude_t)(10.0 * longitude - 1800000000);
				srm->requestor.position->heading =
						#if SAEJ2735_SPEC < 2020
						(DSRC_Angle_t *)calloc(1, sizeof(DSRC_Angle_t));
						#else
						(Common_Angle_t *)calloc(1, sizeof(Common_Angle_t));
						#endif
				if (srm->requestor.position->heading)
					#if SAEJ2735_SPEC < 2020
					*(srm->requestor.position->heading) = (DSRC_Angle_t)(heading / 12500.0);
					#else
					*(srm->requestor.position->heading) = (Common_Angle_t)(heading / 12500.0);
					#endif
				srm->requestor.position->speed =
						(TransmissionAndSpeed *)calloc(1, sizeof(TransmissionAndSpeed));
				if (srm->requestor.position->speed)
				{
					srm->requestor.position->speed->transmisson = TransmissionState_unavailable;
					srm->requestor.position->speed->speed = (Velocity_t)(speed / 20.0);
				}
			}
		}
	}

	// Note that this constructor assumes control of cleaning up the J2735 structure pointer
	return new SrmMessage(srm);
}

void MessageReceiverPlugin::OnMessageReceived(routeable_message &msg)
{
	routeable_message *sendMsg = &msg;

	DecodedBsmMessage decodedBsm;
	BsmEncodedMessage encodedBsm;
	SrmEncodedMessage encodedSrm;

	int msgPSID = api::msgPSID::None_PSID;

	if (msg.get_type() == "Unknown" && msg.get_subtype() == "Unknown")
	{
		if (msg.get_encoding() == api::ENCODING_JSON_STRING)
		{
			// Check to see if the payload is a routable message

			message payloadMsg = msg.get_payload<message>();
			if (payloadMsg.get_untyped("header.type", "Unknown") != "Unknown")
			{
				msg.clear();
				msg.set_contents(payloadMsg.get_container());
				msg.reinit();
			}
		}
		else if (msg.get_encoding() == api::ENCODING_BYTEARRAY_STRING)
		{
			try
			{
				// Check for an abbreviated message
				byte_stream bytesFull = msg.get_payload_bytes();
				byte_stream bytes;
				if (bytes.size() > 8)
				{
					PLOG(logDEBUG) << "Looking for abbreviated message in bytes " << bytes;
					uint16_t msgType;
					uint8_t msgVersion;
					uint16_t id;
					uint16_t dataLength;
					uint32_t vehID, heaDing, spEed, laTi, loNg, eleVate; // stores for data

					std::vector<unsigned char>::iterator cnt = bytesFull.begin();

					while(cnt != bytesFull.end())
					{
							if(*cnt == 0x00 && (*(cnt+1) == 0x14 || *(cnt+1) == 0x12 || *(cnt+1) == 0x13))
							{
									break;
							}
							cnt++;
					}

					while(cnt != bytesFull.end())
					{
							bytes.push_back(*cnt);
							cnt++;

					}


					msgType = ntohs(*((uint16_t*)bytes.data()));
					msgVersion = ntohs(*((uint16_t*)&(bytes.data()[2])));
					id = ntohs(*((uint16_t*)&(bytes.data()[4])));
					dataLength = ntohs(*((uint16_t*)&(bytes.data()[6])));


					PLOG(logDEBUG1) << " Got message,  msgType: " << msgType
							<< ", msgVersion: " << msgVersion
							<< ", id: " << id
							<< ", dataLength: " << dataLength;

					if (dataLength > 0)
					{
						switch (msgType)
						{
						case ABBR_BSM:
							if (bytes.size() >= 32 && dataLength >= 24)
							{
								if (!simBSM && !simLoc) return;

								//extract data
								//vehicleId(4), heading*M(4), speed*K(4), (latitude+180)*M(4), (longitude+180)*M(4), elevation (4)
								BsmMessage *bsm = DecodeBsm(ntohl(*((uint32_t*)&(bytes.data()[8]))),
										ntohl(*((uint32_t*)&(bytes.data()[12]))),
										ntohl(*((uint32_t*)&(bytes.data()[16]))),
										ntohl(*((uint32_t*)&(bytes.data()[20]))),
										ntohl(*((uint32_t*)&(bytes.data()[24]))),
										ntohl(*((uint32_t*)&(bytes.data()[28]))),
										decodedBsm);
								//Print decoded BSM
								_bsmCount++;
								PLOG(logINFO) << "Received BSM Message count MessageReceiver: "<< _bsmCount;

								if (simLoc) {
									LocationMessage loc(::to_string(decodedBsm.get_TemporaryId()),
													    location::SignalQualityTypes::SimulationMode,
														"", ::to_string(msg.get_timestamp()),
														decodedBsm.get_Latitude(), decodedBsm.get_Longitude(),
														location::FixTypes::ThreeD, 0, 0.0,
														decodedBsm.get_Speed_mps(), decodedBsm.get_Heading());
									loc.set_Altitude(decodedBsm.get_Elevation_m());

									routeable_message rMsg;
									rMsg.initialize(loc);

									this->IncomingMessage(rMsg, 0, 0, msg.get_timestamp());
								}

								sendMsg = encode(encodedBsm, bsm);
								msgPSID = api::msgPSID::basicSafetyMessage_PSID;
								if (!simBSM) return;
							}
							break;
						case ABBR_SRM:
							if (bytes.size() >= 32 && dataLength >= 24)
							{
								if (!simSRM) return;

								//extract data
								//vehicleId(4), heading*M(4), speed*K(4), (latitude+180)*M(4), (longitude+180)*M(4), role (4)
								SrmMessage *srm = DecodeSrm(ntohl(*((uint32_t*)&(bytes.data()[8]))),
										ntohl(*((uint32_t*)&(bytes.data()[12]))),
										ntohl(*((uint32_t*)&(bytes.data()[16]))),
										ntohl(*((uint32_t*)&(bytes.data()[20]))),
										ntohl(*((uint32_t*)&(bytes.data()[24]))),
										ntohl(*((uint32_t*)&(bytes.data()[28]))));
								sendMsg = encode(encodedSrm, srm);
								msgPSID = api::msgPSID::signalRequestMessage_PSID;
							}
							break;
						default:
							{
								PLOG(logDEBUG) << "Unknown byte format.  Dropping message";
							}
							return;
						}
					}
				}
			}
			catch (exception &ex)
			{
				this->HandleException(ex, false);
				return;
			}
		}
	}


	// Make sure the timestamp matches the incoming source message

	sendMsg->set_timestamp(msg.get_timestamp());

	// Keep a count of each type of message received
	string name(sendMsg->get_subtype());
	if (!IsJ2735Message(*sendMsg))
	{
		// If not a J2735 message, save the type also
		name.insert(0, "/");
		name.insert(0, sendMsg->get_type());
	}

	if (!totalCount.count(name))
		totalCount[name] = 1;
	else
		totalCount[name]++;

	// Check to see if forward is disabled for this type
	bool fwd = true;
	GetConfigValue(name, fwd);

	if (fwd)
	{
		PLOG(logDEBUG) << "Routing " << name << " message.";

		if (routeDsrc)
		{
			sendMsg->set_flags(IvpMsgFlags_RouteDSRC);
			sendMsg->addDsrcMetadata(msgPSID);
		}
		else
		{

			sendMsg->set_flags(IvpMsgFlags_None);
		}
		this->OutgoingMessage(*sendMsg);
	}
}

void MessageReceiverPlugin::UpdateConfigSettings()
{
	lock_guard<mutex> lock(syncLock);

	// Atomic flags
	GetConfigValue("RouteJ2735", routeDsrc);
	GetConfigValue("EnableSimulatedBSM", simBSM);
	GetConfigValue("EnableSimulatedSRM", simSRM);
	GetConfigValue("EnableSimulatedLocation", simLoc);
	GetConfigValue<unsigned int>("EnableVerification", verState);
	GetConfigValue<string>("HSMurl",baseurl);
	GetConfigValue<string>("messageid",messageidstr);
	GetConfigValue("IP", ip);
	GetConfigValue("Port", port);
	_skippedSignVerifyErrorResponse = 0;
	SetStatus<uint>(Key_SkippedSignVerifyError, _skippedSignVerifyErrorResponse);

	getmessageid();

	std::string request="verifySig";
	url=baseurl+request;
	cfgChanged = true;
}

void MessageReceiverPlugin::OnConfigChanged(const char *key, const char *value)
{
	TmxMessageManager::OnConfigChanged(key, value);
	if (_plugin->state == IvpPluginState_registered)
		UpdateConfigSettings();
	// Reset bsm Count on config update
	PLOG(tmx::utils::logWARNING) << "Message count before resetting MessageReceiver: "<< _bsmCount;
	_bsmCount = 0;
}

void MessageReceiverPlugin::OnStateChange(IvpPluginState state)
{
	TmxMessageManager::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
	}
}

int MessageReceiverPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	byte_stream incoming(4000);
	std::unique_ptr<tmx::utils::UdpServer> server;

	byte_stream extractedpayload(4000);
	int mlen=0;

	while (_plugin->state != IvpPluginState_error)
	{
		// See if the server values are different
		if (cfgChanged) {
			lock_guard<mutex> lock(syncLock);

			if (port > 0 && (
					!server || (server->GetAddress() != ip || server->GetPort() != port)))
			{
				PLOG(logDEBUG) << "Creating UDPServer ip " << ip << " port " << port;
				server.reset(new UdpServer(ip, port));
			}

			cfgChanged = false;
		}

		try
		{
			int len = server ? server->TimedReceive((char *)incoming.data(), incoming.size(), 5) : 0;

			if (len > 0)
			{
				uint64_t time = Clock::GetMillisecondsSinceEpoch();

				totalBytes += len;
				int txlen=0;

				// @SONAR_STOP@
				// if verification enabled, access HSM

				if (verState == 1)
				{

					//  convert unit8_t vector to hex stream

    				stringstream ss;
    				ss << std::hex << std::setfill('0');
					uint16_t it=0;

    				for (uint16_t it=0; it <len; it++) {
        				ss << std::setw(2) << static_cast<unsigned>(incoming[it]);
    				}

					string msg = ss.str();

					//the incoming payload is hex encoded, convert this to base64
					std::string base64msg="";

					hex2base64(msg,base64msg);

					// use this string for verification with base64.

					std::string req = "\'{\"message\":\""+base64msg+"\"}\'";

					string cmd1="curl -X POST "+url+" -H \'Content-Type: application/json\' -d "+req;

					const char *cmd=cmd1.c_str();
					char buffer[2048];
					std::string result="";
					FILE* pipemsg= popen(cmd,"r");

					if (pipemsg == NULL ) throw std::runtime_error("popen() failed!");

					try{
						while (fgets(buffer, sizeof(buffer),pipemsg) != NULL)
						{
							result+=buffer;
						}
					} catch (std::exception const & ex) {

						pclose(pipemsg);
						SetStatus<uint>(Key_SkippedSignVerifyError, ++_skippedSignVerifyErrorResponse);
						PLOG(logERROR) << "Error parsing Messages: " << ex.what();
						continue;
					}
					PLOG(logDEBUG1) << "SCMS Contain response = " << result << std::endl;
					cJSON *root   = cJSON_Parse(result.c_str());
					cJSON *status = cJSON_GetObjectItem(root, "code");
					if ( status ) {
						cJSON *message = cJSON_GetObjectItem(root, "message");
						// IF status code exists this means the SCMS container returned an error response on attempting to sign
						// Set status will increment the count of message skipped due to signature error responses by one each
						// time this occurs. This count will be visible under the "State" tab of this plugin.
						SetStatus<uint>(Key_SkippedSignVerifyError, ++_skippedSignVerifyErrorResponse);
						PLOG(logERROR) << "Error response from SCMS container HTTP code " << status->valueint << "!\n" << message->valuestring << std::endl;
						continue;
					}
					cJSON *sd = cJSON_GetObjectItem(root, "signatureIsValid");


					int msgValid = sd->valueint;

					string extractedmsg="";
					bool foundId=false;

					if (msgValid == 1)
					{
						// look for a valid message type. 0012,0013,0014 etc. and count length of bytes to extract the message

						std::vector<string>::iterator itr=messageid.begin();
						int mlen;

						while(itr != messageid.end())
						{
							//look for the message header within the first 20 bytes.
							size_t idloc = msg.find(*itr);

							if(idloc != string::npos and idloc < IDCHECKLIMIT) // making sure the msgID lies within the first IDCHECKLIMIT Characters
							{
								// message id found
								if (msg[idloc+4] == '8') // if the length is longer than 256
								{
									string tmp = msg.substr(idloc+5,3);
									const char *c = tmp.c_str(); // take out next three nibble for length
									mlen = (strtol(c,nullptr,16)+4)*2; // 5 nibbles added for msgid and the extra 1 byte
									extractedmsg = msg.substr(idloc,mlen);

								}
								else
								{
									string tmp = msg.substr(idloc+4,2);
									const char *c = tmp.c_str(); // take out next three nibble for length
									mlen = (strtol(c,nullptr,16)+3)*2; // 5 nibbles added for msgid and the extra 1 byte
									extractedmsg = msg.substr(idloc,mlen);
								}

								foundId=true;

								int k=0;

								for (unsigned int i = 0; i < extractedmsg.length(); i += 2) {
									string bs = extractedmsg.substr(i, 2);
									uint8_t byte = (uint8_t) strtol(bs.c_str(), nullptr, 16);
									extractedpayload[k++]=byte;
									txlen++;

								}
								break; // can break out if already found a msg id
							}
							itr++;
						}

						if (foundId==false)
						{
							PLOG(logDEBUG) <<" Unable to find any valid msg ID in the incoming message. \n";
							continue;  //do not send the message out to v2x hub if msgid check fails
						}
					}
					else
					{
						PLOG(logDEBUG) <<" Unable to verify the incoming message: Message Verification Error and dropped \n";

						continue; // do not send the message out to v2x hub core if validation fails
					}

				}
				else {
				extractedpayload=incoming;
				txlen=len;
				}

				// @SONAR_START@

				// Support different encodings
				string enc;
				if (extractedpayload.size() > 0)
				{
					switch (extractedpayload[0]) {
					case 0x00:
						enc = api::ENCODING_ASN1_UPER_STRING;
						break;
					case 0x30:
						enc = api::ENCODING_ASN1_BER_STRING;
						break;
					case '{':
						enc = api::ENCODING_JSON_STRING;
						break;
					default:
						enc = api::ENCODING_BYTEARRAY_STRING;
						break;
					}
				}

				this->IncomingMessage(extractedpayload.data(), txlen, enc.empty() ? nullptr : enc.c_str(), 0, 0, time);

			}
			else if (len < 0)
			{
				if (errno != EAGAIN && errThrottle.Monitor(errno))
				{
					PLOG(logERROR) << "Could not receive from socket: " << strerror(errno);
				}
			}
		}
		catch (exception &ex)
		{
			this->HandleException(ex, false);
		}

		if (statThrottle.Monitor(1))
		{
			uint64_t b = totalBytes;
			auto msCount = Clock::GetMillisecondsSinceEpoch() - Clock::GetMillisecondsSinceEpoch(this->getStartTime());

			SetStatus("Total KBytes Received", b / 1024.0);

			for (auto iter = totalCount.begin(); iter != totalCount.end(); iter++) {
				string param("Avg ");
				param += iter->first;
				param += " Message Interval (ms)";

				uint32_t c = totalCount[iter->first];
				SetStatus(param.c_str(), c == 0 ? 0 : 1.0 * msCount / c);

				param = "Total ";
				param += iter->first;
				param += " Messages Received";
				SetStatus(param.c_str(), c);
			}
		}
	}

	return 0;
}

} /* namespace MessageRec*1000eiver */

int main(int argc, char *argv[])
{
	return run_plugin<MessageReceiver::MessageReceiverPlugin>("MessageReceiver", argc, argv);
}
