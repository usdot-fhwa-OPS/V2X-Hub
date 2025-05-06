
/**
 * Copyright (C) 2025 LEIDOS.
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
MessageLoggerPlugin::MessageLoggerPlugin(const std::string &name) : PluginClient(name)
{
	PLOG(logDEBUG)<< "In MessageLoggerPlugin Constructor";

	// Add a message filter and handler for each message this plugin wants to receive.
	AddMessageFilter < BsmMessage > (this, &MessageLoggerPlugin::HandleBasicSafetyMessage);
	AddMessageFilter < SpatMessage > (this, &MessageLoggerPlugin::HandleSpatMessage);

	// Subscribe to all messages specified by the filters above.
	SubscribeToMessages();

	PLOG(logDEBUG) << "Exit MessageLoggerPlugin Constructor";
}

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
		OpenMSGLogFile();
	}
}

void MessageLoggerPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void MessageLoggerPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
	}
}

void MessageLoggerPlugin::HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg) 
{
	// check size of the log file and open new one if needed
	CheckMSGLogFileSizeAndRename();
	char *BsmOut;
	cJSON *BsmRoot, *BsmMessageContent, *_BsmMessageContent;

	PLOG(logDEBUG)<<"HandleBasicSafetyMessage";
	try {
		auto bsm = msg.get_j2735_data();

		float speed_mph;
		int32_t bsmTmpID;

		std::stringstream direction_hex;
		direction_hex<<"01";
		
		std::stringstream signStatus_hex;
		signStatus_hex<<"00";

		bool isSuccess = false;

		// Retrieve bsm lat int32_t
		int32_t latitude = bsm->coreData.lat;
		// Swap bytes to little endian
		int32_t latitude_sw = __builtin_bswap32(latitude);
		std::stringstream latitude_int_hex; 
		// Creates 4 byte hex string including leading zeros
		latitude_int_hex << std::setfill('0') << std::setw(sizeof(int32_t)*2) << std::hex << latitude_sw;
		// Format string include spaces between each byte
		std::stringstream latitude_hex;
		latitude_hex<<latitude_int_hex.str()[0]<<latitude_int_hex.str()[1] << ' '
			<<latitude_int_hex.str()[2]<<latitude_int_hex.str()[3] << ' '
			<<latitude_int_hex.str()[4]<<latitude_int_hex.str()[5] << ' '
			<<latitude_int_hex.str()[6]<<latitude_int_hex.str()[7];
			
		// Retrieve bsm long int32_t
		int32_t longitude = bsm->coreData.Long;
		// Swap bytes to little endian
		int32_t longitude_sw = __builtin_bswap32(longitude);
		std::stringstream longitude_int_hex;
		// Creates 4 byte hex string including leading zeros
		longitude_int_hex << std::setfill('0') << std::setw(sizeof(int32_t)*2) << std::hex << longitude_sw;
		// Format string include spaces between each byte
		std::stringstream longitude_hex;
		longitude_hex<<longitude_int_hex.str()[0]<<longitude_int_hex.str()[1] << ' '
			<<longitude_int_hex.str()[2]<<longitude_int_hex.str()[3] << ' '
			<<longitude_int_hex.str()[4]<<longitude_int_hex.str()[5] << ' '
			<<longitude_int_hex.str()[6]<<longitude_int_hex.str()[7];
			
		// Retrieve bsm elevation int32_t
		int32_t elevation = bsm->coreData.elev;
		// Swap bytes to little endian
		int32_t elevation_sw = __builtin_bswap32(elevation);
		std::stringstream elevation_int_hex;
		// Creates 4 byte hex string including leading zeros
		elevation_int_hex<<std::setfill('0') << std::setw(sizeof(int32_t)*2) << std::hex <<elevation_sw;
		// Format string include spaces between each byte
		std::stringstream elevation_hex;
		elevation_hex<<elevation_int_hex.str()[0]<<elevation_int_hex.str()[1] << ' '
			<<elevation_int_hex.str()[2]<<elevation_int_hex.str()[3] << ' '
			<<elevation_int_hex.str()[4]<<elevation_int_hex.str()[5] << ' '
			<<elevation_int_hex.str()[6]<<elevation_int_hex.str()[7] << ' ';
			


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
		
		// Retrieve bsm length uint16_t
		int16_t bsmlen = header_size+4;
		// Swap bytes to little endian
		uint16_t bsmlen_sw = __builtin_bswap16(bsmlen);
		std::stringstream bsmlen_int_hex;	
		// Creates 2 byte hex string including leading zeros
		bsmlen_int_hex<< std::setfill('0') << std::setw(sizeof(uint16_t)*2) << std::hex << bsmlen_sw;
		std::stringstream bsmlen_hex;
		// Format string include spaces between each byte
		bsmlen_hex<<bsmlen_int_hex.str()[0]<<bsmlen_int_hex.str()[1]<<' '<<bsmlen_int_hex.str()[2]<<bsmlen_int_hex.str()[3];

		// Retrieve bsm speed uint16_t
		uint16_t rawSpeed = bsm->coreData.speed;
		// Swap bytes to little endian
		uint16_t rawspeed_sw = __builtin_bswap16(rawSpeed);
		std::stringstream rawspeed_int_hex;
		// Creates 2 byte hex string including leading zeros
		rawspeed_int_hex<< std::setfill('0') << std::setw(sizeof(uint16_t)*2) <<std::hex<<rawspeed_sw;
		std::stringstream rawspeed_hex;
		// Format string include spaces between each byte
		rawspeed_hex<<rawspeed_int_hex.str()[0]<<rawspeed_int_hex.str()[1] <<' '<<rawspeed_int_hex.str()[2]<<rawspeed_int_hex.str()[3];

		// Retrieve bsm heading uint16_t
		uint16_t rawHeading = bsm->coreData.heading;
		// Swap bytes to little endian
		int16_t rawHeading_sw = __builtin_bswap16(rawHeading);
		std::stringstream rawHeading_int_hex;
		// Creates 2 byte hex string including leading zeros
		rawHeading_int_hex<<std::setfill('0') << std::setw(sizeof(int16_t)*2) <<std::hex<<rawHeading_sw;
		std::stringstream rawHeading_hex;
		// Format string include spaces between each byte
		rawHeading_hex<<rawHeading_int_hex.str()[0]<<rawHeading_int_hex.str()[1]<<' '<<rawHeading_int_hex.str()[2]<<rawHeading_int_hex.str()[3];

		// Retrieve bsm receivetime uint32_t
		uint32_t bsmreceivetime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		// Swap bytes to little endian
		uint32_t bsmreceivetime_sw = __builtin_bswap32(bsmreceivetime);
		std::stringstream bsmreceivetime_int_hex;
		// Creates 4 byte hex string including leading zeros
		bsmreceivetime_int_hex<< std::setfill('0') << std::setw(sizeof(uint32_t)*2) <<std::hex << bsmreceivetime_sw;
		// Format string include spaces between each byte
		std::stringstream bsmreceivetime_hex;
		bsmreceivetime_hex<<bsmreceivetime_int_hex.str()[0]<<bsmreceivetime_int_hex.str()[1] << ' ' 
			<< bsmreceivetime_int_hex.str()[2]<< bsmreceivetime_int_hex.str()[3] << ' '
			<< bsmreceivetime_int_hex.str()[4]<< bsmreceivetime_int_hex.str()[5] << ' '
			<< bsmreceivetime_int_hex.str()[6]<< bsmreceivetime_int_hex.str()[7];
			
		// Retrieve bsm msec uint16_t
		int64_t bsmreceivetimemillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		int bsmmillis = bsmreceivetimemillis - (bsmreceivetime*1000);
		uint16_t bsmmillis16 = (uint16_t) bsmmillis;
		// Swap bytes to little endian
		uint16_t bsmmillis16_sw = __builtin_bswap16(bsmmillis16);
		std::stringstream bsmmillis16_int_hex;
		// Creates 2 byte hex string including leading zeros
		bsmmillis16_int_hex<< std::setfill('0') << std::setw(sizeof(uint16_t)*2) << std::hex<< bsmmillis16_sw;
		std::stringstream bsmmillis16_hex;
		// Format string include spaces between each byte
		bsmmillis16_hex << bsmmillis16_int_hex.str()[0]<<bsmmillis16_int_hex.str()[1]<<' '<<bsmmillis16_int_hex.str()[2]<<bsmmillis16_int_hex.str()[3];

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
			if (bsm->partII[0].list.count >= BSMpartIIExtension__partII_Value_PR_SpecialVehicleExtensions ) {
				try
				{
					if(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.doNotUse !=NULL){
						cJSON_AddItemToObject(_BsmMessageContent, "trailerPivot", cJSON_CreateNumber(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.doNotUse->connection.pivotOffset));
						cJSON_AddItemToObject(_BsmMessageContent, "trailreLength", cJSON_CreateNumber(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.doNotUse->units.list.array[0]->length));
						cJSON_AddItemToObject(_BsmMessageContent, "trailerHeight", cJSON_CreateNumber(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.doNotUse->units.list.array[0]->height[0]));
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
			if(bsm->partII[0].list.count >= BSMpartIIExtension__partII_Value_PR_SupplementalVehicleExtensions){
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
		// ----------------- Header Description-------------------------
		// uint8_t direction ("01")
		// int32_t latitude
		// int32_t longitude
		// int32_t elevation
		// uint16_t speed
		// uint16_t heading
		// uint32_t bsmreceivetime
		// uint16_t bsmmillis
		// int8_t signStatus
		// uint16_t bsmlen
		// "03 80 81"
		// uint8_t header_size
		// -------------------------------------------------------------
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
	catch (J2735Exception &e) {
           FILE_LOG(logERROR) << "Exception caught " << e.what() << std::endl << e.GetBacktrace() << std::endl;
    }

}

void MessageLoggerPlugin::HandleSpatMessage(SpatMessage &msg, routeable_message &routeableMsg) 
{
	// check size of the log file and open new one if needed
    CheckMSGLogFileSizeAndRename();
	char *SpatOut;
        cJSON *SpatRoot, *SpatMessageContent, *_SpatMessageContent;
	
	PLOG(logDEBUG)<<"HandleSPaTMessage" << std::endl;
        try {
			auto spat = msg.get_j2735_data();
			
			std::stringstream direction_hex;
			direction_hex<<"01";

			std::stringstream signStatus_hex;
			signStatus_hex<<"00";

			// Retrieve spat intersectionId uint16_t       
			uint16_t intersectionId = spat->intersections.list.array[0]->id.id;
			// Swap bytes to little endian
			uint16_t intersectionId_sw = __builtin_bswap16(intersectionId);
			std::stringstream intersectionId_int_hex;
			// Creates 2 byte hex string including leading zeros
			intersectionId_int_hex<< std::setfill('0') << std::setw(sizeof(uint16_t)*2) << std::hex <<intersectionId_sw;
			std::stringstream intersectionId_hex;
			// Format string include spaces between each byte 
			intersectionId_hex<<intersectionId_int_hex.str()[0]<<intersectionId_int_hex.str()[1]<<' '<<intersectionId_int_hex.str()[2]<<intersectionId_int_hex.str()[3];

			// Retrieve spat interstatus int8_t       
			std::string interstatus = std::bitset<8>(spat->intersections.list.array[0]->status.buf).to_string();
			int interstatint_sw = stoi(interstatus);
			std::stringstream interstatint_int_hex;
			// Creates 1 byte hex string including leading zeros
			interstatint_int_hex<< std::setfill('0') << std::setw(sizeof(int8_t)*2) << std::hex << interstatint_sw;
			std::stringstream interstatint_hex;
			interstatint_hex<<interstatint_int_hex.str()[0]<<interstatint_int_hex.str()[1];

			// Retrieve spat receivetime uint32_t       
			uint32_t spatreceivetime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			// Swap bytes to little endian
			uint32_t spatreceivetime_sw = __builtin_bswap32(spatreceivetime);
			std::stringstream spatreceivetime_int_hex;
			// Creates 4 byte hex string including leading zeros
			spatreceivetime_int_hex<<std::setfill('0') << std::setw(sizeof(uint32_t)*2) << std::hex <<spatreceivetime_sw;
			std::stringstream spatreceivetime_hex;
			// Format string include spaces between each byte 
			spatreceivetime_hex<<spatreceivetime_int_hex.str()[0]<<spatreceivetime_int_hex.str()[1] << ' '
					<<spatreceivetime_int_hex.str()[2]<<spatreceivetime_int_hex.str()[3] << ' '
					<<spatreceivetime_int_hex.str()[4]<<spatreceivetime_int_hex.str()[5] << ' '
					<<spatreceivetime_int_hex.str()[6]<<spatreceivetime_int_hex.str()[7];
	
			// Retrieve spat receivetimemillis uint16_t       
			uint64_t spatreceivetimemillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			unsigned int spatmillis = spatreceivetimemillis - (spatreceivetime*1000);
			uint16_t spatmillis16 = (uint16_t) spatmillis;
			// Swap bytes to little endian
			uint16_t spatmillis16_sw = __builtin_bswap16(spatmillis16);
			std::stringstream spatmillis16_int_hex;
			// Creates 2 byte hex string including leading zeros
			spatmillis16_int_hex<< std::setfill('0') << std::setw(sizeof(uint16_t)*2) << std::hex <<spatmillis;
			std::stringstream spatmillis16_hex;
			// Format string include spaces between each byte
			spatmillis16_hex<< spatmillis16_int_hex.str()[0]<<spatmillis16_int_hex.str()[1]<<' '<<spatmillis16_int_hex.str()[2]<<spatmillis16_int_hex.str()[3];

			std::stringstream IsCertPresent_hex;
			IsCertPresent_hex<<"00";

			// Retrieve spat spatsize uint16_t       
			uint16_t spat_size;
			if (routeableMsg.get_payload_str().length()%4 == 0){
					spat_size = routeableMsg.get_payload_str().length()/2;
			}
			else{
					spat_size = (routeableMsg.get_payload_str().length()/2)+1;
			}
			// Swap bytes to little endian
			uint16_t spat_size_test = __builtin_bswap16(spat_size);
			std::stringstream spat_size_int_hex;
			// Creates 2 byte hex string including leading zeros
			spat_size_int_hex<< std::setfill('0') << std::setw(sizeof(uint16_t)*2) << std::hex << spat_size_test;
			std::stringstream spat_size_hex;
			// Format string include spaces between each byte
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

			// ----------------- Header Description-------------------------
			// uint8_t direction ("01")
			// int16_t intersectionId
			// int8_t intersectionStatus
			// uint32_t spatreceivetime
			// uint16_t spatmillis
			// int8_t signStatus
			// int8_t isCertPresent
			// uint16_t length
			// -------------------------------------------------------------
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
        catch (J2735Exception &e) {
           FILE_LOG(logERROR) << "Exception caught " << e.what() << std::endl << e.GetBacktrace() << std::endl;
        }

}

void MessageLoggerPlugin::OpenMSGLogFile()
{
	PLOG(logDEBUG) << "Message Log File: " << _curFilename << std::endl;;
	//rename logfile if one already exists
	if ( !boost::filesystem::exists( _fileDirectory + "/json/") ){
			boost::filesystem::create_directory( _fileDirectory + "/json/");            
	}
	if ( !boost::filesystem::exists( _fileDirectory + "/ode/") ){
			boost::filesystem::create_directory( _fileDirectory + "/ode/");            
	}
	std::string newFilename = _fileDirectory + "/json/" + _filename + GetCurDateTimeStr() + ".json";
    std::string newbinFilename = _fileDirectory + "/ode/" + _filename + GetCurDateTimeStr() + ".bin";
	std::string _newFilename = newbinFilename.c_str();
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
		PLOG(logERROR) << "Could not open log : " << strerror(errno);
	else
	{
		_logFile << "Message JSON Logs" << endl;

	}
}

void MessageLoggerPlugin::CheckMSGLogFileSizeAndRename()
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
			OpenMSGLogFile();
		}
	}
}

std::string MessageLoggerPlugin::GetCurDateTimeStr()
{
	auto t = std::time(nullptr);
	struct tm tm;
	localtime_r(&t, &tm);
	std::ostringstream oss;
	oss << std::put_time(&tm, "%d%m%Y%H%M%S");
	auto str = oss.str();
	return str;
}

// Override of main method of the plugin that should not return until the plugin exits.
// This method does not need to be overridden if the plugin does not want to use the main thread.
int MessageLoggerPlugin::Main()
{
	PLOG(logINFO) << "Starting MessageLoggerplugin...";

	while (_plugin->state != IvpPluginState_error)
	{
		PLOG(logDEBUG4) <<"MessageLoggerPlugin: Sleeping 5 minutes";
		this_thread::sleep_for(chrono::milliseconds(300000));
	}

	PLOG(logINFO) << "MessageLoggerPlugin terminating gracefully.";
	return EXIT_SUCCESS;
}

} /* namespace MessageLoggerPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<MessageLoggerPlugin::MessageLoggerPlugin>("MessageLoggerPlugin", argc, argv);
}

