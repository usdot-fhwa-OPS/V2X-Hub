#include "gtest/gtest.h"
#include "kafka/kafka_client.h"

TEST(test_kafka_producer_worker, create_producer)
{
    std::string broker_str = "localhost:9092";
    std::string topic = "test";
    auto client = std::make_shared<tmx::utils::kafka_client>();
    std::shared_ptr<tmx::utils::kafka_producer_worker> worker;
    worker = client->create_producer(broker_str, topic);
    worker->init();
    worker->printCurrConf();
    std::string msg = "test message";
    // // Run this unit test without launching kafka broker will throw connection refused error
    worker->send(msg);
    worker->stop();
}

TEST(test_kafka_producer_worker, create_producer_no_topic)
{
    std::string broker_str = "localhost:9092";
    std::string topic = "test";
    auto client = std::make_shared<tmx::utils::kafka_client>();
    std::shared_ptr<tmx::utils::kafka_producer_worker> worker;
    worker = client->create_producer(broker_str);
    worker->init();
    worker->printCurrConf();
    std::string msg = "test message";
    // // Run this unit test without launching kafka broker will throw connection refused error
    worker->send(msg, topic);
    worker->stop();
}