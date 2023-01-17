/*
 * TmxControl.cpp
 *
 *  Created on: Apr 24, 2017
 *      Author: gmb
 */

#include "TmxControl.h"

#include <grp.h>
#include <functional>
#include <unistd.h>
#include <vector>


using namespace std;
using namespace boost::program_options;
using namespace tmx;
using namespace tmx::utils;

using std::placeholders::_1;

namespace tmxctl {

#if BOOST_VERSION < 105800
typedef typename std::string::value_type xml_settings_type;
#else
typedef std::string xml_settings_type;
#endif

typedef bool (TmxControl::*func_type)(TmxControl::pluginlist &, ...);

static vector<std::pair<std::string, func_type> > fnRegistry;

template <typename T = std::string>
void registerFunction(string name, const char *aliases, const char *descr,
					  func_type func, boost::program_options::typed_value<T> *argVal = 0)
{
	fnRegistry.push_back(make_pair(name, func));

	if (aliases != NULL)
	{
		name += ',';
		name += aliases;
	}

	string optName = name;
	for (size_t i = 0; i < optName.size(); i++)
	{
		if (optName[i] == '_')
			optName[i] = '-';
	}

	if (!argVal)
	{
		AddOptions()(optName.c_str(), descr);
	}
	else
	{
		AddOptions()(optName.c_str(), argVal, descr);
	}
}

TmxControl::TmxControl(): Runnable("plugin", "The plugin to control"), _opts(NULL)
{
	// Register the available functions as options
#define REG_FN(X, Y, Z) registerFunction(#X, Y, Z, &TmxControl::X);
#define REG_FN_ARG(X, Y, Z, T) registerFunction(#X, Y, Z, &TmxControl::X, \
		boost::program_options::value<T>())

	REG_FN(list, NULL, "List the plugin information");
	REG_FN(enable, "e", "Enable the plugin for automatic startup");
	REG_FN(disable, "d", "Disable the plugin for automatic startup");
	REG_FN(start, NULL, "Start the plugin immediately");
	REG_FN(stop, NULL, "Stop a running plugin.  If enabled, it will restart automatically");
	REG_FN(status, NULL, "Return the current running status of the plugin");
	REG_FN(config, NULL, "Return the current configuration parameters");
	REG_FN(state, NULL, "Return the current state, i.e. the status values, of the plugin");
	REG_FN(set, NULL, "Set a configuration value.  Must also set --key and --value.");
	REG_FN(reset, NULL, "Reset a configuration value to its default.  Must also set --key.");
	REG_FN(remove, NULL, "Remove a plugin from the database.");
	REG_FN(messages, NULL, "Show plugin message activity.");
	REG_FN(events, NULL, "Show event log.");
	REG_FN(clear_event_log, NULL, "Clear out event log in database.");
	REG_FN(system_config, NULL, "Return the current system configuration parameters.");
	REG_FN(set_system, NULL, "Set a system configuration value.  Must also set --key and --value.");
	REG_FN(user_info, NULL, "Display user information for a user. Must set --username.");
	REG_FN(all_users_info, NULL, "Display user information for all users.");
	REG_FN(user_add, NULL, "Add a TMX user. Must set --username, --password, and --access-level.");
	REG_FN(user_update, NULL, "Update a TMX users info. Must set --username, --password, and --access-level.");
	REG_FN(user_delete, NULL, "Delete a TMX user.");

	// These have arguments
	REG_FN_ARG(max_message_interval, "M", "Set the max message interval for the plugin", std::string);
	REG_FN_ARG(plugin_log_level, "L", "Set the log level for a running plugin", std::string);
	REG_FN_ARG(plugin_log_output, "O", "Redirect the logging of a running plugin to the specified file", std::string);
	REG_FN_ARG(args, "a", "Set the command line arguments for the plugin", std::string);
	REG_FN_ARG(load_manifest, "m", "(Re-)load the plugin manifest to the database", std::string);
	REG_FN_ARG(plugin_install, NULL, "Decompress and install the specified plugin install file on this system.", std::string);
	REG_FN_ARG(plugin_remove, NULL, "Delete the specified plugin on this system.  No wildcards accepted.", std::string);

#undef REG_FN_ARG
#undef REG_FN

