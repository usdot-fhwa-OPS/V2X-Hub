
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


#include "MessageLoggerPlugin.h"

namespace MessageLoggerPlugin
{

/**
 * Construct a new MessageLoggerPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes.
 */
MessageLoggerPlugin::MessageLoggerPlugin(string name): PluginClient(name)
{
	PLOG(logDEBUG)<< "In MessageLoggerPlugin Constructor";
	// The log level can be changed from the default here.
	//FILELog::ReportingLevel() = FILELog::FromString("DEBUG");

	// Critical section
	std::lock_guard<mutex> lock(_cfgLock);
	GetConfigValue("File Location", _fileDirectory);
	GetConfigValue("File Size In MB", _maxFilesizeInMB);
	GetConfigValue("Messagetype", _cvmsgtype);
	GetConfigValue("Filename", _filename);
	_curFilename = _fileDirectory + "/" + _filename + ".json";
	_curFilenamebin = _fileDirectory + "/" + _filename + ".bin";
	_curFilenamesize = _curFilenamebin;

	OpenMSGLogFile();
	// Add a message filter and handler for each message this plugin wants to receive.
	//AddMessageFilter<DecodedBsmMessage>(this, &BsmLoggerPlugin::HandleDecodedBsmMessage);
	AddMessageFilter < BsmMessage > (this, &MessageLoggerPlugin::HandleBasicSafetyMessage);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();

	PLOG(logDEBUG) << "Exit MessageLoggerPlugin Constructor";
}

/**
 * Destructor
 */

MessageLoggerPlugin::~MessageLoggerPlugin()
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
void MessageLoggerPlugin::UpdateConfigSettings()
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
	GetConfigValue("Messagetype", _cvmsgtype);
	GetConfigValue("Filename", _filename);
	std::string oldFilename = _curFilename;
	std::string oldFilenamebin = _curFilenamebin;
	_curFilename = _fileDirectory + "/" + _filename + ".json";
	_curFilenamebin = _fileDirectory + "/" + _filename + ".bin";

	if (_curFilename.compare (oldFilename) !=0 )
	{
		_logFile.close();
		_logFilebin.close();
		OpenMSGLogFile();
	}
}

/**
 * Called when configuration is changed
 *
 * @param key Key of the configuration value changed
 * @param value Changed value
 */
void MessageLoggerPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

/**
 * Called on plugin state change
 *
 * @para state New plugin state
 */
void MessageLoggerPlugin::OnStateChange(IvpPluginState state)
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
 * subscribed for.  This particular method decodes the message and
 * logs selective fields to a log file.
 *
 * @param msg Message that is received
 * @routeable_message not used
 */
