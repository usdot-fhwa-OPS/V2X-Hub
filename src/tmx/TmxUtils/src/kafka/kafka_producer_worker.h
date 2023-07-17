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

namespace tmx::utils
{  
    class producer_delivery_report_cb : public RdKafka::DeliveryReportCb
    {
        public:
            producer_delivery_report_cb() = default;
            ~producer_delivery_report_cb() override = default;
            void dr_cb (RdKafka::Message &message) override;
    };
    class producer_event_cb:public RdKafka::EventCb
    {
        public:
            producer_event_cb() = default;
            ~producer_event_cb() override = default;
            void event_cb (RdKafka::Event &event) override;
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
            explicit kafka_producer_worker(const std::string &brokers, const std::string &topics, int n_partition = 0);
            /**
             * @brief Construct a new kafka producer worker object
             * 
             * @param broker_str network address of kafka broker.
             */
            explicit kafka_producer_worker(const std::string &brokers);

            virtual ~kafka_producer_worker();

            kafka_producer_worker(kafka_producer_worker& other) = delete;
            kafka_producer_worker& operator=(const kafka_producer_worker& other) = delete;
            kafka_producer_worker(kafka_producer_worker &&producer) = delete;
            kafka_producer_worker const & operator=(kafka_producer_worker &&producer) = delete;
            /**
             * @brief Initialize kafka_producer_worker. This method must be called before send!
             * 
             * @return true if successful.
             * @return false if unsuccessful.
             */
            virtual bool init();
             /**
             * @brief Initialize kafka topic.
             * @return true if successful.
             * @return false if unsuccessful.
             */
            virtual bool init_topic();
             /**
             * @brief Initialize kafka producer.
             * @return true if successful.
             * @return false if unsuccessful.
             */
            virtual bool init_producer();
            /**
             * @brief Produce to topic. Will result in segmentation fault if init() is not called on producer first.
             * @param msg message to produce.
             */
            virtual void send(const std::string &msg);
            /**
             * @brief Produce to a specific topic. Will result in segmentation fault if init() is not called on producer first.
             * @param msg message to produce.
             * @param topic_name topic to send the message to.
             */
            virtual void send(const std::string& message, const std::string& topic_name ) const;
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
            void stop();
            /**
             * @brief Print current configurations.
             */
            virtual void printCurrConf();
           
        };
}

#endif