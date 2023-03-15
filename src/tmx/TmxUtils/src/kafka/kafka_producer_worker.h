#ifndef KAFKA_PRODUCER_WORKER_H
#define KAFKA_PRODUCER_WORKER_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <PluginLog.h>
#include <TmxLog.h>

#include <librdkafka/rdkafkacpp.h>

using namespace tmx::utils;
namespace kafka_clients
{  
    class producer_delivery_report_cb : public RdKafka::DeliveryReportCb
    {
        public:
            producer_delivery_report_cb(){

            };
            ~producer_delivery_report_cb(){

            };
            void dr_cb (RdKafka::Message &message)
            {
                FILE_LOG(logDEBUG) << "Message delivery length : " << message.len() << " content: " <<  message.errstr().c_str() << std::endl;
                if(message.key())                 
                    FILE_LOG(logDEBUG) << " Key:  " << (char*)(message.key()) << std::endl;
            }
    };
    class producer_event_cb:public RdKafka::EventCb
    {
        public:
            producer_event_cb(){

            };
            ~producer_event_cb(){
                
            };
            void event_cb (RdKafka::Event &event) 
            {
                switch (event.type())
                {
                case RdKafka::Event::EVENT_ERROR:                 
                    FILE_LOG(logERROR) <<  RdKafka::err2str(event.err())  << ". " << event.str() << std::endl;
                    if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
                    break;
                case RdKafka::Event::EVENT_STATS:
                    FILE_LOG(logINFO) <<  "STATS: " << RdKafka::err2str(event.err())  << ". " << event.str() << std::endl;
                    break;
                case RdKafka::Event::EVENT_LOG:
                    FILE_LOG(logINFO) <<  "LOG: " << RdKafka::err2str(event.err())  << ". " << event.str() << std::endl;
                    break;
                default:
                    FILE_LOG(logINFO) <<  "EVENT: " << RdKafka::err2str(event.err())  << ". " << event.str() << std::endl;
                    break;
                }
            }
    };

    class kafka_producer_worker
    {
        private:
            const std::string BOOTSTRAP_SERVER="bootstrap.servers";
            const std::string DR_CB="dr_cb";
            const std::string EVENT_CB="event_cb";

            RdKafka::Producer *_producer = nullptr;
            RdKafka::Topic *_topic = nullptr;
            std::string _topics_str = "";
            std::string _broker_str = "";
            bool _run = false;
            int _partition = 0;
            producer_delivery_report_cb _producer_delivery_report_cb;
            producer_event_cb _producer_event_cb;

        public:
            /**
             * @brief Construct a new kafka producer worker object
             * 
             * @param broker_str network address of kafka broker.
             * @param topic_str topic producer should produce to.
             * @param n_partition partition producer should be assigned to.
             */
            kafka_producer_worker(const std::string &brokers, const std::string &topics, int n_partition = 0);
            /**
             * @brief Initialize kafka_producer_worker
             * 
             * @return true if successful.
             * @return false if unsuccessful.
             */
            virtual bool init();
            /**
             * @brief Produce to topic.
             * 
             * @param msg message to produce.
             */
            virtual void send(const std::string &msg);
            /**
             * @brief Is kafka_producer_worker still running?
             * 
             * @return true if kafka producer is still running.
             * @return false if kafka producer is stopped.
             */
            virtual bool is_running() const;
            /**
             * @brief Stop running kafka producer.
             */
            virtual void stop();
            /**
             * @brief Print current configurations.
             */
            virtual void printCurrConf();
            /**
             * @brief Destroy the kafka producer worker object
             * 
             */
            virtual ~kafka_producer_worker() = default;
        };
}

#endif