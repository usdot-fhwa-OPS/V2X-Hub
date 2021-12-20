/*
 * PluginLog.cpp
 *
 *  Created on: Oct 20, 2016
 *      Author: ivp
 */

#include "PluginLog.h"

#include <atomic>
#include <sstream>
#include <syslog.h>

#ifndef NO_EVENTLOG_UDP
#define EVENTLOG_SEC_SLEEP 2
#include "LockFreeThread.h"
#include "UdpClient.h"
#endif

namespace tmx {
namespace utils {

FILE* &Output2FILE::Stream()
{
    static FILE* pStream = stdout;
    return pStream;
}

bool &Output2FILE::Enable()
{
	static bool enable = true;
	return enable;
}

void Output2FILE::Output(LogMessage &msg)
{
	if (!Enable())
		return;

    FILE* pStream = Stream();
    if (!pStream)
        return;

    std::stringstream ss;
	_logtime(ss, msg.timestamp);
	_logsource(ss, msg.file, msg.line);
	_loglevel(ss, msg.level);
	ss << msg.log << std::endl;

    fprintf(pStream, "%s", ss.str().c_str());
    fflush(pStream);
}

int Output2Syslog::ToSyslogLevel(LogLevel lvl)
{
	switch (lvl)
	{
	case logDEBUG:
	case logDEBUG1:
	case logDEBUG2:
	case logDEBUG3:
	case logDEBUG4:
		return LOG_DEBUG;
	case logINFO:
		return LOG_INFO;
	case logWARNING:
		return LOG_WARNING;
	case logERROR:
		return LOG_ERR;
	default:
		return LOG_NOTICE;
	}
}

bool &Output2Syslog::Enable()
{
	static bool enable = false;
	return enable;
}

void Output2Syslog::Output(LogMessage &msg)
{
	if (!Enable())
		return;

	int level = ToSyslogLevel(msg.level);
	openlog(msg.component.empty() ? nullptr : msg.component.c_str(), LOG_NDELAY, LOG_USER);
	syslog(level, "%s", msg.log.c_str());
}

#ifndef NO_EVENTLOG_UDP
class TmxEventLogThread: public LockFreeThread<LogMessage>
{
public:
	void doWork(LogMessage &message);
	void idle();
};

static TmxEventLogThread _eventThread;

IvpLogLevel Output2Eventlog::ToEventLogLevel(LogLevel level)
{
	switch(level)
	{
	case logDEBUG:
	case logDEBUG1:
	case logDEBUG2:
	case logDEBUG3:
	case logDEBUG4:
		return IvpLogLevel_debug;
	case logINFO:
		return IvpLogLevel_info;
	case logWARNING:
		return IvpLogLevel_warn;
	case logERROR:
		return IvpLogLevel_error;
	}
	return IvpLogLevel_info;
}

LogLevel Output2Eventlog::FromEventLogLevel(IvpLogLevel level)
{
	switch (level)
	{
	case IvpLogLevel_debug:
		return logDEBUG;
	case IvpLogLevel_info:
		return logINFO;
	case IvpLogLevel_warn:
		return logWARNING;
	case IvpLogLevel_error:
	case IvpLogLevel_fatal:
		return logERROR;
	}
	return logINFO;
}

bool &Output2Eventlog::Enable()
{
	static bool enable = true;
	return enable;
}

void Output2Eventlog::Output(LogMessage &msg)
{
	// For backward compatibility, always log with file log first
	Output2FILE fileLog;
	fileLog.Output(msg);

	if (!Enable())
		return;

	static bool started = false;

	if (!started)
	{
		_eventThread.start();
		started = true;
	}

	_eventThread.push(msg);
}

void TmxEventLogThread::doWork(LogMessage &message)
{
	// Create a TMX message to send to the logger
	if (message.level <= logDEBUG)
	{
		tmx::messages::TmxEventLogMessage eventLog;
		eventLog.set_level(Output2Eventlog::ToEventLogLevel(message.level));
		eventLog.set_description(message.log);

		tmx::routeable_message tmxMessage;

		std::string plugin = message.component;
		if (plugin.empty())
		{
			plugin = message.file;
			size_t i = plugin.find_last_of('/');
			if (i < plugin.size())
				plugin = plugin.substr(i + 1);

			if (plugin.compare(plugin.size() - 4, 4, ".cpp") == 0)
				plugin = plugin.substr(0, plugin.size() - 4);
			else if (plugin.compare(plugin.size() - 2, 2, ".h") == 0 ||
					 plugin.compare(plugin.size() - 2, 2, ".c") == 0)
				plugin = plugin.substr(0, plugin.size() - 2);
		}

		tmxMessage.initialize(eventLog, plugin);
		tmxMessage.set_timestamp(message.timestamp);

		static UdpClient client("localhost", DEFAULT_EVENTLOG_UDP_PORT);
		std::string msgText(tmxMessage.to_string());
		client.Send(msgText);
	}
}

void TmxEventLogThread::idle()
{
	std::this_thread::sleep_for(std::chrono::seconds(EVENTLOG_SEC_SLEEP));
}

#endif

}} // namespace tmx::utils
