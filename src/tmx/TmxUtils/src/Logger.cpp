/*
 * Logger.cpp
 *
 *  Created on: Jul 26, 2017
 *      Author: gmb
 */

#define FILE_NAME_MAX_WIDTH 32

#include "Logger.h"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <sys/time.h>

namespace tmx {
namespace utils {

LogMessage::LogMessage():
		level(Logger::FromString(DEFAULT_LOG_LEVEL)),
		timestamp(Clock::GetMillisecondsSinceEpoch()),
		line(0),
		logger(0)
{
}

LogMessage::LogMessage(const LogMessage &copy):
		level(copy.level),
		timestamp(copy.timestamp),
		component(copy.component),
		file(copy.file),
		line(copy.line),
		log(copy.log),
		logger(copy.logger)
{
}

LogMessage::LogMessage(Logger *logger, LogLevel level, std::string component, std::string file, uint32_t line):
		level(level),
		timestamp(Clock::GetMillisecondsSinceEpoch()),
		component(component),
		file(file),
		line(line),
		logger(logger)
{
}

Logger::Logger(): message(0)
{
	os.setf(std::ios::boolalpha);
}

Logger::~Logger()
{
	delete message;
	message = 0;
}

std::ostream &Logger::Get(LogLevel level, std::string file, unsigned int line, std::string component)
{
	if (message)
		delete message;

	message = new LogMessage(this, level, component, file, line);

	return os;
}

std::string Logger::ToString(LogLevel level)
{
	static const char* const buffer[] = {"ERROR", "WARNING", "INFO", "DEBUG", "DEBUG1", "DEBUG2", "DEBUG3", "DEBUG4"};
    return buffer[level];
}

LogLevel Logger::FromString(const char *level)
{
    if (strcmp("DEBUG4", level) == 0)
        return logDEBUG4;
    if (strcmp("DEBUG3", level) == 0)
        return logDEBUG3;
    if (strcmp("DEBUG2", level) == 0)
        return logDEBUG2;
    if (strcmp("DEBUG1", level) == 0)
        return logDEBUG1;
    if (strcmp("DEBUG", level) == 0)
        return logDEBUG;
    if (strcmp("INFO", level) == 0)
        return logINFO;
    if (strcmp("WARNING", level) == 0)
        return logWARNING;
    if (strcmp("ERROR", level) == 0)
        return logERROR;

    // Write to standard out
    LOG_TO_STREAM(logWARNING, std::cout) << "Unknown logging level '" << level << "'. Using " << DEFAULT_LOG_LEVEL << " level as default." << std::endl;

    return Logger::FromString(DEFAULT_LOG_LEVEL);
}

LogLevel Logger::FromString(const std::string &level)
{
	return Logger::FromString(level.c_str());
}

std::ostream &_logtime(std::ostream &os, uint64_t timestamp)
{
	struct timeval tv;

	if (timestamp == 0)
	{
		tv.tv_sec = timestamp / 1000;
		tv.tv_usec = timestamp % 1000 * 1000;
	}
	else
	{
		gettimeofday(&tv, NULL);
	}

	time_t time = tv.tv_sec;
	short ms = tv.tv_usec / 1000;

	struct tm *myTm= new struct tm; 
	myTm = localtime_r(&time,myTm);
	char tmBuffer[20];
	strftime(tmBuffer, 20, "%F %T", myTm);

	os << "[" << tmBuffer << "." << std::setfill('0') << std::setw(3) << ms << "] " << std::setfill(' ');

	return os;
}

std::ostream &_logsource(std::ostream &os, std::string file, uint32_t line)
{
	static constexpr int fileMaxLen = FILE_NAME_MAX_WIDTH;

	if (line > 0)
	{
		file.append(" (");
		file.append(std::to_string(line));
		file.append(")");
	}

	if (file.length() > fileMaxLen)
		os << std::right << std::setw(fileMaxLen) << file.substr(file.size() - fileMaxLen);
	else
		os << std::right << std::setw(fileMaxLen) << file;

	return os;
}

std::ostream &_loglevel(std::ostream &os, LogLevel level)
{
	os << " - " << std::left << std::setw(7) << Logger::ToString(level) << ": ";
	return os;
}


}} /* Namespace tmx::utils */
