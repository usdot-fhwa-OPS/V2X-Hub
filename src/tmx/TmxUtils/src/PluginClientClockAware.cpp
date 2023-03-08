#include "PluginClientClockAware.h"

#include "jsoncpp/json/json.h"

namespace tmx ::utils {

// declare the holder for this value
constexpr const char * PluginClientClockAware::KAFKA_TOPIC;

PluginClientClockAware::PluginClientClockAware(const std::string & name)
    : PluginClient(name)
{
    // check for simulation mode enabled by environment variable
  	constexpr const char * EnvVar = "SIMULATION_MODE";
	bool simulationMode = (std::getenv(EnvVar) != nullptr);

    using namespace fwha_stol::lib::time;
    clock = std::make_shared<CarmaClock>(simulationMode);
    if (simulationMode) {
        // if simulation then get time updates via kafka
        startKafkaConsumer();
    }
}

void PluginClientClockAware::startKafkaConsumer() {
    // constants for now, TODO replace with configuration
    constexpr const char * KAFKA_BROKER_IP = "127.0.0.1";
    constexpr const char * KAFKA_BROKER_PORT = "9092";
    constexpr const char * KAFKA_GROUP_ID = "v2xhub_time";

    RdKafka::Conf * kafkaTimeConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    std::string kafkaConnectString = std::string(KAFKA_BROKER_IP) + ':' + KAFKA_BROKER_PORT;
    std::string error_string;

    PLOG(logINFO) << _name << ": Starting Kafka connection for simulated time to: " << kafkaConnectString;

    // set all of the configuration options
    // group id needs to differ if we want each client to get all of the messages
    // so we append the plugin name
    std::string groupId = std::string(KAFKA_GROUP_ID) + "_" + _name;
	if ((kafkaTimeConf->set("bootstrap.servers", kafkaConnectString, error_string)  != RdKafka::Conf::CONF_OK)
		 || (kafkaTimeConf->set("group.id", groupId, error_string) != RdKafka::Conf::CONF_OK)) {
		PLOG(logERROR) << _name << ": Setting kafka config group.id options failed with error:" << error_string << std::endl <<"Exiting with exit code 1";
		exit(1);
	} else {
		PLOG(logDEBUG) << _name << ": Kafka config group.id options set successfully";
	}
	kafkaTimeConf->set("enable.partition.eof", "true", error_string);

    // configuration is set, now create consumer
	kafkaTimeConsumer = RdKafka::KafkaConsumer::create(kafkaTimeConf, error_string);
	if ( !kafkaTimeConsumer) {
		PLOG(logERROR) << _name << ": Failed to create Kafka consumers: " << error_string << std::endl;
		exit(1);
	}
    delete kafkaTimeConf;
	PLOG(logDEBUG) << _name << ": Created consumer " << kafkaTimeConsumer->name() << std::endl;

	// create kafka topics
	RdKafka::Conf * topicConf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
	if (!topicConf)
	{
		PLOG(logERROR) << _name << ": RDKafka create topic conf failed ";
		exit(1);
	}
    RdKafka::Topic * kafkaTimeTopic = RdKafka::Topic::create(kafkaTimeConsumer,
        KAFKA_TOPIC, topicConf, error_string);
	if(!kafkaTimeTopic)
	{
		PLOG(logERROR) << _name << ": RDKafka create time topic failed:" << error_string;
		exit(1);
	}
	delete topicConf;
	consumerThread = std::thread(&PluginClientClockAware::runKafkaConsumer, this);
}

void PluginClientClockAware::runKafkaConsumer() {
    std::vector<std::string> topics;
    topics.emplace_back(KAFKA_TOPIC);

    RdKafka::ErrorCode err = kafkaTimeConsumer->subscribe(topics);
    if (err) {
        PLOG(logERROR) << _name << ": Failed to subscribe to " << topics.size() << " topics: " << RdKafka::err2str(err) << std::endl;
        return;
    }
    Json::Reader payloadReader;
    while (true)
    {
        auto msg = kafkaTimeConsumer->consume( 500 );
        if( msg->err() == RdKafka::ERR_NO_ERROR )
        {
            auto payloadString = static_cast<const char *>( msg->payload() );
            if(msg->len() > 0)
            {
                PLOG(logDEBUG) << _name << ": consumed message payload: " << payloadString <<std::endl;
                Json::Value  payloadJsonValue;
                auto parse_sucessful = payloadReader.parse(payloadString, payloadJsonValue);
                if( !parse_sucessful )
                {
                    PLOG(logERROR) << _name << ": Error parsing payload: " << payloadString << std::endl;
                    continue;
                }
                if (payloadJsonValue.isMember("time_stamp")) {
                    fwha_stol::lib::time::timeStampMilliseconds timeStamp = payloadJsonValue["time_stamp"].asUInt64();
                    clock->update(timeStamp);
                } else {
                    PLOG(logERROR) << _name << ": Failed to read time stamp from message : " << std::endl << payloadString << std::endl;
                }
            }
        }
        delete msg;
    }
}

}