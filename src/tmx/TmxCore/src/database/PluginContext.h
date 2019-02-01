/*
 * PluginContext.h
 *
 *  Created on: Jul 23, 2014
 *      Author: ivp
 */

#ifndef PLUGINCONTEXT_H_
#define PLUGINCONTEXT_H_

#include "DbContext.h"
#include <string>
#include <vector>
#include <set>

struct PluginEntry {
	unsigned int id;
	std::string name;
	std::string description;
	std::string version;
};

struct InstalledPluginEntry {
	unsigned int id;
	PluginEntry plugin;
	std::string path;
	std::string exeName;
	std::string manifestName;
	bool enabled;
	unsigned int maxMessageInterval ;

	bool operator== (const InstalledPluginEntry &cmp) const
	{
		return id == cmp.id;
	}

	bool operator< (const InstalledPluginEntry &cmp) const
	{
		return id < cmp.id;
	}
};

struct PluginStatusItem {
	//unsigned int id;
	//PluginEntry plugin;
	std::string key;
	std::string value;
};

class PluginContext : public DbContext
{
public:
	PluginContext();

	std::vector<PluginEntry> getAllPlugins();
	PluginEntry getPlugin(std::string pluginName);
	void insertOrUpdatePlugin(PluginEntry &entry);
	void removeAllNotInstalled();

	void setStatusForAllPlugins(std::string status);
	void setPluginStatus(unsigned int pluginId, std::string status);
	void setPluginStatusItems(unsigned int pluginId, std::vector<PluginStatusItem> statusItems);
	void removePluginStatusItems(unsigned int pluginId, std::vector<std::string> itemKeys);
	void removeAllPluginStatusItems(unsigned int pluginId);
	void removeAllPluginStatusItems();

	std::set<InstalledPluginEntry> getInstalledPlugins(bool enabledOnly);
	void disableInstalledPlugin(unsigned int id);

};

#endif /* PLUGINCONTEXT_H_ */
