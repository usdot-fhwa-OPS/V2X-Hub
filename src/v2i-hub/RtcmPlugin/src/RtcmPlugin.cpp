/*
 * RtcmPlugin.cpp
 *
 *  Created on: Mar 22, 2018
 *      Author: gmb
 */

#include "RtcmPlugin.h"

#include <tmx/j2735_messages/RtcmMessage.hpp>

#include <arpa/inet.h>
#include <Clock.h>
#include <cstring>
#include <mutex>
#include <sys/socket.h>
#include <thread>
#include <System.h>

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace RtcmPlugin {

RtcmPlugin::RtcmPlugin(string name): PluginClient(name) {
	AddMessageFilter(TmxRtcmMessage::MessageType, "*");
	AddMessageFilter(IVPMSG_TYPE_NMEA, "GGA");

	SubscribeToMessages();
}

RtcmPlugin::~RtcmPlugin() { }

std::mutex _cfgLock;

void RtcmPlugin::UpdateConfigSettings() {
	GetConfigValue("Route RTCM", _routeRTCM);

	std::string ipaddress;
	int port;

	// Critical section
	std::lock_guard<mutex> lock(_cfgLock);
	GetConfigValue("Endpoint IP", ipaddress);
	GetConfigValue("Endpoint Port", port);
	GetConfigValue("Username", _username);
	GetConfigValue("Password", _password);
	GetConfigValue("Mountpoint", _mount);
	GetConfigValue("RTCM Version", _version);

	if (_version.empty())
		_version = TmxRtcmMessage::MessageSubType;

	PLOG(logDEBUG) << "Connecting to " << ipaddress << ":" << port << " for " << _username;

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ipaddress.c_str());
	addr.sin_port = htons(port);

	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (fd > 0) {
		if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
			PLOG(logDEBUG) << "Established socket to " << ipaddress << ":" << port;
			_socket = fd;
		} else {
			PLOG(logERROR) << "Connect failed: " << strerror(errno);
			_socket = 0;
		}
	} else {
		PLOG(logERROR) << "Cannot open socket: " << strerror(errno);
	}
}

void RtcmPlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);
	if (_plugin->state == IvpPluginState_registered)
		UpdateConfigSettings();
}

void RtcmPlugin::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
		UpdateConfigSettings();
}

void RtcmPlugin::OnMessageReceived(IvpMessage *ivpMsg) {
	// Intercept the NMEA messages
	if (ivpMsg && ::strcmp(IVPMSG_TYPE_NMEA, ivpMsg->type) == 0) {
		lock_guard<mutex> lock(_cfgLock);
		if (ivpMsg->payload)
			this->_nmea = ivpMsg->payload->valuestring;

		PLOG(logDEBUG2) << "Received NMEA message " << _nmea;
	} else if (ivpMsg && ::strcmp(TmxRtcmMessage::MessageType, ivpMsg->type) == 0) {
		// Extract the RTCM messages
		routeable_message rMsg(ivpMsg);
		rMsg.get_payload_str();
		TmxRtcmEncodedMessage encodedMsg(rMsg);
		for (auto iter = encodedMsg.begin(); iter != encodedMsg.end(); iter++) {
			if (*iter) BroadcastRTCMMessage(**iter, encodedMsg);
		}
	}
}

void RtcmPlugin::BroadcastRTCMMessage(TmxRtcmMessage &msg, routeable_message &routeableMsg) {
	PLOG(logDEBUG) << "Received " << routeableMsg.get_type() << " message version "
			<< routeableMsg.get_subtype() << ": " << msg;

	try {
		RtcmMessage rtcm = msg.get_RtcmMessage();
		RtcmEncodedMessage encMsg;
		encMsg.initialize(rtcm, "", 0, IvpMsgFlags_RouteDSRC);
		encMsg.addDsrcMetadata(0x8000);

		const routeable_message &rMsg = encMsg;
		this->BroadcastMessage(rMsg);
	} catch (TmxException &ex) {
		this->HandleException(ex, false);
	}
}

