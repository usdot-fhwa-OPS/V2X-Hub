#include "PluginClientClockAware.h"

#include "jsoncpp/json/json.h"

namespace tmx ::utils {

// declare the holder for this value
constexpr const char * PluginClientClockAware::KAFKA_TOPIC;

PluginClientClockAware::PluginClientClockAware(const std::string & name)
    : PluginClient(name)
{
    // check for simulation mode enabled by environment variable
	bool simulationMode = sim::is_simulation_mode();

    using namespace fwha_stol::lib::time;
    clock = std::make_shared<CarmaClock>(simulationMode);
    if (simulationMode) {
        // if simulation then get time updates via kafka
        startKafkaConsumer();
    }
}

void PluginClientClockAware::startKafkaConsumer() {
    kafka_client client;
    timeConsumer = client.create_consumer( std::getenv(sim::KAFKA_BROKER_ADDRESS), std::getenv( sim::TIME_SYNC_TOPIC), "v2xhub_time"+_name );

	consumerThread = std::thread(&PluginClientClockAware::runKafkaConsumer, this);
}

void PluginClientClockAware::runKafkaConsumer() {
    Json::Reader payloadReader;
    while (true)
    {
        auto msg = timeConsumer->consume(500);
        
        PLOG(logDEBUG) << _name << ": consumed message payload: " << msg <<std::endl;
        Json::Value  payloadJsonValue;
        auto parse_sucessful = payloadReader.parse(msg, payloadJsonValue);
        if( !parse_sucessful )
        {
            PLOG(logERROR) << _name << ": Error parsing payload: " << msg << std::endl;
            continue;
        }
        if (payloadJsonValue.isMember("time_stamp")) {
            fwha_stol::lib::time::timeStampMilliseconds timeStamp = payloadJsonValue["time_stamp"].asUInt64();
            clock->update(timeStamp);
        } else {
            PLOG(logERROR) << _name << ": Failed to read time stamp from message : " << std::endl << msg << std::endl;
        }
        
    }
}

}