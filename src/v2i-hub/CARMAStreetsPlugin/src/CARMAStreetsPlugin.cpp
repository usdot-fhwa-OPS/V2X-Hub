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
	AddMessageFilter < BsmMessage > (this, &CARMAStreetsPlugin::HandleBasicSafetyMessage);
	AddMessageFilter < tsm3Message > (this, &CARMAStreetsPlugin::HandleMobilityOperationMessage);
	AddMessageFilter < tsm2Message > (this, &CARMAStreetsPlugin::HandleMobilityPathMessage);
	AddMessageFilter < MapDataMessage > (this, &CARMAStreetsPlugin::HandleMapMessage);
	AddMessageFilter < SrmMessage > (this, &CARMAStreetsPlugin::HandleSRMMessage);
	AddMessageFilter < simulation::ExternalObject > (this, &CARMAStreetsPlugin::HandleSimulatedExternalMessage );
	
	SubscribeToMessages();

}

CARMAStreetsPlugin::~CARMAStreetsPlugin() {
	//Todo: It does not seem the desctructor is called.
	_spat_kafka_consumer_ptr->stop();
	_scheduing_plan_kafka_consumer_ptr->stop();
	_ssm_kafka_consumer_ptr->stop();
}

void CARMAStreetsPlugin::UpdateConfigSettings() {

	lock_guard<mutex> lock(_cfgLock);
	// Populate Header Information for outgoing mobility operation messages and filter Header for incoming mobility operation messages
	GetConfigValue<string>("IntersectionId", _intersectionId);
	// Kafka broker configuration
 	GetConfigValue<string>("KafkaBrokerIp", _kafkaBrokerIp);
 	GetConfigValue<string>("KafkaBrokerPort", _kafkaBrokerPort);
	//
 	GetConfigValue<string>("SchedulingPlanTopic", _subscribeToSchedulingPlanTopic);
	GetConfigValue<string>("SchedulingPlanConsumerGroupId", _subscribeToSchedulingPlanConsumerGroupId);
 	GetConfigValue<string>("SpatTopic", _subscribeToSpatTopic);
	GetConfigValue<string>("SsmTopic", _subscribeToSsmTopic);
	GetConfigValue<string>("SpatConsumerGroupId", _subscribeToSpatConsumerGroupId);
	GetConfigValue<string>("SsmConsumerGroupId", _subscribeToSSMConsumerGroupId);
	GetConfigValue<string>("BsmTopic", _transmitBSMTopic);
	GetConfigValue<string>("MobilityOperationTopic", _transmitMobilityOperationTopic);
	GetConfigValue<string>("MobilityPathTopic", _transmitMobilityPathTopic);
 	GetConfigValue<string>("MapTopic", _transmitMAPTopic);
	GetConfigValue<string>("SRMTopic", _transmitSRMTopic); 
	GetConfigValue<string>("SimExternalObjTopic", _transmitSimExternalObjTopic); 
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
}

void CARMAStreetsPlugin::InitKafkaConsumerProducers()
{
 	std::string kafkaConnectString = _kafkaBrokerIp + ':' + _kafkaBrokerPort;
	std::string error_string;	
	kafkaConnectString = _kafkaBrokerIp + ':' + _kafkaBrokerPort;
	kafka_client client;

	//Producer
	_kafka_producer_ptr = client.create_producer(kafkaConnectString);
	if(!_kafka_producer_ptr->init_producer())
	{
		return;
	}

	//Consumers
	_spat_kafka_consumer_ptr = client.create_consumer(kafkaConnectString, _subscribeToSpatTopic,_subscribeToSpatConsumerGroupId);
	_scheduing_plan_kafka_consumer_ptr = client.create_consumer(kafkaConnectString, _subscribeToSchedulingPlanTopic,_subscribeToSchedulingPlanConsumerGroupId);
	_ssm_kafka_consumer_ptr = client.create_consumer(kafkaConnectString, _subscribeToSsmTopic,_subscribeToSSMConsumerGroupId);
	if(!_scheduing_plan_kafka_consumer_ptr || !_spat_kafka_consumer_ptr || !_ssm_kafka_consumer_ptr)
	{
		PLOG(logERROR) <<"Failed to create Kafka consumers.";
		return;
	}
	PLOG(logDEBUG) <<"Kafka consumers created";
	if(!_spat_kafka_consumer_ptr->init()  || !_scheduing_plan_kafka_consumer_ptr->init() || !_ssm_kafka_consumer_ptr->init())
	{
		PLOG(logERROR) <<"Kafka consumers init() failed!";
		return;
	}

	boost::thread thread_schpl(&CARMAStreetsPlugin::SubscribeSchedulingPlanKafkaTopic, this);
	boost::thread thread_spat(&CARMAStreetsPlugin::SubscribeSpatKafkaTopic, this);
	boost::thread thread_ssm(&CARMAStreetsPlugin::SubscribeSSMKafkaTopic, this);	
}

void CARMAStreetsPlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void CARMAStreetsPlugin::HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg ) {
	try 
	{
		auto mobilityOperation = msg.get_j2735_data();
		PLOG(logDEBUG) << "Body OperationParams : " << mobilityOperation->body.operationParams.buf << "\n"
					  << "Body Strategy : " << mobilityOperation->body.strategy.buf<< "\n"
					  <<"Queueing kafka message:topic:" << _transmitMobilityOperationTopic;

		std::stringstream strat;
		std::stringstream payload; 
 
		ptree pr; 
		strat << mobilityOperation->body.strategy.buf;
		payload << mobilityOperation->body.operationParams.buf;
		std::string strategy_params;
		std::string strategy = strat.str();
		// TODO: Filter vehicle mobility operation messages based on Mobility Header host id == intersection id
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
			produce_kafka_msg(message, _transmitMobilityOperationTopic);
		}
	}
	catch (TmxException &ex) {
		PLOG(logERROR) << "Failed to decode message : " << ex.what();
		SetStatus<uint>(Key_MobilityOperationMessageSkipped, ++_mobilityOperationMessageSkipped);

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

		trajectory["location"] 		= location;
		mobilityPathJsonRoot["metadata"] 		= metadata; 
		mobilityPathJsonRoot["trajectory"]		= trajectory;
		const std::string json_message 			= Json::writeString(builder, mobilityPathJsonRoot);
		PLOG(logDEBUG) <<"MobilityPath Json message:" << json_message;
		produce_kafka_msg(json_message, _transmitMobilityPathTopic);
	}
	catch (TmxException &ex) 
	{
		PLOG(logERROR) << "Failed to decode message : " << ex.what();
		SetStatus<uint>(Key_MobilityPathMessageSkipped, ++_mobilityPathMessageSkipped);

	}
}

void CARMAStreetsPlugin::HandleSRMMessage(SrmMessage &msg, routeable_message &routeableMsg)
{
	J2735ToSRMJsonConverter srmJsonConverter;
	std::vector<Json::Value> srmJsonV;
	try{
		srmJsonConverter.toSRMJsonV(srmJsonV , &msg);
	}catch(std::exception& ex)
	{
		PLOG(logERROR) << "Fatal error with SRM To JSON converter. " << ex.what() << std::endl;
		SetStatus<uint>(Key_SRMMessageSkipped, ++_srmMessageSkipped);
		return;
	}
	
	if(srmJsonV.empty())
	{
		PLOG(logERROR) << "SRM message content is empty." << std::endl;
		SetStatus<uint>(Key_SRMMessageSkipped, ++_srmMessageSkipped);
		
	}else{
		for (auto srmJson : srmJsonV)
        {
			Json::StreamWriterBuilder builder;
			const std::string srmJsonStr = Json::writeString(builder, srmJson);
			PLOG(logINFO) << "SRM Json message: " << srmJsonStr << std::endl;
			produce_kafka_msg(srmJsonStr, _transmitSRMTopic);           
        }		
	}	
}

