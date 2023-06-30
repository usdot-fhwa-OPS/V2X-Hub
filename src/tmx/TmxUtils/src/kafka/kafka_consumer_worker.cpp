#include "kafka_consumer_worker.h"

namespace tmx::utils
{
    kafka_consumer_worker::kafka_consumer_worker(const std::string &broker_str, const std::string &topic_str,
                                                const std::string &group_id_str, int64_t cur_offset, int32_t partition)
        :_topics_str(topic_str), _broker_str(broker_str), _group_id_str(group_id_str), _cur_offset(cur_offset),
        _partition(partition)
    {
    }

    bool kafka_consumer_worker::init()
    {
        FILE_LOG(logDEBUG1) << "kafka_consumer_worker init()... " << std::endl;

        std::string errstr;
        RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        if (!conf)
        {
            FILE_LOG(logWARNING) << "RDKafka create global conf failed " << std::endl;
            return false;
        }

        // set bootstrap server
        if (conf->set(BOOTSTRAP_SERVER, _broker_str, errstr) != RdKafka::Conf::CONF_OK)
        {
            FILE_LOG(logWARNING) << "RDKafka conf set bootstrap server failed: " << errstr.c_str() << std::endl;
            return false;
        }

        conf->set("rebalance_cb", &_consumer_rebalance_cb, errstr);

        if (conf->set(EVENT_CB, &_consumer_event_cb, errstr) != RdKafka::Conf::CONF_OK)
        {
            FILE_LOG(logWARNING) << "RDKafka conf set event call back failed: " <<  errstr.c_str() << std::endl;
            return false;
        }

        if (conf->set(ENABLE_PARTITION_END_OF, "true", errstr) != RdKafka::Conf::CONF_OK)
        {
            FILE_LOG(logWARNING) << "RDKafka conf set partition end of failed: " << errstr.c_str() << std::endl;
            return false;
        }

        if (conf->set(ENABLE_AUTO_COMMIT, "true", errstr) != RdKafka::Conf::CONF_OK)
        {
            FILE_LOG(logWARNING) << "RDKafka conf set enable auto commit failed: " << errstr.c_str() << std::endl;
            return false;
        }

        // set consumer group
        if (conf->set(GROUP_ID, _group_id_str, errstr) != RdKafka::Conf::CONF_OK)
        {
            FILE_LOG(logWARNING) << "RDKafka conf set group id failed: " << errstr.c_str() << std::endl;
            return false;
        }

        if (conf->set(MAX_PARTITION_FETCH_SIZE, STR_FETCH_NUM, errstr) != RdKafka::Conf::CONF_OK)
        {
            FILE_LOG(logWARNING) << "RDKafka cof set max.partition failed: " << errstr.c_str() << std::endl;
        }

        // create consumer
        _consumer = RdKafka::KafkaConsumer::create(conf, errstr);
        if (!_consumer)
        {
            FILE_LOG(logWARNING) << "Failed to create consumer: " << errstr.c_str() << std::endl;
            return false;
        }

        FILE_LOG(logINFO) << "Created consumer: " << _consumer->name() << std::endl;
        delete conf;

        // create kafka topic
        RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
        if (!tconf)
        {
            FILE_LOG(logWARNING) << "RDKafka create topic conf failed " << std::endl;
            return false;
        }

        _topic = RdKafka::Topic::create(_consumer, _topics_str, tconf, errstr);
        if (!_topic)
        {
            FILE_LOG(logWARNING) << "RDKafka create topic failed: " <<  errstr.c_str() << std::endl;
            return false;
        }

        delete tconf;
        printCurrConf();
        return true;
    }

    void kafka_consumer_worker::stop()
    {
        _run = false;
        _consumer->close();
        /*Destroy kafka instance*/ // Wait for RdKafka to decommission.
        RdKafka::wait_destroyed(5000);
    }

    void kafka_consumer_worker::subscribe()
    {
        std::vector<std::string> _topics_str_list;
        _topics_str_list.push_back(_topics_str);
        RdKafka::ErrorCode err = _consumer->subscribe(_topics_str_list);
        if (err)
        {
            FILE_LOG(logWARNING) <<  _consumer->name() << " failed to subscribe to " <<  _topics_str_list.size() << " topics: " <<  RdKafka::err2str(err).c_str() << std::endl;
            _run = false;
            exit(1);
        } else {
            FILE_LOG(logINFO) <<  _consumer->name() <<  " successfully to subscribe to " << _topics_str_list.size() << " topics " 
                << RdKafka::err2str(err).c_str() << std::endl;
            _run = true;
        }
    }

