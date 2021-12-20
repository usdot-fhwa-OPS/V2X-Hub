/*
 * ConfigContext.cpp
 *
 *  Created on: Jul 26, 2014
 *      Author: ivp
 */

#include "ConfigContext.h"
#include <sstream>

using namespace std;

ConfigContext::ConfigContext()
{

}

std::map<std::string, SystemConfigurationParameterEntry> ConfigContext::getSystemConfigParameters()
{
	map<string, SystemConfigurationParameterEntry> results;

	unique_ptr<sql::Statement> stmt(this->getStatement());

	unique_ptr< sql::ResultSet > rset(stmt->executeQuery("SELECT * FROM `systemConfigurationParameter`;"));
	while(rset->next())
	{
		SystemConfigurationParameterEntry entry;

		entry.id = rset->getUInt("id");
		entry.key = rset->getString("key");
		entry.value = rset->getString("value");
		entry.defaultValue = rset->getString("defaultValue");

		results[entry.key] = entry;
	}

	return results;
}

SystemConfigurationParameterEntry ConfigContext::getSystemConfigParameter(std::string key)
{
	SystemConfigurationParameterEntry results;

	unique_ptr<sql::Statement> stmt(this->getStatement());

	stringstream query;
	query << "SELECT * FROM `systemConfigurationParameter` WHERE `key` = '";
	query << DbContext::formatStringValue(key);
	query << "';";

	unique_ptr< sql::ResultSet > rset(stmt->executeQuery(query.str()));
	if(rset->next())
	{
		results.id = rset->getUInt("id");
		results.key = rset->getString("key");
		results.value = rset->getString("value");
		results.defaultValue = rset->getString("defaultValue");
	}

	return results;
}


void ConfigContext::initializeSystemConfigParameter(SystemConfigurationParameterEntry *entry)
{
	unique_ptr<sql::Statement> stmt(this->getStatement());

	{
		stringstream query;
		query << "INSERT INTO `systemConfigurationParameter` (`key`, `value`, `defaultValue`) VALUES (";
		query << "'" << DbContext::formatStringValue(entry->key) << "'";
		query << ", ";
		query << "'" << DbContext::formatStringValue(entry->value) << "'";
		query << ", ";
		query << "'" << DbContext::formatStringValue(entry->defaultValue) << "'";
		query << ") ON DUPLICATE KEY UPDATE defaultValue = VALUES(defaultValue);";

		stmt->execute(query.str());
	}

	SystemConfigurationParameterEntry dbEntry = getSystemConfigParameter(entry->key);
	if (dbEntry.id)
		*entry = dbEntry;
}


void ConfigContext::initializePluginConfigParameters(unsigned int pluginId, std::vector<PluginConfigurationParameterEntry> &entries)
{
	unique_ptr<sql::Statement> stmt(this->getStatement());

	for(vector<PluginConfigurationParameterEntry>::iterator itr = entries.begin(); itr != entries.end(); itr++)
	{
		stringstream query;
		query << "INSERT INTO `pluginConfigurationParameter` (`pluginId`, `key`, `value`, `defaultValue`, `description`) VALUES (";
		query << "'" << pluginId << "'";
		query << ", ";
		query << "'" << DbContext::formatStringValue(itr->key) << "'";
		query << ", ";
		query << "'" << DbContext::formatStringValue(itr->value) << "'";
		query << ", ";
		query << "'" << DbContext::formatStringValue(itr->defaultValue) << "'";
		query << ", ";
		query << "'" << DbContext::formatStringValue(itr->description) << "'";
		query << ") ON DUPLICATE KEY UPDATE defaultValue = VALUES(defaultValue), description = VALUES(description)";

		// Plugin system parameters always should update the value
		if (pluginId == 0)
			query << ", value = VALUES(value)";

		query << ";";

		stmt->execute(query.str());
	}

	if (pluginId > 0)
	{
		stringstream query;
		query << "DELETE FROM `pluginConfigurationParameter` WHERE `pluginId` = '";
		query << pluginId;
		query << "'";

		if (entries.size() > 0)
		{
			query << " AND `key` NOT IN (";

			bool first = true;
			for(vector<PluginConfigurationParameterEntry>::iterator itr = entries.begin(); itr != entries.end(); itr++)
			{
				if (!first)
					query << ",";
				query << "'" << DbContext::formatStringValue(itr->key) << "'";
				first = false;
			}
			query << ");";
		}
		else
		{
			query << ";";
		}

		stmt->execute(query.str());
	}
}

std::map<std::string, PluginConfigurationParameterEntry> ConfigContext::getPluginConfigParameters(unsigned int pluginId)
{
	map<string, PluginConfigurationParameterEntry> results;

	unique_ptr<sql::Statement> stmt(this->getStatement());

	stringstream query;
	query << "SELECT * FROM `pluginConfigurationParameter` WHERE `pluginId` IN ( '0', '";
	query << pluginId;
	query << "');";

	unique_ptr< sql::ResultSet > rset(stmt->executeQuery(query.str()));
	while(rset->next())
	{
		PluginConfigurationParameterEntry entry;

		entry.id = rset->getUInt("id");
		entry.pluginId = rset->getUInt("pluginId");
		entry.key = rset->getString("key");
		entry.value = rset->getString("value");
		entry.defaultValue = rset->getString("defaultValue");

		results[entry.key] = entry;
	}

	return results;
}

void ConfigContext::updatePluginConfigParameterValue(const PluginConfigurationParameterEntry &entry)
{
	unique_ptr<sql::Statement> stmt(this->getStatement());

	//affected_rows = stmt->executeUpdate("UPDATE test SET label = 'y' WHERE id = 100");

	stringstream query;
	query << "UPDATE `pluginConfigurationParameter` SET `value` = '";
	query << DbContext::formatStringValue(entry.value);
	query << "' WHERE `id` = '";
	query << entry.id;
	query << "';";
	/*
	query << "' WHERE `pluginId` = '";
	query << entry.pluginId;
	query << "' AND `key` = '";
	query << entry.key;
	query << "';";
	 */

	stmt->executeUpdate(query.str());
}


