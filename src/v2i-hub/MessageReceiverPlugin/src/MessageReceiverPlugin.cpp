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
			int len = server ? server->TimedReceive((char *)incoming.data(), incoming.size(), 5) : 0;

			if (len > 0)
			{
				uint64_t time = Clock::GetMillisecondsSinceEpoch();

				totalBytes += len;
				int txlen=0;

				if (fullSPDUMode){
					tmx::byte_stream payload;
					std::shared_ptr<Ieee1609Dot2Data_t> decoded;
					uint psid = 0;
					bool psidSet = false;
					try {
						decoded = decodeSpdu(incoming.data(), len);
					}
					catch (const tmx::TmxException &ex) {
						PLOG(logERROR) << "Error decoding SPDU: " << ex.what();
						//Broadcast TmxEventLog message
						tmx::messages::TmxEventLogMessage eventLog(ex, "Error decoding SPDU: ", false);
						BroadcastMessage(eventLog);
						_failedSPDU++;
						SetStatus<uint>(Key_FailedSPDU, _failedSPDU);
					}
					if (!unwrapSpdu(decoded.get(),payload, psid, psidSet, 0)) {
						PLOG(logERROR) << "Error unwrapping SPDU";
						//Broadcast TmxEventLog message
						tmx::messages::TmxEventLogMessage eventLog("Error unwrapping SPDU");
						eventLog.set_level(IvpLogLevel_warn);
						BroadcastMessage(eventLog);
						_failedSPDU++;
						SetStatus<uint>(Key_FailedSPDU, _failedSPDU);
					}
					//Create RawSpdu message and send to TMX Core
					auto spduMsg = buildRawSpdu(psid, payload, time, _uuidGen());
					_processedSPDU++;
					SetStatus<uint>(Key_ProcessedSPDU, _processedSPDU);
					tmx::routeable_message rMsg;
					rMsg.initialize<tmx::messages::RawSpdu>(spduMsg);
					this->OutgoingMessage(rMsg);

				}
				
				extractedpayload=incoming;
				txlen=len;

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
