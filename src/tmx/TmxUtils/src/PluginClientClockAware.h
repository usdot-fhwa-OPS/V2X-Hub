#pragma once

#include "PluginClient.h"
#include "carma-clock/carma_clock.h"

#include <librdkafka/rdkafkacpp.h>

namespace tmx ::utils {

/**
 * A base plugin class which is can handle simulated time as needed.
*/
class PluginClientClockAware : public PluginClient {
public:
    explicit PluginClientClockAware(const std::string & name);

protected:
    inline std::shared_ptr<fwha_stol::lib::time::CarmaClock> getClock() const {
        return clock;
    } 

private:
    static constexpr const char * KAFKA_TOPIC = "TimeTopic";

    std::shared_ptr<fwha_stol::lib::time::CarmaClock> clock;
	RdKafka::KafkaConsumer * kafkaTimeConsumer = nullptr;
    std::thread consumerThread;

    void startKafkaConsumer();
    void runKafkaConsumer();
};

}