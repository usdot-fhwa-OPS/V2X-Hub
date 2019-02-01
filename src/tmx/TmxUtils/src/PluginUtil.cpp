/*
 * PluginUtil.cpp
 *
 *  Created on: Nov 20, 2015
 *      Author: ivp
 */

#include "PluginUtil.h"

using namespace std;

namespace tmx {
namespace utils {

std::string PluginUtil::IvpPluginStateToString(IvpPluginState state)
{
	switch (state)
	{
	case IvpPluginState_disconnected:
		return "Disconnected";
	case IvpPluginState_connected:
		return "Connected";
	case IvpPluginState_registered:
		return "Registered";
	case IvpPluginState_error:
		return "Error";
	default:
		return "Unknown";
	}
}

bool PluginUtil::GetConfigValue(IvpPlugin *plugin, const char *key, bool *value, pthread_mutex_t *mutex)
{
	bool success = false;
	char *text = ivp_getCopyOfConfigurationValue(plugin, key);

	if (mutex != NULL)
		pthread_mutex_lock(mutex);

	if (text != NULL)
	{
		*value = (strncasecmp(text, "True", 4) == 0);
		free(text);
		success = true;
	}

	if (mutex != NULL)
		pthread_mutex_unlock(mutex);

	return success;
}

bool PluginUtil::GetConfigValue(IvpPlugin *plugin, const char *key, char **value, pthread_mutex_t *mutex)
{
	char *text = ivp_getCopyOfConfigurationValue(plugin, key);

	if (mutex != NULL)
		pthread_mutex_lock(mutex);

	if (*value != NULL)
	{
		free(*value);
	}

	*value = text;

	if (mutex != NULL)
		pthread_mutex_unlock(mutex);

	return (text != NULL);
}

bool PluginUtil::GetConfigValue(IvpPlugin *plugin, const char *key, std::string *value, pthread_mutex_t *mutex)
{
	bool success = false;
	char *text = ivp_getCopyOfConfigurationValue(plugin, key);

	if (mutex != NULL)
		pthread_mutex_lock(mutex);

	if (text != NULL)
	{
		*value = text;
		free(text);
		success = true;
	}

	if (mutex != NULL)
		pthread_mutex_unlock(mutex);

	return success;
}

bool PluginUtil::GetConfigValue(IvpPlugin *plugin, const char *key, int32_t *value, pthread_mutex_t *mutex)
{
	bool success = false;
	char *text = ivp_getCopyOfConfigurationValue(plugin, key);

	if (mutex != NULL)
		pthread_mutex_lock(mutex);

	if (text != NULL)
	{
		*value = strtol(text, NULL, 0);
		free(text);
		success = true;
	}

	if (mutex != NULL)
		pthread_mutex_unlock(mutex);

	return success;
}

bool PluginUtil::GetConfigValue(IvpPlugin *plugin, const char *key, uint32_t *value, pthread_mutex_t *mutex)
{
	bool success = false;
	char *text = ivp_getCopyOfConfigurationValue(plugin, key);

	if (mutex != NULL)
		pthread_mutex_lock(mutex);

	if (text != NULL)
	{
		*value = strtoul(text, NULL, 0);
		free(text);
		success = true;
	}

	if (mutex != NULL)
		pthread_mutex_unlock(mutex);

	return success;
}

bool PluginUtil::GetConfigValue(IvpPlugin *plugin, const char *key, uint64_t *value, pthread_mutex_t *mutex)
{
	bool success = false;
	char *text = ivp_getCopyOfConfigurationValue(plugin, key);

	if (mutex != NULL)
		pthread_mutex_lock(mutex);

	if (text != NULL)
	{
		*value = strtoull(text, NULL, 0);
		free(text);
		success = true;
	}

	if (mutex != NULL)
		pthread_mutex_unlock(mutex);

	return success;
}

}} // namespace tmx::utils
