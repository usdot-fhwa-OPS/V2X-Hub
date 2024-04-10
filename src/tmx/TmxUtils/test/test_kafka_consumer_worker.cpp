#include "gtest/gtest.h"
#include "kafka/kafka_client.h"
#include "PluginLog.h"

TEST(test_kafka_consumer_worker, create_consumer)
{
    std::string broker_str = "127.0.0.1:9092";
    std::string topic = "test";
    std::string group = "group_one";
    auto client = std::make_shared<tmx::utils::kafka_client>();
    std::shared_ptr<tmx::utils::kafka_consumer_worker>  worker;
    worker = client->create_consumer(broker_str, topic, group);
    worker->init();
    worker->subscribe();
    worker->printCurrConf();
    std::string payload = worker->consume(1000);

    // Run this unit test without launching kafka broker nor produce any messages to this topic.
    EXPECT_EQ(0, payload.length());
    if (worker->is_running())
    {
        worker->stop();
        EXPECT_FALSE(worker->is_running());
    }
}
