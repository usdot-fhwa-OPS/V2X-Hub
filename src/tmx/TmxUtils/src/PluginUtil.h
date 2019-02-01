/*
 * PluginUtil.h
 *
 *  Created on: Nov 20, 2015
 *      Author: ivp
 */

#ifndef SRC_PLUGINUTIL_H_
#define SRC_PLUGINUTIL_H_

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <stdio.h>
#include "tmx/IvpPlugin.h"

namespace tmx {
namespace utils {

class PluginUtil
{
public:
	// ToString for IvpPluginState enumeration.
	static std::string IvpPluginStateToString(IvpPluginState state);

	// Set a status item for the specified plugin.
	// Returns true if the status string is new and it was set; false otherwise.
	template<typename T> static bool SetStatus(IvpPlugin *plugin, const char *key, T value, std::streamsize precision = 2);

	// Get a bool configuration value for the specified IVP plugin.
	// plugin - The IVP plugin.
	// key - The name of the configuration value.
	// value - The returned bool value.
	// mutex - If non-NULL, this mutex is locked while value is set.
	// Returns true on success; false if the value could not be retrieved.
	static bool GetConfigValue(IvpPlugin *plugin, const char *key, bool *value, pthread_mutex_t *mutex = NULL);

	// Get a string configuration value for the specified IVP plugin.
	// plugin - The IVP plugin.
	// key - The name of the configuration value.
	// value - The returned string value.  Note that if the value passed is is non-NULL, it is first freed.
	// mutex - If non-NULL, this mutex is locked while value is set.
	// Returns true on success; false if the value could not be retrieved.
	static bool GetConfigValue(IvpPlugin *plugin, const char *key, char **value, pthread_mutex_t *mutex = NULL);

	static bool GetConfigValue(IvpPlugin *plugin, const char *key, std::string *value, pthread_mutex_t *mutex = NULL);
	static bool GetConfigValue(IvpPlugin *plugin, const char *key, int32_t *value, pthread_mutex_t *mutex = NULL);
	static bool GetConfigValue(IvpPlugin *plugin, const char *key, uint32_t *value, pthread_mutex_t *mutex = NULL);
	static bool GetConfigValue(IvpPlugin *plugin, const char *key, uint64_t *value, pthread_mutex_t *mutex = NULL);
};

// Set a status item.
// The status is only set if the string representation of the value is not the same as the last
// time this method was called.
// Returns true if the status string is new and it was set; false otherwise.
template<typename T>
bool PluginUtil::SetStatus(IvpPlugin *plugin, const char *key, T value, std::streamsize precision)
{
	// Map a plugin status key to the last value set for that key.
	// Since each plugin is a standalone executable, the map does not need to track which plugin the status goes with.
	static std::map<const char *, std::string> statusMap;

	std::ostringstream ss;
	ss.setf(std::ios::boolalpha);
	ss.precision(precision);
	ss << std::fixed << value;

	bool isNewValue = false;

	std::map<const char*, std::string>::iterator pair = statusMap.find(key);
	if (pair == statusMap.end())
	{
		statusMap.insert(std::pair<const char*, std::string>(key, ss.str()));
		isNewValue = true;
	}
	else
	{
		if (ss.str().compare(pair->second) != 0)
		{
			pair->second = ss.str();
			isNewValue = true;
		}
	}

	if (isNewValue)
	{
		//printf("New Status. %s: %s.\n", key, ss.str().c_str());
		ivp_setStatusItem(plugin, key, ss.str().c_str());
	}

	return isNewValue;
}

}} // namespace tmx::utils

#endif /* SRC_PLUGINUTIL_H_ */
