/*
 * PluginControl.cpp
 *
 *  Created on: Apr 24, 2017
 *      @author: gmb
 */

#include "TmxControl.h"
#include <cstdarg>
#include <sys/wait.h>
#include <sys/stat.h>

constexpr const char * PLUGIN = "plugin";

constexpr const char * ENABLE_STMT = "\
	UPDATE IVP.installedPlugin \
	SET enabled = ?";

constexpr const char * EXE_QUERY = "\
	SELECT path, exeName \
	FROM IVP.installedPlugin";

constexpr const char * STATUS_QUERY = "\
	SELECT name, value \
	FROM IVP.pluginStatus \
	LEFT JOIN IVP.plugin ON IVP.pluginStatus.pluginId = IVP.plugin.id";

constexpr const char * PROC_SCRIPT = "\
	ls -L -i /proc/[0-9][0-9]*/exe 2>/dev/null | \
	awk -F\\/ '$1 = /?/{print $3}'";

using namespace std;
using namespace sql;
using namespace tmx;
using namespace tmx::utils;

namespace tmxctl {

bool TmxControl::enable(pluginlist &plugins, ...)
{
	if (!checkPerm())
		return false;

	string query = add_constraint(ENABLE_STMT, plugins);

	try
	{
		PLOG(logDEBUG1) << "Executing query (?1 = 1): " << query;

		std::string pwd = _pool.GetPwd();
		DbConnection conn = _pool.Connection("tcp://127.0.0.1:3306","IVP", pwd, "IVP");
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		stmt->setUInt(1, 1);
		for (size_t i = 0; i < plugins.size(); i++)
		{
			stmt->setString(i + 2, plugins[i]);
		}
		int rows = stmt->executeUpdate();
		PLOG(logDEBUG1) << "Updated " << rows << " rows.";
		return rows > 0;
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}
}

bool TmxControl::disable(pluginlist &plugins, ...)
{
	if (!checkPerm())
		return false;

	string query = add_constraint(ENABLE_STMT, plugins);

	try
	{
		PLOG(logDEBUG1) << "Executing query (?1 = 0): " << query;

		std::string pwd = _pool.GetPwd();
		DbConnection conn = _pool.Connection("tcp://127.0.0.1:3306","IVP", pwd, "IVP");
		unique_ptr<PreparedStatement> stmt(conn.Get()->prepareStatement(query));
		stmt->setUInt(1, 0);
		for (size_t i = 0; i < plugins.size(); i++)
		{
			stmt->setString(i + 2, plugins[i]);
		}
		int rows = stmt->executeUpdate();
		PLOG(logDEBUG1) << "Updated " << rows << " rows.";
		return rows > 0;
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}
}

bool TmxControl::start(pluginlist &plugins, ...)
{
	if (!checkPerm())
		return false;

	string query = add_constraint(EXE_QUERY, plugins);

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
		unique_ptr<ResultSet> rs(stmt->executeQuery());

		while (rs->next())
		{
			string dir = rs->getString(1).asStdString();
			string exe = rs->getString(2).asStdString();

			PLOG(logDEBUG1) << "Changing directory to " << dir;
			if (chdir(dir.c_str()) < 0)
			{
				cerr << strerror(errno) << endl;
				continue;
			}

			if (exe.length() <= 0)
			{
				cerr << "No binary name" << endl;
				continue;
			}

			if (exe[0] != '/')
				exe.insert(exe.begin(), '/');

			exe.insert(exe.begin(), '.');

			PLOG(logDEBUG1) << "Running '" << exe << "'";

			int pid = fork();

			// su to plugin user
			if (pid == 0)
			{
				// Child process
				int ret = 0;

				ret = execl(exe.c_str(), exe.c_str(), NULL);

				if (ret < 0)
					cerr << strerror(errno) << endl;

				return ret;
			}
		}
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	sleep(1);
	return status(plugins);
}

bool TmxControl::stop(pluginlist &plugins, ...)
{
	if (!checkPerm())
		return false;

	string query = add_constraint(EXE_QUERY, plugins);

	struct stat fStat;

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
		unique_ptr<ResultSet> rs(stmt->executeQuery());

		while (rs->next())
		{
			string dir = rs->getString(1).asStdString();
			string exe = rs->getString(2).asStdString();

			if (exe.length() > 0 && exe[0] == '/')
				exe.erase(0, 1);

			string path = dir + "/" + exe;

			if (::stat(path.c_str(), &fStat) < 0)
			{
				cerr << strerror(errno) << endl;
				continue;
			}

			string script = PROC_SCRIPT;
			script.replace(strlen(PROC_SCRIPT) - 13, 1, to_string(fStat.st_ino));

			script += " | xargs -L1 kill 2>/dev/null";

			PLOG(logDEBUG1) << "Executing script " << script;

			if (::system(script.c_str()) < 0)
				cerr << strerror(errno) << endl;
		}
	}
	catch (exception &ex)
	{
		PLOG(logERROR) << TmxException(ex);
		return false;
	}

	sleep(1);
	return status(plugins);
}

bool TmxControl::status(pluginlist &plugins, ...)
{
	string query = add_constraint(STATUS_QUERY, plugins);
	query += " AND `key` = ''";
	query += " AND substr(name, 1, 8) <> 'ivpcore.'";

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
			string val = rs->getString(2).asStdString();
			message_path_type keyPath(name, ATTRIBUTE_PATH_CHARACTER);
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

} /* namespace tmxctl */
