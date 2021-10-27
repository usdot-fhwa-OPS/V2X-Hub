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
	AddMessageFilter < BsmMessage > (this, &CARMAStreetsPlugin::HandleBasicSafetyMessage);
	AddMessageFilter < tsm3Message > (this, &CARMAStreetsPlugin::HandleMobilityOperationMessage);
	AddMessageFilter < tsm2Message > (this, &CARMAStreetsPlugin::HandleMobilityPathMessage);
	
	SubscribeToMessages();

	SubscribeKafkaTopics();

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
 	GetConfigValue<int>("run_kafka_consumer", _run_kafka_consumer);
 	GetConfigValue<string>("subscribeMobilityOperationTopic", _subscribeToSchedulingPlanTopic);
	GetConfigValue<string>("transmitBSMTopic", _transmitBSMTopic);
 	GetConfigValue<string>("intersectionType", _intersectionType);
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

	kafka_consumer = RdKafka::KafkaConsumer::create(kafka_conf, error_string);
	if ( !kafka_consumer ) {
		FILE_LOG(logERROR) << "Failed to create Kafka consumer: " << error_string << std::endl;
		exit(1);
	}
	FILE_LOG(logERROR) << "Created consumer " << kafka_consumer->name() << std::endl;
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
		std::string strategy_params;
		std::string strategy = strat.str();
		if ( std::find( _strategies.begin(), _strategies.end(), strategy) != _strategies.end() ) 
		{
			strategy_params = payload.str();
			Json::Value mobilityOperationJsonRoot;
			Json::StreamWriterBuilder builder;

			Json::Value metadata;
			std::stringstream hostStaticId;

			hostStaticId << mobilityOperation->header.hostStaticId.buf;
			metadata["hostStaticId"] = hostStaticId.str();

			std::stringstream targetStaticId;
			targetStaticId << mobilityOperation->header.targetStaticId.buf;
			metadata["targetStaticId"] = targetStaticId.str();

			std::stringstream hostBSMId;
			hostBSMId << mobilityOperation->header.hostBSMId.buf;
			metadata["hostBSMId"] =hostBSMId.str();

			std::stringstream planId;
			planId << mobilityOperation->header.planId.buf;
			metadata["planId"] = planId.str();

			std::stringstream timestamp;
			timestamp << mobilityOperation->header.timestamp.buf;
			metadata["timestamp"] = timestamp.str();
			
			mobilityOperationJsonRoot["strategy"] 			= strategy;
			mobilityOperationJsonRoot["strategy_params"] 	= strategy_params;
			mobilityOperationJsonRoot["metadata"] 			= metadata; 
			const std::string message 						= Json::writeString(builder, mobilityOperationJsonRoot);			
			PLOG(logDEBUG) <<"MobilityOperation message:" << message <<std::endl;

			while (retry) 
			{
				RdKafka::ErrorCode produce_error = kafka_producer->produce(_transmitMobilityOperationTopic, 
																	RdKafka::Topic::PARTITION_UA,
																	RdKafka::Producer::RK_MSG_COPY, 
																	const_cast<char *>(message.c_str()),
																	message.size(), 
																	NULL, NULL, 0, 0);

				if (produce_error == RdKafka::ERR_NO_ERROR) {
					FILE_LOG(logDEBUG) <<"Queued message:" << message;
					retry = false;
				}
				else 
				{
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

		mobilityPathJsonValue["location"] 		= location;
		mobilityPathJsonValue["trajectory"] 	= trajectory;
		mobilityPathJsonRoot["metadata"] 		= metadata; 
		mobilityPathJsonRoot["MobilityPath"] 	= mobilityPathJsonValue;
		const std::string json_message 			= Json::writeString(builder, mobilityPathJsonRoot);
		PLOG(logDEBUG) <<"MobilityPath Json message:" << json_message;
		RdKafka::ErrorCode produce_error 		= kafka_producer->produce(	_transmitMobilityPathTopic, 
																			RdKafka::Topic::PARTITION_UA,
																			RdKafka::Producer::RK_MSG_COPY, const_cast<char *>(json_message.c_str()),
																			json_message.size(), NULL, NULL, 0, 0 );

		if (produce_error == RdKafka::ERR_NO_ERROR) 
		{
			FILE_LOG(logDEBUG) << "Queued message:" << json_message;
		}
		else 
		{
			FILE_LOG(logERROR) << "Failed to queue message:" << json_message <<" with error:" << RdKafka::err2str(produce_error);
			if (produce_error == RdKafka::ERR__QUEUE_FULL) 
			{
				FILE_LOG(logERROR) << "MobilityPath producer Message queue is full.";
			}
		}	
	}
	catch (TmxException &ex) 
	{
		FILE_LOG(logERROR) << "Failed to decode message : " << ex.what();

	}
}

void CARMAStreetsPlugin::HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg)
{
	try {

		auto bsm = msg.get_j2735_data();
		bool retry = true;
		FILE_LOG(logERROR) << "BSM msgCnt: " << bsm->coreData.msgCnt;
		FILE_LOG(logERROR) << "BSM size length: " << bsm->coreData.size.length;
		FILE_LOG(logERROR) << "BSM size width: " << bsm->coreData.size.width;
		FILE_LOG(logERROR) << "BSM lat: " << bsm->coreData.lat;
		FILE_LOG(logERROR) << "BSM long: " << bsm->coreData.Long;
		FILE_LOG(logERROR) << "BSM elev: " << bsm->coreData.elev;
		FILE_LOG(logERROR) << "BSM long: " << bsm->coreData.speed;
		FILE_LOG(logERROR) << "BSM secMark: " << bsm->coreData.secMark;

  		FILE_LOG(logERROR) <<"Queueing kafka message:topic:" << _transmitBSMTopic << " " 
		<< kafka_producer->outq_len() <<"messages already in queue";

		Json::Value bsmJsonRoot;
		Json::StreamWriterBuilder builder;

		std::stringstream msgCnt;
		msgCnt << bsm->coreData.msgCnt;
		bsmJsonRoot["msg_count"] = msgCnt.str();

		std::stringstream length;
		length << bsm->coreData.size.length;
		bsmJsonRoot["length"] = length.str();

		std::stringstream width;
		width << bsm->coreData.size.width;
		bsmJsonRoot["width"] = width.str();

		std::stringstream lat;
		lat << bsm->coreData.lat;
		bsmJsonRoot["lat"] = lat.str();

		std::stringstream Long;
		Long << bsm->coreData.Long;
		bsmJsonRoot["long"] = Long.str();

		std::stringstream elev;
		elev << bsm->coreData.elev;
		bsmJsonRoot["elev"] = elev.str();

		std::stringstream speed;
		speed << bsm->coreData.speed;
		bsmJsonRoot["speed"] = speed.str();

		std::stringstream secMark;
		secMark << bsm->coreData.secMark;
		bsmJsonRoot["sec_mark"]  = secMark.str();


		std::stringstream id;
		id << bsm->coreData.id.buf;
		bsmJsonRoot["sec_mark"]  = id.str();
		
		bsmJsonRoot["core_data"]  = bsmJsonRoot; 
		const std::string message = Json::writeString(builder, bsmJsonRoot);
		PLOG(logDEBUG) <<"BSM Json message:" << message;

		while (retry) 
		{
			RdKafka::ErrorCode produce_error = kafka_producer->produce(_transmitBSMTopic, 
																		RdKafka::Topic::PARTITION_UA,
																		RdKafka::Producer::RK_MSG_COPY, 
																		const_cast<char *>(message.c_str()),
																		message.size(), 
																		NULL, NULL, 0, 0);

			if (produce_error == RdKafka::ERR_NO_ERROR) {
				FILE_LOG(logDEBUG) <<"Queued message:" << message;
				retry = false;
			}
			else 
			{
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


void CARMAStreetsPlugin::SubscribeKafkaTopics()
{
  	std::vector<std::string> topics;
  	topics.push_back(_subscribeToSchedulingPlanTopic);
	RdKafka::ErrorCode err = kafka_consumer->subscribe(topics);
	if (err) 
	{
		PLOG(logERROR) <<  "Failed to subscribe to " << topics.size() << " topics: " << RdKafka::err2str(err) << std::endl;
		return;
	}

	while (_run_kafka_consumer) 
	{
		RdKafka::Message *msg = kafka_consumer->consume( 1000 );
		if( msg->err() == RdKafka::ERR_NO_ERROR )
		{
			const char * payload_str = static_cast<const char *>( msg->payload() );
			if(msg->len() > 0)
			{
				PLOG(logERROR) << "consumed message payload: " << payload_str <<std::endl;
				Json::Value  payload_root;
				Json::Reader payload_reader;
				bool parse_sucessful = payload_reader.parse(payload_str, payload_root);
				if( !parse_sucessful )
				{	
					PLOG(logERROR) << "Error parsing payload: " << payload_str << std::endl;
					continue;
				}

				Json::Value metadata = payload_root["metadata"];
				Json::Value payload_json_array = payload_root["payload"];
				
				for ( int index = 0; index < payload_json_array.size(); ++index )
				{
					PLOG(logDEBUG) << payload_json_array[index] << std::endl;
					Json::Value payload_json =  payload_json_array[index];
					tsm3EncodedMessage tsm3EncodedMsgs;
					if( getEncodedtsm3 (&tsm3EncodedMsgs,  metadata,  payload_json) )
					{
						tsm3EncodedMsgs.set_flags( IvpMsgFlags_RouteDSRC );
						tsm3EncodedMsgs.addDsrcMetadata( 172, 0xBFEE );
						PLOG(logDEBUG) << "tsm3EncodedMsgs: " << tsm3EncodedMsgs;
						BroadcastMessage(static_cast<routeable_message &>( tsm3EncodedMsgs ));
					}
				}			
			}
		}
		else
		{
			PLOG(logERROR) << "Consume failed: " << msg->errstr() << std::endl;
			_run_kafka_consumer = 0;
			return;
		}
		delete msg;
	}
}

bool CARMAStreetsPlugin::getEncodedtsm3( tsm3EncodedMessage *tsm3EncodedMsg,  Json::Value metadata, Json::Value payload_json )
{
	std::lock_guard<std::mutex> lock(data_lock);
	TestMessage03* mobilityOperation = (TestMessage03 *) calloc(1, sizeof(TestMessage03));
	std::string sender_id = "NA", recipient_id_str = payload_json["v_id"].asString(), sender_bsm_id_str = "NA", plan_id_str = "NA",timestamp_str = std::to_string(payload_json["timestamp"].asInt64()), strategy_str = payload_json["intersection_type"].asString();
	std::string strategy_params_str = std::to_string(payload_json["st"].asInt64()) + std::to_string(payload_json["dt"].asInt64()) + std::to_string(payload_json["et"].asInt64()) + std::to_string(payload_json["dp"].asInt64()) + std::to_string(payload_json["access"].asInt64()); 

	//content host id
	size_t string_size = sender_id.size();
	uint8_t string_content_hostId[string_size];
	for(size_t i=0; i< string_size; i++)
	{
		string_content_hostId[i] = sender_id[i];
	}
	mobilityOperation->header.hostStaticId.buf  = string_content_hostId;
	mobilityOperation->header.hostStaticId.size = string_size;

	//recipient id
	std::string recipient_id = recipient_id_str;
	string_size = recipient_id.size();
	uint8_t string_content_targetId[string_size];
	for(size_t i=0; i< string_size; i++)
	{
		string_content_targetId[i] = recipient_id[i];
	}
	mobilityOperation->header.targetStaticId.buf  = string_content_targetId;
	mobilityOperation->header.targetStaticId.size = string_size;

	//sender bsm id
	std::string sender_bsm_id = sender_bsm_id_str;
	string_size = sender_bsm_id.size();
	uint8_t string_content_BSMId[string_size];
	for(size_t i=0; i< string_size; i++)
	{
		string_content_BSMId[i] = sender_bsm_id[i];
	}
	mobilityOperation->header.hostBSMId.buf = string_content_BSMId;
	mobilityOperation->header.hostBSMId.size = string_size;

	//plan id
	std::string plan_id = plan_id_str;
	string_size = plan_id.size();
	uint8_t string_content_planId[string_size];
	for(size_t i=0; i< string_size; i++)
	{
		string_content_planId[i] = plan_id[i];
	}
	mobilityOperation->header.planId.buf = string_content_planId;
	mobilityOperation->header.planId.size = string_size;

	//get timestamp and convert to char array;
	string_size = timestamp_str.size();
	uint8_t string_content_timestamp[string_size];
	for(size_t i=0; i< string_size; i++)
	{
		string_content_timestamp[i] = timestamp_str[i];
	}
	mobilityOperation->header.timestamp.buf = string_content_timestamp;
	mobilityOperation->header.timestamp.size = string_size;

	//convert strategy string to char array
	std::string strategy = strategy_str;
	string_size = strategy.size();
	uint8_t string_content_strategy[string_size];
	for(size_t i=0; i< string_size; i++)
	{
		string_content_strategy[i] = strategy[i];
	}
	mobilityOperation->body.strategy.buf = string_content_strategy;
	mobilityOperation->body.strategy.size = string_size;        

	//convert parameters string to char array
	std::string strategy_params = strategy_params_str;
	string_size = strategy_params.size();       
	uint8_t string_content_params[string_size];
	for(size_t i=0; i < string_size; i++)
	{
		string_content_params[i] = strategy_params[i];
	}
	mobilityOperation->body.operationParams.buf = string_content_params;
	mobilityOperation->body.operationParams.size = string_size;

	tmx::messages::tsm3Message* _tsm3Message = new tmx::messages::tsm3Message(mobilityOperation);
	PLOG(logDEBUG) << *_tsm3Message;
	tsm3EncodedMsg->initialize(*_tsm3Message);
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

