
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


#include "ODEForwardPlugin.h"

 namespace ODEForwardPlugin
 {
			
 /**
  * Construct a new ODEForwardPlugin with the given name.
  *
  * @param name The name to give the plugin for identification purposes.
  */
 ODEForwardPlugin::ODEForwardPlugin(string name): PluginClient(name)
 {
 	PLOG(logDEBUG)<<"ODEForwardPlugin::In ODEForwardPlugin Constructor";
 	// The log level can be changed from the default here.
 	//FILELog::ReportingLevel() = FILELog::FromString("DEBUG");

	AddMessageFilter < BsmMessage > (this, &ODEForwardPlugin::HandleRealTimePublish);
 	AddMessageFilter < SpatMessage > (this, &ODEForwardPlugin::HandleSPaTPublish);
	AddMessageFilter < MapDataMessage > (this, &ODEForwardPlugin::HandleMapPublish);
	AddMessageFilter < TimMessage > (this, &ODEForwardPlugin::HandleTimPublish);
 	// Subscribe to all messages specified by the filters above.
 	SubscribeToMessages();
	_udpMessageForwarder = std::make_shared<UDPMessageForwarder>();
	_communicationModeHelper = std::make_shared<CommunicationModeHelper>();
 	PLOG(logDEBUG) <<"ODEForwardPlugin: Exiting ODEForwardPlugin Constructor";
 }

 /**
  * Destructor
  */

 ODEForwardPlugin::~ODEForwardPlugin()
 {

 }


 /**
  * Updates configuration settings
  */
 void ODEForwardPlugin::UpdateConfigSettings()
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
	GetConfigValue<int>("MAPUDPPort", _MAPUDPPort);
	GetConfigValue<int>("TIMUDPPort", _TIMUDPPort);
	GetConfigValue<int>("BSMUDPPort", _BSMUDPPort);
	GetConfigValue<int>("SPATUDPPort", _SPATUDPPort);
	GetConfigValue<string>("UDPServerIpAddress", _udpServerIpAddress);
	GetConfigValue<string>("CommunicationMode", _communicationMode);
	//Update communication mode
	_communicationModeHelper->setMode(_communicationMode);	

	//Throw an error if the communication mode is neither KAFKA not UDP
	if(_communicationModeHelper->getCurrentMode() == CommunicationMode::UNSUPPORTED){
		PLOG(logERROR) << "Unsuppported communication mode: " << _communicationMode;
		throw TmxException("Unsuppported communication mode: " +_communicationMode);
	}
	PLOG(logINFO) << "Communication Mode: " << _communicationMode;
	
	if(_communicationModeHelper->getCurrentMode()==CommunicationMode::UDP){
		//Create UDP clients for different messages
		_udpMessageForwarder->attachUdpClient(UDPMessageType::BSM, std::make_shared<UdpClient>(_udpServerIpAddress, _BSMUDPPort));
		_udpMessageForwarder->attachUdpClient(UDPMessageType::MAP, std::make_shared<UdpClient>(_udpServerIpAddress, _MAPUDPPort));
		_udpMessageForwarder->attachUdpClient(UDPMessageType::TIM, std::make_shared<UdpClient>(_udpServerIpAddress, _TIMUDPPort));
		_udpMessageForwarder->attachUdpClient(UDPMessageType::SPAT, std::make_shared<UdpClient>(_udpServerIpAddress, _SPATUDPPort));
	}

	if(_communicationModeHelper->getCurrentMode() == CommunicationMode::KAFKA){
		kafkaConnectString = _kafkaBrokerIp + ':' + _kafkaBrokerPort;
		std::string error_string;
		_freqCounter=1;
		if(_forwardMSG == 1) {
			kafkaConnectString = _kafkaBrokerIp + ':' + _kafkaBrokerPort;
			kafka_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

			PLOG(logDEBUG) <<"ODEForwardPlugin: Attempting to connect to " << kafkaConnectString;
			if ((kafka_conf->set("bootstrap.servers", kafkaConnectString, error_string) != RdKafka::Conf::CONF_OK)) {
				PLOG(logERROR) <<"ODEForwardPlugin: Setting kafka config options failed with error:" << error_string;
				PLOG(logERROR) <<"ODEForwardPlugin: Exiting with exit code 1";
				exit(1);
			} else {
				PLOG(logDEBUG) <<"ODEForwardPlugin: Kafka config options set successfully";
			}
			
			kafka_producer = RdKafka::Producer::create(kafka_conf, error_string);
			if (!kafka_producer) {
				PLOG(logERROR) <<"ODEForwardPlugin: Creating kafka producer failed with error:" << error_string;
				PLOG(logERROR) <<"ODEForwardPlugin: Exiting with exit code 1";
				exit(1);
			} 			
			PLOG(logDEBUG) <<"ODEForwardPlugin: Kafka producer created";
		}
	}
 }

 /**
  * Called when configuration is changed
  *
  * @param key Key of the configuration value changed
  * @param value Changed value
  */
 void ODEForwardPlugin::OnConfigChanged(const char *key, const char *value)
 {
 	PluginClient::OnConfigChanged(key, value);
 	UpdateConfigSettings();
 }

 /**
  * Called on plugin state change
  *
  * @para state New plugin state
  */
 void ODEForwardPlugin::OnStateChange(IvpPluginState state)
 {
 	PluginClient::OnStateChange(state);

 	if (state == IvpPluginState_registered)
 	{
 		UpdateConfigSettings();
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
 void ODEForwardPlugin::HandleRealTimePublish(BsmMessage &msg, routeable_message &routeableMsg) {
	if(_communicationModeHelper->getCurrentMode()== CommunicationMode::UDP){
		sendUDPMessage(routeableMsg, UDPMessageType::BSM);
	}else if(_communicationModeHelper->getCurrentMode() == CommunicationMode::KAFKA){
		sendBsmKafkaMessage(msg, routeableMsg);
	}else{
		PLOG(logERROR) << "Unsuppported communication mode: " << _communicationMode;
	}
 }

 void ODEForwardPlugin::HandleSPaTPublish(SpatMessage &msg, routeable_message &routeableMsg) {
	if(_communicationModeHelper->getCurrentMode() == CommunicationMode::UDP){
		sendUDPMessage(routeableMsg, UDPMessageType::SPAT);
	}else if(_communicationModeHelper->getCurrentMode() == CommunicationMode::KAFKA){
		sendSpatKafkaMessage(msg, routeableMsg);
	}else{
		PLOG(logERROR) << "Unsuppported communication mode: " << _communicationMode;
	}
 }

 void ODEForwardPlugin::HandleTimPublish(TimMessage &msg, routeable_message &routeableMsg) {
	if(_communicationModeHelper->getCurrentMode() == CommunicationMode::UDP){
		sendUDPMessage(routeableMsg, UDPMessageType::TIM);
	}else{
		PLOG(logERROR) << "Unsuppported communication mode: " << _communicationMode;
	}
 }


 void ODEForwardPlugin::HandleMapPublish(MapDataMessage &msg, routeable_message &routeableMsg) {
	if(_communicationModeHelper->getCurrentMode() == CommunicationMode::UDP){
		sendUDPMessage(routeableMsg, UDPMessageType::MAP);
	}else{
		PLOG(logERROR) << "Unsuppported communication mode: " << _communicationMode;
	}
 }

void ODEForwardPlugin::sendBsmKafkaMessage(BsmMessage &msg, routeable_message &routeableMsg){
	auto bsm=msg.get_j2735_data();

	char *teststring=new char[10000];

	std::sprintf(teststring, "{\"BsmMessageContent\":[{\"metadata\":{\"utctimestamp\":\"%s\"},\"payload\":\"%s\"}]}",getISOCurrentTimestamp<chrono::microseconds>().c_str(),routeableMsg.get_payload_str().c_str());

        //  check for schedule
        if(_freqCounter++%_scheduleFrequency == 0) {
			QueueKafkaMessage(kafka_producer, _BSMkafkaTopic, teststring);
		}
		
		delete [] teststring;
}

void ODEForwardPlugin::sendUDPMessage(routeable_message &routeableMsg, UDPMessageType udpMessageType) const{
	std::string message = routeableMsg.get_payload_str().c_str();
	PLOG(logDEBUG) << "Sending UDP Message: " << message;
	_udpMessageForwarder->sendMessage(udpMessageType, message);
}

 void ODEForwardPlugin::sendSpatKafkaMessage(SpatMessage &msg, routeable_message &routeableMsg){
	auto spat=msg.get_j2735_data();

        char *spatstring=new char[10000];

        std::sprintf(spatstring, "{\"SpatMessageContent\":[{\"metadata\":{\"utctimestamp\":\"%s\"},\"payload\":\"%s\"}]}",getISOCurrentTimestamp<chrono::microseconds>().c_str(),routeableMsg.get_payload_str().c_str());

        //  check for schedule
        if(_freqCounter++%_scheduleFrequency == 0) {
			QueueKafkaMessage(kafka_producer, _SPaTkafkaTopic, spatstring);
		}
		
		delete [] spatstring;	
 }

 void ODEForwardPlugin::QueueKafkaMessage(RdKafka::Producer *producer, std::string topic, std::string message)
 {
	bool retry = true, return_value = false;
  	PLOG(logDEBUG) <<"ODEForwardPlugin: Queueing kafka message:topic:" << topic << " " << producer->outq_len() <<"messages already in queue";

 	while (retry) {
 		RdKafka::ErrorCode produce_error = producer->produce(topic, RdKafka::Topic::PARTITION_UA,
 			RdKafka::Producer::RK_MSG_COPY, const_cast<char *>(message.c_str()),
 			message.size(), nullptr, 0, 0, nullptr);

     	if (produce_error == RdKafka::ERR_NO_ERROR) {
       		PLOG(logDEBUG) <<"ODEForwardPlugin: Queued message:" << message;
       		retry = false; return_value = true;
     	}
 		else {
 			PLOG(logERROR) <<"ODEForwardPlugin: Failed to queue message:" << message <<" with error:" << RdKafka::err2str(produce_error);
			if (produce_error == RdKafka::ERR__QUEUE_FULL) {
				PLOG(logERROR) <<"ODEForwardPlugin: Message queue full...retrying...";
				producer->poll(500);  /* ms */
				retry = true;
			}
			else {
				PLOG(logERROR) <<"ODEForwardPlugin: Unhandled error in queue_kafka_message:" << RdKafka::err2str(produce_error);
				retry = false;
			}
     	}
   }
   PLOG(logDEBUG) <<"ODEForwardPlugin: Queueing kafka message completed";
 }

 // Override of main method of the plugin that should not return until the plugin exits.
 // This method does not need to be overridden if the plugin does not want to use the main thread.
 int ODEForwardPlugin::Main()
 {
 	PLOG(logDEBUG) <<"ODEForwardPlugin: Starting ODEForwardPlugin...";

 	uint msCount = 0;
 	while (_plugin->state != IvpPluginState_error)
 	{
 		PLOG(logDEBUG4) <<"ODEForwardPlugin: Sleeping 5 minutes" << endl;

 		this_thread::sleep_for(chrono::milliseconds(300000));

 	}

 	PLOG(logDEBUG) <<"ODEForwardPlugin: ODEForwardPlugin terminating gracefully.";
 	return EXIT_SUCCESS;
 }

 } /* namespace ODEForwardPlugin */


 /**
  * Main method for running the plugin
  * @param argc number of arguments
  * @param argv array of arguments
  */
 int main(int argc, char *argv[])
 {
 	return run_plugin<ODEForwardPlugin::ODEForwardPlugin>("ODEForwardPlugin", argc, argv);
 }
