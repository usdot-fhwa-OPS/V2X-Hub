/*
 * LogContext.cpp
 *
 *  Created on: Jul 30, 2014
 *      Author: ivp
 */

#include "LogContext.h"
#include "../logger.h"
#include <sstream>

using namespace std;

LogContext::LogContext() { }

void LogContext::addEventLogEntry(std::string source, std::string description, LogLevel level)
{
	string levelString = "Debug";
	switch(level)
	{
		break;
	case LogLevel_Info:
		levelString = "Info";
		break;
	case LogLevel_Warning:
		levelString = "Warning";
		break;
	case LogLevel_Error:
		levelString = "Error";
		break;
	case LogLevel_Fatal:
		levelString = "Fatal";
		break;
	case LogLevel_Debug:
	default:
		levelString = "Debug";
		break;
	}

	try {
		unique_ptr<sql::Statement> stmt(this->getStatement());

		stringstream query;
		query << "INSERT INTO `eventLog` (`source`,`description`,`logLevel`) VALUES (";
		query << "'" << source << "'";
		query << ", ";
		query << "'" << DbContext::formatStringValue(description) << "'";
		query << ", ";
		query << "'" << levelString << "'";
		query << ");";

		stmt->execute(query.str());
	} catch (DbException &e) {
		LOG_WARN("MySQL: Unable to add event log entry [" << e.what() << "]");
	}
}


int LogContext::purgeOldLogEntries(unsigned int numberToKeep)
{
	unique_ptr<sql::Statement> stmt(this->getStatement());

	stringstream query;
	query << "SELECT `id` FROM `eventLog`;";

	unique_ptr<sql::ResultSet> res(stmt->executeQuery(query.str()));

	unsigned int count = res->rowsCount();

	if (numberToKeep > count)
		return 0;

	unsigned int numberToRemove = count - numberToKeep;

	stringstream query2;
	query2 << "DELETE FROM `eventLog` ORDER BY `timestamp` ASC LIMIT " << numberToRemove << ";";
	stmt->execute(query2.str());

	return numberToRemove;
}
