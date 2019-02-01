/*
 * ConfigContext.h
 *
 *  Created on: Jul 26, 2014
 *      Author: ivp
 */

#ifndef CONFIGCONTEXT_H_
#define CONFIGCONTEXT_H_

#include "DbContext.h"
#include <string>
#include <vector>

struct SystemConfigurationParameterEntry {
	unsigned int id;
	std::string key;
	std::string value;
	std::string defaultValue;

	SystemConfigurationParameterEntry() {
		this->id = 0;
	}

	SystemConfigurationParameterEntry(std::string key, std::string defaultValue)
	{
		this->id = 0;
		this->key = key;
		this->value = defaultValue;
		this->defaultValue = defaultValue;
	}
};

struct PluginConfigurationParameterEntry {
	unsigned int id;
	unsigned int pluginId;
	std::string key;
	std::string value;
	std::string defaultValue;
	std::string description;

	PluginConfigurationParameterEntry() {
		this->id = 0;
		this->pluginId = 0;
	}

	PluginConfigurationParameterEntry(std::string key, std::string defaultValue, std::string description = "")
	{
		this->id = 0;
		this->pluginId = 0;
		this->key = key;
		this->value = defaultValue;
		this->defaultValue = defaultValue;
		this->description = description;
	}
};


class ConfigContext : public DbContext
{
public:
	ConfigContext();

	void initializeSystemConfigParameter(SystemConfigurationParameterEntry *entry);
	std::map<std::string, SystemConfigurationParameterEntry> getSystemConfigParameters();
	SystemConfigurationParameterEntry getSystemConfigParameter(std::string key);


	void initializePluginConfigParameters(unsigned int pluginId, std::vector<PluginConfigurationParameterEntry> &entries);
	std::map<std::string, PluginConfigurationParameterEntry> getPluginConfigParameters(unsigned int pluginId);
	void updatePluginConfigParameterValue(const PluginConfigurationParameterEntry &entry);


};



#endif /* CONFIGCONTEXT_H_ */
