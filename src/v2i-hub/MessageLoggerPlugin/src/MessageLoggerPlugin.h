
/**
 * Copyright (C) 2019 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this plogFile except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#ifndef TMX_PLUGINS_MESSAGELOGGERPLUGIN_H_
#define TMX_PLUGINS_MESSAGELOGGERPLUGIN_H_



#include "PluginClient.h"
#include "PluginDataMonitor.h"

#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <tmx/json/cJSON.h>
#include <tmx/messages/IvpJ2735.h>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <BasicSafetyMessage.h>
#include <tmx/j2735_messages/SpatMessage.hpp>
#include <SPAT.h>
#include <tmx/messages/auto_message.hpp>
#include <boost/filesystem.hpp>


using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace boost::filesystem;


namespace MessageLoggerPlugin
{


#define BYTESTOMB 1048576

/**
 * This plugin logs the messages received in the following json format.
 */
class MessageLoggerPlugin: public PluginClient
{
public:
	/**
	 * @brief Construct a new MessageLoggerPlugin with the given name.
	 * @param name The name to give the plugin for identification purposes.
	 */
	explicit MessageLoggerPlugin(const std::string &name);

	/**
	 * @brief Main method for running the plugin.
	 * @param argc number of arguments
	 * @param argv array of arguments
	 */
	int Main() override;

protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	/**
	 * @brief Called when configuration is changed
	 * @param key Key of the configuration value changed
	 * @param value Changed value
 	 */
	void OnConfigChanged(const char *key, const char *value);

	/**
	 * @brief Called on plugin state change
	 * @param state New plugin state
	 */
	void OnStateChange(IvpPluginState state);

	/**
	 * @brief Method that's called to process a message that this plugin has
	 * subscribed for. This particular method decodes the message and
	 * logs selective fields to a log file.
	 * @param msg Message that is received
	 * @param routeable_message not used
	 */
	void HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg);

	/**
	 * @brief Method that's called to process a message that this plugin has
	 * subscribed for. This particular method decodes the SPaT message and
	 * logs selective fields to a log file.
	 * @param msg SPaTMessage that is received
	 * @param routeable_message not used
	 */
	void HandleSpatMessage(SpatMessage &msg, routeable_message &routeableMsg);
	
	void GetInt32(unsigned char *buf, int32_t *value)
	{
		*value = (int32_t)((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
	}

	/**
	 * @brief Opens a new log file in the directory specified of specified name for logging messages. Once the
	 * current binary logfile size reaches the configurable maxSize this file is closed, renamed by the current
	 * time and date and moved to a /ode/ directory where it can be sent to an ODE using the filewatchscript.sh.
	 */
	void OpenMSGLogFile();

	/**
	 * @brief Checks the size of the logfile and opens a new_fileDirectory file if it's size is greater
	 * than the max size specified.
	 */
	void CheckMSGLogFileSizeAndRename();

	/**
	 * @brief Returns the current data time as string.
	 * @return current time in ddmmyyhhmiss format.
	 */
	std::string GetCurDateTimeStr();

private:
	std::mutex _cfgLock;

	std::atomic<uint64_t> _frequency{0};
	DATA_MONITOR(_frequency);

	std::ofstream _logFile;
	std::ofstream _logFilebin;
	std::string _filename, _fileDirectory;
	std::string _curFilename;
	std::string _curFilenamebin;
	std::string _newFilename;
	std::string _curFilenamesize;
	int _logFilesize;
	int _logFilebinsize;
	int _maxFilesizeInMB;

};
} /* namespace MessageLoggerPlugin */

#endif /* MessageLoggerPlugin.h */