void MessageLoggerPlugin::HandleBasicSafetyMessage(BsmMessage &msg,
		routeable_message &routeableMsg) {

	char *BsmOut;
	cJSON *BsmRoot, *BsmMessageContent, *_BsmMessageContent;

	PLOG(logDEBUG)<<"HandleBasicSafetyMessage";
	auto bsm = msg.get_j2735_data();

	float speed_mph;
	int32_t bsmTmpID;

        std::stringstream direction_hex;
	direction_hex<<"01";
	
	std::stringstream signStatus_hex;
	signStatus_hex<<"00";

	bool isSuccess = false;

	//asn_fprint(stdout, &asn_DEF_BasicSafetyMessage, bsm);
	
	int32_t latitude = bsm->coreData.lat;
	int32_t latitude_sw = __builtin_bswap32(abs(latitude));
	std::stringstream latitude_int_hex;
        latitude_int_hex<<std::hex<<latitude_sw;
	unsigned int latitude_size = latitude_int_hex.str().size();
        std::stringstream latitude_hex;
        latitude_hex<<latitude_int_hex.str()[0]<<latitude_int_hex.str()[1];
        for (int fsize = 2; fsize < latitude_size; fsize = fsize+2) {
        latitude_hex << ' ' << latitude_int_hex.str()[fsize] << latitude_int_hex.str()[fsize+1];
        }


	int32_t longitude = bsm->coreData.Long;
	int32_t longitude_sw = __builtin_bswap32(abs(longitude));
        std::stringstream longitude_int_hex;
        longitude_int_hex<<std::hex<<longitude_sw;
        unsigned int longitude_size = longitude_int_hex.str().size();
        std::stringstream longitude_hex;
        longitude_hex<<longitude_int_hex.str()[0]<<longitude_int_hex.str()[1];
        for (int fsize = 2; fsize < longitude_size; fsize = fsize+2) {
        longitude_hex << ' ' << longitude_int_hex.str()[fsize] << longitude_int_hex.str()[fsize+1];
        }


	int32_t elevation = bsm->coreData.elev;
        int32_t elevation_sw = __builtin_bswap32(abs(elevation));
        std::stringstream elevation_int_hex;
        elevation_int_hex<<std::hex<<elevation_sw;
        unsigned int elevation_size = elevation_int_hex.str().size();
        std::stringstream elevation_hex;
        elevation_hex<<elevation_int_hex.str()[0]<<elevation_int_hex.str()[1];
        for (int fsize = 2; fsize < elevation_size; fsize = fsize+2) {
        elevation_hex << ' ' << elevation_int_hex.str()[fsize] << elevation_int_hex.str()[fsize+1];
        }


	int32_t longAcceleration = bsm->coreData.accelSet.Long;

	int16_t transtime = bsm->coreData.secMark;

	std::stringstream header_hex;
        header_hex<<"03 80 81";

	int header_size;
	if (routeableMsg.get_payload_str().length()%4 == 0){
		header_size = routeableMsg.get_payload_str().length()/2;
	}
	else{
		header_size = (routeableMsg.get_payload_str().length()/2)+1;
	}
        std::stringstream header_size_int_hex;
        header_size_int_hex<<std::hex<<header_size;
        std::stringstream header_size_hex;
        header_size_hex<<header_size_int_hex.str()[0]<<header_size_int_hex.str()[1];

        int16_t bsmlen = header_size+4;
        uint16_t bsmlen_sw = __builtin_bswap16(bsmlen);
        std::stringstream bsmlen_int_hex;
        bsmlen_int_hex<<std::hex<<bsmlen_sw;
        std::stringstream bsmlen_hex;
        bsmlen_hex<<bsmlen_int_hex.str()[0]<<bsmlen_int_hex.str()[1]<<' '<<bsmlen_int_hex.str()[2]<<bsmlen_int_hex.str()[3];

	int16_t rawSpeed = bsm->coreData.speed;
	int16_t rawspeed_sw = __builtin_bswap16(abs(rawSpeed));
        std::stringstream rawspeed_size;
	rawspeed_size << rawSpeed;
	std::stringstream rawspeed_int_hex;
        if (rawspeed_size.str().length() == 1)
	{
		rawspeed_int_hex<<"0"<<std::hex<<rawspeed_sw;
	        unsigned int rawspeed_size2 = rawspeed_int_hex.str().size();
		std::stringstream rawspeed_hex;
        	rawspeed_hex<<rawspeed_int_hex.str()[1]<<' '<<rawspeed_int_hex.str()[2]<<rawspeed_int_hex.str()[3];
	}
	else
		rawspeed_int_hex<<std::hex<<rawspeed_sw;
                std::stringstream rawspeed_hex;
                rawspeed_hex<<rawspeed_int_hex.str()[0]<<rawspeed_int_hex.str()[1]<<' '<<rawspeed_int_hex.str()[2]<<rawspeed_int_hex.str()[3];


	int16_t rawHeading = bsm->coreData.heading;
	int16_t rawHeading_sw = __builtin_bswap16(abs(rawHeading));
        std::stringstream rawHeading_int_hex;
        rawHeading_int_hex<<std::hex<<rawHeading_sw;
	std::stringstream rawHeading_hex;
        rawHeading_hex<<rawHeading_int_hex.str()[0]<<rawHeading_int_hex.str()[1]<<' '<<rawHeading_int_hex.str()[2]<<rawHeading_int_hex.str()[3];

	uint32_t bsmreceivetime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	int32_t bsmreceivetime_sw = __builtin_bswap32(bsmreceivetime);
        std::stringstream bsmreceivetime_int_hex;
        bsmreceivetime_int_hex<<std::hex<<bsmreceivetime_sw;
        unsigned int bsmreceivetime_size = bsmreceivetime_int_hex.str().size();
        std::stringstream bsmreceivetime_hex;
        bsmreceivetime_hex<<bsmreceivetime_int_hex.str()[0]<<bsmreceivetime_int_hex.str()[1];
        for (int fsize = 2; fsize < bsmreceivetime_size; fsize = fsize+2) {
        bsmreceivetime_hex << ' ' << bsmreceivetime_int_hex.str()[fsize] << bsmreceivetime_int_hex.str()[fsize+1];
        }

	int64_t bsmreceivetimemillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	int bsmmillis = bsmreceivetimemillis - (bsmreceivetime*1000);
	uint16_t bsmmillis16 = (uint16_t) bsmmillis;
	uint16_t bsmmillis16_sw = __builtin_bswap16(abs(bsmmillis16));
        std::stringstream bsmmillis16_int_hex;
        bsmmillis16_int_hex<<"0"<<std::hex<<bsmmillis;
	std::stringstream bsmmillis16_size;
        bsmmillis16_size<<bsmmillis16_int_hex.str()[2]<<bsmmillis16_int_hex.str()[3];
        std::stringstream bsmmillis16_hex;
	if (bsmmillis16_int_hex.str()[2] == NULL){
                bsmmillis16_hex<<"0"<<bsmmillis16_int_hex.str()[3]<<' '<<bsmmillis16_int_hex.str()[0]<<bsmmillis16_int_hex.str()[1];
        }
	else if (bsmmillis16_int_hex.str()[3] == NULL){
                bsmmillis16_hex<<bsmmillis16_int_hex.str()[2]<<"0"<<' '<<bsmmillis16_int_hex.str()[0]<<bsmmillis16_int_hex.str()[1];
        }

	else{
		bsmmillis16_hex<<bsmmillis16_int_hex.str()[2]<<bsmmillis16_int_hex.str()[3]<<' '<<bsmmillis16_int_hex.str()[0]<<bsmmillis16_int_hex.str()[1];
	}

	GetInt32((unsigned char *)bsm->coreData.id.buf, &bsmTmpID);

	// Heading units are 0.0125 degrees.
	float heading = rawHeading / 80.0;

	// The speed is contained in bits 0-12.  Units are 0.02 meters/sec.
	// A value of 8191 is used when the speed is not known.
	if (rawSpeed != 8191)
	{
		// Convert from .02 meters/sec to mph.
		speed_mph = rawSpeed / 50 * 2.2369362920544;

		isSuccess = true;
	}
	else
		speed_mph = 8191;

	BsmRoot = cJSON_CreateObject(); // create root node
	BsmMessageContent = cJSON_CreateArray(); // create root array

	cJSON_AddItemToObject(BsmRoot, "BsmMessageContent", BsmMessageContent); // add BsmMessageContent array to Bsmroot

	PLOG(logDEBUG)<<"Logging BasicSafetyMessage data";

	cJSON_AddItemToArray(BsmMessageContent, _BsmMessageContent = cJSON_CreateObject()); //add message content to BsmMessageContent array
	cJSON_AddItemToObject(_BsmMessageContent, "DSRC_MessageID", cJSON_CreateNumber(DSRCmsgID_basicSafetyMessage)); // DSRC_MessageID,  vehicle_ID
	cJSON_AddItemToObject(_BsmMessageContent, "BSM_tmp_ID", cJSON_CreateNumber(bsmTmpID)); // BSM_tmp_ID
	cJSON_AddItemToObject(_BsmMessageContent, "transtime", cJSON_CreateNumber(transtime)); // transtime
	cJSON_AddItemToObject(_BsmMessageContent, "latitude", cJSON_CreateNumber(latitude)); // latitude
	cJSON_AddItemToObject(_BsmMessageContent, "longitude", cJSON_CreateNumber(longitude)); // longitude
	cJSON_AddItemToObject(_BsmMessageContent, "speed_mph", cJSON_CreateNumber(speed_mph)); // speed_mph
	cJSON_AddItemToObject(_BsmMessageContent, "longAcceleration", cJSON_CreateNumber(longAcceleration)); // longAcceleration
	cJSON_AddItemToObject(_BsmMessageContent, "Heading", cJSON_CreateNumber(heading)); // Heading
	cJSON_AddItemToObject(_BsmMessageContent, "brakeStatus", cJSON_CreateString("")); // brakeStatus
	cJSON_AddItemToObject(_BsmMessageContent, "brakePressed", cJSON_CreateString("")); // brakePressed
	cJSON_AddItemToObject(_BsmMessageContent, "hardBraking", cJSON_CreateString("")); // hardBraking
	cJSON_AddItemToObject(_BsmMessageContent, "transTo", cJSON_CreateString("")); // transTo
	cJSON_AddItemToObject(_BsmMessageContent, "transmission_received_time", cJSON_CreateNumber(routeableMsg.get_millisecondsSinceEpoch())); // transmission_received_time in milliseconds since epoch


	if(bsm->partII != NULL) {
		if (bsm->partII[0].list.count >= partII_Value_PR_SpecialVehicleExtensions ) {
			try
			{
				if(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.trailers !=NULL){
					cJSON_AddItemToObject(_BsmMessageContent, "trailerPivot", cJSON_CreateNumber(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.trailers->connection.pivotOffset));
					cJSON_AddItemToObject(_BsmMessageContent, "trailreLength", cJSON_CreateNumber(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.trailers->units.list.array[0]->length));
					cJSON_AddItemToObject(_BsmMessageContent, "trailerHeight", cJSON_CreateNumber(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.trailers->units.list.array[0]->height[0]));
				}
				else 
				{
					cJSON_AddItemToObject(_BsmMessageContent, "trailerPivot", cJSON_CreateString(""));
					cJSON_AddItemToObject(_BsmMessageContent, "trailreLength", cJSON_CreateString(""));
					cJSON_AddItemToObject(_BsmMessageContent, "trailerHeight", cJSON_CreateString(""));
				}
			}
			catch(exception &e)
			{
				PLOG(logDEBUG)<<"Standard Exception:: Trailers unavailable "<<e.what();
			}
			try {
				if(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts != NULL){
					cJSON_AddItemToObject(_BsmMessageContent, "SirenState", cJSON_CreateNumber(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->sirenUse));	
					cJSON_AddItemToObject(_BsmMessageContent, "LightState", cJSON_CreateNumber(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->lightsUse));
				}
				else
				{
					cJSON_AddItemToObject(_BsmMessageContent, "SirenState", cJSON_CreateString(""));
					cJSON_AddItemToObject(_BsmMessageContent, "LightState", cJSON_CreateString(""));
				}
				
			}
			catch(exception &e)
			{
				PLOG(logDEBUG)<<"Standard Exception:: VehicleAlerts unavailable "<<e.what();
			}
		}
		if(bsm->partII[0].list.count >= partII_Value_PR_SupplementalVehicleExtensions){
		try {
			if(bsm->partII[0].list.array[2]->partII_Value.choice.SupplementalVehicleExtensions.classDetails != NULL) {	
				cJSON_AddItemToObject(_BsmMessageContent, "role", cJSON_CreateNumber(bsm->partII[0].list.array[2]->partII_Value.choice.SupplementalVehicleExtensions.classDetails->role[0]));
				cJSON_AddItemToObject(_BsmMessageContent, "keyType", cJSON_CreateNumber(bsm->partII[0].list.array[2]->partII_Value.choice.SupplementalVehicleExtensions.classDetails->keyType[0]));
			}
			else {
				cJSON_AddItemToObject(_BsmMessageContent, "role", cJSON_CreateString(""));
				cJSON_AddItemToObject(_BsmMessageContent, "keyType", cJSON_CreateString(""));
				cJSON_AddItemToObject(_BsmMessageContent, "responderType", cJSON_CreateString(""));
			}
		}			
		catch(exception &e)
			{
				PLOG(logDEBUG)<<"Standard Exception:: classDetails unavailable "<<e.what();
			}
		}
	}

	
	cJSON_AddItemToObject(_BsmMessageContent, "payload", cJSON_CreateString(routeableMsg.get_payload_str().c_str())); // payload

	std::stringstream metadata;
	std::string bsm_output_payload = routeableMsg.get_payload_str();

	metadata<<direction_hex.str()<<' '<<latitude_hex.str()<<' '<<longitude_hex.str()<<' '<<elevation_hex.str()<<' '<<rawspeed_hex.str()<<' '<<rawHeading_hex.str()<<' '<<bsmreceivetime_hex.str()<<' '<<bsmmillis16_hex.str()<<' '<<signStatus_hex.str()<<' '<<bsmlen_hex.str()<<' '<<header_hex.str()<<' '<<header_size_hex.str();

	unsigned int payload_size = bsm_output_payload.size();
	std::stringstream bsm_output_data;
	bsm_output_data<<metadata.str();
	for (int bsize = 0; bsize <= payload_size; bsize = bsize+2) {
     	bsm_output_data << ' ' << bsm_output_payload[bsize] << bsm_output_payload[bsize+1];
    	}
	int actualsize = bsm_output_data.str().size();
	int showmethesize = ((bsm_output_data.str().size()+1)/3);
	unsigned int binary_array[((bsm_output_data.str().size()+1)/3)];
	int dsize = 0;
	while (bsm_output_data.good() && dsize < bsm_output_data.str().size()){
		bsm_output_data >> std::hex >> binary_array[dsize];
		++dsize;
	}
	unsigned char binary_output[((bsm_output_data.str().size()+1)/3)];	
	for(int k=0;k<(bsm_output_data.str().size()+1)/3;k++) {
        binary_output[k]=static_cast<char>(binary_array[k]);
    	}
	std::ofstream _logFileBin(_curFilenamebin, std::ios::out | std::ios::binary | std::ios::app);
	_logFileBin.write((const char*)binary_output,sizeof(binary_output));
	BsmOut = cJSON_Print(BsmRoot);
	_logFile << BsmOut;
	free(BsmOut);

}



