#pragma once
#include "kafka_producer_worker.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
namespace tmx::utils {
    /**
     * @brief Mock kafka consumer used for unit testing using gmock. For documentation using gmock mocks 
     * (https://google.github.io/googletest/gmock_for_dummies.html).
     * 
     * @author
     */
    class mock_kafka_producer_worker : public kafka_producer_worker {
        public:
            /**
             * @brief Mock constructor with all default parameters. Can be used as an default constructor.
             * 
             * @param broker_str 
             * @param topic_str 
             * @param n_partition 
             */
            mock_kafka_producer_worker(const std::string &broker_str="",
                                        const std::string &topic_str="", 
                                        int n_partition = 0) : kafka_producer_worker(broker_str,topic_str, n_partition ) 
                                        {};
            ~mock_kafka_producer_worker() = default;
            MOCK_METHOD(bool, init,(),(override));
            MOCK_METHOD(void, send, (const std::string &msg), (override));
            MOCK_METHOD(bool, is_running, (), (const, override));
            MOCK_METHOD(void, stop, (), (override));
            MOCK_METHOD(void, printCurrConf, (), (override));
    };
}