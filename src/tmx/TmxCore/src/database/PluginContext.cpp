/*
 * PluginContext.cpp
 *
 *  Created on: Jul 23, 2014
 *      Author: ivp
 */

#include "PluginContext.h"
#include <assert.h>
#include <sstream>
using namespace std;

PluginContext::PluginContext()
{

}

vector<PluginEntry> PluginContext::getAllPlugins()
{
	vector<PluginEntry> results;

	std::unique_ptr<sql::Statement> stmt(this->getStatement());

	std::unique_ptr< sql::ResultSet > rset(stmt->executeQuery("SELECT * FROM plugin"));
	while(rset->next())
	{
		PluginEntry entry;

		entry.id = rset->getUInt("id");
		entry.name = rset->getString("name");
		entry.description = rset->getString("description");
		entry.version = rset->getString("version");

		results.push_back(entry);
	}

	return results;
}

PluginEntry PluginContext::getPlugin(std::string pluginName)
{
	PluginEntry results;

	std::unique_ptr<sql::Statement> stmt(this->getStatement());

	string query = "SELECT * FROM `plugin` WHERE `plugin`.`name` = '" + DbContext::formatStringValue(pluginName) + "';";
	std::unique_ptr< sql::ResultSet > rset(stmt->executeQuery(query));
	if (rset->next())
	{
		results.id = rset->getUInt("id");
		results.name = rset->getString("name");
		results.description = rset->getString("description");
		results.version = rset->getString("version");
	}

	return results;
}

void PluginContext::insertOrUpdatePlugin(PluginEntry &entry)
{
	// Insert / Update Plugin
	std::string query =
		"INSERT INTO `plugin` (`name`, `description`, `version`) "
		"VALUES (?, ?, ?) as new "
		"ON DUPLICATE KEY UPDATE "
		"`name` = new.name, "
		"`description` = new.description, "
		"`version` = new.version";

	std::unique_ptr<sql::PreparedStatement> insertStmt(this->getPreparedStatement(query));
	insertStmt->setString(1, entry.name);
	insertStmt->setString(2, entry.description);
	insertStmt->setString(3, entry.version);
	insertStmt->execute();

	std::unique_ptr<sql::PreparedStatement> selectStmt(
		this->getPreparedStatement("SELECT `id` FROM `plugin` WHERE `plugin`.`name` = ?")
	);

	selectStmt->setString(1, entry.name);

	// Query for id of row that was just inserted/updated.
	std::unique_ptr<sql::ResultSet> rset(selectStmt->executeQuery());
	if (rset->next())
	{
		entry.id = rset->getUInt("id");
	}
}

void PluginContext::removeAllNotInstalled()
{
	std::unique_ptr<sql::Statement> stmt(this->getStatement());

	stringstream query;
	query << "DELETE FROM `plugin` WHERE `plugin`.`id` NOT IN (SELECT `installedPlugins`.`pluginId` FROM `installedPlugins`);";
	stmt->execute(query.str());
}

void PluginContext::setStatusForAllPlugins(std::string status)
{
	std::unique_ptr<sql::Statement> stmt(this->getStatement());

	stringstream query;
	query << "UPDATE `pluginStatus` SET `value` = '" << DbContext::formatStringValue(status) << "' WHERE `key` = '';";
	stmt->execute(query.str());

}

void PluginContext::setPluginStatus(unsigned int pluginId, std::string status)
{
	std::unique_ptr<sql::Statement> stmt(this->getStatement());

	stringstream query;
	query << "INSERT INTO `pluginStatus` (`pluginId`,`key`,`value`) VALUES ('" << pluginId << "', '', '" << DbContext::formatStringValue(status) << "') ON DUPLICATE KEY UPDATE value = VALUES(value);";
	stmt->execute(query.str());
}