/**
 *  Opens a new log file in the directory specified of specified name for logging BSM messages and
 *  inserts a header row with names of fields that will be logged when data is received. If a log file
 *  with the same name already exists before opening a new file, it's renamed with current timestamp suffix.
 */
void MessageLoggerPlugin::OpenMSGLogFile()
{
	PLOG(logDEBUG) << "Message Log File: " << _curFilename << std::endl;;
	//rename logfile if one already exists
	std::string newFilename = _fileDirectory + "/" + _filename + GetCurDateTimeStr() + ".json";
        std::string newbinFilename = _fileDirectory + "/" + _filename + GetCurDateTimeStr() + ".bin";
	std::string _newFilename = newbinFilename.c_str();
	std::rename(_curFilename.c_str(), newFilename.c_str());
	std::rename(_curFilenamebin.c_str(), newbinFilename.c_str());
	_logFile.open(_curFilename);
	_logFilebin.open(_curFilenamebin, std::ios::out | std::ios::binary | std::ios::app);
	if (!_logFile.is_open())
		std::cerr << "Could not open log : " << strerror(errno) <<  std::endl;
	else
	{
		_logFile << "Message Log file" << GetCurDateTimeStr() << endl;

	}
}

/**
 * Checks the size of the logfile and opens a new_fileDirectory file if it's size is greater
 * than the max size specified.
 */
