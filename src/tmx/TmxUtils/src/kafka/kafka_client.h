#ifndef KAFKA_CLIENT_H
#define KAFKA_CLIENT_H

#include "kafka_producer_worker.h"
#include "kafka_consumer_worker.h"



namespace kafka_clients
{

    class kafka_client
    {
    public:
        std::shared_ptr<kafka_clients::kafka_consumer_worker> create_consumer(const std::string &broker_str, const std::string &topic_str,
                                                                              const std::string &group_id_str) const;
        std::shared_ptr<kafka_clients::kafka_producer_worker> create_producer(const std::string &broker_str, const std::string &topic_str) const;
    };

}

#endif // !KAFKA_CLIENT_H