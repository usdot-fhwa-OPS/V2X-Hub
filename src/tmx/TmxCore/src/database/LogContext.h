/*
 * LogContext.h
 *
 *  Created on: Jul 30, 2014
 *      Author: ivp
 */

#ifndef LOGCONTEXT_H_
#define LOGCONTEXT_H_

#include "DbContext.h"
#include <string>

typedef enum {
	LogLevel_Debug = 0,
	LogLevel_Info,
	LogLevel_Warning,
	LogLevel_Error,
	LogLevel_Fatal
} LogLevel;

class LogContext : public DbContext {
public:
	LogContext();

	void addEventLogEntry(std::string source, std::string description, LogLevel level);

	int purgeOldLogEntries(unsigned int numberToKeep);
};

#endif /* LOGCONTEXT_H_ */