	// Other program options
	AddOptions()
			("json,j","Format any output in JSON.  Default is false")
			("xml,x","Format any output in XML.  Default is false")
			("no-pretty-print", "Do not pretty print the output.  Default is false")
			("key",
				boost::program_options::value<string>(),
				"The parameter key name")
			("value",
				boost::program_options::value<string>(),
				"The parameter value")
			("defaultValue",
				boost::program_options::value<string>()->default_value(""),
				"The parameter default value for insert.  Defaults to ''")
			("description",
				boost::program_options::value<string>()->default_value("Added by tmxctl"),
				"The parameter description for insert.  Defaults to 'Added by tmxctl'")
			("host,h",
				boost::program_options::value<string>()->default_value("127.0.0.1"),
				"The MySQL DB host, if different than the default localhost")
			("port,p",
				boost::program_options::value<string>()->default_value("3306"),
				"The MySQL DB port, if different than the default 3306")
			("plugin-directory,d",
				boost::program_options::value<string>()->default_value(DEFAULT_PLUGINDIRECTORY),
				"Directory to find the plugins")
			("eventTime",
				boost::program_options::value<string>()->default_value(""),
				"Event log entries greater than this time returned")
			("username",
				boost::program_options::value<string>(),
				"A TMX system user")
			("password",
				boost::program_options::value<string>(),
				"A TMX system users password")
			("access-level",
				boost::program_options::value<string>(),
				"A TMX system users access level. 1 = ReadOnly, 2 = ApplicationAdministrator, 3 = SystemAdministrator")
			("rowLimit",
				boost::program_options::value<string>(),
				"Max number of rows to return");
}

TmxControl::~TmxControl() {}

bool TmxControl::ProcessOptions(const variables_map &opts)
{
	_opts = const_cast<variables_map *>(&opts);
	return Runnable::ProcessOptions(opts);
}

bool TmxControl::checkPerm()
{
	if (_disablePermissionCheck)
		return true;

	static bool isRoot = (geteuid() == 0);
	if (isRoot)
		return true;

	struct group admGrp;
	struct group * admGrpResult = nullptr;
	std::array<char, 1024> dataBuf;
	if (getgrnam_r("adm", &admGrp, dataBuf.data(), dataBuf.size(), &admGrpResult)) {
		return false;
	}

	gid_t groups[256];
	int total = getgroups(256, groups);

	for (int i = 0; i < total; i++)
		if (groups[i] == admGrp.gr_gid)
			return true;

	PLOG(logDEBUG) << "Trying to set effective group id to " << admGrp.gr_gid << " (" << admGrp.gr_name << ")";
	if (::setregid(-1, admGrp.gr_gid) < 0)
	{
		cerr << strerror(errno) << endl;
		return false;
	}

	return true;
}

int convert(int c)
{
	if (c == '*')
		return '%';
	else
		return ::tolower(c);
}

