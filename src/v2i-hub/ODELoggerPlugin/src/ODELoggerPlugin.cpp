
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


#include "ODELoggerPlugin.h"

 namespace ODELoggerPlugin
 {

 /**
  * Construct a new ODELoggerPlugin with the given name.
  *
  * @param name The name to give the plugin for identification purposes.
  */
 ODELoggerPlugin::ODELoggerPlugin(string name): PluginClient(name)
 {
 	PLOG(logDEBUG)<<"ODELoggerPlugin::In ODELoggerPlugin Constructor";
 	// The log level can be changed from the default here.
 	//FILELog::ReportingLevel() = FILELog::FromString("DEBUG");

 	// Critical section
 	std::lock_guard<mutex> lock(_cfgLock);
 	GetConfigValue<uint16_t>("schedule_frequency", _scheduleFrequency);
 	GetConfigValue<uint16_t>("ForwardBsm", _forwardBSM);
 	GetConfigValue<string>("BSMKafkaTopic", _BSMkafkaTopic);
 	GetConfigValue<string>("KafkaBrokerIp", _kafkaBrokerIp);
 	GetConfigValue<string>("KafkaBrokerPort", _kafkaBrokerPort);
	std::string error_string;
	_freqCounter=1;

 	if(_forwardBSM == 1) {
 		kafkaConnectString = _kafkaBrokerIp + ':' + _kafkaBrokerPort;
 		kafka_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

 		PLOG(logDEBUG) <<"ODELoggerPlugin: Attempting to connect to " << kafkaConnectString;
 		if ((kafka_conf->set("bootstrap.servers", kafkaConnectString, error_string) != RdKafka::Conf::CONF_OK)) {
 			PLOG(logDEBUG) <<"ODELoggerPlugin: Setting kafka config options failed with error:" << error_string;
 			PLOG(logDEBUG) <<"ODELoggerPlugin: Exiting with exit code 1";
 			exit(1);
 		} else {
 			PLOG(logDEBUG) <<"ODELoggerPlugin: Kafka config options set succesfully";
 		}

 		kafka_producer = RdKafka::Producer::create(kafka_conf, error_string);
 		if (!kafka_producer) {
 			PLOG(logDEBUG) <<"ODELoggerPlugin: Creating kafka producer failed with error:" << error_string;
 			PLOG(logDEBUG) <<"ODELoggerPlugin: Exiting with exit code 1";
 			exit(1);
 		}
 		PLOG(logDEBUG) <<"ODELoggerPlugin: Kafka producer created";

 		AddMessageFilter < BsmMessage > (this, &ODELoggerPlugin::HandleRealTimePublish);
 	}
 	// Subscribe to all messages specified by the filters above.
 	SubscribeToMessages();

 	PLOG(logDEBUG) <<"ODELoggerPlugin: Exiting ODELoggerPlugin Constructor";
 }

 /**
  * Destructor
  */

 ODELoggerPlugin::~ODELoggerPlugin()
 {

 }


 /**
  * Updates configuration settings
  */
 void ODELoggerPlugin::UpdateConfigSettings()
 {
 	// Configuration settings are retrieved from the API using the GetConfigValue template class.
 	// This method does NOT execute in the main thread, so variables must be protected
 	// (e.g. using std::atomic, std::mutex, etc.).

 	int instance;
 	GetConfigValue("Instance", instance);


 	std::lock_guard<mutex> lock(_cfgLock);
 	GetConfigValue<uint16_t>("schedule_frequency", _scheduleFrequency);
 	GetConfigValue<uint16_t>("ForwardBsm", _forwardBSM);
 	GetConfigValue<string>("BSMKafkaTopic", _BSMkafkaTopic);
 	GetConfigValue<string>("KafkaBrokerIp", _kafkaBrokerIp);
 	GetConfigValue<string>("KafkaBrokerPort", _kafkaBrokerPort);
 	kafkaConnectString = _kafkaBrokerIp + ':' + _kafkaBrokerPort;

 }

 /**
  * Called when configuration is changed
  *
  * @param key Key of the configuration value changed
  * @param value Changed value
  */
 void ODELoggerPlugin::OnConfigChanged(const char *key, const char *value)
 {
 	PluginClient::OnConfigChanged(key, value);
 	UpdateConfigSettings();
 }

 /**
  * Called on plugin state change
  *
  * @para state New plugin state
  */
 void ODELoggerPlugin::OnStateChange(IvpPluginState state)
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
  * subscribed for.  This particular method decodes the BSM message and
  * logs selective fields to a log file.
  *
  * @param msg BSMMessage that is received
  * @routeable_message not used
  */
 void ODELoggerPlugin::HandleRealTimePublish(BsmMessage &msg,
 		routeable_message &routeableMsg) {

 	//cout<<"ODELoggerPlugin: Starting BSM publish1\n";
 	auto bsm = msg.get_j2735_data();

 	float speed_mph;
 	int32_t bsmTmpID;

 	bool isSuccess = false;
 	//asn_fprint(stdout, &asn_DEF_BasicSafetyMessage, bsm);
 	int32_t latitude = bsm->coreData.lat;
 	int32_t longitude = bsm->coreData.Long;
 	int32_t longAcceleration = bsm->coreData.accelSet.Long;


 	uint16_t rawSpeed = bsm->coreData.speed;
 	uint16_t rawHeading = bsm->coreData.heading;
 	GetInt32((unsigned char *)bsm->coreData.id.buf, &bsmTmpID);

 	rapidjson::Document document;
 	rapidjson::Value bsmPartOne(rapidjson::kObjectType);
 	rapidjson::Value bsmPartTwo(rapidjson::kObjectType);

 	document.SetObject();
 	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
 	bsmPartOne.AddMember("latitude", latitude, allocator);
 	bsmPartOne.AddMember("longitude", longitude, allocator);
 	bsmPartOne.AddMember("longitudinal_accel", longAcceleration, allocator);
 	bsmPartOne.AddMember("raw_speed", rawSpeed, allocator);
 	bsmPartOne.AddMember("raw_heading", rawHeading, allocator);
 	document.AddMember("PartOneContents", bsmPartOne, allocator);

 	// Heading units are 0.0125 degrees.
 	float heading = rawHeading / 80.0;
	//cout<<"ODELoggerPlugin: Starting BSM publish2\n";
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

 	if(bsm->partII != NULL) {
 		if (bsm->partII[0].list.count >= partII_Value_PR_SpecialVehicleExtensions ) {
 			try
 			{
 				if(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.trailers !=NULL){
 					bsmPartTwo.AddMember("trailers_pivot_offset", bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.trailers->connection.pivotOffset, allocator);
 					bsmPartTwo.AddMember("trailers_length", bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.trailers->units.list.array[0]->length, allocator);
 					bsmPartTwo.AddMember("trailers_height", bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.trailers->units.list.array[0]->height[0], allocator);
 				}
 				else
 				{
 					PLOG(logDEBUG)<<"ODELoggerPlugin: No BSM Part 2 trailer contents";
 				}
				//cout<<"ODELoggerPlugin: Starting BSM publish3\n";
 			}
 			catch(exception &e)
 			{
 				PLOG(logDEBUG)<<"Standard Exception:: Trailers unavailable "<<e.what();
				//cout<<"ODELoggerPlugin: Starting BSM publish3\n";
 			}
 			try {
 				if(bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts != NULL){
 					bsmPartTwo.AddMember("alert_sirenUse", bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->sirenUse, allocator);
 					bsmPartTwo.AddMember("alerts_lightsUse", bsm->partII[0].list.array[1]->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->lightsUse, allocator);

 				}
 				else
 				{
 					PLOG(logDEBUG)<<"ODELoggerPlugin: No BSM Part 2 vehicleAlerts contents";
 				}
				//cout<<"ODELoggerPlugin: Starting BSM publish4\n";

 			}
 			catch(exception &e)
 			{
 				PLOG(logDEBUG)<<"Standard Exception:: VehicleAlerts unavailable "<<e.what();				//cout<<"ODELoggerPlugin: Starting BSM publish4\n";

 			}
 		}
 		if(bsm->partII[0].list.count >= partII_Value_PR_SupplementalVehicleExtensions){
		//cout<<"ODELoggerPlugin: Starting BSM publish5\n";

 		try {
 			if(bsm->partII[0].list.array[2]->partII_Value.choice.SupplementalVehicleExtensions.classDetails != NULL) {
 				bsmPartTwo.AddMember("classDetails_role", bsm->partII[0].list.array[2]->partII_Value.choice.SupplementalVehicleExtensions.classDetails->role[0], allocator);
				//cout<<"ODELoggerPlugin: Starting BSM publish5\n";

 				bsmPartTwo.AddMember("classDetails_keyType", bsm->partII[0].list.array[2]->partII_Value.choice.SupplementalVehicleExtensions.classDetails->keyType[0], allocator);
				//cout<<"ODELoggerPlugin: Starting BSM publish5\n";

 				//bsmPartTwo.AddMember("classDetails_responderType", bsm->partII[0].list.array[2]->partII_Value.choice.SupplementalVehicleExtensions.classDetails->responderType[0], allocator);
				//cout<<"ODELoggerPlugin: Starting BSM publish5\n";

 			}
 			else {
 					PLOG(logDEBUG)<<"ODELoggerPlugin: No BSM Part 2 classDetails contents";
 			}
			//cout<<"ODELoggerPlugin: Starting BSM publish5\n";

 		}
 		catch(exception &e)
 			{
 				PLOG(logDEBUG)<<"Standard Exception:: classDetails unavailable "<<e.what();
				//cout<<"ODELoggerPlugin: Starting BSM publish5\n";

 			}
 		}
 	}

	//cout<<"ODELoggerPlugin: Starting BSM publish6\n";

 	document.AddMember("PartTwoContents", bsmPartTwo, allocator);
 	StringBuffer buffer;
 	Writer<StringBuffer> writer(buffer);
 	document.Accept(writer);
	//  check for schedule 
	if(_freqCounter++%_scheduleFrequency == 0)
 		QueueKafkaMessage(kafka_producer, _BSMkafkaTopic, buffer.GetString());
 }

 void ODELoggerPlugin::QueueKafkaMessage(RdKafka::Producer *producer, std::string topic, std::string message)
 {
	bool retry = true, return_value = false;
  	PLOG(logDEBUG) <<"ODELoggerPlugin: Queueing kafka message:topic:" << topic << " " << producer->outq_len() <<"messages already in queue";

 	while (retry) {
 		RdKafka::ErrorCode produce_error = producer->produce(topic, RdKafka::Topic::PARTITION_UA,
 			RdKafka::Producer::RK_MSG_COPY, const_cast<char *>(message.c_str()),
 			message.size(), NULL, NULL, 0, 0);

     	if (produce_error == RdKafka::ERR_NO_ERROR) {
       		PLOG(logDEBUG) <<"ODELoggerPlugin: Queued message:" << message;
       		retry = false; return_value = true;
     	}
 		else {
 			PLOG(logDEBUG) <<"ODELoggerPlugin: Failed to queue message:" << message <<" with error:" << RdKafka::err2str(produce_error);
			if (produce_error == RdKafka::ERR__QUEUE_FULL) {
				PLOG(logDEBUG) <<"ODELoggerPlugin: Message queue full...retrying...";
				producer->poll(500);  /* ms */
				retry = true;
			}
			else {
				PLOG(logDEBUG) <<"ODELoggerPlugin: Unhandled error in queue_kafka_message:" << RdKafka::err2str(produce_error);
				retry = false;
			}
     	}
   }
   PLOG(logDEBUG) <<"ODELoggerPlugin: Queueing kafka message completed";
   //return(return_value);	 	
 }

 /**
  *  Opens a new log file in the directory specified of specified name for logging BSM messages and
  *  inserts a header row with names of fields that will be logged when data is received. If a log file
  *  with the same name already exists before opening a new file, it's renamed with current timestamp suffix.
  */
//  void ODELoggerPlugin::OpenBSMLogFile()
//  {
//  	PLOG(logDEBUG) << "BSM Log File: " << _curFilename << std::endl;;
//  	//rename logfile if one already exists
//  	std::string newFilename = _fileDirectory + "/" + _filename + GetCurDateTimeStr() + ".csv";
//  	std::rename(_curFilename.c_str(), newFilename.c_str());

//  	_logFile.open(_curFilename);
//  	if (!_logFile.is_open())
//  		std::cerr << "Could not open log : " << strerror(errno) <<  std::endl;
//  	else
//  	{
//  		_logFile << "DSRC_MessageID, "
//  				"Vehicle ID, "
//  				"BSM_tmp_ID, "
//  				"transtime, "
//  				"X, Y, "
//  				"Speed, "
//  				"Instant_Acceleration, "
//  				"Heading, "
//  				"brakeStatus, "
//  				"brakePressure, "
//  				"hardBraking,  "
//  				"transTo, "
//  				"transmission_received_time, "
//  				"trailerPivot, "
//  				"trailreLength, "
//  				"trailerHeight, "
//  				"vehicleRole, "
//  				"vehicletype, "
//  				"Respondertype, "
//  				"SirenState, "
//  				"LightState, "
//  				"VehicleDescription, "
//  				"" << endl;

//  	}
//  }

//  /**
//   * Checks the size of the logfile and opens a new file if it's size is greater
//   * than the max size specified.
//   */
//  void ODELoggerPlugin::CheckBSMLogFileSizeAndRename(bool createNewFile)
//  {
//  	if (_logFile.is_open())
//  	{
//  		std::lock_guard<mutex> lock(_cfgLock);
//  		_logFile.seekp( 0, std::ios::end );
//  		int curFilesizeInMB = _logFile.tellp()/BYTESTOMB;
//  		if (curFilesizeInMB > _maxFilesizeInMB || createNewFile)
//  		{
//  			_logFile.close();
//  			OpenBSMLogFile();
//  		}
//  	}
//  }

//  /**
//   * Returns the current data time as string.
//   * @return current time in ddmmyyhhmiss format.
//   */
//  std::string ODELoggerPlugin::GetCurDateTimeStr()
//  {
//  	auto t = std::time(nullptr);
//  	auto tm = *std::localtime(&t);

//  	std::ostringstream oss;
//  	oss << std::put_time(&tm, "%d%m%Y%H%M%S");
//  	auto str = oss.str();
//  	return str;
//  }

 // Override of main method of the plugin that should not return until the plugin exits.
 // This method does not need to be overridden if the plugin does not want to use the main thread.
 int ODELoggerPlugin::Main()
 {
 	PLOG(logDEBUG) <<"ODELoggerPlugin: Starting ODELoggerPlugin...";

 	uint msCount = 0;
 	while (_plugin->state != IvpPluginState_error)
 	{
 		PLOG(logDEBUG4) <<"ODELoggerPlugin: Sleeping 5 minutes" << endl;

 		this_thread::sleep_for(chrono::milliseconds(300000));

 		// check size of the log file and open new one if needed
 		//CheckBSMLogFileSizeAndRename(true);
 	}

 	PLOG(logDEBUG) <<"ODELoggerPlugin: ODELoggerPlugin terminating gracefully.";
 	return EXIT_SUCCESS;
 }

 } /* namespace ODELoggerPlugin */


 /**
  * Main method for running the plugin
  * @param argc number of arguments
  * @param argv array of arguments
  */
 int main(int argc, char *argv[])
 {
 	return run_plugin<ODELoggerPlugin::ODELoggerPlugin>("ODELoggerPlugin", argc, argv);
 }