void CARMAStreetsPlugin::HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg)
{
	try 
	{
		auto bsm = msg.get_j2735_data();

		Json::Value bsmJsonRoot;
		Json::Value coreData;
		Json::Value size;
		Json::StreamWriterBuilder builder;

		std::stringstream msgCnt;
		msgCnt << bsm->coreData.msgCnt;
		coreData["msg_count"] = msgCnt.str();

		std::stringstream length;
		length << bsm->coreData.size.length;
		size["length"] = length.str();

		std::stringstream width;
		width << bsm->coreData.size.width;
		size["width"] = width.str();

		std::stringstream lat;
		lat << bsm->coreData.lat;
		coreData["lat"] = lat.str();

		std::stringstream Long;
		Long << bsm->coreData.Long;
		coreData["long"] = Long.str();

		std::stringstream elev;
		elev << bsm->coreData.elev;
		coreData["elev"] = elev.str();

		std::stringstream speed;
		speed << bsm->coreData.speed;
		coreData["speed"] = speed.str();

		std::stringstream secMark;
		secMark << bsm->coreData.secMark;
		coreData["sec_mark"]  = secMark.str();

		auto id_len = bsm->coreData.id.size;
		unsigned long id_num = 0;
		for(auto i = 0; i < id_len; i++)
		{			
			 id_num = (id_num << 8) | bsm->coreData.id.buf[i];
		}
		std::stringstream id_fill_ss;
		id_fill_ss << std::setfill('0') << std::setw(8) <<std::hex << id_num;
		coreData["id"]  = id_fill_ss.str();
		
		Json::Value accuracy;

		std::stringstream orientation;
		orientation << bsm->coreData.accuracy.orientation;
		accuracy["orientation"] = orientation.str();
		
		std::stringstream semiMajor;
		semiMajor << bsm->coreData.accuracy.semiMajor;
		accuracy["semi_major"] = semiMajor.str();

		std::stringstream semiMinor;
		semiMinor << bsm->coreData.accuracy.semiMinor;
		accuracy["semi_minor"] = semiMinor.str();

		std::stringstream angle;
		angle << bsm->coreData.angle;
		coreData["angle"] = angle.str();

		std::stringstream heading;
		heading << bsm->coreData.heading;
		coreData["heading"] = heading.str();

		Json::Value accel_set;

		std::stringstream accelSet_lat;
		accelSet_lat << bsm->coreData.accelSet.lat;
		accel_set["lat"] = accelSet_lat.str();

		std::stringstream accelSet_long;
		accelSet_long << bsm->coreData.accelSet.Long;
		accel_set["long"] = accelSet_long.str();

		std::stringstream accelSet_vert;
		accelSet_vert << bsm->coreData.accelSet.vert;
		accel_set["vert"] = accelSet_vert.str();

		std::stringstream accelSet_yaw;
		accelSet_yaw << bsm->coreData.accelSet.yaw;
		accel_set["yaw"] = accelSet_yaw.str();

		std::stringstream transmission;
		transmission << bsm->coreData.transmission;
		coreData["transmission"] = transmission.str();

		Json::Value brakes;

		std::stringstream abs;
		abs << bsm->coreData.brakes.abs;
		brakes["abs"] = abs.str();

		std::stringstream auxBrakes;
		auxBrakes << bsm->coreData.brakes.auxBrakes;
		brakes["aux_brakes"] = auxBrakes.str();

		std::stringstream brake_boost;
		brake_boost << bsm->coreData.brakes.brakeBoost;
		brakes["brake_boost"] = brake_boost.str();

		std::stringstream scs;
		scs << bsm->coreData.brakes.scs;
		brakes["scs"] = scs.str();

		std::stringstream traction;
		traction << bsm->coreData.brakes.traction;
		brakes["traction"] = traction.str();

		uint8_t binary = bsm->coreData.brakes.wheelBrakes.buf[0] >> 3;
		unsigned int brake_applied_status_type = 4;
		// e.g. shift the binary right until it equals to 1 (0b00000001) to determine the location of the non-zero bit
		for (int i = 0; i < 4; i ++)
		{
			if ((int)binary == 1) 
			{
				brakes["wheel_brakes"] = brake_applied_status_type;
				break;
			}
			else
			{
				brake_applied_status_type -= 1;
				binary = binary >> 1;
			}
		}

		coreData["accel_set"]		= accel_set;
		coreData["brakes"]			= brakes;
		coreData["accuracy"] 		= accuracy;
		coreData["size"] 			= size;		
		bsmJsonRoot["core_data"]  	= coreData; 
		const std::string message 	= Json::writeString(builder, bsmJsonRoot);
		produce_kafka_msg(message, _transmitBSMTopic);
		
	}
	catch (TmxException &ex) {
		PLOG(logERROR) << "Failed to decode message : " << ex.what();
		SetStatus<uint>(Key_BSMMessageSkipped, ++_bsmMessageSkipped);

	}
}

