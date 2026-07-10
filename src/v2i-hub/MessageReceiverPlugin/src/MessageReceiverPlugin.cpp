/*
 * BsmReceiver.cpp
 *
 *  Created on: May 10, 2016
 *      Author: ivp
 */

#include "MessageReceiverPlugin.h"


#define IDCHECKLIMIT 60
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

MessageReceiverPlugin::MessageReceiverPlugin(const std::string &name): TmxMessageManager(name)
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





void MessageReceiverPlugin::OnMessageReceived(routeable_message &msg)
{
	// Keep a count of each type of message received
	string name(msg.get_subtype());
	if (!IsJ2735Message(msg))
	{
		// If not a J2735 message, save the type also
		name.insert(0, "/");
		name.insert(0, msg.get_type());
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
			msg.set_flags(IvpMsgFlags_RouteDSRC);
		}
		else
		{

			msg.set_flags(IvpMsgFlags_None);
		}
		this->OutgoingMessage(msg);
	}
}

void MessageReceiverPlugin::UpdateConfigSettings()
{
	lock_guard<mutex> lock(syncLock);

	// Atomic flags
	GetConfigValue("RouteJ2735", routeDsrc);
	GetConfigValue<unsigned int>("EnableVerification", verState);
	GetConfigValue<string>("HSMurl",baseurl);
	GetConfigValue<string>("messageid",messageidstr);
	ip = tmx::utils::environment::get_local_ip();
	GetConfigValue("Port", port);
	GetConfigValue("FullSPDUMode", fullSPDUMode);
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
}

void MessageReceiverPlugin::OnStateChange(IvpPluginState state)
{
	TmxMessageManager::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
	}
}

bool MessageReceiverPlugin::unwrapSpdu(const Ieee1609Dot2Data_t* d, std::vector<uint8_t>& payloadOut, uint32_t& psidOut, bool& psidSet)
{
	//Implementation notes for this function: psidOut and psidSet are not overwritten unless a PSID exists within the message.
	//psidOut and psidSet might be modified even if the overall return is false and the unwrap operation failed.
    if (d->protocolVersion != 3)      return false;

    switch (d->content->present) {
        case Ieee1609Dot2Content_PR_unsecuredData: {
            const Opaque_t& op = d->content->choice.unsecuredData;
            if (!op.buf || op.size <= 0) return false;
            payloadOut.assign(op.buf, op.buf + op.size);
            return true;
        }
        case Ieee1609Dot2Content_PR_signedData: {              // recurse
            const SignedData_t* sd = d->content->choice.signedData;
            if (!sd) return false;
            if (!psidSet) {
                psidOut = static_cast<uint32_t>(sd->tbsData->headerInfo.psid);
                psidSet = true;
            }
            const Ieee1609Dot2Data_t* next = sd->tbsData->payload->data;  // OPTIONAL
            if (!next) {
				return false;
			}
            return unwrapSpdu(next, payloadOut, psidOut, psidSet);
        }

        default:
            return false;                // unknown CHOICE
    }
}

int64_t MessageReceiverPlugin::identifyJ2735Type(const std::vector<uint8_t>& payload)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    size_t n = std::min<size_t>(payload.size(), IDCHECKLIMIT / 2);
    for (size_t i = 0; i < n; ++i)
        ss << std::setw(2) << static_cast<unsigned>(payload[i]);

    size_t idloc; int mlen; long msgId;
    return findMessageId(ss.str(), idloc, mlen, msgId) ? msgId : -1;
}

bool MessageReceiverPlugin::findMessageId(const std::string& hex, size_t& idloc,
                                          int& hexLen, long& dsrcMsgId)
{
    for (const auto& id : messageid)
    {
        size_t loc = hex.find(id);
        if (loc != std::string::npos && loc < IDCHECKLIMIT)
        {
            int mlen;
            if (hex[loc + 4] == '8')                       // length > 256, long form
            {
                std::string tmp = hex.substr(loc + 5, 3);
                mlen = (strtol(tmp.c_str(), nullptr, 16) + 4) * 2;
            }
            else                                           // short form
            {
                std::string tmp = hex.substr(loc + 4, 2);
                mlen = (strtol(tmp.c_str(), nullptr, 16) + 3) * 2;
            }
            idloc    = loc;
            hexLen   = mlen;
            dsrcMsgId = strtol(id.c_str(), nullptr, 16);
            return true;
        }
    }
    return false;
}

