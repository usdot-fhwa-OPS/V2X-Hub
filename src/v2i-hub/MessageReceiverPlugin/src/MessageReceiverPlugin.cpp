/*
 * BsmReceiver.cpp
 *
 *  Created on: May 10, 2016
 *      Author: ivp
 */

#include "MessageReceiverPlugin.h"
#include "Utils.h"


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
	ip = tmx::utils::environment::get_local_ip();
	GetConfigValue("Port", port);
	GetConfigValue("FullSPDUMode", fullSPDUMode);
	SetStatus<uint>(Key_FailedSPDU, _failedSPDU);
	SetStatus<uint>(Key_ProcessedSPDU, _processedSPDU);

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

inline const char* get_encoding(byte_t b){
	switch (b) {
		case 0x00:
			return api::ENCODING_ASN1_UPER_STRING;
		case 0x30:
			return api::ENCODING_ASN1_BER_STRING;
		case '{':
			return api::ENCODING_JSON_STRING;
		default:
			return api::ENCODING_BYTEARRAY_STRING;
	}
}

int MessageReceiverPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	byte_stream incoming(4000);
	std::unique_ptr<tmx::utils::UdpServer> server;

	while (_plugin->state != IvpPluginState_error)
	{
		// See if the server values are different
		if (cfgChanged) {
			std::scoped_lock lock(syncLock);

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
			int nBytesReceived = server ? server->TimedReceive((char *)incoming.data(), incoming.size(), 5) : 0;
			if (nBytesReceived > 0){
				uint64_t time = Clock::GetMillisecondsSinceEpoch();
				byte_stream fullPayload;
				byte_stream msgPayload;

				totalBytes += nBytesReceived;
				fullPayload.assign(incoming.begin(), incoming.begin() + nBytesReceived);

				// Support different encodings
				string enc;

				if(!fullSPDUMode){
					// if not in full SPDU mode, just send the raw bytes corresponding 
					// to raw message payload bytes (unsecured) to TMX Core
					enc = get_encoding(fullPayload[0]);		
					this->IncomingMessage(fullPayload.data(), nBytesReceived, enc.c_str(), 0, 0, time);
				}
				else {
					// extract the unsecured payload from the SPDU
					std::shared_ptr<Ieee1609Dot2Data_t> decoded;
					uint psid = 0;
					bool psidSet = false;
					try {
						decoded = decodeSpdu(fullPayload, nBytesReceived);
					}
					catch (const tmx::TmxException &ex) {
						PLOG(logERROR) << "Error decoding SPDU: " << ex.what();
						//Broadcast TmxEventLog message
						tmx::messages::TmxEventLogMessage eventLog(ex, "Error decoding SPDU: ", false);
						BroadcastMessage(eventLog);
						_failedSPDU++;
						SetStatus<uint>(Key_FailedSPDU, _failedSPDU);
					}
					if (!unwrapSpdu(decoded.get(), msgPayload, psid, psidSet, 0) || msgPayload.empty()) {
						// if unwrap fails, no message is sent to TMX core
						PLOG(logERROR) << "Error unwrapping SPDU";
						//Broadcast TmxEventLog message
						tmx::messages::TmxEventLogMessage eventLog("Error unwrapping SPDU");
						eventLog.set_level(IvpLogLevel_warn);
						BroadcastMessage(eventLog);
						_failedSPDU++;
						SetStatus<uint>(Key_FailedSPDU, _failedSPDU);
					}
					else {
						// put the extracted msg payload on the TMX core
						enc = get_encoding(msgPayload[0]);  // note the encoding is retrieved from the unsecured payload
						this->IncomingMessage(msgPayload.data(), msgPayload.size(), enc.c_str(), 0, 0, time); 

						//Create RawSpdu message and send to TMX Core
						auto spduMsg = buildRawSpdu(psid, fullPayload, msgPayload, time, _uuidGen());
						_processedSPDU++;
						SetStatus<uint>(Key_ProcessedSPDU, _processedSPDU);
						tmx::routeable_message rMsg;
						rMsg.initialize<tmx::messages::RawSpdu>(spduMsg);
						this->OutgoingMessage(rMsg);
					}
				}
			}
			else if (nBytesReceived < 0)
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
