//============================================================================
// Name        : CARMAStreetsPlugin.cpp
// Author      : Paul Bourelly
// Version     : 5.0
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "CARMAStreetsPlugin.h"


namespace CARMAStreetsPlugin {



/**
 * Construct a new CARMAStreetsPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes
 */
CARMAStreetsPlugin::CARMAStreetsPlugin(string name) :
		PluginClient(name) {
	
	FILELog::ReportingLevel() = FILELog::FromString("INFO");
	//AddMessageFilter < tsm3Message > (this, &CARMAStreetsPlugin::HandleMobilityOperationMessage);
	AddMessageFilter < tsm3Message > (this, &CARMAStreetsPlugin::HandleMobilityOperationMessage);
	AddMessageFilter < tsm2Message > (this, &CARMAStreetsPlugin::HandleMobilityPathMessage);
	
	SubscribeToMessages();

}

CARMAStreetsPlugin::~CARMAStreetsPlugin() {
}

void CARMAStreetsPlugin::UpdateConfigSettings() {

	lock_guard<mutex> lock(_cfgLock);
	GetConfigValue<string>("receiveTopic", _receiveTopic);	
	GetConfigValue<string>("transmitMobilityOperationTopic", _transmitMobilityOperationTopic);
	GetConfigValue<string>("transmitMobilityPathTopic", _transmitMobilityPathTopic);
 	GetConfigValue<string>("KafkaBrokerIp", _kafkaBrokerIp);
 	GetConfigValue<string>("KafkaBrokerPort", _kafkaBrokerPort);
	 // Populate strategies config
	string config;
	GetConfigValue<string>("MobilityOperationStrategies", config);
	std::stringstream ss(config);
	_strategies.clear();
	while( ss.good() ) {
		std::string substring;
		getline( ss, substring, ',');
		_strategies.push_back( substring);
	}
 	std::string kafkaConnectString = _kafkaBrokerIp + ':' + _kafkaBrokerPort;
	std::string error_string;	
	kafkaConnectString = _kafkaBrokerIp + ':' + _kafkaBrokerPort;
	kafka_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

	FILE_LOG(logERROR) <<"Attempting to connect to " << kafkaConnectString;
	if ((kafka_conf->set("bootstrap.servers", kafkaConnectString, error_string) != RdKafka::Conf::CONF_OK)) {
		FILE_LOG(logERROR) <<"Setting kafka config options failed with error:" << error_string;
		FILE_LOG(logERROR) <<"Exiting with exit code 1";
		exit(1);
	} else {
		FILE_LOG(logERROR) <<"Kafka config options set successfully";
	}
	
	kafka_producer = RdKafka::Producer::create(kafka_conf, error_string);
	if (!kafka_producer) {
		FILE_LOG(logERROR) <<"Creating kafka producer failed with error:" << error_string;
		FILE_LOG(logERROR) <<"Exiting with exit code 1";
		exit(1);
	} 			
	FILE_LOG(logERROR) <<"Kafka producer created";

	
}

void CARMAStreetsPlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}


void CARMAStreetsPlugin::HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg ) {
	try {

		auto mobilityOperation = msg.get_j2735_data();
		bool retry = true;
		FILE_LOG(logERROR) << "Body OperationParams : " << mobilityOperation->body.operationParams.buf;
		FILE_LOG(logERROR) << "Body Strategy : " << mobilityOperation->body.strategy.buf;

  		FILE_LOG(logERROR) <<"Queueing kafka message:topic:" << _transmitMobilityOperationTopic << " " 
		  << kafka_producer->outq_len() <<"messages already in queue";

		std::stringstream strat;
		std::stringstream payload; 
 
		ptree pr; 
		strat << mobilityOperation->body.strategy.buf;
		payload << mobilityOperation->body.operationParams.buf;
		std::string message;
		std::string strategy = strat.str();
		if ( std::find( _strategies.begin(), _strategies.end(), strategy) != _strategies.end() ) {
			message = payload.str();
	
			while (retry) {
				RdKafka::ErrorCode produce_error = kafka_producer->produce(_transmitMobilityOperationTopic, RdKafka::Topic::PARTITION_UA,
				RdKafka::Producer::RK_MSG_COPY, const_cast<char *>(message.c_str()),
				message.size(), NULL, NULL, 0, 0);

				if (produce_error == RdKafka::ERR_NO_ERROR) {
					FILE_LOG(logDEBUG) <<"Queued message:" << message;
					retry = false;
				}
				else {
					FILE_LOG(logERROR) <<"Failed to queue message:" << message <<" with error:" << RdKafka::err2str(produce_error);
					if (produce_error == RdKafka::ERR__QUEUE_FULL) {
						FILE_LOG(logERROR) <<"Message queue full...retrying...";
						kafka_producer->poll(500);  /* ms */
						retry = true;
					}
					else {
						FILE_LOG(logERROR) <<"Unhandled error in queue_kafka_message:" << RdKafka::err2str(produce_error);
						retry = false;
					}
				}	
			}
		}
	}
	catch (TmxException &ex) {
		FILE_LOG(logERROR) << "Failed to decode message : " << ex.what();

	}
	

}