void MessageLoggerPlugin::CheckMSGLogFileSizeAndRename(bool createNewFile)
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
			createNewFile = true;
			_logFile.close();
			_logFilebin.close();
			OpenMSGLogFile();
		}
	}
}

/**
 * Returns the current data time as string.
 * @return current time in ddmmyyhhmiss format.
 */
std::string MessageLoggerPlugin::GetCurDateTimeStr()
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
int MessageLoggerPlugin::Main()
{
	PLOG(logDEBUG) << "Starting MessageLoggerplugin...";

	uint msCount = 0;
	while (_plugin->state != IvpPluginState_error)
	{
		PLOG(logDEBUG4) << "MessageLoggerPlugin Sleeping" << endl;

		this_thread::sleep_for(chrono::milliseconds(1000));

		// check size of the log file and open new one if needed
		CheckMSGLogFileSizeAndRename(true);
	}

	PLOG(logDEBUG) << "MessageLoggerPlugin terminating gracefully.";
	return EXIT_SUCCESS;
}

} /* namespace MessageLoggerPlugin */


/**
 * Main method for running the plugin
 * @param argc number of arguments
 * @param argv array of arguments
 */
int main(int argc, char *argv[])
{
	return run_plugin<MessageLoggerPlugin::MessageLoggerPlugin>("MessageLoggerPlugin", argc, argv);
}