int TmxControl::Main()
{
	if (!_opts)
	{
		cerr << "Processing error" << endl;
		return 0;
	}

	pluginlist plugins = (*_opts)["plugin"].as< pluginlist >();
	if (plugins.size() == 0)
		plugins.push_back("%");

	// Convert all the plugin names to lowercase for case-insensitive compare
	for (size_t p = 0; p < plugins.size(); p++)
		std::transform(plugins[p].begin(), plugins[p].end(), plugins[p].begin(), convert);

	string url = "tcp://";
	url += (*_opts)["host"].as<string>();
	url += ':';
	url += (*_opts)["port"].as<string>();

	_pool.SetConnectionUrl(url);

	int Ret = 0;

	// Call the correct function
	for (auto iter = fnRegistry.begin(); iter != fnRegistry.end(); iter++)
	{
		string fnName = iter->first;
		for (size_t i = 0; i < fnName.size(); i++)
		{
			if (fnName[i] == '_')
				fnName[i] = '-';
		}

		PLOG(logDEBUG2) << "Checking for function " << fnName;
		if ((*_opts).count(fnName))
		{
			PLOG(logDEBUG) << "Invoking function " << fnName;
			func_type fn = iter->second;
			if (!(this->*fn)(plugins))
			{
				PLOG(logDEBUG) << "Function returned a failure";
				Ret = -1;
			}
			else
			{
				PLOG(logDEBUG) << "Function returned successfully";
			}
		}
	}

	bool pretty = !(_opts->count("no-pretty-print"));

	if (!_output.get_storage().get_tree().empty())
	{
		if (_opts->count("json"))
		{
			boost::property_tree::write_json(cout, _output.get_storage().get_tree(), pretty);
		}
		else if (_opts->count("xml"))
		{
			boost::property_tree::xml_writer_settings<xml_settings_type> xmlSettings;
			if (pretty)
				xmlSettings = boost::property_tree::xml_writer_make_settings<xml_settings_type>('\t', 1);
			boost::property_tree::write_xml(cout, _output.get_storage().get_tree(), xmlSettings);
			cout << endl;
		}
		else
		{
			_output.save<battelle::attributes::INFO>(cout);
		}
	}


	return Ret;
}

string TmxControl::add_constraint(string query, TmxControl::pluginlist &plugins, string col)
{
	if (plugins.size() > 0)
	{
		query += " WHERE " + col + " IN ( ";
		query += " SELECT id";
		query += " FROM IVP.plugin";
		query += " WHERE ";

		for (size_t i = 0; i < plugins.size(); i++)
		{
			if (i > 0)
				query += " OR ";

			query += "lower(name) like ?";

		}
		query += " )";
	}

	return query;
}

string TmxControl::add_subconstraint(string query, TmxControl::pluginlist &plugins)
{
	if (plugins.size() > 0)
	{
		query += " WHERE ";

		for (size_t i = 0; i < plugins.size(); i++)
		{
			if (i > 0)
				query += " OR ";

			query += "lower(name) like ?";
		}
	}
	return query;
}

//methods for using the class in other applications
void TmxControl::SetConnectionUrl(string url)
{
	_pool.SetConnectionUrl(url);
}

void TmxControl::SetOption(std::string option, std::string value)
{
	if (!_opts)
		_opts = new variables_map();
	if (_opts->count(option) == 0)
		_opts->insert(std::make_pair(option, variable_value(value, false)));
	else
		_opts->at(option).value() = value;
}

void TmxControl::ClearOptions()
{
	if (!_opts)
		_opts = new variables_map();
	_opts->clear();
}

void TmxControl::DisablePermissionCheck()
{
	_disablePermissionCheck = true;
}

string TmxControl::GetOutput(TmxControlOutputFormat format, bool pretty)
{
	stringstream ss;
	ss.clear();
	ss.str(string());

	if (!_output.get_storage().get_tree().empty())
	{
		if (format == TmxControlOutputFormat_JSON)
		{
			boost::property_tree::write_json(ss, _output.get_storage().get_tree(), pretty);
		}
		else if (format == TmxControlOutputFormat_XML)
		{
			boost::property_tree::xml_writer_settings<xml_settings_type> xmlSettings;
			if (pretty)
				xmlSettings = boost::property_tree::xml_writer_make_settings<xml_settings_type>('\t', 1);
			boost::property_tree::write_xml(ss, _output.get_storage().get_tree(), xmlSettings);
			ss << endl;
		}
	}

	return ss.str();
}

tmx::message_container_type* TmxControl::GetOutput()
{
	return &_output;
}


} /* namespace tmxctl */


