/*
 * MessageContext.cpp
 *
 *  Created on: Jul 29, 2014
 *      Author: ivp
 */

#include "MessageContext.h"
#include <assert.h>
#include <sstream>

using namespace std;

MessageContext::MessageContext()
{

}

void MessageContext::insertOrUpdateMessageActivity(MessageActivityEntry &entry)
{
	// Convert time_t struct into UTC timestamp string.
	struct tm *tm;
	struct tm buf;
	tm = gmtime_r(&entry.lastReceivedTimestamp, &buf):
	stringstream timestamp;
	timestamp << tm->tm_year + 1900 << "-" << tm->tm_mon + 1 << "-" << tm->tm_mday << " " << tm->tm_hour << ":" << tm->tm_min << ":" << tm->tm_sec;

	// Insert or update messageActivity row.

	unique_ptr<sql::Statement> stmt(this->getStatement());

	stringstream query;
	query << "INSERT INTO messageActivity (messageTypeId, pluginId, count, lastReceivedTimestamp, averageInterval)";
	query << " VALUES ('" << entry.messageTypeId << "','" << entry.pluginId << "','" << entry.count << "','" << timestamp.str() << "','" << entry.averageInterval << "')";
	query << " ON DUPLICATE KEY UPDATE count = VALUES(count), lastReceivedTimestamp = VALUES(lastReceivedTimestamp), averageInterval = VALUES(averageInterval)";

	stmt->execute(query.str());

	if (entry.pluginId != 0 && entry.messageTypeId != 0)
		this->mapPluginToMessageType(entry.pluginId, entry.messageTypeId);

	//TODO: this might not be required... make it optional?
	// Query for id of row that was just inserted/updated.

	query.clear();
	query.str("");
	query << "SELECT `id` FROM `messageActivity`";
	query << " WHERE `messageActivity`.`messageTypeId` = '" << entry.messageTypeId << "' AND `messageActivity`.`pluginId` = '" << entry.pluginId << "'";

	unique_ptr<sql::ResultSet> rset(stmt->executeQuery(query.str()));
	if (rset->next())
	{
		entry.id = rset->getUInt("id");
	}
}

void MessageContext::insertMessageType(MessageTypeEntry &entry, bool updateDescriptionOnDuplicate)
{
	unique_ptr<sql::Statement> stmt(this->getStatement());

	stringstream query;
	query << "INSERT INTO messageType (type, subtype, description)";
	query << " VALUES ('" << entry.type << "','" << entry.subtype << "','" << entry.description << "')";
	if (updateDescriptionOnDuplicate)
		query << " ON DUPLICATE KEY UPDATE description=VALUES(description)";
	else
		query << " ON DUPLICATE KEY UPDATE `description`=IF(LENGTH(`description`)=0, '" << entry.description << "', `description`)";

	query << ";";

	stmt->execute(query.str());

	// Query for id of row that was just inserted/updated.

	query.clear();
	query.str("");
	query << "SELECT * FROM `messageType`";
	query << " WHERE `messageType`.`type` = '" << entry.type << "' AND `messageType`.`subtype` = '" << entry.subtype << "'";

	unique_ptr<sql::ResultSet> rset(stmt->executeQuery(query.str()));
	if (rset->next())
	{
		entry.id = rset->getUInt("id");
		entry.description = rset->getUInt("description");
	}
}

void MessageContext::mapPluginToMessageType(unsigned int pluginId, unsigned int messageTypeId)
{
	unique_ptr<sql::Statement> stmt(this->getStatement());

	stringstream query;
	query << "INSERT IGNORE INTO `pluginMessageMap` (`pluginId`, `messageTypeId`)";
	query << " VALUES ('" << pluginId << "','" << messageTypeId << "')";
	query << ";";

	stmt->execute(query.str());
}

std::set<MessageTypeEntry> MessageContext::getAllMessageTypes()
{
	set<MessageTypeEntry> results;

	unique_ptr<sql::Statement> stmt(this->getStatement());

	unique_ptr< sql::ResultSet > rset(stmt->executeQuery("SELECT * FROM `messageType`;"));
	while(rset->next())
	{
		MessageTypeEntry entry;

		entry.id = rset->getUInt("id");
		entry.type = rset->getString("type");
		entry.subtype = rset->getString("subtype");
		entry.description = rset->getString("description");

		results.insert(entry);
	}

	return results;
}
