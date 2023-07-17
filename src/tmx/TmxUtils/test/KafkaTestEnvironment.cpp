#include "gtest/gtest.h"
#include <librdkafka/rdkafkacpp.h>

/**
 * @brief Kafka Test Environment which allows for Setup/Teardown configuration at the 
 * test program level. Teardown waits on all rd_kafka_t objects to be destroyed.
 */
class KafkaTestEnvironment : public ::testing::Environment {
    public:
        ~KafkaTestEnvironment() override {}

        // Override this to define how to set up the environment.
        void SetUp() override {}

        // Override this to define how to tear down the environment.
        void TearDown() override {
            std::cout << "Waiting for all RDKafka objects to be destroyed!" << std::endl;
            // Wait for all rd_kafka_t objects to be destroyed 
            auto error = RdKafka::wait_destroyed(5000);
            if (error == RdKafka::ERR__TIMED_OUT) {
                std::cout << "Wait destroy attempted timed out!" << std::endl;
            }
            else {
                std::cout << "All Objects are destroyed!" << std::endl;
            }
        }
};