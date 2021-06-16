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
 * Construct a new MobililtyOperationPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes
 */
CARMAStreetsPlugin::CARMAStreetsPlugin(string name) :
		PluginClient(name) {
	
	FILELog::ReportingLevel() = FILELog::FromString("INFO");
	//AddMessageFilter < tsm3Message > (this, &CARMAStreetsPlugin::HandleMobilityOperationMessage);
	AddMessageFilter < tsm3Message > (this, &CARMAStreetsPlugin::HandleMobilityOperationMessage);

	
	SubscribeToMessages();

}

CARMAStreetsPlugin::~CARMAStreetsPlugin() {
}

void CARMAStreetsPlugin::UpdateConfigSettings() {

	lock_guard<mutex> lock(_cfgLock);
	GetConfigValue<string>("receiveTopic", _receiveTopic);	
	GetConfigValue<string>("transmitTopic", _transmitTopic);
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

  		FILE_LOG(logERROR) <<"Queueing kafka message:topic:" << _receiveTopic << " " 
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
				RdKafka::ErrorCode produce_error = kafka_producer->produce(_receiveTopic, RdKafka::Topic::PARTITION_UA,
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