bool MessageReceiverPlugin::buildRawSpduMessage(const byte_stream& incoming, int len, uint64_t rxTime, tmx::messages::RawSpdu& out, std::vector<uint8_t>& payloadOut)
{
	PLOG(logDEBUG) << "Entering buildRawSpduMessage";
    // 1. Decode the SPDU
    Ieee1609Dot2Data_t* decodedPtr = nullptr;
	asn_dec_rval_t rv = oer_decode(nullptr, &asn_DEF_Ieee1609Dot2Data,
                                   (void**)&decodedPtr, incoming.data(), len);
	
	// asn_dec_rval_t rv = uper_decode_complete(nullptr, &asn_DEF_Ieee1609Dot2Data,
    //                                (void**)&decodedPtr, incoming.data(), len);

	auto del = [](Ieee1609Dot2Data_t* p){ ASN_STRUCT_FREE(asn_DEF_Ieee1609Dot2Data, p); };
	std::unique_ptr<Ieee1609Dot2Data_t, decltype(del)> decoded(decodedPtr, del);								   
	
	if (rv.code != RC_OK || !decoded) {
        PLOG(logDEBUG) << "SPDU uper decode failed (rc=" << rv.code << ")";
        return false;
    }

    // 2. Walk to the inner J2735 payload
    uint32_t psid = 0;
	bool psidSet = false;
    if (!unwrapSpdu(decodedPtr, payloadOut, psid, psidSet)) {
        PLOG(logDEBUG) << "No unsecured J2735 payload found in SPDU";
        return false;
    }

    // 3. Populate tmx message
	out.set_spdu_data(tmx::byte_stream(incoming.data(), incoming.data() + len));
    boost::uuids::uuid u = _uuidGen();
	out.set_uuid(tmx::byte_stream(u.begin(), u.end())); 
    out.set_timestamp_ms(rxTime);
    out.set_psid(psid);
	int _dsrcChannel = 0; // TODO: This needs to be updated
    out.set_channel(_dsrcChannel);
    out.set_msg_type(std::to_string(identifyJ2735Type(payloadOut))); // This should use a mapping from Message id to string name instead
	
	tmx::byte_stream sd = out.get_spdu_data();
    tmx::byte_stream id = out.get_uuid();
    std::ostringstream so, uo;
    so << std::hex << std::setfill('0');
    for (uint8_t b : sd) so << std::setw(2) << (unsigned)b;
    uo << std::hex << std::setfill('0');
    for (uint8_t b : id) uo << std::setw(2) << (unsigned)b;

	PLOG(logDEBUG) << "RawSpdu.spdu_data  (" << sd.size() << " B): " << so.str();
    PLOG(logDEBUG) << "RawSpdu.uuid       (" << id.size() << " B): " << uo.str();
    PLOG(logDEBUG) << "RawSpdu.timestamp_ms = " << out.get_timestamp_ms();
	string name(payloadOut.get_subtype());
	PLOG(logDEBUG) << "RawSpdu.msg_type     = " << name;
    PLOG(logDEBUG) << "RawSpdu.psid         = " << out.get_psid();
    PLOG(logDEBUG) << "RawSpdu.channel      = " << out.get_channel();
    PLOG(logDEBUG) << "RawSpdu.msg_type     = " << out.get_msg_type();

    return true;
}


int MessageReceiverPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	byte_stream incoming(4000);
	std::unique_ptr<tmx::utils::UdpServer> server;

	byte_stream extractedpayload(4000);

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
			PLOG(logDEBUG) << "Received message of size "<< len;
			if (len > 0)
			{
				PLOG(logDEBUG) << "Received Message";
				uint64_t time = Clock::GetMillisecondsSinceEpoch();

				totalBytes += len;
				int txlen=0;

				// @SONAR_STOP@
				// if verification enabled, access HSM
				if (fullSPDUMode){
					RawSpdu spduMsg;
					std::vector<uint8_t> payload;
					if (!buildRawSpduMessage(incoming, len, time, spduMsg, payload))
					{
						PLOG(logERROR) << "Error parsing SPDU Messages";
						continue;
					}
					
					

					std::copy(payload.begin(), payload.end(), extractedpayload.begin());
					txlen = payload.size();
    
					std::ostringstream oss;
					oss << std::hex << std::setfill('0');
					for (uint8_t b : payload) oss << std::setw(2) << (unsigned)b;
					PLOG(logDEBUG) << "SPDU inner payload (" << payload.size() << " bytes): " << oss.str();
					
					std::string result = oss.str();
					PLOG(logDEBUG) << "Received SPDU Message with payload: "<< result;

					tmx::routeable_message rMsg;
					rMsg.initialize<tmx::messages::RawSpdu>(spduMsg);
					this->OutgoingMessage(rMsg, true);

				}

				if (verState == 1)
				{

					//  convert unit8_t vector to hex stream

    				stringstream ss;
    				ss << std::hex << std::setfill('0');

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

						size_t idloc;
						int mlen;
						long msgId;

						if (!findMessageId(msg, idloc, mlen, msgId))
						{
							PLOG(logDEBUG) <<" Unable to find any valid msg ID in the incoming message. \n";
							continue;  //do not send the message out to v2x hub if msgid check fails
						}

						string extractedmsg = msg.substr(idloc, mlen);

						int k=0;

						for (unsigned int i = 0; i < extractedmsg.length(); i += 2) {
							string bs = extractedmsg.substr(i, 2);
							uint8_t byte = (uint8_t) strtol(bs.c_str(), nullptr, 16);
							extractedpayload[k++]=byte;
							txlen++;
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
