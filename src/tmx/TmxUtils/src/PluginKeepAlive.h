/*
 * PluginKeepAlive.h
 *
 *  Created on: Sep 29, 2016
 *      Author: gmb
 */

#ifndef SRC_PLUGINKEEPALIVE_H_
#define SRC_PLUGINKEEPALIVE_H_

#define USE_STD_CHRONO
#include "FrequencyThrottle.h"
#undef USE_STD_CHRONO
#include <MessageTypes.h>
#include <mutex>
#include <thread>
#include <tmx/messages/message.hpp>

#include "ThreadTimer.h"

// 7.8 minute intervals by default to support a 5000000 millisecond setting for the GUI
// 8.333 - 7.8 = 32 second window / 10 second wake ups = 3 wake up periods
#define DEFAULT_KEEPALIVE_FREQUENCY 7800 * 60

namespace tmx {
namespace messages {

class KeepAliveMessage: public tmx::message
{
public:
	KeepAliveMessage(): tmx::message() {}
	~KeepAliveMessage() {}

	static constexpr const char *MessageType = tmx::messages::MSGTYPE_SYSTEM_STRING;
	static constexpr const char *MessageSubType = "KeepAlive";

	std_attribute(this->msg, uint64_t, currentTimestamp, 0, )
	std_attribute(this->msg, uint64_t, lastTimestamp, 0, )
};

} /* namespace messages */

namespace utils {

class PluginClient;

class PluginKeepAlive
{
public:
	PluginKeepAlive(PluginClient *);
	virtual ~PluginKeepAlive();

	std::chrono::milliseconds get_Frequency();
	void set_Frequency(std::chrono::milliseconds);
	void touch();
private:
	void check();
	PluginClient *_client;
	std::string _name;
	tmx::messages::KeepAliveMessage msg;
	FrequencyThrottle<std::string> monitor;

	// Thread stuff
	void runKeepAlive();
	std::mutex freqLock;
	std::mutex timeLock;
	std::thread *_thread = nullptr;
	std::atomic<bool> _isRunning;
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_PLUGINKEEPALIVE_H_ */
