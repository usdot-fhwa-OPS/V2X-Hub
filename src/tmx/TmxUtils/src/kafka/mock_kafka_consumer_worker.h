#pragma once
#include "kafka_consumer_worker.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
namespace kafka_clients {
    /**
     * @brief Mock kafka consumer used for unit testing using gmock. For documentation using gmock mocks 
     * (https://google.github.io/googletest/gmock_for_dummies.html).
     * 
     * @author
     */
    class mock_kafka_consumer_worker : public kafka_clients::kafka_consumer_worker {
        public:
            /**
             * @brief Mock constructor with all default parameters. Can be used as an default constructor.
             * 
             * @param broker_str 
             * @param topic_str 
             * @param group_id 
             * @param cur_offset 
             * @param partition 
             */
            mock_kafka_consumer_worker(const std::string &broker_str="",
                                        const std::string &topic_str="", 
                                        const std::string &group_id="", 
                                        int64_t cur_offset = 0, 
                                        int32_t partition = 0) : kafka_consumer_worker(broker_str,topic_str, group_id, cur_offset, partition ) 
                                        {};
            ~mock_kafka_consumer_worker() = default;

            MOCK_METHOD(bool, init,(),(override));
            MOCK_METHOD(const char*, consume, (int timeout_ms), (override));
            MOCK_METHOD(void, subscribe, (), (override));
            MOCK_METHOD(void, stop, (), (override));
            MOCK_METHOD(void, printCurrConf, (), (override));
            MOCK_METHOD(bool, is_running, (), (const override));
    };
}