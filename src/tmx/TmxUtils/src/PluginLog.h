/*
 * PluginLog.h : A header-only logging implementation for use in the plugins
 *
 * Adapted from:
 * http://www.drdobbs.com/cpp/logging-in-c/201804215
 *
 * Logging In C++
 * By Petru Marginean, September 05, 2007
 *
 *  Created on: Mar 3, 2016
 *      Author: BAUMGARDNER
 */

#ifndef SRC_PLUGINLOG_H_
#define SRC_PLUGINLOG_H_

#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <ios>
#include <sstream>
#include "Logger.h"

#include <tmx/messages/routeable_message.hpp>
#ifndef NO_EVENTLOG_UDP
#include <tmx/apimessages/TmxEventLog.hpp>
#endif

#define DEFAULT_EVENTLOG_UDP_PORT 24625

namespace tmx {
namespace utils {

template <typename T>
class PluginLog: public Logger
{
public:
	PluginLog();
	virtual ~PluginLog();
	void WriteMessage(LogMessage &msg);
	std::string GetName();

	PluginLog(const PluginLog&) = delete;
	PluginLog &operator=(const PluginLog &) = delete;
};

template <typename T>
PluginLog<T>::PluginLog(): Logger() { }

template <typename T>
PluginLog<T>::~PluginLog()
{
	if (message)
	{
		message->log = os.str();
		WriteMessage(*message);
	}
}

template <typename T>
void PluginLog<T>::WriteMessage(LogMessage &msg) {
	T writer;
	writer.Output(msg);
}

template <typename T>
std::string PluginLog<T>::GetName()
{
	return battelle::attributes::type_name<T>();
}
/**
 * These classes are specific logging writer classes that do the work
 * Each require only one function:
 * void Output(const PluginLogMessage &msg); // Output the message
 * However, each also might have some configuration like an enable flag
 */
class Output2FILE
{
public:
    static FILE* &Stream();
    static bool &Enable();
    void Output(LogMessage& msg);
};

class Output2Syslog
{
public:
	static int ToSyslogLevel(LogLevel lvl);
	static bool &Enable();
	void Output(LogMessage& msg);
};

class Output2Eventlog
{
public:
    static IvpLogLevel ToEventLogLevel(LogLevel level);
    static LogLevel FromEventLogLevel(IvpLogLevel level);
#ifndef NO_EVENTLOG_UDP
	static bool &Enable();
	void Output(LogMessage& msg);
#endif
};


typedef PluginLog<Output2FILE> FILELog;
typedef PluginLog<Output2Syslog> SYSLog;
#ifndef NO_EVENTLOG_UDP
typedef PluginLog<Output2Eventlog> EVENTLog;
typedef EVENTLog PLUGINLog;
#else
typedef FILELog PLUGINLog;
#endif

#define FILE_LOG(level) \
    if (level > LOGGER_MAX_LEVEL) ;\
    else if (level > tmx::utils::FILELog::ReportingLevel() || !tmx::utils::Output2FILE::Stream()) ; \
    else tmx::utils::FILELog().Get(level, __FILE__, __LINE__)
#define SYS_LOG(level, plugin) \
    if (level > LOGGER_MAX_LEVEL) ;\
    else if (level > tmx::utils::SYSLog::ReportingLevel()) ; \
    else tmx::utils::SYSLog().Get(level, __FILE__, __LINE__, plugin)
#define PLUGIN_LOG(level, plugin) \
	    if (level > LOGGER_MAX_LEVEL) ;\
	    else if (level > tmx::utils::FILELog::ReportingLevel()) ; \
	    else tmx::utils::PLUGINLog().Get(level, __FILE__, __LINE__, plugin)

#ifndef PLOG
#define PLOG(X) FILE_LOG(X)
#endif

}} // namespace tmx::utils

#endif /* SRC_PLUGINLOG_H_ */
