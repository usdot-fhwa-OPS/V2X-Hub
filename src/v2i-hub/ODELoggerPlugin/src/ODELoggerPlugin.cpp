
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
 	GetConfigValue<uint16_t>("ForwardMSG", _forwardMSG);
 	GetConfigValue<string>("BSMKafkaTopic", _BSMkafkaTopic);
 	GetConfigValue<string>("SPaTKafkaTopic", _BSMkafkaTopic);
 	GetConfigValue<string>("KafkaBrokerIp", _kafkaBrokerIp);
 	GetConfigValue<string>("KafkaBrokerPort", _kafkaBrokerPort);
	std::string error_string;
	_freqCounter=1;

 	if(_forwardMSG == 1) {
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
 		AddMessageFilter < SpatMessage > (this, &ODELoggerPlugin::HandleSPaTPublish);
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
 	GetConfigValue<uint16_t>("ForwardMSG", _forwardMSG);
 	GetConfigValue<string>("BSMKafkaTopic", _BSMkafkaTopic);
 	GetConfigValue<string>("SPaTKafkaTopic", _SPaTkafkaTopic);
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

 template <class Precision>
 string getISOCurrentTimestamp()
 {
     auto now = chrono::system_clock::now();
     return date::format("%FT%TZ", date::floor<Precision>(now));
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

	auto bsm=msg.get_j2735_data();

        char *teststring=new char[10000];

        std::sprintf(teststring, "{\"BsmMessageContent\":[{\"metadata\":{\"utctimestamp\":\"%s\"},\"payload\":\"%s\"}]}",getISOCurrentTimestamp<chrono::microseconds>().c_str(),routeableMsg.get_payload_str().c_str());

        //  check for schedule
        if(_freqCounter++%_scheduleFrequency == 0)
                QueueKafkaMessage(kafka_producer, _BSMkafkaTopic, teststring);	
	

 }


 void ODELoggerPlugin::HandleSPaTPublish(SpatMessage &msg,
 		routeable_message &routeableMsg) {

	auto spat=msg.get_j2735_data();

        char *spatstring=new char[10000];

        std::sprintf(spatstring, "{\"SpatMessageContent\":[{\"metadata\":{\"utctimestamp\":\"%s\"},\"payload\":\"%s\"}]}",getISOCurrentTimestamp<chrono::microseconds>().c_str(),routeableMsg.get_payload_str().c_str());

        //  check for schedule
        if(_freqCounter++%_scheduleFrequency == 0)
                QueueKafkaMessage(kafka_producer, _SPaTkafkaTopic, spatstring);	
	

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
