
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


#include "SPaTLoggerPlugin.h"

namespace SPaTLoggerPlugin
{

/**
 * Construct a new SPaTLoggerPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes.
 */
SPaTLoggerPlugin::SPaTLoggerPlugin(string name): PluginClient(name)
{
	PLOG(logDEBUG)<< "In SPaTLoggerPlugin Constructor";
	// The log level can be changed from the default here.
	//FILELog::ReportingLevel() = FILELog::FromString("DEBUG");

	// Critical section
	std::lock_guard<mutex> lock(_cfgLock);
	GetConfigValue("File Location", _fileDirectory);
	GetConfigValue("File Size In MB", _maxFilesizeInMB);
	GetConfigValue("Filename", _filename);
	_curFilename = _fileDirectory + "/" + _filename + ".csv";

	OpenSPaTLogFile();
	// Add a message filter and handler for each message this plugin wants to receive.
	//AddMessageFilter<DecodedSPaTMessage>(this, &SPaTLoggerPlugin::HandleDecodedSPaTMessage);
	AddMessageFilter < SpatMessage > (this, &SPaTLoggerPlugin::HandleSPaTMessage);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();

	PLOG(logDEBUG) << "Exit SPaTLoggerPlugin Constructor";
}

/**
 * Destructor
 */

SPaTLoggerPlugin::~SPaTLoggerPlugin()
{
	if (_logFile.is_open())
	{
		_logFile.close();
	}
}


/**
 * Updates configuration settings
 */
void SPaTLoggerPlugin::UpdateConfigSettings()
{
	// Configuration settings are retrieved from the API using the GetConfigValue template class.
	// This method does NOT execute in the main thread, so variables must be protected
	// (e.g. using std::atomic, std::mutex, etc.).

	int instance;
	GetConfigValue("Instance", instance);

	GetConfigValue("Frequency", __frequency_mon.get());
	__frequency_mon.check();

	std::lock_guard<mutex> lock(_cfgLock);
	GetConfigValue("File Location", _fileDirectory);
	GetConfigValue("File Size In MB", _maxFilesizeInMB);
	GetConfigValue("Filename", _filename);
	std::string oldFilename = _curFilename;
	_curFilename = _fileDirectory + "/" + _filename + ".csv";

	if (_curFilename.compare (oldFilename) !=0 )
	{
		_logFile.close();
		OpenSPaTLogFile();

	}

}

/**
 * Called when configuration is changed
 *
 * @param key Key of the configuration value changed
 * @param value Changed value
 */
void SPaTLoggerPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

/**
 * Called on plugin state change
 *
 * @para state New plugin state
 */
void SPaTLoggerPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
		//SetStatus("ReceivedMaps", 0);
	}
}

/**
 * Method that's called to process a message that this plugin has
 * subscribed for.  This particular method decodes the SPaT message and
 * logs selective fields to a log file.
 *
 * @param msg SPaTMessage that is received
 * @routeable_message not used
 */
void SPaTLoggerPlugin::HandleSPaTMessage(SpatMessage &msg, routeable_message &routeableMsg) 
{

	PLOG(logDEBUG)<<"HandleSPaTMessage";
	auto spat = msg.get_j2735_data();

	
	PLOG(logDEBUG)<<"Logging SPaTMessage data";
	_logFile << GetCurDateTimeStr() << ","  << routeableMsg.get_payload_str() << endl;


}



/**
 *  Opens a new log file in the directory specified of specified name for logging SPaT messages and
 *  inserts a header row with names of fields that will be logged when data is received. If a log file
 *  with the same name already exists before opening a new file, it's renamed with current timestamp suffix.
 */
void SPaTLoggerPlugin::OpenSPaTLogFile()
{
	PLOG(logDEBUG) << "SPaT Log File: " << _curFilename << std::endl;;
	//rename logfile if one already exists
	std::string newFilename = _fileDirectory + "/" + _filename + GetCurDateTimeStr() + ".csv";
	std::rename(_curFilename.c_str(), newFilename.c_str());

	_logFile.open(_curFilename);
	if (!_logFile.is_open())
		std::cerr << "Could not open log : " << strerror(errno) <<  std::endl;
	else
	{
		_logFile << "Time, payload " << endl;

	}
}

/**
 * Checks the size of the logfile and opens a new file if it's size is greater
 * than the max size specified.
 */
void SPaTLoggerPlugin::CheckSPaTLogFileSizeAndRename(bool createNewFile)
{
	if (_logFile.is_open())
	{
		std::lock_guard<mutex> lock(_cfgLock);
		_logFile.seekp( 0, std::ios::end );
		int curFilesizeInMB = _logFile.tellp()/BYTESTOMB;
		if (curFilesizeInMB > _maxFilesizeInMB || createNewFile)
		{
			_logFile.close();
			OpenSPaTLogFile();
		}
	}
}

/**
 * Returns the current data time as string.
 * @return current time in ddmmyyhhmiss format.
 */
std::string SPaTLoggerPlugin::GetCurDateTimeStr()
{
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	std::ostringstream oss;
	oss << std::put_time(&tm, "%d%m%Y%H%M%S");
	auto str = oss.str();
	return str;
}

// Override of main method of the plugin that should not return until the plugin exits.
// This method does not need to be overridden if the plugin does not want to use the main thread.
int SPaTLoggerPlugin::Main()
{
	PLOG(logDEBUG) << "Starting SPaTLoggerplugin...";

	uint msCount = 0;
	while (_plugin->state != IvpPluginState_error)
	{
		PLOG(logDEBUG4) << "SPaTLoggerPlugin Sleeping 5 minutes" << endl;

		this_thread::sleep_for(chrono::milliseconds(300000));

		// check size of the log file and open new one if needed
		CheckSPaTLogFileSizeAndRename(false);
	}

	PLOG(logDEBUG) << "SPaTLoggerPlugin terminating gracefully.";
	return EXIT_SUCCESS;
}

} /* namespace SPaTLoggerPlugin */


/**
 * Main method for running the plugin
 * @param argc number of arguments
 * @param argv array of arguments
 */
int main(int argc, char *argv[])
{
	return run_plugin<SPaTLoggerPlugin::SPaTLoggerPlugin>("SPaTLoggerPlugin", argc, argv);
}