void CARMAStreetsPlugin::HandleMapMessage(MapDataMessage &msg, routeable_message &routeableMsg)
{
	std::shared_ptr<MapData> mapMsgPtr = msg.get_j2735_data();
	PLOG(logDEBUG) << "Intersection count: " << mapMsgPtr->intersections->list.count <<std::endl;
	Json::Value mapJson;
	Json::StreamWriterBuilder builder;
	J2735MapToJsonConverter jsonConverter;
	jsonConverter.convertJ2735MAPToMapJSON(mapMsgPtr, mapJson);
	PLOG(logDEBUG) << "mapJson: " << mapJson << std::endl;
	const std::string message 	= Json::writeString(builder, mapJson);
	produce_kafka_msg(message, _transmitMAPTopic);	
}

void CARMAStreetsPlugin::produce_kafka_msg(const string& message, const string& topic_name) const
{
	_kafka_producer_ptr->send(message, topic_name);
}

void CARMAStreetsPlugin::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered) {
		UpdateConfigSettings();
		InitKafkaConsumerProducers();
	}
}

void CARMAStreetsPlugin::SubscribeSchedulingPlanKafkaTopic()
{	
	if(_subscribeToSchedulingPlanTopic.length() > 0)
	{
		PLOG(logDEBUG) << "SubscribeSchedulingPlanKafkaTopics:" <<_subscribeToSchedulingPlanTopic << std::endl;
		_scheduing_plan_kafka_consumer_ptr->subscribe();

		while (_scheduing_plan_kafka_consumer_ptr->is_running()) 
		{
			std::string payload_str = _scheduing_plan_kafka_consumer_ptr->consume(500);
			if(payload_str.length() > 0)
			{
				PLOG(logDEBUG) << "consumed message payload: " << payload_str <<std::endl;
				Json::Value  payload_root;
				Json::Reader payload_reader;
				bool parse_sucessful = payload_reader.parse(payload_str, payload_root);
				if( !parse_sucessful )
				{	
					PLOG(logERROR) << "Error parsing payload: " << payload_str << std::endl;
					SetStatus<uint>(Key_ScheduleMessageSkipped, ++_scheduleMessageSkipped);
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
						tsm3EncodedMsgs.addDsrcMetadata(0xBFEE );
						PLOG(logDEBUG) << "tsm3EncodedMsgs: " << tsm3EncodedMsgs;
						BroadcastMessage(static_cast<routeable_message &>( tsm3EncodedMsgs ));
					}
				}
				//Empty payload
				if(payload_json_array.empty())
				{
					Json::Value payload_json = {};
					tsm3EncodedMessage tsm3EncodedMsgs;
					if( getEncodedtsm3 (&tsm3EncodedMsgs,  metadata,  payload_json) )
					{
						tsm3EncodedMsgs.set_flags( IvpMsgFlags_RouteDSRC );
						tsm3EncodedMsgs.addDsrcMetadata(0xBFEE);
						PLOG(logDEBUG) << "tsm3EncodedMsgs: " << tsm3EncodedMsgs;
						BroadcastMessage(static_cast<routeable_message &>( tsm3EncodedMsgs ));
					}
				}			
			}
		}

	}
}

