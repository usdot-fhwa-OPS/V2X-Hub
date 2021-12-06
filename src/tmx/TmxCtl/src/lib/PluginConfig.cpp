/*
 * PluginConfig.cpp
 *
 *  Created on: Apr 24, 2017
 *      @author: gmb
 */

#include "TmxControl.h"
#include <libgen.h>
#include <tmx/messages/auto_message.hpp>
#include <sys/stat.h>

#define CONFIG_QUERY "\
	SELECT name, `key`, value, defaultValue, IVP.pluginConfigurationParameter.description \
	FROM IVP.pluginConfigurationParameter \
	LEFT JOIN IVP.plugin ON IVP.pluginConfigurationParameter.pluginId = IVP.plugin.id"

#define CFG_UPDATE "\
	UPDATE IVP.pluginConfigurationParameter \
	SET value = ?"

#define CFG_INFO_UPDATE "\
	UPDATE IVP.pluginConfigurationParameter \
	SET defaultValue = ?, description = ?"

#define CFG_INSERT "\
	INSERT INTO IVP.pluginConfigurationParameter \
	( pluginId, `key`, value, defaultValue, description ) \
	SELECT id, ?, ?, ?, ? \
	FROM IVP.plugin"

#define CFG_KEY_NE "\
	AND NOT EXISTS ( \
	SELECT * FROM IVP.pluginConfigurationParameter \
	WHERE pluginId = IVP.plugin.id \
	AND `key` = ? \
	)"

#define PLUGIN_UPDATE "\
	UPDATE IVP.plugin \
	SET description = ?, version = ? \
	WHERE name = ?"

#define PLUGIN_INSERT "\
	INSERT INTO IVP.plugin ( name, description, version ) \
	SELECT ?, ?, ? FROM DUAL \
	WHERE NOT EXISTS ( \
	SELECT * FROM IVP.plugin \
	WHERE name = ? \
	)"

#define IPLUGIN_UPDATE "\
	UPDATE IVP.installedPlugin \
	SET path = ?, exeName = ?, manifestName = ?"

#define IPLUGIN_INSERT "\
	INSERT INTO IVP.installedPlugin \
	( pluginId, path, exeName, manifestName, commandLineParameters, enabled, maxMessageInterval ) \
	SELECT id, ?, ?, ?, '', 0, 500000 \
	FROM IVP.plugin"

#define PLUGINID_NE "\
	AND NOT EXISTS ( \
	SELECT * FROM IVP.installedPlugin \
	WHERE pluginId = IVP.plugin.id \
	)"

#define PLUGIN_DELETE "\
	DELETE FROM IVP.plugin"

#define IPLUGIN_DELETE "\
	DELETE FROM IVP.installedPlugin"

#define SYSTEM_CFG_UPDATE "\
	UPDATE IVP.systemConfigurationParameter \
	SET value = ? \
	WHERE `key` = ?"

#define SYSTEM_CFG_INSERT "\
	INSERT INTO IVP.systemConfigurationParameter \
	(`key`, value, defaultValue ) \
	SELECT DISTINCT ?, ?, ? \
	FROM DUAL \
	WHERE NOT EXISTS ( \
	SELECT `key` FROM IVP.systemConfigurationParameter \
	WHERE `key` = ? )"

using namespace std;
using namespace sql;
using namespace tmx;
using namespace tmx::utils;