void PluginContext::setPluginStatusItems(unsigned int pluginId, const std::vector<PluginStatusItem>& statusItems)
{
	// No Status Changes
    if (statusItems.empty()){
        return;
	}

	// Insert / Update Plugin Status
    std::string query = "INSERT INTO `pluginStatus` (`pluginId`, `key`, `value`) VALUES ";
    //NOSONAR Build multirow insert of tuples e.g., (?, ?, ?), (?, ?, ?), (?, ?, ?)
    for (size_t i = 0; i < statusItems.size(); ++i)
    {
        if (i > 0){
            query += ", ";
		}

        query += "(?, ?, ?)";
    }

    query += "  as new ON DUPLICATE KEY UPDATE `value` = new.value";

    std::unique_ptr<sql::PreparedStatement> stmt(this->getPreparedStatement(query));

	// Assign multirow insert inputs
    int paramIndex = 0;
    for (const auto &item : statusItems)
    {
        stmt->setUInt(paramIndex + 1, pluginId);
        stmt->setString(paramIndex + 2, item.key);
        stmt->setString(paramIndex + 3, item.value);

        // Increment for next row's tuple
        paramIndex += 3;
    }

    stmt->execute();
}

void PluginContext::removePluginStatusItems(unsigned int pluginId, const std::vector<std::string>& itemKeys)
{
    std::unique_ptr<sql::PreparedStatement> stmt(
        this->getPreparedStatement(
            "DELETE FROM `pluginStatus` WHERE `pluginId` = ? AND `key` = ?;"
        )
    );

	for (const auto& key : itemKeys)
    {
        stmt->setUInt(1, pluginId);
        stmt->setString(2, key);
        stmt->executeUpdate();
    }
}

void PluginContext::removeAllPluginStatusItems(unsigned int pluginId)
{
	std::unique_ptr<sql::Statement> stmt(this->getStatement());

	stringstream query;
	query << "DELETE FROM `pluginStatus` WHERE `pluginId` = '" << pluginId << "' AND `key` > '';";
	stmt->execute(query.str());
}

void PluginContext::removeAllPluginStatusItems()
{
	std::unique_ptr<sql::Statement> stmt(this->getStatement());

	stringstream query;
	query << "DELETE FROM `pluginStatus` WHERE `key` > '';";
	stmt->execute(query.str());
}


set<InstalledPluginEntry> PluginContext::getInstalledPlugins(bool enabledOnly)
{
	set<InstalledPluginEntry> results;

	std::unique_ptr<sql::Statement> stmt(this->getStatement());

	/*
	 * select
    		cars.ID,
    		models.model
		from
    		cars
        		join models
            		on cars.model=models.ID
	 */

	string query = "SELECT *, `installedPlugin`.`id` AS installedPluginId FROM `installedPlugin` JOIN `plugin` ON `plugin`.`id` = `installedPlugin`.`pluginId`";
	if (enabledOnly)
	{
		query += " WHERE `installedPlugin`.`enabled` = '1'";
	}

	query += ";";

	std::unique_ptr< sql::ResultSet > rset(stmt->executeQuery(query));
	while(rset->next())
	{
		InstalledPluginEntry entry;

		entry.id = rset->getUInt("installedPluginId");
		entry.path = rset->getString("path");
		entry.exeName = rset->getString("exeName");
		//entry.manifestName = rset->getString("manifestName");
		entry.enabled = rset->getBoolean("enabled");
		entry.maxMessageInterval = rset->getUInt("maxMessageInterval");

		entry.plugin.id = rset->getUInt("id");
		entry.plugin.name = rset->getString("name");
		entry.plugin.description = rset->getString("description");
		entry.plugin.version = rset->getString("version");

		results.insert(entry);
	}

	return results;
}

void PluginContext::disableInstalledPlugin(unsigned int id)
{
	std::unique_ptr<sql::Statement> stmt(this->getStatement());

	stringstream query;
	query << "UPDATE `installedPlugin` SET `enabled` = '0' WHERE `id` = '" << id << "';";
	stmt->execute(query.str());
}