void CARMAStreetsPlugin::SubscribeSpatKafkaTopic(){
	if(_subscribeToSpatTopic.length() > 0)
	{
		PLOG(logDEBUG) << "SubscribeSpatKafkaTopics:" <<_subscribeToSpatTopic << std::endl;
		_spat_kafka_consumer_ptr->subscribe();
		//Initialize Json to J2735 Spat convertor		
		JsonToJ2735SpatConverter spat_convertor;
		while (_spat_kafka_consumer_ptr->is_running()) 
		{
			std::string payload_str = _spat_kafka_consumer_ptr->consume(500);
			if(payload_str.length() > 0)
			{
				PLOG(logDEBUG) << "consumed message payload: " << payload_str <<std::endl;
				Json::Value  payload_root;
				Json::Reader payload_reader;
				bool parse_sucessful = payload_reader.parse(payload_str, payload_root);
				if( !parse_sucessful )
				{	
					PLOG(logERROR) << "Error parsing payload: " << payload_str << std::endl;
					SetStatus<uint>(Key_SPATMessageSkipped, ++_spatMessageSkipped);
					continue;
				}
				//Convert the SPAT JSON string into J2735 SPAT message and encode it.
				auto spat_ptr = std::make_shared<SPAT>();
				spat_convertor.convertJson2Spat(payload_root, spat_ptr.get());
				tmx::messages::SpatEncodedMessage spatEncodedMsg;
				try
				{
					spat_convertor.encodeSpat(spat_ptr, spatEncodedMsg);
				}
				catch (TmxException &ex) 
				{
					// Skip messages that fail to encode.
					PLOG(logERROR) << "Failed to encoded SPAT message : \n" << payload_str << std::endl << "Exception encountered: " 
						<< ex.what() << std::endl;
					ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SPAT, spat_ptr.get());
					SetStatus<uint>(Key_SPATMessageSkipped, ++_spatMessageSkipped);

					continue;
				}
				
				ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SPAT, spat_ptr.get());
				PLOG(logDEBUG) << "SpatEncodedMessage: "  << spatEncodedMsg;

				//Broadcast the encoded SPAT message
				spatEncodedMsg.set_flags(IvpMsgFlags_RouteDSRC);
				spatEncodedMsg.addDsrcMetadata(0x8002);
				BroadcastMessage(static_cast<routeable_message &>(spatEncodedMsg));		
			}
		}
	}
}

void CARMAStreetsPlugin::SubscribeSSMKafkaTopic(){

	if(_subscribeToSsmTopic.length() > 0)
	{
		PLOG(logDEBUG) << "SubscribeSSMKafkaTopics:" <<_subscribeToSsmTopic << std::endl;
		_ssm_kafka_consumer_ptr->subscribe();
		//Initialize Json to J2735 SSM convertor 
		JsonToJ2735SSMConverter ssm_convertor;
		while (_ssm_kafka_consumer_ptr->is_running()) 
		{
			std::string payload_str = _ssm_kafka_consumer_ptr->consume(500);			
			if(payload_str.length() > 0)
			{
				PLOG(logDEBUG) << "consumed message payload: " << payload_str <<std::endl;
				Json::Value ssmDoc;
				auto parse_sucessful = ssm_convertor.parseJsonString(payload_str, ssmDoc);
				if( !parse_sucessful )
				{	
					PLOG(logERROR) << "Error parsing payload: " << payload_str << std::endl;
					SetStatus<uint>(Key_SSMMessageSkipped, ++_ssmMessageSkipped);
					continue;
				}
				//Convert the SSM JSON string into J2735 SSM message and encode it.
				auto ssm_ptr = std::make_shared<SignalStatusMessage>();
				ssm_convertor.toJ2735SSM(ssmDoc, ssm_ptr);
				tmx::messages::SsmEncodedMessage ssmEncodedMsg;
				try
				{
					ssm_convertor.encodeSSM(ssm_ptr, ssmEncodedMsg);
				}
				catch (TmxException &ex) 
				{
					// Skip messages that fail to encode.
					PLOG(logERROR) << "Failed to encoded SSM message : \n" << payload_str << std::endl << "Exception encountered: " 
						<< ex.what() << std::endl;
					ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssm_ptr.get());
					SetStatus<uint>(Key_SSMMessageSkipped, ++_ssmMessageSkipped);
					continue;
				}
				
				ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssm_ptr.get());
				PLOG(logDEBUG) << "ssmEncodedMsg: "  << ssmEncodedMsg;

				//Broadcast the encoded SSM message
				ssmEncodedMsg.set_flags(IvpMsgFlags_RouteDSRC);
				ssmEncodedMsg.addDsrcMetadata(0x8002);
				BroadcastMessage(static_cast<routeable_message &>(ssmEncodedMsg));		
			}
		}
	}

}

