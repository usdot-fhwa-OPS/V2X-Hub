/*
 * PluginStatus.cpp
 *
 *  Created on: Apr 24, 2017
 *      Author: gmb
 */

#include "TmxControl.h"
#include <array>
#include <algorithm> // For std::move

#include <database/DbConnectionConfig.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define LIST_QUERY "\
	SELECT IVP.plugin.id, name, description, version, coalesce(enabled, -1), \
	path, exeName, manifestName, \
	maxMessageInterval, commandLineParameters \
	FROM IVP.plugin \
	LEFT JOIN IVP.installedPlugin ON IVP.plugin.id = IVP.installedPlugin.pluginId"

#define STATE_QUERY "\
	SELECT name, `key`, value \
	FROM IVP.pluginStatus \
	LEFT JOIN IVP.plugin ON IVP.pluginStatus.pluginId = IVP.plugin.id"

#define MAX_UPDATE_STMT "\
	UPDATE IVP.installedPlugin \
	SET maxMessageInterval = ?"

#define ARGS_UPDATE_STMT "\
	UPDATE IVP.installedPlugin \
	SET commandLineParameters = ?"

#define MESSAGE_ACTIVITY_QUERY "\
	SELECT IVP.plugin.name, IVP.messageType.type, IVP.messageType.subtype, IVP.messageActivity.id, count, lastReceivedTimestamp, averageInterval \
	FROM IVP.messageActivity \
	INNER JOIN IVP.plugin ON IVP.messageActivity.pluginId = IVP.plugin.id \
	INNER JOIN IVP.messageType ON IVP.messageActivity.messageTypeId = IVP.messageType.id"

#define EVENT_LOG_QUERY "\
	SELECT IVP.eventLog.id, IVP.eventLog.logLevel, IVP.eventLog.source, IVP.eventLog.description, IVP.eventLog.timestamp \
	FROM IVP.eventLog"

#define EVENT_LOG_DELETE "\
	DELETE FROM IVP.eventLog"

#define USER_INFO_QUERY "\
	SELECT IVP.user.id, IVP.user.username, IVP.user.password, IVP.user.accessLevel \
	FROM IVP.user"

#define HASHED_USER_QUERY "\
	SELECT IVP.user.password \
	FROM IVP.user"

#define USER_UPDATE_QUERY "\
	UPDATE IVP.user \
	SET"

#define USER_ADD_QUERY "\
	INSERT INTO IVP.user \
	(username, password, accessLevel ) \
	SELECT ?, SHA2(?, 256), ? \
	FROM DUAL \
	WHERE NOT EXISTS ( \
	SELECT username FROM IVP.user \
	WHERE username = ? )"

#define USER_DELETE_QUERY "\
	DELETE FROM IVP.user"

#define SYSTEM_CONFIG_QUERY "\
	SELECT IVP.systemConfigurationParameter.key, IVP.systemConfigurationParameter.value, IVP.systemConfigurationParameter.defaultValue \
	FROM IVP.systemConfigurationParameter \
	ORDER BY IVP.systemConfigurationParameter.key"

#define INSTALLED_PLUGINS_QUERY "\
	SELECT IVP.plugin.name, IVP.plugin.version, IVP.plugin.description \
	FROM IVP.plugin \
	ORDER BY IVP.plugin.name"

using namespace std;
using namespace sql;
using namespace tmx;
using namespace tmx::utils;