int RtcmPlugin::Main() {
	PLOG(logINFO) << "Plugin started";

	int oldSock = 0;

	static byte_stream inBytes(4000);

	while (_plugin->state != IvpPluginState_error) {
		uint64_t clockStart = Clock::GetMillisecondsSinceEpoch();

		string user;
		string pass;
		string mount;
		string nmea;
		string ver;
		int sock;

		int ntripVer = 0;
		int httpVer = 0;

		// Critical section
		{
			std::lock_guard<mutex> lock(_cfgLock);
			user = _username;
			pass = _password;
			mount = _mount;
			sock = _socket;
			nmea = _nmea;
			ver = _version;
		}

		// Make sure we have a connection and a NMEA string
		if (sock > 0 && !nmea.empty()) {
			// See if this is a new socket
			if (oldSock != sock) {
				_connected = false;

				if (oldSock > 0) close(oldSock);
			}

			if (!_connected) {
				// Speak NTRIP 2.0 over HTTP 1.1
				std::stringstream header;
				header << "GET /" << mount <<" HTTP/1.1\r\n";
				header << "User-Agent: TMX NTRIP " << this->_name << "/20\r\n";
				header << "Accept: */*\r\n";
				header << "Ntrip-Version: Ntrip/2.0\r\n";
				header << "Ntrip-GGA: " << nmea << "\r\n";
				header << "Connection: close\r\n";

				std::string userpass = user + std::string(":") + pass;

				header << "Authorization: Basic "
					   << Base64::Encode((unsigned char *)userpass.c_str(), userpass.length())
					   << "\r\n\r\n";

				PLOG(logDEBUG) << "Writing header: \n" << header.str();
				if (::send(sock, header.str().c_str(), header.str().length(), 0) < 0) {
					PLOG(logDEBUG) << "Unable to write bytes to socket: " << strerror(errno);

					close(sock);
					UpdateConfigSettings();

					sleep(1);
					continue;
				}

				oldSock = sock;
			}

			int recv = ::recv(sock, inBytes.data(), inBytes.size(), 0);
			if (recv < 0) {
				PLOG(logDEBUG) << "Unable to receive bytes from socket: " << strerror(errno);
				close(sock);
				UpdateConfigSettings();

				sleep(1);
				continue;
			}

			PLOG(logDEBUG) << "Received " << recv << " bytes from NTRIP caster.";

			vector<string> response;
			if (recv > 0) {
				if (!_connected) {
					PLOG(logDEBUG1) << "Recevied bytes:" << inBytes << endl;

					// Read incoming message by line
					istringstream inStream(string((const char *)inBytes.data(), recv));
					string line;
					while (getline(inStream, line)) {
						if (line[line.length()-1] == '\r') line.erase(line.length()-1);
						PLOG(logDEBUG1) << line;
						PLOG(logDEBUG1) << System::ExecCommand("echo '" + line + "' | gpsdecode");
						response.push_back(line);
					}

					// Check to see if the connection to the castor was successful
					if (response.size() > 1) {
						if (::strncmp("200 OK", response[0].substr(response[0].length() - 6).c_str(), 6) == 0) {
							_connected = true;
							SetStatus("Connected", true);

							if (::strncmp("ICY", response[0].c_str(), 3) == 0) {
								// Ntrip 1.0 response.  Need to send NMEA string
								ntripVer = 1;
								httpVer = 0;

								PLOG(logDEBUG) << "Connected to NTRIP 1.0 server.  Sending NMEA string";

								::send(sock, _nmea.c_str(), _nmea.length(), 0);

								sleep(1);
								continue;
							} else {
								PLOG(logDEBUG) << "Connected to NTRIP 2.0 server.";

								ntripVer = 2;
								if (::strncmp("HTTP/1.0", response[0].c_str(), 8) == 0)
									httpVer = 0;
								else if (::strncmp("HTTP/1.1", response[0].c_str(), 8) == 0)
									httpVer = 1;
							}
						} else {
							_connected = false;
							PLOG(logERROR) << "Invalid response: " << response[0];
						}
					}
				} else {
					// Convert the bytes to a set of new message
					tmx::byte_stream bytes(inBytes.begin(), inBytes.begin() + recv);

					PLOG(logDEBUG1) << "RTCM Message Bytes:" << bytes;

					TmxRtcmEncodedMessage encodedMsg;
					encodedMsg.set_subtype(ver);
					encodedMsg.set_payload_bytes(bytes);

					PLOG(logDEBUG2) << "Trying to decode " << encodedMsg;

					for (auto iter = encodedMsg.begin(); iter != encodedMsg.end(); iter++) {
						if (*iter) this->BroadcastRTCMMessage(**iter, encodedMsg);
					}

					if (_routeRTCM)
						this->BroadcastMessage(static_cast<routeable_message &>(encodedMsg));
				}
			} else {
				_connected = false;
				SetStatus("Connected", false);
			}
		}
	}

	return(0);
}

} /* namespace RtcmPlugin */

int main(int argc, char *argv[]) {
	return tmx::utils::run_plugin<RtcmPlugin::RtcmPlugin>("RTCM", argc, argv);
}
