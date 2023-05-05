
#include "kafka_producer_worker.h"
namespace tmx::utils
{
    void producer_delivery_report_cb::dr_cb (RdKafka::Message &message) 
    {
        FILE_LOG(logDEBUG) << "Message delivery length : " << message.len() << " content: " <<  message.errstr().c_str() << std::endl;
        if(message.key())                 
            FILE_LOG(logDEBUG) << " Key:  " << message.key() << std::endl;
    }

    void producer_event_cb::event_cb (RdKafka::Event &event) 
            {
                switch (event.type())
                {
                case RdKafka::Event::EVENT_ERROR:                 
                    FILE_LOG(logERROR) <<  RdKafka::err2str(event.err())  << ". " << event.str() << std::endl;
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

    kafka_producer_worker::kafka_producer_worker(const std::string &brokers, const std::string &topics, int partition)
        : _topics_str(topics), _broker_str(brokers), _run(true), _partition(partition)
    {
    }

    bool kafka_producer_worker::init()
    {
        std::string errstr = "";

        // Create configuration objects
        RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

        /***
         * Set Configuration properties
         */
        // set broker list
        if (conf->set(BOOTSTRAP_SERVER, _broker_str, errstr) != RdKafka::Conf::CONF_OK)
        {
            FILE_LOG(logWARNING) << "RdKafka conf set brokerlist failed: " <<  errstr.c_str() << std::endl;
            return false;
        }

        // set delivery report callback
        if (conf->set(DR_CB, &_producer_delivery_report_cb, errstr) != RdKafka::Conf::CONF_OK)
        {
            FILE_LOG(logWARNING) << "RdKafka conf set delivery report callback failed: " <<  errstr.c_str();
            return false;
        }

        // set event callback
        if (conf->set(EVENT_CB, &_producer_event_cb, errstr) != RdKafka::Conf::CONF_OK)
        {
            FILE_LOG(logWARNING) << "RdKafka conf set event callback failed: " << errstr.c_str() << std::endl;
            return false;
        }

        // create producer using accumulated global configuration.
        _producer = RdKafka::Producer::create(conf, errstr);
        if (!_producer)
        {
            FILE_LOG(logWARNING) << "Failed to create producer: " << errstr.c_str() << std::endl;
            return false;
        }
        delete conf;

        FILE_LOG(logINFO) << "Created producer: " << _producer->name() << std::endl;

        // Create topic handle
        _topic = RdKafka::Topic::create(_producer, _topics_str, tconf, errstr);
        if (!_topic)
        {
            FILE_LOG(logWARNING) <<  "Failed to create producer: " << errstr.c_str();

            return false;
        }
        delete tconf;
        printCurrConf();
        return true;
    }

    bool kafka_producer_worker::is_running() const {
        return _run;
    }

    void kafka_producer_worker::send(const std::string &msg)
    {

        if (!_run)
            return;

        if (msg.empty())
        {
            _producer->poll(0);
            return;
        }
        // produce messages
        while (true)
        {
            RdKafka::ErrorCode resp = _producer->produce(_topic,
                                                         _partition,
                                                         RdKafka::Producer::RK_MSG_COPY,
                                                         const_cast<char *>(msg.c_str()),
                                                         msg.size(),
                                                         nullptr,
                                                         nullptr);
            if (resp != RdKafka::ERR_NO_ERROR)
            {
                FILE_LOG(logWARNING) <<  _producer->name() << " produce failed: " <<  RdKafka::err2str(resp) << std::endl;
                if (resp == RdKafka::ERR__QUEUE_FULL)
                {
                    /* If the internal queue is full, wait for
                     * messages to be delivered and then retry.
                     * The internal queue represents both
                     * messages to be sent and messages that have
                     * been sent or failed, awaiting their
                     * delivery report callback to be called.
                     *
                     * The internal queue is limited by the
                     * configuration property
                     * queue.buffering.max.messages */
                    _producer->poll(1000 /*block for max 1000ms*/);
                    continue;
                }
            }
            else
            {
                FILE_LOG(logDEBUG) << _producer->name()  << " produced message size: " <<  msg.size() 
                    << " message content: " << msg.c_str() << std::endl;
            }

            // break the loop regardless of sucessfully sent or failed
            break;
        }

        /* A producer application should continually serve
         * the delivery report queue by calling poll()
         * at frequent intervals.
         * Either put the poll call in your main loop, or in a
         * dedicated thread, or call it after every produce() call.
         * Just make sure that poll() is still called
         * during periods where you are not producing any messages
         * to make sure previously produced messages have their
         * delivery report callback served (and any other callbacks
         * you register). */
        _producer->poll(0);
    }

    void kafka_producer_worker::stop()
    {
        /* Wait for final messages to be delivered or fail.
         * flush() is an abstraction over poll() which
         * waits for all messages to be delivered. */
        _run = false;
        FILE_LOG(logWARNING) << "Stopping producer client.. " << std::endl << "Flushing final messages... " << std::endl;
       try
        {
            if (_producer)
            {
                _producer->flush(10 * 1000 /* wait for max 10 seconds */);

                if (_producer->outq_len() > 0)
                   FILE_LOG(logWARNING) << _producer->name() << _producer->outq_len() << " message(s) were not delivered." << std::endl;
            }
        }
        catch (const std::runtime_error &e)
        {
            FILE_LOG(logERROR) << "Error encountered flushing producer : " << e.what() << std::endl;
        }
    }

    void kafka_producer_worker::printCurrConf()
    {
        FILE_LOG(logINFO) << "Producer connect to bootstrap_server: " << (_broker_str.empty() ? "UNKNOWN" : _broker_str) 
                << " topic: " << (_topics_str.empty() ? "UNKNOWN" : _topics_str) << " ,partition: " <<  _partition << std::endl;
    }
}