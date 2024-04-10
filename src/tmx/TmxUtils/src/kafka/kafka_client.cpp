#include "kafka_client.h"

namespace tmx::utils
{
    std::shared_ptr<kafka_consumer_worker> kafka_client::create_consumer(const std::string &bootstrap_server, const std::string &topic_str,
                                                                                        const std::string &group_id_str) const
    {
        try
        {
            int partition = 0;
            int64_t cur_offset = RdKafka::Topic::OFFSET_END;
            auto consumer_ptr = std::make_shared<kafka_consumer_worker>(bootstrap_server, topic_str, group_id_str, cur_offset, partition);
            return consumer_ptr;
        }
        catch (const std::runtime_error &e)
        {
            FILE_LOG(logERROR) <<  "Create consumer failure: " <<  e.what() << std::endl;
            exit(1);
        }
    }

    std::shared_ptr<kafka_producer_worker> kafka_client::create_producer(const std::string &bootstrap_server, const std::string &topic_str) const
    {
        try
        {
            int partition = 0;
            auto producer_ptr = std::make_shared<kafka_producer_worker>(bootstrap_server, topic_str, partition);
            return producer_ptr;
        }
        catch (const std::runtime_error &e)
        {
            FILE_LOG(logERROR) <<  "Create producer failure: " <<  e.what() << std::endl;
            exit(1);
        }
    }

    std::shared_ptr<kafka_producer_worker> kafka_client::create_producer(const std::string &bootstrap_server) const
    {
        try
        {
            auto producer_ptr = std::make_shared<kafka_producer_worker>(bootstrap_server);
            return producer_ptr;
        }
        catch (const std::runtime_error &e)
        {
            FILE_LOG(logERROR) <<  "Create producer failure: " <<  e.what() << std::endl;
            exit(1);
        }
    }


}