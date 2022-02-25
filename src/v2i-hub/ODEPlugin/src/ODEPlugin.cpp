/*
 *  ODEPlugin.cpp
 *
 *  Created on: May 10, 2016
 *      Author: ivp
 */

#include "ODEPlugin.h"

#include <mutex>
#include <stdexcept>
#include <thread>
#include <tmx/apimessages/TmxEventLog.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>

using namespace std;
using namespace boost::asio;
using namespace boost::chrono;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

// BSMs may be 10 times a second, so only send errors at most every 2 minutes
#define ERROR_WAIT_MIN 2
#define STATUS_WAIT_SEC 2

namespace ODE {

mutex syncLock;

ODEPlugin::ODEPlugin(std::string name): PluginClient(name)
{
	AddMessageFilter<BsmMessage>(this, &ODEPlugin::handleBsm);
	SubscribeToMessages();
}

ODEPlugin::~ODEPlugin()
{
	delete client;
}

void ODEPlugin::sendBytes(const byte_stream &bytes)
{
	static uint64_t totalBytes = 0;
	static uint32_t totalCount = 0;

	size_t sentBytes;

PLOG(logDEBUG) <<"sendBytes";

	try
	{
		syncLock.lock();
		if (client && (client->GetAddress() != ip || client->GetPort() != port))
			delete client;

		if (!client && port > 0)
			client = new UdpClient(ip, port);
		syncLock.unlock();

		if (client)
		{
			sentBytes = client->Send((void *)bytes.data(), bytes.size());

			PLOG(logDEBUG) << "Sent " << sentBytes << " bytes to " <<
					client->GetAddress() << ":" << client->GetPort() << ".";

			totalCount ++;
			totalBytes += sentBytes;
		}
		else
		{
			PLOG(logDEBUG)<<"client null";
		}
	}
	catch (exception &ex)
	{
		if (errno <= 0 || _errThrottle.Monitor(errno))
		{
			TmxEventLogMessage elm(ex, "Unable to send message: ");
			PLOG(logERROR) << elm.get_description();
			BroadcastMessage(elm);
		}
	}

	if (_statThrottle.Monitor(0))
	{
		SetStatus("Total BSM Messages Sent", totalCount);
		SetStatus("Total Bytes Sent", totalBytes);
		SetStatus("Bytes Sent per Second", 1.0 * totalBytes / STATUS_WAIT_SEC);
	}
}

void ODEPlugin::recvBytes(const byte_stream &bytes)
{
	static uint64_t totalBytes = 0;
	static uint32_t totalCount = 0;

	totalBytes += bytes.size();

	static J2735MessageFactory factory;
	routeable_message *msg = factory.NewMessage(const_cast<byte_stream &>(bytes));

	if (msg)
	{
		PLOG(logDEBUG) << *msg;

		// Only forward TIMs
		if (msg->get_subtype().compare(TimMessage::get_messageType()) == 0)
		{
			PLOG(logDEBUG) << "Received TIM.  Routing.";
			msg->set_flags(IvpMsgFlags_RouteDSRC);
			BroadcastMessage(*msg);

			totalCount ++;
		}
	}

	if (_statThrottle.Monitor(1))
	{
		SetStatus("Total TIM Messages Received", totalCount);
		SetStatus("Total Bytes Received", totalBytes);
		SetStatus("Bytes Received per Second", 1.0 * totalBytes / STATUS_WAIT_SEC);
	}
}

void ODEPlugin::handleBsm(BsmMessage &message, routeable_message &routeableMsg)
{
	try
	{
	PLOG(logDEBUG) << "Received BSM.  Forwarding.";
	sendBytes(routeableMsg.get_payload_bytes());
	}
	catch (exception &ex)
	{
		if (errno <= 0 || _errThrottle.Monitor(errno))
		{
			TmxEventLogMessage elm(ex, "Unable to send message: ");
			PLOG(logERROR) << elm.get_description();
			BroadcastMessage(elm);
		}
	}
}

void ODEPlugin::UpdateConfigSettings()
{
	lock_guard<mutex> lock(syncLock);

	GetConfigValue("ODEIP", ip);
	GetConfigValue("ODEPort", port);
}

void ODEPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void ODEPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
	}
}

int ODEPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	byte_stream incoming(4000);

	while (_plugin->state != IvpPluginState_error)
	{
		// Wait a bit longer to try again
		this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}

} /* namespace ODEPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<ODE::ODEPlugin>("ODEPlugin", argc, argv);
}