void CARMAStreetsPlugin::HandleMobilityPathMessage(tsm2Message &msg, routeable_message &routeableMsg ) 
{
	try 
	{
		auto mobilityPathMsg = msg.get_j2735_data();

		Json::Value mobilityPathJsonRoot;
		Json::StreamWriterBuilder builder;

		Json::Value metadata;
		std::stringstream hostStaticId;

		hostStaticId << mobilityPathMsg->header.hostStaticId.buf;
		metadata["hostStaticId"] = hostStaticId.str();

		std::stringstream targetStaticId;
		targetStaticId << mobilityPathMsg->header.targetStaticId.buf;
		metadata["targetStaticId"] = targetStaticId.str();

		std::stringstream hostBSMId;
		hostBSMId << mobilityPathMsg->header.hostBSMId.buf;
		metadata["hostBSMId"] =hostBSMId.str();

		std::stringstream planId;
		planId << mobilityPathMsg->header.planId.buf;
		metadata["planId"] = planId.str();

		std::stringstream timestamp;
		timestamp << mobilityPathMsg->header.timestamp.buf;
		metadata["timestamp"] = timestamp.str();

		Json::Value mobilityPathJsonValue;
		Json::Value location;
		Json::Value trajectory;

		std::stringstream location_ecefX;
		location_ecefX << mobilityPathMsg->body.location.ecefX;
		location["ecefX"] = std::stoi(location_ecefX.str());
		
		std::stringstream location_ecefY;
		location_ecefY << mobilityPathMsg->body.location.ecefY;
		location["ecefY"] = std::stoi(location_ecefY.str());

		std::stringstream location_ecefZ;
		location_ecefZ << mobilityPathMsg->body.location.ecefZ;
		location["ecefZ"] = std::stoi(location_ecefZ.str());

		std::stringstream location_timestamp;
		location_timestamp << mobilityPathMsg->body.location.timestamp.buf;
		location["timestamp"] = location_timestamp.str();

		for(int i=0; i < mobilityPathMsg->body.trajectory.list.count; i++)
		{
			Json::Value offset;
			std::stringstream trajectory_offsetX;
			trajectory_offsetX<<mobilityPathMsg->body.trajectory.list.array[i]->offsetX;
			offset["offsetX"] = std::stoi(trajectory_offsetX.str());


			std::stringstream trajectory_offsetY;
			trajectory_offsetY<<mobilityPathMsg->body.trajectory.list.array[i]->offsetY;
			offset["offsetY"] = std::stoi(trajectory_offsetY.str());

			std::stringstream trajectory_offsetZ;
			trajectory_offsetZ<<mobilityPathMsg->body.trajectory.list.array[i]->offsetZ;
			offset["offsetZ"] = std::stoi(trajectory_offsetZ.str());

			trajectory["offsets"].append(offset);
		}

		mobilityPathJsonValue["location"] = location;
		mobilityPathJsonValue["trajectory"] = trajectory;
		mobilityPathJsonRoot["metadata"] = metadata; 
		mobilityPathJsonRoot["MobilityPath"] = mobilityPathJsonValue;
		const std::string json_message = Json::writeString(builder, mobilityPathJsonRoot);
		RdKafka::ErrorCode produce_error = kafka_producer->produce(_transmitMobilityPathTopic, RdKafka::Topic::PARTITION_UA,
		RdKafka::Producer::RK_MSG_COPY, const_cast<char *>(json_message.c_str()),
		json_message.size(), NULL, NULL, 0, 0);

		if (produce_error == RdKafka::ERR_NO_ERROR) 
		{
			FILE_LOG(logDEBUG) <<"Queued message:" << json_message;
		}
		else 
		{
			FILE_LOG(logERROR) <<"Failed to queue message:" << json_message <<" with error:" << RdKafka::err2str(produce_error);
			if (produce_error == RdKafka::ERR__QUEUE_FULL) 
			{
				FILE_LOG(logERROR) <<"MobilityPath producer Message queue is full.";
			}
		}	
	}
	catch (TmxException &ex) 
	{
		FILE_LOG(logERROR) << "Failed to decode message : " << ex.what();

	}
}

void CARMAStreetsPlugin::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered) {
		UpdateConfigSettings();
	}
}


int CARMAStreetsPlugin::Main() {
	FILE_LOG(logINFO) << "Starting plugin.";

	uint64_t lastSendTime = 0;

	while (_plugin->state != IvpPluginState_error) {
		


		usleep(100000); //sleep for microseconds set from config.
	}

	return (EXIT_SUCCESS);
}
}

int main(int argc, char *argv[]) {
	return run_plugin < CARMAStreetsPlugin::CARMAStreetsPlugin > ("CARMAStreetsPlugin", argc, argv);
}

