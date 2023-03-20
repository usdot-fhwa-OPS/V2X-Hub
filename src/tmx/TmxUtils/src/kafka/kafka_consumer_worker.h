#ifndef KAFKA_CONSUMER_WORKER_H
#define KAFKA_CONSUMER_WORKER_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <PluginLog.h>
#include <librdkafka/rdkafkacpp.h>

namespace tmx::utils {
    static int partition_cnt = 0;
    static int eof_cnt = 0;

    class consumer_rebalance_cb : public RdKafka::RebalanceCb 
    {
        private:
            static void part_list_print (const std::vector<RdKafka::TopicPartition*>&partitions);
        public:
            void rebalance_cb (RdKafka::KafkaConsumer *consumer, RdKafka::ErrorCode err, 
                            std::vector<RdKafka::TopicPartition*> &partitions) override;
    };

    class consumer_event_cb : public RdKafka::EventCb 
    {
        public:
            consumer_event_cb() = default;
            ~consumer_event_cb() override = default;
            void event_cb (RdKafka::Event &event) override ;
    };
        
    class kafka_consumer_worker
    {
        private:
            const std::string BOOTSTRAP_SERVER="bootstrap.servers";
            const std::string DR_CB="dr_cb";
            const std::string EVENT_CB="event_cb";
            const std::string GROUP_ID="group.id";
            const std::string MAX_PARTITION_FETCH_SIZE="max.partition.fetch.bytes";
            const std::string ENABLE_PARTITION_END_OF="enable.partition.eof";

            //maximum size for pulling message from a single partition at a time
            std::string STR_FETCH_NUM = "10240000";
            
            std::string _topics_str = "";
            std::string _broker_str = "";
            std::string _group_id_str = "";
            RdKafka::KafkaConsumer *_consumer = nullptr;
            RdKafka::Topic *_topic = nullptr;
            int64_t _cur_offset =  RdKafka::Topic::OFFSET_BEGINNING;
            int32_t _partition = 0;
            bool _run = false;
            consumer_event_cb _consumer_event_cb;
            consumer_rebalance_cb _consumer_rebalance_cb;
            const char* msg_consume(const RdKafka::Message *message);

        public:
            /**
             * @brief Construct a new kafka consumer worker object
             * 
             * @param broker_str network adress of kafka broker.
             * @param topic_str topic consumer should consume from.
             * @param group_id consumer group id.
             * @param cur_offset offset to start event consuming at. Defaults to 0.
             * @param partition partition consumer should be assigned to.
             */
            kafka_consumer_worker(const std::string &broker_str, const std::string &topic_str, const std::string & group_id, int64_t cur_offset = 0, int32_t partition = 0);
            /**
             * @brief Initialize kafka_consumer_worker
             * 
             * @return true if successful.
             * @return false if unsuccessful.
             */
            virtual bool init();
            /**
             * @brief Consume from topic.
             * 
             * @param timeout_ms timeout in milliseconds to wait before failing.;
             * @return const char* of payload consumed.
             */
            virtual const char* consume(int timeout_ms);
            /**
             * @brief Subscribe consumer to topic
             */
            virtual void subscribe();
            /**
             * @brief Stop running kafka consumer.
             */
            virtual void stop();
            /**
             * @brief Print current configurations.
             */
            virtual void printCurrConf();
            /**
             * @brief Is kafka_consumer_worker still running?
             * 
             * @return true if kafka consumer is still running.
             * @return false if kafka consumer is stopped.
             */
            virtual bool is_running() const;
            /**
             * @brief Destroy the kafka consumer worker object
             * 
             */
            virtual ~kafka_consumer_worker() = default;
    };
    
}
#endif