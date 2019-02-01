/*
 * PluginKeepAlive.cpp
 *
 *  Created on: Sep 29, 2016
 *      Author: gmb
 */

#include "PluginKeepAlive.h"
#include "PluginClient.h"

using namespace std;
using namespace tmx::messages;

namespace tmx {
namespace utils {

PluginKeepAlive::PluginKeepAlive(PluginClient *plugin): _client(plugin), _isRunning(false)
{
	this->set_Frequency(chrono::milliseconds(DEFAULT_KEEPALIVE_FREQUENCY));
	if (_client)
		this->_name = _client->GetName();

	if (!_isRunning)
	{
		PLOG(logDEBUG) << "Starting keep alive thread for " << this->_name;
		_isRunning = true;
		_thread = new std::thread(&PluginKeepAlive::runKeepAlive, this);
	}
}

PluginKeepAlive::~PluginKeepAlive()
{
	if (_isRunning)
	{
		PLOG(logDEBUG) << "Stopping keep alive thread for " << this->_name;
		_isRunning = false;
		if (_thread) _thread->join();
	}
}

void PluginKeepAlive::touch()
{
	static routeable_message timeKeeper;

	PLOG(logDEBUG3) << "Touching keep alive for " << this->_name;

	lock_guard<mutex> lock(timeLock);
	timeKeeper.refresh_timestamp();
	msg.set_lastTimestamp(msg.get_currentTimestamp());
	msg.set_currentTimestamp(timeKeeper.get_timestamp());
	if (_client)
		monitor.Touch(this->_name);
}

void PluginKeepAlive::set_Frequency(chrono::milliseconds frequency)
{
	if (frequency > chrono::milliseconds(0))
	{
		lock_guard<mutex> lock(this->freqLock);
		monitor.set_Frequency(frequency);
	}
}

chrono::milliseconds PluginKeepAlive::get_Frequency()
{
	lock_guard<mutex> lock(this->freqLock);
	return monitor.get_Frequency();
}

void PluginKeepAlive::check()
{
	// Send a message
	if (_client && _client->IsPluginState(IvpPluginState_registered))
	{
		PLOG(logDEBUG2) << "In check for " << this->_name;

		if (monitor.Monitor(this->_name))
		{
			PLOG(logDEBUG1) << "No message sent from " << this->_name << " in last " << get_Frequency().count() << " ms.";
			touch();
			_client->BroadcastMessage(msg);
		}
	}
}

void PluginKeepAlive::runKeepAlive()
{
	while (_isRunning)
	{
		// Assume minute boundaries at minimum. Wake up at least every 10 seconds.
		this_thread::sleep_for(chrono::seconds(10));

		check();
	}
}

} /* namespace utils */
} /* namespace tmx */
