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
/**
 * Helper function to validate that input string is not empty and does not contain
 * any dangerous characters for command injection : |&<>`$()*?"
 * @param input_string to validate
 * @return bool true if successful validation
 * @throws TmxException if validation fails
 * */	
bool validate_input_string(const std::string &input_string);
/**
 * Helper function to validate that filepath is not empty, file exists, and does not contain
 * any dangerous characters for command injection : |&<>`$()*?"
 * @param filepath to validate
 * @return bool true if successful validation
 * @throws TmxException if validation fails
 * */
bool validate_filepath(const std::string &filepath);
/**
 * Helper function to call waitpid to get return value of posix_spawnp spawned process
 * @param process_name a string name for the process for logging failure purposes only
 * @param posix_spawnp_ret the int return value from posix_spawnp
 * @param pid_t the pid of the process launched with posix_spawnp
 * @return bool true if process has a return value of 0
 * @throw TmxException if process has a return value != 0  
 */
bool check_posix_process_status(const std::string &process_name, int posix_spawnp_ret, const pid_t &pid);
/**
 * Helper function to clean up file descriptor. First checks if file descriptor has already been closed, if 
 * not it closes file descriptor and sets value to -1 to indicate it has been closed and avoid undefined 
 * behaviour with multple calls to close 
 * @param const &fd file descriptor to close and set to -1
 */
void clean_up_file_descriptor(int &fd);


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
	bool hashed_info();
	bool hashed_info(pluginlist &, ...);
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
	bool save_state(const std::string &passphrase);
	bool save_state(pluginlist &, ...);
	bool upload_state(pluginlist &, ...);
	bool upload_state(const std::string &filePath, const std::string &passphrase);

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
