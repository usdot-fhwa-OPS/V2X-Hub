/*
 * PluginExtender.h
 *
 *  Created on: Sep 12, 2016
 *      Author: ivp
 */

#ifndef SRC_PLUGINEXTENDER_H_
#define SRC_PLUGINEXTENDER_H_

#include <string>
#include "PluginClient.h"

namespace tmx {
namespace utils {

/**
 * The PluginExtender class is used to pass a PluginClient reference to another class.
 * Since PluginExtender is a friend of PluginClient, it has access to the _name variable,
 * which is needed by the PLOG macro whenever PluginClient.h is included.
 */
class PluginExtender
{
public:
	PluginExtender(PluginClient& pluginClient) :
		_pluginClient(pluginClient),
		_plugin(pluginClient._plugin),
		_name(pluginClient._name) {	}

	virtual ~PluginExtender() { }

protected:
	PluginClient& _pluginClient;
	IvpPlugin *_plugin;
	std::string _name;
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_PLUGINEXTENDER_H_ */
