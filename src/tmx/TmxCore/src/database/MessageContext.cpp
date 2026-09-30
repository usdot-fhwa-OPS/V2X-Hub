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
	struct tm tm;
	gmtime_r (&entry.lastReceivedTimestamp, &tm);
	stringstream timestamp;
	timestamp << tm.tm_year + 1900 << "-" << tm.tm_mon + 1 << "-" << tm.tm_mday << " " << tm.tm_hour << ":" << tm.tm_min << ":" << tm.tm_sec;

	// Insert or update messageActivity row.

	std::unique_ptr<sql::Statement> stmt(this->getStatement());

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

	std::unique_ptr<sql::ResultSet> rset(stmt->executeQuery(query.str()));
	if (rset->next())
	{
		entry.id = rset->getUInt("id");
	}
}

void MessageContext::insertMessageType(MessageTypeEntry &entry, bool updateDescriptionOnDuplicate)
{
	// INSERT / UPSERT Message Type
    std::string query =
        "INSERT INTO `messageType` (`type`, `subtype`, `description`) "
        "VALUES (?, ?, ?) AS new ";

    if (updateDescriptionOnDuplicate)
    {
		// Always update description
        query += "ON DUPLICATE KEY UPDATE `description` = new.description";
    }
    else
    {
		// Only update description, if current description is empty
        query += "ON DUPLICATE KEY UPDATE `description` = IF(LENGTH(`messageType`.`description`) = 0, new.description, `messageType`.`description`)";
    }

    std::unique_ptr<sql::PreparedStatement> pstmt(this->getPreparedStatement(query));
    pstmt->setString(1, entry.type);
    pstmt->setString(2, entry.subtype);
    pstmt->setString(3, entry.description);
    pstmt->execute();

    // Query for id of row that was just inserted/updated.
    std::unique_ptr<sql::PreparedStatement> qstmt(
        this->getPreparedStatement(
            "SELECT `id`, `description` FROM `messageType` "
            "WHERE `type` = ? AND `subtype` = ?"
        )
    );

    qstmt->setString(1, entry.type);
    qstmt->setString(2, entry.subtype);

    std::unique_ptr<sql::ResultSet> rset(qstmt->executeQuery());
    if (rset->next())
    {
		// Retrieve current id and description for use outside of function
        entry.id = rset->getUInt("id");
        entry.description = rset->getString("description");
    }
}

void MessageContext::mapPluginToMessageType(unsigned int pluginId, unsigned int messageTypeId)
{
	std::unique_ptr<sql::Statement> stmt(this->getStatement());

	stringstream query;
	query << "INSERT IGNORE INTO `pluginMessageMap` (`pluginId`, `messageTypeId`)";
	query << " VALUES ('" << pluginId << "','" << messageTypeId << "')";
	query << ";";

	stmt->execute(query.str());
}

std::set<MessageTypeEntry> MessageContext::getAllMessageTypes()
{
	set<MessageTypeEntry> results;

	std::unique_ptr<sql::Statement> stmt(this->getStatement());

	std::unique_ptr< sql::ResultSet > rset(stmt->executeQuery("SELECT * FROM `messageType`;"));
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