    const char *kafka_consumer_worker::consume(int timeout_ms)
    {
        const RdKafka::Message *msg = nullptr;
        msg = _consumer->consume(timeout_ms);
        const char *msg_str = msg_consume(msg);
        return msg_str;
    }

    bool kafka_consumer_worker::is_running() const
    {
        return _run;
    }
    void kafka_consumer_worker::printCurrConf()
    {
        FILE_LOG(logINFO) << "Consumer connect to bootstrap_server: " << (_broker_str.empty() ? "UNKNOWN" : _broker_str) 
            << " , topic: " << (_topics_str.empty() ? "UNKNOWN" : _topics_str) << " , partition:  " << _partition << ", group id: "
            << (_group_id_str.empty() ? "UNKNOWN" : _group_id_str) << std::endl;
    }

    const char *kafka_consumer_worker::msg_consume(const RdKafka::Message *message)
    {
        const char *return_msg_str = "";
        switch (message->err())
        {
        case RdKafka::ERR__TIMED_OUT:
            FILE_LOG(logDEBUG4) << _consumer->name() << " consume failed: " <<  message->errstr() << std::endl;
            break;
        case RdKafka::ERR_NO_ERROR:
            FILE_LOG(logDEBUG1) << _consumer->name() << " read message at offset " <<  message->offset() << std::endl;
            FILE_LOG(logDEBUG1) << _consumer->name() << " message Consumed: " << static_cast<int>(message->len())  << " bytes : " << static_cast<const char *>(message->payload()) << std::endl;
            _cur_offset = message->offset();
            return_msg_str = static_cast<const char *>(message->payload());
            break;
        case RdKafka::ERR__PARTITION_EOF:
            FILE_LOG(logWARNING) << _consumer->name() << " reached the end of the queue, offset : " <<  _cur_offset << std::endl;
            break;
        case RdKafka::ERR__UNKNOWN_TOPIC:
            FILE_LOG(logWARNING) << _consumer->name() << " consume failed: " <<  message->errstr() << std::endl;
            break;
        case RdKafka::ERR__UNKNOWN_PARTITION:
            FILE_LOG(logWARNING) << _consumer->name() << " consume failed: " << message->errstr() << std::endl;
            stop();
            break;

        default:
            /* Errors */
            FILE_LOG(logWARNING) << _consumer->name() << " consume failed: " << message->errstr() << std::endl;
            stop();
            break;
        }
        return return_msg_str;
    }

    void consumer_rebalance_cb::part_list_print(const std::vector<RdKafka::TopicPartition*>&partitions)
    {
        for (unsigned int i = 0 ; i < partitions.size() ; i++) {
            FILE_LOG(logDEBUG) << "Topic " << partitions[i]->topic() << ", Partition " << partitions[i]->partition() << std::endl;
        }
    }

    void consumer_rebalance_cb::rebalance_cb(RdKafka::KafkaConsumer *consumer, RdKafka::ErrorCode err, std::vector<RdKafka::TopicPartition*> &partitions) 
    {
        FILE_LOG(logDEBUG) << "RebalanceCb: " <<  RdKafka::err2str(err) << std::endl;
        part_list_print(partitions);

        RdKafka::Error *error = nullptr;
        RdKafka::ErrorCode ret_err = RdKafka::ERR_NO_ERROR;

        if (err == RdKafka::ERR__ASSIGN_PARTITIONS) {
            if (consumer->rebalance_protocol() == "COOPERATIVE") {
                error = consumer->incremental_assign(partitions);
            }
            else {
                ret_err = consumer->assign(partitions);
            }
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

    void consumer_event_cb::event_cb (RdKafka::Event &event) {
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
                FILE_LOG(logINFO)  << " THROTTLED:  " << event.throttle_time() << "ms BROKER: " << event.broker_id() << std::endl;
                break;

            default:
                FILE_LOG(logINFO) << "DEFAULT: " <<RdKafka::err2str(event.err()) << ", " << event.str() << std::endl;
                break;
        }
    }
    
}