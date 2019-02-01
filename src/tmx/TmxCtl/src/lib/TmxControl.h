/*
 * TmxControl.h
 *
 *  Created on: Apr 24, 2017
 *      Author: gmb
 */

#ifndef SRC_TMXCTL_TMXCONTROL_H_
#define SRC_TMXCTL_TMXCONTROL_H_

#define ATTRIBUTE_PATH_CHARACTER '|'

#include <PluginExec.h>

#include <cstdarg>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <database/DbConnectionPool.h>
#include <memory>
#include <vector>
#include <string>
#include <boost/regex.hpp>

#ifndef DEFAULT_PLUGINDIRECTORY
#define DEFAULT_PLUGINDIRECTORY "/var/www/plugins"
#endif

namespace tmxctl {

enum TmxControlOutputFormat
{
	TmxControlOutputFormat_JSON,
	TmxControlOutputFormat_XML
};

class TmxControl: public tmx::utils::Runnable {
public:
	typedef std::vector<std::string> pluginlist;

	TmxControl();
	virtual ~TmxControl();

	// Command line version
	bool ProcessOptions(const boost::program_options::variables_map &);
	int Main();

	// Available options should be functions
	bool list(pluginlist &, ...);
	bool load_manifest(pluginlist &, ...);
	bool remove(pluginlist &, ...);
	bool enable(pluginlist &, ...);
	bool disable(pluginlist &, ...);
	bool start(pluginlist &, ...);
	bool stop(pluginlist &, ...);
	bool status(pluginlist &, ...);
	bool config(pluginlist &, ...);
	bool state(pluginlist &, ...);
	bool set(pluginlist &, ...);
	bool reset(pluginlist &, ...);
	bool max_message_interval(pluginlist &, ...);
	bool plugin_log_level(pluginlist &, ...);
	bool plugin_log_output(pluginlist &, ...);
	bool args(pluginlist &, ...);
	bool messages(pluginlist &, ...);
	bool events(pluginlist &, ...);
	bool clear_event_log(pluginlist &, ...);
	bool system_config(pluginlist &, ...);
	bool user_info(bool showPassword = false);
	bool user_info(pluginlist &, ...);
	bool set_system(pluginlist &, ...);
	bool all_users_info(bool showPassword = false);
	bool all_users_info(pluginlist &, ...);
	bool user_delete();
	bool user_delete(pluginlist &, ...);
	bool user_add();
	bool user_add(pluginlist &, ...);
	bool user_update();
	bool user_update(pluginlist &, ...);
	bool plugin_install();
	bool plugin_install(pluginlist &, ...);
	bool plugin_remove();
	bool plugin_remove(pluginlist &, ...);

	//methods for using the class in other applications
	void SetConnectionUrl(std::string url);
	void SetOption(std::string option, std::string value);
	void ClearOptions();
	void DisablePermissionCheck();
	std::string GetOutput(TmxControlOutputFormat format, bool pretty);
	tmx::message_container_type* GetOutput();

private:
	boost::program_options::variables_map *_opts;
	tmx::utils::DbConnectionPool _pool;
	tmx::message_container_type _output;
	bool _disablePermissionCheck{false};

	bool checkPerm();
	std::string add_constraint(std::string, pluginlist &, std::string col = "pluginId");
	std::string add_subconstraint(std::string, pluginlist &);
};

} /* namespace tmxctl */

#endif /* SRC_TMXCTL_TMXCONTROL_H_ */
