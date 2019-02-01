/*
 * DbContext.cpp
 *
 *  Created on: Jul 23, 2014
 *      Author: ivp
 */

#include "DbContext.h"
#include <sstream>

DbConnectionInformation DbContext::ConnectionInformation;

DbContext::DbContext()
{
	mDriver = sql::mysql::get_driver_instance();
	mCon = std::auto_ptr< sql::Connection >(mDriver->connect(ConnectionInformation.url, ConnectionInformation.username, ConnectionInformation.password));

	std::auto_ptr< sql::Statement > stmt(mCon->createStatement());

	stmt->execute("USE " + ConnectionInformation.db);
}

DbContext::~DbContext()
{

}

sql::Statement *DbContext::getStatement()
{
	return mCon->createStatement();
}


std::string DbContext::formatStringValue(std::string str)
{
	std::stringstream ss;

	for(unsigned int i = 0; i < str.length(); i++)
	{
		char c = str.at(i);
		if (c == '\'')
			ss << "\\'";
		else
			ss << c;
	}

	return ss.str();
}

