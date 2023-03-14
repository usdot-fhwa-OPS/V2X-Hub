#include "gtest/gtest.h"
#include "kafka/kafka_client.h"

TEST(test_kafka_client, read_json_file_get_value_by_doc)
{
    std::string expect_val = "localhost:9092";
    auto client = std::make_shared<kafka_clients::kafka_client>();
    std::string input_bootstrap_server_param1 = "BOOTSTRAP_SERVER";
}
