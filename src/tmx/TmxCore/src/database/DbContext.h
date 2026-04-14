/*
 * DbContext.h
 *
 *  Created on: Jul 23, 2014
 *      Author: ivp
 */

#ifndef DBCONTEXT_H_
#define DBCONTEXT_H_

#include <string.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/warning.h>
#include <cppconn/metadata.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/resultset_metadata.h>
#include <cppconn/statement.h>
#include "mysql_driver.h"
#include "mysql_connection.h"


struct DbConnectionInformation {
	std::string url;
	std::string username;
	std::string password;
	std::string db;
};

typedef sql::SQLException DbException;

class DbContext {
public:
	virtual ~DbContext();

	static DbConnectionInformation ConnectionInformation;
protected:
	DbContext();

	sql::Statement *getStatement();

	/**
	 * Creates a prepared SQL statement using the active database connection.
	 * The caller is responsible for binding parameters before executing it.
	 *
	 * @param sql The SQL statement text containing parameter `?` placeholders.
	 * @return A prepared statement ready for parameter binding and execution.
	 */
	std::unique_ptr<sql::PreparedStatement> getPreparedStatement(const std::string& sql);

	/**
	 * @deprecated This function is obsolete and will be removed after references
	 * switch to database prepared statements which more reliably prevents SQL
	 * injection attacks. Use getPreparedStatement() instead.
	 */
	static std::string formatStringValue(std::string str);

private:
	sql::Driver *mDriver;
	std::unique_ptr< sql::Connection > mCon;
};


#endif /* DBCONTEXT_H_ */
