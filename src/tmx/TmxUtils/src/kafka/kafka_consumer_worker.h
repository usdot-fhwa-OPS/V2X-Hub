#ifndef KAFKA_CONSUMER_WORKER_H
#define KAFKA_CONSUMER_WORKER_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <PluginLog.h>

// #ifndef _WIN32
// #include <sys/time.h>
// #else
// #include <windows.h> /* for GetLocalTime */
// #endif

// #if _AIX
// #include <unistd.h>
// #endif

#include <librdkafka/rdkafkacpp.h>

using namespace tmx::utils;

namespace kafka_clients
{
    static int partition_cnt = 0;
    static int eof_cnt = 0;

    class consumer_rebalance_cb : public RdKafka::RebalanceCb 
    {
        private:
            static void part_list_print (const std::vector<RdKafka::TopicPartition*>&partitions)
            {
                for (unsigned int i = 0 ; i < partitions.size() ; i++)
                  FILE_LOG(logDEBUG) << "Topic " << partitions[i]->topic() << ", Partition " << partitions[i]->partition() << std::endl;
            }

        public:
            void rebalance_cb (RdKafka::KafkaConsumer *consumer, RdKafka::ErrorCode err, std::vector<RdKafka::TopicPartition*> &partitions) 
            {
                FILE_LOG(logDEBUG) << "RebalanceCb: " <<  RdKafka::err2str(err) << std::endl;
                part_list_print(partitions);

                RdKafka::Error *error = NULL;
                RdKafka::ErrorCode ret_err = RdKafka::ERR_NO_ERROR;

                if (err == RdKafka::ERR__ASSIGN_PARTITIONS) {
                if (consumer->rebalance_protocol() == "COOPERATIVE")
                    error = consumer->incremental_assign(partitions);
                else
                    ret_err = consumer->assign(partitions);
                    partition_cnt += (int)partitions.size();
                } 
                else 
                {
                    if (consumer->rebalance_protocol() == "COOPERATIVE") 
                    {
                        error = consumer->incremental_unassign(partitions);
                        partition_cnt -= (int)partitions.size();
                    } else {
                        ret_err = consumer->unassign();
                        partition_cnt = 0;
                    }
                }
                eof_cnt = 0; /* FIXME: Won't work with COOPERATIVE */

                if (error) {
                    FILE_LOG(logWARNING) << "Incremental assign failed: " << error->str() << std::endl;
                    delete error;
                } else if (ret_err)
                    FILE_LOG(logWARNING) << "Assign failed: " << error->str() << std::endl ;
            }
    };

    class consumer_event_cb : public RdKafka::EventCb 
    {
        public:
            consumer_event_cb(){};
            ~consumer_event_cb(){};
            void event_cb (RdKafka::Event &event) 
            {
                switch (event.type())
                {
                case RdKafka::Event::EVENT_ERROR:
                    if (event.fatal()) 
                    {
                        FILE_LOG(logERROR) << "FATAL: " << RdKafka::err2str(event.err()) << ", " << event.str() << std::endl;
                    }
                    FILE_LOG(logERROR) << RdKafka::err2str(event.err()) << ", " << event.str() << std::endl;
                    break;

                case RdKafka::Event::EVENT_STATS:
                    FILE_LOG(logINFO) << "STATS: " << RdKafka::err2str(event.err()) << ", " << event.str() << std::endl;
                    break;

                case RdKafka::Event::EVENT_LOG:
                    FILE_LOG(logINFO) << "EVENT: SEVERITY: " <<  event.severity() << " FAC:" << event.fac().c_str() 
                        << " MESSAGE :" << event.str().c_str() << std::endl;
                    break;

                case RdKafka::Event::EVENT_THROTTLE:
                    FILE_LOG(logINFO)  << " THROTTLED:  " << event.throttle_time() << "ms BROKER: " << (int)event.broker_id() << std::endl;
                    break;

                default:
                    FILE_LOG(logINFO) << "DEFAULT: " <<RdKafka::err2str(event.err()) << ", " << event.str() << std::endl;
                    break;
                }
            }
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
            int64_t _last_offset = 0;
            RdKafka::KafkaConsumer *_consumer = nullptr;
            RdKafka::Topic *_topic = nullptr;
            int64_t _cur_offet =  RdKafka::Topic::OFFSET_BEGINNING;
            int32_t _partition = 0;
            bool _run = false;
            consumer_event_cb _consumer_event_cb;
            consumer_rebalance_cb _consumer_rebalance_cb;
            const char* msg_consume(RdKafka::Message *message, void *opaque);

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