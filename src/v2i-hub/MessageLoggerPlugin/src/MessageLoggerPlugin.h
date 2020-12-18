
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

#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <../../../tmx/TmxApi/tmx/json/cJSON.h>
//#include <DecodedBsmMessage.h>
#include <tmx/messages/IvpJ2735.h>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <BasicSafetyMessage.h>
#include <tmx/messages/auto_message.hpp>


using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;


namespace MessageLoggerPlugin
{


#define BYTESTOMB 1048576

/**
 * This plugin logs the messages received in the following json format.
 */
class MessageLoggerPlugin: public PluginClient
{
public:
	MessageLoggerPlugin(std::string);
	virtual ~MessageLoggerPlugin();
	int Main();
protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);

	void HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg);
	void GetInt32(unsigned char *buf, int32_t *value)
	{
		*value = (int32_t)((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
	}
private:
	std::atomic<uint64_t> _frequency{0};
	DATA_MONITOR(_frequency);   // Declares the

	void OpenMSGLogFile();
	void CheckMSGLogFileSizeAndRename(bool createNewFile=false);
	std::string  GetCurDateTimeStr();

	std::ofstream _logFile;
	std::ofstream _logFilebin;
	std::string _cvmsgtype;
	std::string _filename, _fileDirectory;
	std::string _curFilename;
	std::string _curFilenamebin;
	std::string _newFilename;
	std::string _curFilenamesize;
	int _logFilesize;
	int _logFilebinsize;
	int _maxFilesizeInMB;

};
std::mutex _cfgLock;


} /* namespace MessageLoggerPlugin */

#endif /* MessageLoggerPlugin.h */