namespace tmxctl {

// Helper function to get database connection using configuration
static DbConnection getConfiguredConnection(DbConnectionPool& pool) {
	auto& config = DbConnectionConfig::getInstance();
	std::string pwd = pool.GetPwd();
	return pool.Connection(config.getConnectionUrl(), config.getUser(), pwd, config.getDatabase());
}

bool TmxControl::list(pluginlist &plugins, ...)
{
	string query = add_constraint(LIST_QUERY, plugins, "IVP.plugin.id");

	try
	{
		PLOG(logDEBUG1) << "Executing query " << query;

		_output.get_storage().get_tree().clear();

		DbConnection conn = getConfiguredConnection(_pool);

		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		for (size_t i = 0; i < plugins.size(); i++)
		{
			stmt->setString(i + 1, plugins[i]);
		}
		unique_ptr<ResultSet> rs(stmt->executeQuery());

		while (rs->next())
		{
			int id = rs->getInt(1);
			string name = rs->getString(2).asStdString();
			string description = rs->getString(3).asStdString();
			string version = rs->getString(4).asStdString();
			int enabled = rs->getInt(5);
			string path = rs->getString(6).asStdString();
			string exe = rs->getString(7).asStdString();
			string manifest = rs->getString(8).asStdString();
			int maxInt = rs->getInt(9);
			string args = rs->getString(10).asStdString();

			PLOG(logDEBUG) << name << "," << description << "," << version << "," << enabled << "," << maxInt << "," << args;

			message_path_type keyPath(name, ATTRIBUTE_PATH_CHARACTER);
			message_path_type key;
			key = keyPath;
			key /= message_path_type("id", ATTRIBUTE_PATH_CHARACTER);
			_output.store(key, to_string(id));

			key = keyPath;
			key /= message_path_type("description", ATTRIBUTE_PATH_CHARACTER);
			_output.store(key, description);

			key = keyPath;
			key /= message_path_type("version", ATTRIBUTE_PATH_CHARACTER);
			_output.store(key, version);

			key = keyPath;
			key /= message_path_type("enabled", ATTRIBUTE_PATH_CHARACTER);
			_output.store(key, !enabled ? "Disabled" : enabled > 0 ? "Enabled" : "External");

			if (enabled < 0)
				continue;

			key = keyPath;
			key /= message_path_type("path", ATTRIBUTE_PATH_CHARACTER);
			_output.store(key, path);

			key = keyPath;
			key /= message_path_type("exeName", ATTRIBUTE_PATH_CHARACTER);
			_output.store(key, exe);

			key = keyPath;
			key /= message_path_type("manifest", ATTRIBUTE_PATH_CHARACTER);
			_output.store(key, manifest);

			key = keyPath;
			key /= message_path_type("maxMessageInterval", ATTRIBUTE_PATH_CHARACTER);
			_output.store(key, to_string(maxInt));

			key = keyPath;
			key /= message_path_type("commandLineParameters", ATTRIBUTE_PATH_CHARACTER);
			_output.store(key, args);
		}
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	return true;
}

bool TmxControl::state(pluginlist &plugins, ...)
{
	string query = add_constraint(STATE_QUERY, plugins);
	query += " AND `key` <> ''";
	query += " AND substr(name, 1, 8) <> 'ivpcore.'";

	try
	{
		PLOG(logDEBUG) << "Executing query " << query;

		_output.get_storage().get_tree().clear();
		DbConnection conn = getConfiguredConnection(_pool);
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

			message_path_type keyPath(name, ATTRIBUTE_PATH_CHARACTER);
			keyPath /= message_path_type(key, ATTRIBUTE_PATH_CHARACTER);

			_output.store(keyPath, val);
		}
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	return true;
}

bool TmxControl::max_message_interval(pluginlist &plugins, ...)
{
	if (!checkPerm())
		return false;

	string query = add_constraint(MAX_UPDATE_STMT, plugins);

	try
	{
		string val = (*_opts)["max-message-interval"].as<string>();

		PLOG(logDEBUG1) << "Executing query (?1 = " << val << ")" << query;

		DbConnection conn = getConfiguredConnection(_pool);
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		stmt->setString(1, val);
		for (size_t i = 0; i < plugins.size(); i++)
		{
			stmt->setString(i + 2, plugins[i]);
		}
		return stmt->execute();
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}
}

bool TmxControl::plugin_log_level(pluginlist &plugins, ...)
{
	string key = "LogLevel";
	string value = (*_opts)["plugin-log-level"].as<string>();

	boost::any k(key);
	boost::any val(value);
	(*_opts).insert(make_pair("key", boost::program_options::variable_value(k, false)));
	(*_opts).insert(make_pair("value", boost::program_options::variable_value(val, false)));

	return this->set(plugins);
}

bool TmxControl::plugin_log_output(pluginlist &plugins, ...)
{
	string key = "LogOutput";
	string value = (*_opts)["plugin-log-output"].as<string>();

	boost::any k(key);
	boost::any val(value);
	(*_opts).insert(make_pair("key", boost::program_options::variable_value(k, false)));
	(*_opts).insert(make_pair("value", boost::program_options::variable_value(val, false)));

	return this->set(plugins);
}

bool TmxControl::args(pluginlist &plugins, ...)
{
	if (!checkPerm())
		return false;

	string query = add_constraint(ARGS_UPDATE_STMT, plugins);
	string val = (*_opts)["args"].as<string>();

	try
	{
		PLOG(logDEBUG1) << "Executing query (?1 = " << val << ")" << query;

		DbConnection conn = getConfiguredConnection(_pool);
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		stmt->setString(1, val);
		for (size_t i = 0; i < plugins.size(); i++)
		{
			stmt->setString(i + 2, plugins[i]);
		}
		return stmt->execute();
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	return true;
}

bool TmxControl::messages(pluginlist &plugins, ...)
{
	string query = add_constraint(MESSAGE_ACTIVITY_QUERY, plugins);
	query += " ORDER BY IVP.plugin.name, IVP.messageType.type, IVP.messageType.subtype, IVP.messageActivity.id";

	try
	{
		PLOG(logDEBUG1) << "Executing query " << query;

		_output.get_storage().get_tree().clear();

		DbConnection conn = getConfiguredConnection(_pool);
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		for (size_t i = 0; i < plugins.size(); i++)
		{
			stmt->setString(i + 1, plugins[i]);
		}
		unique_ptr<ResultSet> rs(stmt->executeQuery());

		message payload;

		while (rs->next())
		{
			string name = rs->getString(1).asStdString();
			string type = rs->getString(2).asStdString();
			string subtype = rs->getString(3).asStdString();
			string id = rs->getString(4);
			string count = rs->getString(5);
			string time = rs->getString(6).asStdString();
			string interval = rs->getString(7);

			//PLOG(logDEBUG) << name << "," << type << "," << subtype << "," << id << "," << count << "," << time  << "," << interval;

			message_tree_type tmpTree;
			tmpTree.put("id", id);
			tmpTree.put("type", type);
			tmpTree.put("subtype", subtype);
			tmpTree.put("count", count);
			tmpTree.put("lastTimestamp", time);
			tmpTree.put("averageInterval", interval);

			message tmpSubTree;
			tmpSubTree.set_contents(tmpTree);

			payload.add_array_element(name, tmpSubTree);
		}
		_output = payload.get_container();
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	return true;
}

bool TmxControl::events(pluginlist &, ...)
{
	string query = EVENT_LOG_QUERY;
	bool haveEventTime = false;
	bool haveRowLimit = false;
	if (_opts->count("eventTime") > 0 && (*_opts)["eventTime"].as<string>() != "")
	{
		query += " WHERE IVP.eventLog.timestamp > ?";
		query += " ORDER BY IVP.eventLog.timestamp";
		haveEventTime = true;
	}
	else
	{
		if (_opts->count("rowLimit") > 0 && (*_opts)["rowLimit"].as<string>() != "" && boost::regex_match((*_opts)["rowLimit"].as<string>(), boost::regex("[0-9]+")))
		{
			query += " ORDER BY IVP.eventLog.timestamp DESC";
			query += " LIMIT ?";
			string innerQuery = query;
			query = "SELECT e.* FROM (";
			query += innerQuery;
			query += ") e ORDER BY e.timestamp ASC";
			haveRowLimit = true;
		}
		else
		{
			query += " ORDER BY IVP.eventLog.timestamp";
		}
	}

	try
	{
		PLOG(logDEBUG1) << "Executing query " << query;

		_output.get_storage().get_tree().clear();

		DbConnection conn = getConfiguredConnection(_pool);

		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));

		if (haveEventTime)
			stmt->setString(1, (*_opts)["eventTime"].as<string>());
		if (haveRowLimit)
			stmt->setInt(1, stoi((*_opts)["rowLimit"].as<string>()));

		unique_ptr<ResultSet> rs(stmt->executeQuery());

		message payload;

		while (rs->next())
		{
			string id = rs->getString(1).asStdString();
			string level = rs->getString(2).asStdString();
			string source = rs->getString(3).asStdString();
			string description = rs->getString(4);
			string timestamp = rs->getString(5);

			message_tree_type tmpTree;
			tmpTree.put("id", id);
			tmpTree.put("level", level);
			tmpTree.put("source", source);
			tmpTree.put("description", description);
			tmpTree.put("timestamp", timestamp);

			message tmpSubTree;
			tmpSubTree.set_contents(tmpTree);

			payload.add_array_element("EventLog", tmpSubTree);
		}
		_output = payload.get_container();
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	return true;
}

bool TmxControl::system_config(pluginlist &, ...)
{
	string query = SYSTEM_CONFIG_QUERY;

	try
	{
		PLOG(logDEBUG1) << "Executing query " << query;

		_output.get_storage().get_tree().clear();

		DbConnection conn = getConfiguredConnection(_pool);
		unique_ptr<Statement> stmt(conn.Get()->createStatement());
		unique_ptr<ResultSet> rs(stmt->executeQuery(query));

		message payload;

		while (rs->next())
		{
			string key = rs->getString(1);
			string value = rs->getString(2);
			string defaultValue = rs->getString(3);

			message_tree_type tmpTree;
			tmpTree.put("name", key);
			tmpTree.put("value", value);
			tmpTree.put("defaultValue", defaultValue);

			message tmpSubTree;
			tmpSubTree.set_contents(tmpTree);

			payload.add_array_element("SystemConfig", tmpSubTree);
		}
		_output = payload.get_container();
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	return true;
}

bool TmxControl::clear_event_log(pluginlist &, ...)
{
	string query = EVENT_LOG_DELETE;
	try
	{
		PLOG(logDEBUG1) << "Executing query " << query;

		_output.get_storage().get_tree().clear();

		DbConnection conn = getConfiguredConnection(_pool);
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		stmt->executeUpdate();
		//unique_ptr<Statement> stmt(conn.Get()->createStatement());
		//unique_ptr<ResultSet> rs(stmt->executeQuery(query));
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	return true;
}

bool TmxControl::user_info(pluginlist &, ...)
{
	if (!checkPerm())
		return false;
	return user_info();
}

bool TmxControl::user_info(bool showPassword)
{
	string query = USER_INFO_QUERY;
	if (_opts->count("username") == 0 || (*_opts)["username"].as<string>() == "")
		return false;
	query += " WHERE IVP.user.username = ?";

	try
	{
		PLOG(logDEBUG1) << "Executing query " << query;

		_output.get_storage().get_tree().clear();

		DbConnection conn = getConfiguredConnection(_pool);
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		stmt->setString(1, (*_opts)["username"].as<string>());
		unique_ptr<ResultSet> rs(stmt->executeQuery());


		message payload;

		while (rs->next())
		{
			string id = rs->getString(1).asStdString();
			string username = rs->getString(2);
			string password = rs->getString(3);
			string accessLevel = rs->getString(4).asStdString();

			message_tree_type tmpTree;
			tmpTree.put("id", id);
			tmpTree.put("username", username);
			if (showPassword)
				tmpTree.put("password", password);
			tmpTree.put("accessLevel", accessLevel);

			message tmpSubTree;
			tmpSubTree.set_contents(tmpTree);

			payload.add_array_element("UserInfo", tmpSubTree);
		}
		_output = payload.get_container();
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	return true;
}

bool TmxControl::hashed_info(pluginlist &, ...)
{
	if (!checkPerm())
		return false;
	return hashed_info();
}

bool TmxControl::hashed_info()
{
	string query = HASHED_USER_QUERY;
	if (_opts->count("password") == 0 || (*_opts)["password"].as<string>() == "")
		return false;
	query += " WHERE IVP.user.password = SHA2(?, 256)";

	try
	{
		PLOG(logDEBUG1) << "Executing query " << query;

		_output.get_storage().get_tree().clear();

		DbConnection conn = getConfiguredConnection(_pool);
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		stmt->setString(1, (*_opts)["password"].as<string>());
		unique_ptr<ResultSet> rs(stmt->executeQuery());


		message payload;

		while (rs->next())
		{
			string password = rs->getString(1);

			message_tree_type tmpTree;
			tmpTree.put("password", password);

			message tmpSubTree;
			tmpSubTree.set_contents(tmpTree);

			payload.add_array_element("UserInfo", tmpSubTree);
		}
		_output = payload.get_container();
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	return true;
}

bool TmxControl::all_users_info(pluginlist &, ...)
{
	if (!checkPerm())
		return false;
	return all_users_info();
}

bool TmxControl::all_users_info(bool showPassword)
{
	string query = USER_INFO_QUERY;

	try
	{
		PLOG(logDEBUG1) << "Executing query " << query;

		_output.get_storage().get_tree().clear();

		DbConnection conn = getConfiguredConnection(_pool);
		unique_ptr<Statement> stmt(conn.Get()->createStatement());
		unique_ptr<ResultSet> rs(stmt->executeQuery(query));

		message payload;

		while (rs->next())
		{
			string id = rs->getString(1).asStdString();
			string username = rs->getString(2);
			string password = rs->getString(3);
			string accessLevel = rs->getString(4).asStdString();

			message_tree_type tmpTree;
			tmpTree.put("id", id);
			tmpTree.put("username", username);
			if (showPassword)
				tmpTree.put("password", password);
			tmpTree.put("accessLevel", accessLevel);

			message tmpSubTree;
			tmpSubTree.set_contents(tmpTree);

			payload.add_array_element("UserInfo", tmpSubTree);
		}
		_output = payload.get_container();
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	return true;
}

bool TmxControl::user_add(pluginlist &plugins, ...)
{
	if (!checkPerm())
		return false;
	return user_add();
}

bool TmxControl::user_add()
{
	if (_opts->count("username") == 0 || (*_opts)["username"].as<string>() == "")
		return false;
	if (_opts->count("password") == 0)
		return false;
	if (_opts->count("access-level") == 0 || (*_opts)["access-level"].as<string>() == "" || !boost::regex_match((*_opts)["access-level"].as<string>(), boost::regex("[0-9]+")))
		return false;

    string username = (*_opts)["username"].as<string>();
    string password = (*_opts)["password"].as<string>();
    int access_level = stoi((*_opts)["access-level"].as<string>());

	PLOG(logDEBUG1) << "Setting " << username << " = " << password << ", " << access_level;

	try
	{
		string query = USER_ADD_QUERY;

		PLOG(logDEBUG1) << "Executing query (?1 = " << username << ", ?2 = " << password <<
				", ?3 = " << access_level << ", ?4 = " << username << "): " << query;

		DbConnection conn = getConfiguredConnection(_pool);
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		stmt.reset(conn.Get()->prepareStatement(query));
		stmt->setString(1, username);
		stmt->setString(2, password);
		stmt->setInt(3, access_level);
		stmt->setString(4, username);
		int inserted = stmt->executeUpdate();

		if (inserted > 0)
			return true;
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}


	return false;
}

bool TmxControl::user_update(pluginlist &plugins, ...)
{
	if (!checkPerm())
		return false;
	return user_update();
}

bool TmxControl::user_update()
{
	bool havePassword = false;
	bool haveAccess = false;
	string password = "";
	int access_level = -1;
	if (_opts->count("username") == 0 || (*_opts)["username"].as<string>() == "")
		return false;
	if ((_opts->count("password") == 0) && (_opts->count("access-level") == 0 || (*_opts)["access-level"].as<string>() == "" || !boost::regex_match((*_opts)["access-level"].as<string>(), boost::regex("[0-9]+"))))
		return false;
    string username = (*_opts)["username"].as<string>();
	if (_opts->count("password") != 0)
	{
		havePassword = true;
		password = (*_opts)["password"].as<string>();
	}
	if (_opts->count("access-level") != 0 && (*_opts)["access-level"].as<string>() != "" && boost::regex_match((*_opts)["access-level"].as<string>(), boost::regex("[0-9]+")))
	{
		haveAccess = true;
		access_level = stoi((*_opts)["access-level"].as<string>());
	}

	PLOG(logDEBUG1) << "Setting " << username << " = " << password << ", " << access_level;

	try
	{
		string query = USER_UPDATE_QUERY;
		if (havePassword)
			query += "  password = SHA2(?, 256)";
		if (haveAccess)
		{
			if (havePassword)
				query += ",  accessLevel = ?";
			else
				query += "  accessLevel = ?";
		}
		query += " WHERE username = ?";
		PLOG(logDEBUG1) << "Executing query : " << query;

		DbConnection conn = getConfiguredConnection(_pool);
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		if (havePassword)
		{
			stmt->setString(1, password);
			if (haveAccess)
			{
				stmt->setInt(2, access_level);
				stmt->setString(3, username);
			}
			else
			{
				stmt->setString(2, username);
			}
		}
		else
		{
			stmt->setInt(1, access_level);
			stmt->setString(2, username);
		}
		int updated = stmt->executeUpdate();

		if (updated > 0)
			return true;
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}


	return false;
}

bool TmxControl::user_delete(pluginlist &, ...)
{
	if (!checkPerm())
		return false;
	return user_delete();
}

bool TmxControl::user_delete()
{
	string query = USER_DELETE_QUERY;
	if (_opts->count("username") == 0 || (*_opts)["username"].as<string>() == "")
		return false;
	query += " WHERE IVP.user.username = ?";

	try
	{
		PLOG(logDEBUG1) << "Executing query " << query;

		_output.get_storage().get_tree().clear();

		DbConnection conn = getConfiguredConnection(_pool);
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		stmt->setString(1, (*_opts)["username"].as<string>());
		int deleted = stmt->executeUpdate();

		if (deleted > 0)
			return true;
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	return false;
}
bool validate_passphrase(const std::string &passphrase ) {
	if (passphrase.empty())
	{
		throw TmxException("Passphrase is empty!");
	}

	// Validate passphrase - reject dangerous shell metacharacters
	if (passphrase.find_first_of(R"(;|&<>`$()*?")") != std::string::npos)
	{
		throw TmxException(R"(Passphrase includes illegal characters ;|&<>`$()*?")");
	}
	return true;
}

bool validate_filepath(const std::string &filepath) {
	 std::ifstream test(filepath);
	if (!test.good())
	{
		throw TmxException("File does not exist: " + filepath);
	}
	test.close();

	// Reject unencrypted files — must end with .sql.gz.enc
	if (filepath.size() < 11 ||
		filepath.substr(filepath.size() - 11) != ".sql.gz.enc")
	{
		throw TmxException("File does not have appropriate file extension *.sql.gz.enc");
	}

	// Validate filePath - reject dangerous shell metacharacters
	if (filepath.find_first_of(R"(;|&<>`$()*?")") != std::string::npos)
	{
		throw TmxException(R"(File has illegal characters ;|&<>`$()*?")");
	}
	return true;
}

bool TmxControl::save_state([[maybe_unused]] pluginlist &plugins, ...)
{
	if (!_opts || !_opts->count("passphrase"))
    {
        FILE_LOG(logERROR) << "Missing required argument: --passphrase <value>";
        return false;
    }

    std::string passphrase = (*_opts)["passphrase"].as<std::string>();

    return save_state(passphrase);
}

// bool spawn_posix_process(const std::string &process, posix_spawn_file_actions_t &fileActions, const std::vector<std::string> &processArgs) {
// 	processArgs.size();
// 	std::array<std::string, processArgs.size()> arr;

// }

bool TmxControl::save_state(const std::string &passphrase)
{
    try
    {
		const tmx::utils::DbConnectionConfig& dbConfig = tmx::utils::DbConnectionConfig::getInstance();

        std::string user = dbConfig.getUser();
        std::string password = dbConfig.getPassword(); 
        std::string host = dbConfig.getHost();
        std::string dbname = dbConfig.getDatabase();
		// Input validation included protection against command injection
		// Throws TmxExeption on passphrase validation failure		
		validate_passphrase(passphrase);
        std::string backupFile = "/var/www/download/v2x_hub_state_" + std::to_string(std::time(nullptr)) + ".sql.gz.enc";

        // Use posix_spawn for safer execution without shell interpretation
        pid_t pid;
        posix_spawn_file_actions_t fileActions;
        posix_spawn_file_actions_init(&fileActions);

        // Create pipes for piping mysqldump | gzip | openssl
        int pipe1[2], pipe2[2];
        if (pipe(pipe1) == -1 || pipe(pipe2) == -1)
        {
            PLOG(logERROR) << "Failed to create pipes";
            return false;
        }

        // First child: mysqldump
        posix_spawn_file_actions_adddup2(&fileActions, pipe1[1], STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&fileActions, pipe1[0]);
        posix_spawn_file_actions_addclose(&fileActions, pipe2[0]);
        posix_spawn_file_actions_addclose(&fileActions, pipe2[1]);

        std::array<std::string, 13> mysqldumpArgStorage = {
			"--no-defaults",
            "-u", user,
            "--password=" + password,
            "-h", host,
            dbname,
            "--no-tablespaces",
            ("--ignore-table=" + dbname + ".eventLog"),
            ("--ignore-table=" + dbname + ".messageActivity"),
            ("--ignore-table=" + dbname + ".messageType"),
            ("--ignore-table=" + dbname + ".pluginActivity"),
            ("--ignore-table=" + dbname + ".user")
        };
        std::array<char*, 14> mysqldumpArgs;
        for (size_t i = 0; i < mysqldumpArgStorage.size(); ++i) {
            mysqldumpArgs[i] = mysqldumpArgStorage[i].data();
        }
        mysqldumpArgs[13] = nullptr;

        int ret = posix_spawnp(&pid, "mysqldump", &fileActions, nullptr, mysqldumpArgs.data(), environ);
        if (ret != 0)
        {
            PLOG(logERROR) << "Failed to spawn mysqldump: " << strerror(ret);
            posix_spawn_file_actions_destroy(&fileActions);
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);
            return false;
        }
		int status = 0;

		if (waitpid(pid, &status, 0) == -1) {
			
			PLOG(logERROR) << "Failed to wait for mysqldump process: " << strerror(errno);
			posix_spawn_file_actions_destroy(&fileActions);
			close(pipe1[0]);
			close(pipe1[1]);
			close(pipe2[0]);
			close(pipe2[1]);
			return false;
		}

		// 3. Inspect the exit status of mysqldump
		if (WIFEXITED(status)) {
			int exit_code = WEXITSTATUS(status);
			if (exit_code != 0) {
				PLOG(logERROR) << "Database backup failed with exit code: " << exit_code;
				posix_spawn_file_actions_destroy(&fileActions);
				close(pipe1[0]);
				close(pipe1[1]);
				close(pipe2[0]);
				close(pipe2[1]);
				return false;
			} else {
				PLOG(logINFO) << "Database backup completed successfully.";
			}
		} 
		else if (WIFSIGNALED(status)) {
			PLOG(logERROR) << "Database backup was killed by signal: " << WTERMSIG(status);
			posix_spawn_file_actions_destroy(&fileActions);
			close(pipe1[0]);
			close(pipe1[1]);
			close(pipe2[0]);
			close(pipe2[1]);
			return false;
		}


        posix_spawn_file_actions_destroy(&fileActions);
		// Close the write end of the first pipe in the parent process
        close(pipe1[1]);

        // Second child: gzip
        posix_spawn_file_actions_init(&fileActions);
        posix_spawn_file_actions_adddup2(&fileActions, pipe1[0], STDIN_FILENO);
        posix_spawn_file_actions_adddup2(&fileActions, pipe2[1], STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&fileActions, pipe1[0]);
        posix_spawn_file_actions_addclose(&fileActions, pipe2[0]);
        posix_spawn_file_actions_addclose(&fileActions, pipe2[1]);

        pid_t gzipPid;
        std::array<std::string, 1> gzipArgStorage = { "gzip" };
        std::array<char*, 2> gzipArgs = { gzipArgStorage[0].data(), nullptr };
        ret = posix_spawnp(&gzipPid, "gzip", &fileActions, nullptr, gzipArgs.data(), environ);
        if (ret != 0)
        {
            PLOG(logERROR) << "Failed to spawn gzip: " << strerror(ret);
            posix_spawn_file_actions_destroy(&fileActions);
            close(pipe1[0]);
            close(pipe2[0]);
            close(pipe2[1]);
            return false;
        }
		// Wait for GZip Command 
		if (waitpid(gzipPid, &status, 0) == -1) {
			
			PLOG(logERROR) << "Failed to wait for Gzip process: " << strerror(errno);
			posix_spawn_file_actions_destroy(&fileActions);
			close(pipe1[0]);
			close(pipe1[1]);
			close(pipe2[0]);
			close(pipe2[1]);
			return false;
		}

		// 3. Inspect the exit status of mysqldump
		if (WIFEXITED(status)) {
			int exit_code = WEXITSTATUS(status);
			if (exit_code != 0) {
				PLOG(logERROR) << "Database backup compression failed with exit code: " << exit_code;
				posix_spawn_file_actions_destroy(&fileActions);
				close(pipe1[0]);
				close(pipe1[1]);
				close(pipe2[0]);
				close(pipe2[1]);
				return false;
			} else {
				PLOG(logINFO) << "Database backup completed successfully.";
			}
		} 
		else if (WIFSIGNALED(status)) {
			PLOG(logERROR) << "Database backup compression was killed by signal: " << WTERMSIG(status);
			posix_spawn_file_actions_destroy(&fileActions);
			close(pipe1[0]);
			close(pipe1[1]);
			close(pipe2[0]);
			close(pipe2[1]);
			return false;
		}

        posix_spawn_file_actions_destroy(&fileActions);
        close(pipe1[0]);
        close(pipe2[1]);

        // Third child: openssl enc
        posix_spawn_file_actions_init(&fileActions);
        posix_spawn_file_actions_adddup2(&fileActions, pipe2[0], STDIN_FILENO);
        posix_spawn_file_actions_addclose(&fileActions, pipe2[0]);

        int outFd = open(backupFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (outFd == -1)
        {
            PLOG(logERROR) << "Failed to open output file: " << backupFile;
            posix_spawn_file_actions_destroy(&fileActions);
            close(pipe2[0]);
            return false;
        }
        posix_spawn_file_actions_adddup2(&fileActions, outFd, STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&fileActions, outFd);

        pid_t opensslPid;
        std::string passArg = "pass:" + passphrase;
        std::array<std::string, 6> opensslArgStorage = {
            "enc",
            "-aes-256-cbc",
            "-salt",
            "-pbkdf2",
            "-pass",
            passArg
        };
        std::array<char*, 7> opensslArgs = {
            opensslArgStorage[0].data(),
            opensslArgStorage[1].data(),
            opensslArgStorage[2].data(),
            opensslArgStorage[3].data(),
            opensslArgStorage[4].data(),
            opensslArgStorage[5].data(),
            nullptr
        };
        ret = posix_spawnp(&opensslPid, "openssl", &fileActions, nullptr, opensslArgs.data(), environ);
        if (ret != 0)
        {
            PLOG(logERROR) << "Failed to spawn openssl: " << strerror(ret);
            posix_spawn_file_actions_destroy(&fileActions);
            close(pipe2[0]);
            close(outFd);
            return false;
        }
		// Wait for openssl Command 
		if (waitpid(opensslPid, &status, 0) == -1) {
			
			PLOG(logERROR) << "Failed to wait for openssl process: " << strerror(errno);
			posix_spawn_file_actions_destroy(&fileActions);
			close(pipe1[0]);
			close(pipe1[1]);
			close(pipe2[0]);
			close(pipe2[1]);
			return false;
		}

		// 3. Inspect the exit status of mysqldump
		if (WIFEXITED(status)) {
			int exit_code = WEXITSTATUS(status);
			if (exit_code != 0) {
				PLOG(logERROR) << "Database backup encryption failed with exit code: " << exit_code;
				posix_spawn_file_actions_destroy(&fileActions);
				close(pipe1[0]);
				close(pipe1[1]);
				close(pipe2[0]);
				close(pipe2[1]);
				return false;
			} else {
				PLOG(logINFO) << "Database backup encryption completed successfully.";
			}
		} 
		else if (WIFSIGNALED(status)) {
			PLOG(logERROR) << "Database backup encryption was killed by signal: " << WTERMSIG(status);
			posix_spawn_file_actions_destroy(&fileActions);
			close(pipe1[0]);
			close(pipe1[1]);
			close(pipe2[0]);
			close(pipe2[1]);
			return false;
		}

        posix_spawn_file_actions_destroy(&fileActions);
        close(pipe2[0]);
        close(outFd);


		_output.get_storage().get_tree().clear();
		message payload;
		message_tree_type tree;
		tree.put("file", backupFile);	
		payload.set_contents(tree);
		_output = payload.get_container();

		PLOG(logDEBUG) << "Encrypted database backup written to " << backupFile;
		return true;
    }
	catch (const boost::property_tree::ptree_error &ex) {
		PLOG(logERROR) << "Configuration/Tree error: " << ex.what();
		return false;
	}
	catch (const std::system_error &ex) {
		PLOG(logERROR) << "System/OS error during backup: " << ex.what();
		return false;
	}
    catch (const std::bad_alloc &ex)
    {
        PLOG(logERROR) << "Memory allocation failed during backup: " << ex.what();
        return false;
    }
	catch (const TmxException &ex) {
		PLOG(logERROR) << "Input validation failed : " << ex.what();
		return false;
	}
}

bool TmxControl::upload_state([[maybe_unused]] pluginlist &plugins, ...)
{
    if (!_opts || !_opts->count("upload-state"))
	{
		FILE_LOG(logERROR) << "Missing required argument: --upload-state <filePath>";
		return false;
	}

	std::string filePath = (*_opts)["upload-state"].as<std::string>();
    if (!_opts->count("passphrase"))
    {
        FILE_LOG(logERROR) << "Missing required argument: --passphrase <value>";
        return false;
    }

    std::string passphrase = (*_opts)["passphrase"].as<std::string>();


    return upload_state(filePath, passphrase);
}

bool TmxControl::upload_state(const std::string &filePath, const std::string &passphrase)
{
    if (!checkPerm())
        return false;

    try
    {
		// Input validation included protection against command injection
		// Throws TmxExeption on passphrase validation failure
		validate_passphrase(passphrase);
		// Throws TmxException on filepath validation failure 
		validate_filepath(filePath);

        const auto &dbConfig = tmx::utils::DbConnectionConfig::getInstance();

        // Use posix_spawn for safer execution without shell interpretation
        posix_spawn_file_actions_t fileActions;
        posix_spawn_file_actions_init(&fileActions);

        // Create pipes for piping openssl | gunzip | mysql
        int pipe1[2], pipe2[2];
        if (pipe(pipe1) == -1 || pipe(pipe2) == -1)
        {
            PLOG(logERROR) << "Failed to create pipes";
            return false;
        }

        // First child: openssl dec
        posix_spawn_file_actions_adddup2(&fileActions, pipe1[1], STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&fileActions, pipe1[0]);
        posix_spawn_file_actions_addclose(&fileActions, pipe2[0]);
        posix_spawn_file_actions_addclose(&fileActions, pipe2[1]);

        pid_t opensslPid;
		std::array<std::string, 8> opensslArgsStorage = {
            "enc",
            "-d",
            "-aes-256-cbc",
            "-pbkdf2",
            "-in",
            filePath,
            "-pass",
            "pass:" + passphrase,
        };
		 std::array<char*, 9> opensslArgs = {
            opensslArgsStorage[0].data(),
            opensslArgsStorage[1].data(),
            opensslArgsStorage[2].data(),
            opensslArgsStorage[3].data(),
            opensslArgsStorage[4].data(),
            opensslArgsStorage[5].data(),
            opensslArgsStorage[6].data(),
            opensslArgsStorage[7].data(),
            nullptr
        };
		int status = 0;

		int ret = posix_spawnp(&opensslPid, "openssl", &fileActions, nullptr, opensslArgs.data(), environ);
        if (ret != 0)
        {
            PLOG(logERROR) << "Failed to spawn openssl: " << strerror(ret);
            posix_spawn_file_actions_destroy(&fileActions);
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);
            return false;
        }
		// Wait for openssl Command 
		if (waitpid(opensslPid, &status, 0) == -1) {
			
			PLOG(logERROR) << "Failed to wait for openssl process: " << strerror(errno);
			posix_spawn_file_actions_destroy(&fileActions);
			close(pipe1[0]);
			close(pipe1[1]);
			close(pipe2[0]);
			close(pipe2[1]);
			return false;
		}

		// 3. Inspect the exit status of openssl
		if (WIFEXITED(status)) {
			int exit_code = WEXITSTATUS(status);
			if (exit_code != 0) {
				PLOG(logERROR) << "Database backup decryption failed with exit code: " << exit_code;
				posix_spawn_file_actions_destroy(&fileActions);
				close(pipe1[0]);
				close(pipe1[1]);
				close(pipe2[0]);
				close(pipe2[1]);
				return false;
			} else {
				PLOG(logINFO) << "Database backup decryption completed successfully.";
			}
		} 
		else if (WIFSIGNALED(status)) {
			PLOG(logERROR) << "Database backup decryption was killed by signal: " << WTERMSIG(status);
			posix_spawn_file_actions_destroy(&fileActions);
			close(pipe1[0]);
			close(pipe1[1]);
			close(pipe2[0]);
			close(pipe2[1]);
			return false;
		}

        posix_spawn_file_actions_destroy(&fileActions);
        close(pipe1[1]);

        // Second child: gunzip
        posix_spawn_file_actions_init(&fileActions);
        posix_spawn_file_actions_adddup2(&fileActions, pipe1[0], STDIN_FILENO);
        posix_spawn_file_actions_adddup2(&fileActions, pipe2[1], STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&fileActions, pipe1[0]);
        posix_spawn_file_actions_addclose(&fileActions, pipe2[0]);
        posix_spawn_file_actions_addclose(&fileActions, pipe2[1]);

        pid_t gzipPid;
		std::array<char*, 1> gzipArgs = {
			nullptr
		};
        ret = posix_spawnp(&gzipPid, "gunzip", &fileActions, nullptr, gzipArgs.data(), environ);
        if (ret != 0)
        {
            PLOG(logERROR) << "Failed to spawn gunzip: " << strerror(ret);
            posix_spawn_file_actions_destroy(&fileActions);
            close(pipe1[0]);
            close(pipe2[0]);
            close(pipe2[1]);
            return false;
        }
		// Wait for gzipPid Command 
		if (waitpid(gzipPid, &status, 0) == -1) {
			
			PLOG(logERROR) << "Failed to wait for gzip process: " << strerror(errno);
			posix_spawn_file_actions_destroy(&fileActions);
			close(pipe1[0]);
			close(pipe1[1]);
			close(pipe2[0]);
			close(pipe2[1]);
			return false;
		}

		// 3. Inspect the exit status of gzip
		if (WIFEXITED(status)) {
			int exit_code = WEXITSTATUS(status);
			if (exit_code != 0) {
				PLOG(logERROR) << "Database backup decompression failed with exit code: " << exit_code;
				posix_spawn_file_actions_destroy(&fileActions);
				close(pipe1[0]);
				close(pipe1[1]);
				close(pipe2[0]);
				close(pipe2[1]);
				return false;
			} else {
				PLOG(logINFO) << "Database backup decompression completed successfully.";
			}
		} 
		else if (WIFSIGNALED(status)) {
			PLOG(logERROR) << "Database backup decompression was killed by signal: " << WTERMSIG(status);
			posix_spawn_file_actions_destroy(&fileActions);
			close(pipe1[0]);
			close(pipe1[1]);
			close(pipe2[0]);
			close(pipe2[1]);
			return false;
		}

        posix_spawn_file_actions_destroy(&fileActions);
        close(pipe1[0]);
        close(pipe2[1]);

        // Third child: mysql
        posix_spawn_file_actions_init(&fileActions);
        posix_spawn_file_actions_adddup2(&fileActions, pipe2[0], STDIN_FILENO);
        posix_spawn_file_actions_addclose(&fileActions, pipe2[0]);

        pid_t mysqlPid;
		std::array<std::string, 7> mysqlArgsStorage = {
        	"--no-defaults",
			"-u",
            dbConfig.getUser(),
            ("--password=" + dbConfig.getPassword()),
            "-h",
            dbConfig.getHost(),
           	dbConfig.getDatabase()
        };
		std::array<char*, 8> mysqlArgs = {
			mysqlArgsStorage[0].data(),
			mysqlArgsStorage[1].data(),
			mysqlArgsStorage[2].data(),
			mysqlArgsStorage[3].data(),
			mysqlArgsStorage[4].data(),
			mysqlArgsStorage[5].data(),
			mysqlArgsStorage[6].data(),
			nullptr
		};
		ret = posix_spawnp(&mysqlPid, "mysql", &fileActions, nullptr, mysqlArgs.data(), environ);
        if (ret != 0)
        {
            PLOG(logERROR) << "Failed to spawn mysql: " << strerror(ret);
            posix_spawn_file_actions_destroy(&fileActions);
            close(pipe2[0]);
            return false;
        }
		// Wait for mysql Command 
		if (waitpid(mysqlPid, &status, 0) == -1) {
			
			PLOG(logERROR) << "Failed to wait for mysql process: " << strerror(errno);
			posix_spawn_file_actions_destroy(&fileActions);
			close(pipe1[0]);
			close(pipe1[1]);
			close(pipe2[0]);
			close(pipe2[1]);
			return false;
		}

		// 3. Inspect the exit status of mysql
		if (WIFEXITED(status)) {
			int exit_code = WEXITSTATUS(status);
			if (exit_code != 0) {
				PLOG(logERROR) << "Database backup upload failed with exit code: " << exit_code;
				posix_spawn_file_actions_destroy(&fileActions);
				close(pipe1[0]);
				close(pipe1[1]);
				close(pipe2[0]);
				close(pipe2[1]);
				return false;
			} else {
				PLOG(logINFO) << "Database backup upload completed successfully.";
			}
		} 
		else if (WIFSIGNALED(status)) {
			PLOG(logERROR) << "Database backup upload was killed by signal: " << WTERMSIG(status);
			posix_spawn_file_actions_destroy(&fileActions);
			close(pipe1[0]);
			close(pipe1[1]);
			close(pipe2[0]);
			close(pipe2[1]);
			return false;
		}

        posix_spawn_file_actions_destroy(&fileActions);
        close(pipe2[0]);

        FILE_LOG(logDEBUG) << "Database restore successful from file: " << filePath;
        return true;
    }
    catch (const std::ios_base::failure &e)
	{
		FILE_LOG(logERROR) << "File I/O error: " << e.what();
		return false;
	}
	catch (const std::bad_alloc &e)
	{
		FILE_LOG(logERROR) << "Memory allocation failed: " << e.what();
		return false;
	}
	catch (const TmxException &ex) {
		PLOG(logERROR) << "Input validation failed : " << ex.what();
		return false;
	}
}
} /* namespace tmxctl */
