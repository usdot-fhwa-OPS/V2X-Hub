
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

	// Add a message filter and handler for each message this plugin wants to receive.
	AddMessageFilter < SpatMessage > (this, &SPaTLoggerPlugin::HandleSpatMessage);

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
		_logFilebin.close();
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
        std::string oldFilenamebin = _curFilenamebin;
        _curFilename = _fileDirectory + "/" + _filename + ".json";
        _curFilenamebin = _fileDirectory + "/" + _filename + ".bin";
        _curFilenamesize = _curFilenamebin;


        if (_curFilename.compare (oldFilename) !=0 )
        {
                _logFile.close();
                _logFilebin.close();
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
void SPaTLoggerPlugin::HandleSpatMessage(SpatMessage &msg, routeable_message &routeableMsg) 
{

        CheckSPaTLogFileSizeAndRename();
	char *SpatOut;
        cJSON *SpatRoot, *SpatMessageContent, *_SpatMessageContent;
	
	PLOG(logDEBUG)<<"HandleSPaTMessage";
	auto spat = msg.get_j2735_data();
	
	std::stringstream direction_hex;
        direction_hex<<"01";

        std::stringstream signStatus_hex;
        signStatus_hex<<"00";

	uint16_t intersectionId = spat->intersections.list.array[0]->id.id;
        uint16_t intersectionId_sw = __builtin_bswap16(abs(intersectionId));
	std::stringstream intersectionId_size;
	intersectionId_size << intersectionId;
	std::stringstream intersectionId_int_hex;
        if (intersectionId_size.str().length() == 1)
	{
		intersectionId_int_hex<<"0"<<std::hex<<intersectionId_sw;
	        unsigned int intersectionId_size2 = intersectionId_int_hex.str().size();
		std::stringstream intersectionId_hex;
        	intersectionId_hex<<intersectionId_int_hex.str()[1]<<' '<<intersectionId_int_hex.str()[2]<<intersectionId_int_hex.str()[3];
	}
	else
		intersectionId_int_hex<<std::hex<<intersectionId_sw;
                std::stringstream intersectionId_hex;
                intersectionId_hex<<intersectionId_int_hex.str()[0]<<intersectionId_int_hex.str()[1]<<' '<<intersectionId_int_hex.str()[2]<<intersectionId_int_hex.str()[3];
	
	std::string interstatus = std::bitset<8>(spat->intersections.list.array[0]->status.buf).to_string();
	int interstatint_sw = stoi(interstatus);
        std::stringstream interstatint_size;
        interstatint_size << interstatint_sw;
        std::stringstream interstatint_int_hex;
        if (interstatint_size.str().length() == 1)
        {
                interstatint_int_hex<<"0"<<std::hex<<interstatint_sw;
                unsigned int interstatint_size2 = interstatint_int_hex.str().size();
                std::stringstream interstatint_hex;
                interstatint_hex<<interstatint_int_hex.str()[1];
        }
        else
                interstatint_int_hex<<std::hex<<interstatint_sw;
                std::stringstream interstatint_hex;
                interstatint_hex<<interstatint_int_hex.str()[0]<<interstatint_int_hex.str()[1];

        uint32_t spatreceivetime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        uint32_t spatreceivetime_sw = __builtin_bswap32(spatreceivetime);
        std::stringstream spatreceivetime_int_hex;
        spatreceivetime_int_hex<<std::hex<<spatreceivetime_sw;
        unsigned int spatreceivetime_size = spatreceivetime_int_hex.str().size();
        std::stringstream spatreceivetime_hex;
        spatreceivetime_hex<<spatreceivetime_int_hex.str()[0]<<spatreceivetime_int_hex.str()[1];
        for (int fsize = 2; fsize < spatreceivetime_size; fsize = fsize+2) {
        spatreceivetime_hex << ' ' << spatreceivetime_int_hex.str()[fsize] << spatreceivetime_int_hex.str()[fsize+1];
        }

        uint64_t spatreceivetimemillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        unsigned int spatmillis = spatreceivetimemillis - (spatreceivetime*1000);
        uint16_t spatmillis16 = (uint16_t) spatmillis;
        uint16_t spatmillis16_sw = __builtin_bswap16(abs(spatmillis16));
        std::stringstream spatmillis16_int_hex;
        spatmillis16_int_hex<<"0"<<std::hex<<spatmillis;
        std::stringstream spatmillis16_size;
        spatmillis16_size<<spatmillis16_int_hex.str()[2]<<spatmillis16_int_hex.str()[3];
        std::stringstream spatmillis16_hex;
        if (spatmillis16_int_hex.str()[2] == NULL){
                spatmillis16_hex<<"0"<<spatmillis16_int_hex.str()[3]<<' '<<spatmillis16_int_hex.str()[0]<<spatmillis16_int_hex.str()[1];
        }
        else if (spatmillis16_int_hex.str()[3] == NULL){
                spatmillis16_hex<<spatmillis16_int_hex.str()[2]<<"0"<<' '<<spatmillis16_int_hex.str()[0]<<spatmillis16_int_hex.str()[1];
        }

        else{
                spatmillis16_hex<<spatmillis16_int_hex.str()[2]<<spatmillis16_int_hex.str()[3]<<' '<<spatmillis16_int_hex.str()[0]<<spatmillis16_int_hex.str()[1];
        }

	//Add is_cert_present
        std::stringstream IsCertPresent_hex;
        IsCertPresent_hex<<"00";

	uint16_t spat_size;
        if (routeableMsg.get_payload_str().length()%4 == 0){
                spat_size = routeableMsg.get_payload_str().length()/2;
        }
        else{
                spat_size = (routeableMsg.get_payload_str().length()/2)+1;
        }
        uint16_t spat_size_test = __builtin_bswap16(spat_size);
        std::stringstream spat_size_int_hex;
        spat_size_int_hex<<std::hex<< spat_size_test;
	std::stringstream spat_size_hex;
        spat_size_hex<<spat_size_int_hex.str()[0]<<spat_size_int_hex.str()[1]<<' '<<spat_size_int_hex.str()[2]<<spat_size_int_hex.str()[3];
	
	
	SpatRoot = cJSON_CreateObject(); // create root node
        SpatMessageContent = cJSON_CreateArray(); // create root array

        cJSON_AddItemToObject(SpatRoot, "SpatMessageContent", SpatMessageContent); // add SpatMessageContent array to Spatroot

        cJSON_AddItemToArray(SpatMessageContent, _SpatMessageContent = cJSON_CreateObject()); //add message content to SpatMessageContent array
        cJSON_AddItemToObject(_SpatMessageContent, "IntersectionID", cJSON_CreateNumber(intersectionId)); // Intersection_ID
        cJSON_AddItemToObject(_SpatMessageContent, "IntersecionStatus", cJSON_CreateNumber(interstatint_sw)); // Intersection_status
	cJSON_AddItemToObject(_SpatMessageContent, "payload", cJSON_CreateString(routeableMsg.get_payload_str().c_str())); // payload


	std::stringstream metadata;
        std::string spat_output_payload = routeableMsg.get_payload_str();

        metadata<<direction_hex.str()<<' '<<intersectionId_hex.str()<<' '<<interstatint_hex.str()<<' '<<spatreceivetime_hex.str()<<' '<<spatmillis16_hex.str()<<' '<<signStatus_hex.str()<<' '<<IsCertPresent_hex.str()<<' '<<spat_size_hex.str();

        unsigned int payload_size = spat_output_payload.size();
        std::stringstream spat_output_data;
        spat_output_data<<metadata.str();
        for (int bsize = 0; bsize <= payload_size; bsize = bsize+2) {
        spat_output_data << ' ' << spat_output_payload[bsize] << spat_output_payload[bsize+1];
        }
        int actualsize = spat_output_data.str().size();
        int showmethesize = ((spat_output_data.str().size()+1)/3);
        unsigned int binary_array[((spat_output_data.str().size()+1)/3)];
        int dsize = 0;
        while (spat_output_data.good() && dsize < spat_output_data.str().size()){
                spat_output_data >> std::hex >> binary_array[dsize];
                ++dsize;
        }
        unsigned char binary_output[((spat_output_data.str().size()+1)/3)];
        for(int k=0;k<(spat_output_data.str().size()+1)/3;k++) {
        binary_output[k]=static_cast<char>(binary_array[k]);
        }
        std::ofstream _logFileBin(_curFilenamebin, std::ios::out | std::ios::binary | std::ios::app);
        _logFileBin.write((const char*)binary_output,sizeof(binary_output));

	PLOG(logDEBUG)<<"Logging SPaTMessage data";
	SpatOut = cJSON_Print(SpatRoot);
        _logFile << SpatOut;
        free(SpatOut);

}



/**
 * Opens a new log file in the directory specified of specified name for logging SPaT messages. Once the
 * current binary logfile size reaches the configurable maxSize this file is closed, renamed by the current
 * time and date and moved to a /ode/ directory where it can be sent to an ODE using the filewatchscript.sh.
 */
void SPaTLoggerPlugin::OpenSPaTLogFile()
{
        //rename logfile if one already exists
        if ( !boost::filesystem::exists( _fileDirectory + "/json/") ){
                boost::filesystem::create_directory( _fileDirectory + "/json/");            
        }
        if ( !boost::filesystem::exists( _fileDirectory + "/ode/") ){
                boost::filesystem::create_directory( _fileDirectory + "/ode/");            
        }
        std::string newFilename = _fileDirectory + "/json/" + _filename + GetCurDateTimeStr() + ".json";
        std::string newbinFilename = _fileDirectory + "/ode/" + _filename + GetCurDateTimeStr() + ".bin";

        int error;
        if ( boost::filesystem::exists( _curFilenamebin.c_str() ) ) {
                error = std::rename(_curFilename.c_str(), newFilename.c_str() );
    	        if ( error != 0 ) {
        	        FILE_LOG(logERROR) << "Failed to mv " << _curFilename.c_str() << " to " << newFilename.c_str() << std::endl;
    	        }
	
    	        error = std::rename(_curFilenamebin.c_str(), newbinFilename.c_str() );
                if ( error != 0 ) {
                        FILE_LOG(logERROR) << "Failed to mv " << _curFilenamebin.c_str() << " to " << newbinFilename.c_str() << std::endl;
                }
	}
        _logFile.open(_curFilename);
        _logFilebin.open(_curFilenamebin, std::ios::out | std::ios::binary | std::ios::app);
	if (!_logFile.is_open())
		std::cerr << "Could not open log : " << strerror(errno) <<  std::endl;
	else
	{
		_logFile << "SPaT JSON Logs" << endl;

	}
}

/**
 * Checks the size of the logfile and opens a new file if it's size is greater
 * than the max size specified.
 */
void SPaTLoggerPlugin::CheckSPaTLogFileSizeAndRename()
{
        if (_logFile.is_open())
        {
                std::lock_guard<mutex> lock(_cfgLock);
                std::fstream logFilesize(_curFilenamesize);
                logFilesize.seekg(0, std::ios::end);
                int _logFilesize = logFilesize.tellg();
                int curFilesizeInMB = _logFilesize/1048576;
                if (curFilesizeInMB >= _maxFilesizeInMB)
                {
                        _logFile.close();
                        _logFilebin.close();
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