namespace tmxctl {

bool TmxControl::load_manifest(pluginlist &, ...)
{
	if (!checkPerm())
		return false;

	string manifest = (*_opts)["load-manifest"].as<string>();

	struct stat fStat;

	if (stat(manifest.c_str(), &fStat) < 0)
	{
		cerr << strerror(errno) << endl;
		return false;
	}

	_output.get_storage().get_tree().clear();
	_output.load<JSON>(manifest);

	// Always output in JSON
	bool flag = true;
	(*_opts).insert(make_pair("json", boost::program_options::variable_value(flag, false)));

	string name = _output.retrieve("name", "", false);
	if (name.empty())
	{
		cerr << "Missing plugin name in manifest" << endl;
		return false;
	}

	string query;

	string descr = _output.retrieve("description", "", false);
	string version = _output.retrieve("version", "", false);
	string exe = _output.retrieve("exeLocation", "", false);

	char *dirpath = strdup(manifest.c_str());
	char *basepath = strdup(manifest.c_str());

	string pluginPath = dirname(dirpath);
	string manifestFile = basename(basepath);

	free(dirpath);
	free(basepath);

	string pluginName = name;
	transform(pluginName.begin(), pluginName.end(), pluginName.begin(), ::tolower);
	pluginlist self { pluginName };

	try
	{
		std::string pwd = _pool.GetPwd();
		DbConnection conn = _pool.Connection("tcp://127.0.0.1:3306","IVP", pwd, "IVP");
		unique_ptr<PreparedStatement> stmt;

		// Update any existing entry in the plugin table first
		query = PLUGIN_UPDATE;

		PLOG(logDEBUG1) << "Executing query (?1 = " << descr << ", ?2 = " << version <<
				", ?3 = " << name << ")" << query;

		stmt.reset(conn.Get()->prepareStatement(query));
		stmt->setString(1, descr);
		stmt->setString(2, version);
		stmt->setString(3, name);
		stmt->execute();

		// Insert into the plugin table if this is a new entry
		query = PLUGIN_INSERT;

		PLOG(logDEBUG1) << "Executing query (?1 = " << name << ", ?2 = " << descr <<
				", ?3 = " << version << ", ?4 = " << name << ")" << query;

		stmt.reset(conn.Get()->prepareStatement(query));
		stmt->setString(1, name);
		stmt->setString(2, descr);
		stmt->setString(3, version);
		stmt->setString(4, name);
		stmt->execute();

		// Update any existing entry in the installed plugin table first
		query = add_constraint(IPLUGIN_UPDATE, self);

		PLOG(logDEBUG1) << "Executing query (?1 = " << pluginPath << ", ?2 = " << exe <<
				", ?3 = " << manifestFile << ")" << query;

		stmt.reset(conn.Get()->prepareStatement(query));
		stmt->setString(1, pluginPath);
		stmt->setString(2, exe);
		stmt->setString(3, manifestFile);
		for (size_t i = 0; i < self.size(); i++)
		{
			stmt->setString(i + 4, self[i]);
		}
		stmt->execute();

		// Insert into the installed plugin table if this is a new entry
		query = add_constraint(IPLUGIN_INSERT, self, "id");
		query += PLUGINID_NE;

		PLOG(logDEBUG1) << "Executing query (?1 = " << pluginPath << ", ?2 = " << exe <<
				", ?3 = " << manifestFile << ")" << query;

		stmt.reset(conn.Get()->prepareStatement(query));
		stmt->setString(1, pluginPath);
		stmt->setString(2, exe);
		stmt->setString(3, manifestFile);
		for (size_t i = 0; i < self.size(); i++)
		{
			stmt->setString(i + 4, self[i]);
		}
		stmt->execute();

		// Update the configuration parameters
		tmx::message manifestAsMsg(_output);
		for (auto cfg : manifestAsMsg.get_array<tmx::message>("configuration"))
		{
			string cfgKey = cfg.get<string>("key", "");
			string cfgDef = cfg.get<string>("default", "");
			string cfgDesc = cfg.get<string>("description", "");

			if (cfgKey.empty())
				continue;

			query = add_constraint(CFG_INFO_UPDATE, self, "id");
			query += " AND `key` = ?";

			// Try to update existing values
			PLOG(logDEBUG1) << "Executing query (?1 = " << cfgDef << ", ?2 = " << cfgDesc <<
					", ?3 = " << cfgKey << ")" << query;

			stmt.reset(conn.Get()->prepareStatement(query));
			stmt->setString(1, cfgDef);
			stmt->setString(2, cfgDesc);
			for (size_t i = 0; i < self.size(); i++)
			{
				stmt->setString(i + 3, self[i]);
			}
			stmt->setString(self.size() + 3, cfgKey);
			stmt->execute();

			// Try to add new values
			query = add_constraint(CFG_INSERT, self, "id");
			query += CFG_KEY_NE;

			PLOG(logDEBUG1) << "Executing query (?1 = " << cfgKey << ", ?2 = " << cfgDef <<
					", ?3 = " << cfgDef << ", ?4 = " << cfgDesc <<
					", ?5 = " << cfgKey << "): " << query;
			stmt.reset(conn.Get()->prepareStatement(query));
			stmt->setString(1, cfgKey);
			stmt->setString(2, cfgDef);
			stmt->setString(3, cfgDef);
			stmt->setString(4, cfgDesc);
			for (size_t i = 0; i < self.size(); i++)
			{
				stmt->setString(i + 5, self[i]);
			}
			stmt->setString(self.size() + 5, cfgKey);
			stmt->executeUpdate();
		}
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	return true;
}

bool TmxControl::set(pluginlist &plugins, ...)
{
	if (!checkPerm())
		return false;

	string query = add_constraint(CFG_UPDATE, plugins);
	query += " AND `key` = ?";

	if (!_opts->count("key") || !_opts->count("value"))
		return false;

    string key = (*_opts)["key"].as<string>();
    string val = (*_opts)["value"].as<string>();

	PLOG(logDEBUG1) << "Setting " << key << " = " << val;

    if (key.empty())
    	return false;

    string def = (*_opts)["defaultValue"].as<string>();
    string comment = (*_opts)["description"].as<string>();

	try
	{
		PLOG(logDEBUG1) << "Executing query (?1 = " << val << ", ?2 = " << key << "): " << query;

		std::string pwd = _pool.GetPwd();
		DbConnection conn = _pool.Connection("tcp://127.0.0.1:3306","IVP", pwd, "IVP");
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		stmt->setString(1, val);
		for (size_t i = 0; i < plugins.size(); i++)
		{
			stmt->setString(i + 2, plugins[i]);
		}
		stmt->setString(plugins.size() + 2, key);
		stmt->executeUpdate();

		query = add_constraint(CFG_INSERT, plugins, "id");
		query += CFG_KEY_NE;

		PLOG(logDEBUG1) << "Executing query (?1 = " << val << ", ?2 = " << key <<
				", ?3 = " << def << ", ?4 = " << comment <<
				", ?5 = " << key << "): " << query;
		stmt.reset(conn.Get()->prepareStatement(query));
		stmt->setString(1, key);
		stmt->setString(2, val);
		stmt->setString(3, def);
		stmt->setString(4, comment);
		for (size_t i = 0; i < plugins.size(); i++)
		{
			stmt->setString(i + 5, plugins[i]);
		}
		stmt->setString(plugins.size() + 5, key);
		stmt->executeUpdate();

		return true;
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}


	return true;
}

bool TmxControl::set_system(pluginlist &plugins, ...)
{
	if (!checkPerm())
		return false;

	string query = SYSTEM_CFG_UPDATE;

	if (!_opts->count("key") || !_opts->count("value"))
		return false;

    string key = (*_opts)["key"].as<string>();
    string val = (*_opts)["value"].as<string>();

	PLOG(logDEBUG1) << "Setting " << key << " = " << val;

    if (key.empty())
    	return false;

    string def = (*_opts)["defaultValue"].as<string>();

	try
	{
		PLOG(logDEBUG1) << "Executing query (?1 = " << val << ", ?2 = " << key << "): " << query;

		std::string pwd = _pool.GetPwd();
		DbConnection conn = _pool.Connection("tcp://127.0.0.1:3306","IVP", pwd, "IVP");
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		stmt->setString(1, val);
		stmt->setString(2, key);
		stmt->executeUpdate();

		query = SYSTEM_CFG_INSERT;

		PLOG(logDEBUG1) << "Executing query (?1 = " << val << ", ?2 = " << key <<
				", ?3 = " << def << ", ?4 = " << key << "): " << query;
		stmt.reset(conn.Get()->prepareStatement(query));
		stmt->setString(1, key);
		stmt->setString(2, val);
		stmt->setString(3, def);
		stmt->setString(4, key);
		stmt->executeUpdate();

		return true;
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}


	return true;
}

bool TmxControl::reset(pluginlist &plugins, ...)
{
	if (!checkPerm())
		return false;

	string query = add_constraint(CFG_UPDATE, plugins);

	// Copy from the default column
    query.replace(strlen(CFG_UPDATE) - 1, 1, "defaultValue");
	query += " AND `key` = ?";

	if (!_opts->count("key"))
		return false;

    string key = (*_opts)["key"].as<string>();

    if (key.empty())
    	return false;

	try
	{
		PLOG(logDEBUG1) << "Executing query (?1 = " << key << "): " << query;

		std::string pwd = _pool.GetPwd();
		DbConnection conn = _pool.Connection("tcp://127.0.0.1:3306","IVP", pwd, "IVP");
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		for (size_t i = 0; i < plugins.size(); i++)
		{
			stmt->setString(i + 1, plugins[i]);
		}
		stmt->setString(plugins.size() + 1, key);
		int rows = stmt->executeUpdate();
		PLOG(logDEBUG1) << "Updated " << rows << " rows.";
		return rows > 0;
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}


	return true;
}

bool TmxControl::config(pluginlist &plugins, ...)
{
	string query = add_constraint(CONFIG_QUERY, plugins);
	query += " ORDER BY name";

	try
	{
		PLOG(logDEBUG1) << "Executing query " << query;

		_output.get_storage().get_tree().clear();
		std::string pwd = _pool.GetPwd();
		DbConnection conn = _pool.Connection("tcp://127.0.0.1:3306","IVP", pwd, "IVP");
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		for (size_t i = 0; i < plugins.size(); i++)
		{
			stmt->setString(i + 1, plugins[i]);
		}
		unique_ptr<ResultSet> rs(stmt->executeQuery());

		while (rs->next())
		{
			string name = rs->getString(1).asStdString();
			string key = rs->getString(2).asStdString();
			string val = rs->getString(3).asStdString();
			string def = rs->getString(4).asStdString();
			string dsc = rs->getString(5).asStdString();

			message_path_type keyPath(name, ATTRIBUTE_PATH_CHARACTER);
			keyPath /= message_path_type(key, ATTRIBUTE_PATH_CHARACTER);

			message_path_type path;
			path = keyPath;
			path /= message_path_type("value", ATTRIBUTE_PATH_CHARACTER);
			_output.store(path, val);

			path = keyPath;
			path /= message_path_type("defaultValue", ATTRIBUTE_PATH_CHARACTER);
			_output.store(path, def);

			path = keyPath;
			path /= message_path_type("description", ATTRIBUTE_PATH_CHARACTER);
			_output.store(path, dsc);
		}
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	//_output.save<JSON>(cerr);
	return true;
}

bool TmxControl::remove(pluginlist &plugins, ...)
{
	if (!checkPerm())
		return false;

	string query = add_constraint(IPLUGIN_DELETE, plugins);

	try
	{
		PLOG(logDEBUG1) << "Executing query " << query;

		std::string pwd = _pool.GetPwd();
		DbConnection conn = _pool.Connection("tcp://127.0.0.1:3306","IVP", pwd, "IVP");
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		for (size_t i = 0; i < plugins.size(); i++)
		{
			stmt->setString(i + 1, plugins[i]);
		}
		int rows = stmt->executeUpdate();
		PLOG(logDEBUG1) << "Deleted " << rows << " rows.";

		// Need to use the sub-query where clause
		query = add_subconstraint(PLUGIN_DELETE, plugins);

		PLOG(logDEBUG1) << "Executing query " << query;

		stmt.reset(conn.Get()->prepareStatement(query));
		for (size_t i = 0; i < plugins.size(); i++)
		{
			stmt->setString(i + 1, plugins[i]);
		}
		rows = stmt->executeUpdate();
		PLOG(logDEBUG1) << "Deleted " << rows << " rows.";

		return true;
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

}

// static std::string TmxControl::GetPwd(){
// 	const char* EnvVar = "MYSQL_ROOT_PASSWORD";
// 	const char* psw;
// 	psw = std::getenv(EnvVar);

// 	if(psw == NULL){
// 		PLOG(logERROR) << "Unable to set MYSQL_ROOT_PASSWORD)";
// 		return "";
// 	}
// 	else{
// 		std::string PswStr(psw);
// 		return PswStr;
// 	}
// }

} /* namespace tmxctl */