void CARMAStreetsPlugin::HandleSimulatedExternalMessage(simulation::ExternalObject &msg, routeable_message &routeableMsg)
{
	auto json_str = tmx::utils::sim::SimulationExternalObjectConverter::simExternalObjToJsonStr(msg);
	PLOG(logINFO) <<  "Produce External Object Message in JSON format:  " << json_str <<std::endl;
	produce_kafka_msg(json_str, _transmitSimExternalObjTopic);
}

bool CARMAStreetsPlugin::getEncodedtsm3( tsm3EncodedMessage *tsm3EncodedMsg,  Json::Value metadata, Json::Value payload_json )
{
	try
	{			
		std::lock_guard<std::mutex> lock(data_lock);
		TestMessage03* mobilityOperation = (TestMessage03 *) calloc(1, sizeof(TestMessage03));
		std::string sender_id 			 = _intersectionId;
		std::string recipient_id_str 	 = payload_json != Json::nullValue && payload_json.isMember("v_id") ? payload_json["v_id"].asString(): "UNSET";
		std::string sender_bsm_id_str 	 = "00000000";
		std::string plan_id_str 		 = "00000000-0000-0000-0000-000000000000";
		std::string strategy_str 		 = metadata != Json::nullValue && metadata.isMember("intersection_type")? metadata["intersection_type"].asString(): "UNSET";
		
		std::string strategy_params_str  = "null";
		if( payload_json != Json::nullValue && !payload_json.empty())
		{
			strategy_params_str = (payload_json.isMember("st") ? "st:" + std::to_string(payload_json["st"].asUInt64()) + "," : "") + 
											  (payload_json.isMember("et") ? "et:" + std::to_string(payload_json["et"].asUInt64())+ ","  : "") +
											  (payload_json.isMember("dt") ?  "dt:" + std::to_string(payload_json["dt"].asUInt64())+ ","  : "") + 
											  (payload_json.isMember("dp") ? "dp:" + std::to_string(payload_json["dp"].asUInt64())+ ","  : "") +
											  (payload_json.isMember("access") ?  "access:" + std::to_string(payload_json["access"].asUInt64()) : "");
		}
		
		
		std::string timestamp_str 		= (metadata.isMember("timestamp") ? std::to_string(metadata["timestamp"].asUInt64()) : "0"); 

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
		size_t timestamp_size = 19; 
		uint8_t string_content_timestamp[timestamp_size];
		size_t offset = timestamp_size-string_size;
		if(offset > 0)
		{
			timestamp_str = std::string(offset,'0').append(timestamp_str);
		}
		for(size_t i= 0; i< timestamp_size; i++)
		{
			string_content_timestamp[i] = timestamp_str[i];
		}
		mobilityOperation->header.timestamp.buf = string_content_timestamp;
		mobilityOperation->header.timestamp.size = timestamp_size;

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
		free(mobilityOperation);
		return true;
	}
	catch(const std::runtime_error &e )
	{
		PLOG(logERROR) << "Failed to encoded Intersection Schedule into MobilityOperation message: " << e.what() <<std::endl;
		SetStatus<uint>(Key_ScheduleMessageSkipped, ++_scheduleMessageSkipped);
		return false;
	}
}

int CARMAStreetsPlugin::Main() {
	PLOG(logINFO) << "Starting plugin.";

	uint64_t lastSendTime = 0;

	while (_plugin->state != IvpPluginState_error) {


		usleep(100000); //sleep for microseconds set from config.
	}

	return (EXIT_SUCCESS);
}
} /* namespace */

int main(int argc, char *argv[]) {
	return run_plugin < CARMAStreetsPlugin::CARMAStreetsPlugin > ("CARMAStreetsPlugin", argc, argv);
}

