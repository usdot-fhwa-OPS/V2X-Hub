/*
 * Logger.h
 *
 *  Created on: Jul 26, 2017
 *      Author: gmb
 */

#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_

#ifndef LOGGER_MAX_LEVEL
#define LOGGER_MAX_LEVEL tmx::utils::logERROR
#endif

#ifndef DEFAULT_LOG_LEVEL
#define DEFAULT_LOG_LEVEL "DEBUG"
#endif

#define UNKNOWN_SOURCE "Unknown source"

#include <sstream>
#include "TmxLog.h"
#include "Clock.h"

namespace tmx {
namespace utils {

class Logger;

struct LogMessage {
	LogMessage();
	LogMessage(const LogMessage &);
	LogMessage(Logger *logger, LogLevel level, std::string component, std::string file, uint32_t line);

	LogLevel level;
	uint64_t timestamp;
	std::string component;
	std::string file;
	uint32_t line;
	std::string log;
	Logger *logger;
};

class Logger {
public:
	virtual ~Logger();

	// No assignments or copies allowed
	Logger(const Logger&) = delete;
	Logger &operator=(const Logger &) = delete;

	// Static functions
    static inline LogLevel& ReportingLevel()
    {
    	// Defined in-line in order to facilitate an optionally compiled-in change in the default level
        static LogLevel reportingLevel = FromString(DEFAULT_LOG_LEVEL);
        return reportingLevel;
    }

    static std::string ToString(LogLevel level);
    static LogLevel FromString(const char *level);
    static LogLevel FromString(const std::string &level);

	// Pure virtual functions to override
	virtual void WriteMessage(LogMessage &message) = 0;
	virtual std::string GetName() = 0;

	// The access operation
	std::ostream &Get(LogLevel level = FromString(DEFAULT_LOG_LEVEL), std::string file = UNKNOWN_SOURCE, uint32_t line = 0, std::string component = "");
protected:
	Logger();
	LogMessage *message;
	std::ostringstream os;
};

std::ostream & _logtime(std::ostream &os, uint64_t timestamp);
std::ostream & _logsource(std::ostream &os, std::string file, uint32_t line);
std::ostream & _loglevel(std::ostream &os, LogLevel level);

#define LOG_TO_STREAM(L, X) \
	    _logtime(X, 0); _logsource(X, __FILE__, __LINE__); _loglevel(X, L); X

}} /* Namespace tmx::utils */


#endif /* SRC_LOGGER_H_ */
