//============================================================================
// Name        : EpcwPlugin.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#pragma once
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include <tmx/tmx.h>
#include <tmx/IvpPlugin.h>
#include <PluginClientClockAware.h>
#include "CDASimConnection.hpp"
#include <kafka/kafka_producer_worker.h>
#include <kafka/kafka_client.h>
#include <simulation/SimulationEnvVar.h>
#include "ThreadWorker.h"



namespace CDASimAdapter {
    /**
     * @brief V2X-Hub Plugin that acts as a adapter for integration with CARMA-Simulation. Plugin used 
     * environment variable to be installed and enabled by default.
     */
    class CDASimAdapter: public tmx::utils::PluginClientClockAware {
    public:
        /**
         * @brief CCARMA-Simulation Infrastucture Adapter constructor.
         * @param name name of plugin.
         */
        explicit CDASimAdapter(const std::string &name);
        
        int Main() override ;
    protected:
        /**
         * @brief Called everytime a configuration value is changed for the plugin.
         */
        void UpdateConfigSettings();
        // Virtual method overrides START
        /**
         * @brief Overrides PluginClient OnConfigChanged(const char *key, const char *value) method
         * and calls UpdateConfigSettings() on each configuration change.
         * @param key string key of the configuration value that has changed.
         * @param value new value of the configuration that has changed.
         */
        void OnConfigChanged(const char *key, const char *value) override;
        /**
         * @brief Overrides PluginClient OnStateChange(IvpPluginState state) method.
         * @param state new state of the plugin.
         */
        void OnStateChange(IvpPluginState state) override;
        // Virtual method overrides END.
        
        /**
         * @brief Get Kafka Connection string from environment variable KAFKA_BROKER_ADDRESS and time sync topic name from
         * CARMA_INFRASTRUCTURE_TIME_SYNC_TOPIC and initialize a Kafka producer to forward time synchronization messages to
         * all infrastructure services.
         * @return true if initialization is successful and false if initialization fails.
         */
        bool initialize_time_producer();
        /**
         * @brief Method to attempt to establish connection between CARMA Simulation and Infrastructure Software (V2X-Hub).
         * @return true if successful and false if unsuccessful.
         */
        bool connect();

        /**
         * @brief Method to start thread timer for processing msg from v2xhub
         */
        void start_amf_msg_thread();

        /**
         * @brief Method to start thread timer for processing msg from CDASimConnection
         */
        void start_binary_msg_thread();

        /**
         * @brief Method to consume msg in amf fromat from V2Xhub and forward to CDASimConnection
         */
        void attempt_message_from_v2xhub();

        /**
         * @brief Method to consume ans1 binary msg from CDASimConnection and forward to V2Xhub
         */
        void attempt_message_from_simulation();
        /**
         * @brief Forward time sychronization message to TMX message bus for other V2X-Hub Plugin and to infrastructure Kafka Broker for
         * CARMA Streets services
         * @param msg TimeSyncMessage.
         */
        void forward_time_sync_message(tmx::messages::TimeSyncMessage &msg);
        /**
         * @brief Method to start thread timer for regular interval actions lauched on seperate thread.
         */
        void start_time_sync_thread_timer();
        /**
         * @brief Method to consume time sychrononization from CDASimConnection and forward to tmx core and CARMA Streets
         */
        void attempt_time_sync();
        
    private:

        std::string simulation_ip;
        uint simulation_registration_port;
        std::string local_ip;
        uint time_sync_port;
        uint v2x_port;
        tmx::utils::WGS84Point location;
        std::shared_ptr<tmx::utils::kafka_producer_worker> time_producer;
        std::unique_ptr<CDASimConnection> connection;
        std::mutex _lock;
        std::unique_ptr<tmx::utils::ThreadTimer> thread_timer;
        int time_sync_tick_id;

        std::unique_ptr<tmx::utils::ThreadTimer> amf_thread_timer;
        std::unique_ptr<tmx::utils::ThreadTimer> binary_thread_timer;
        int amf_msg_tick_id;
        int binary_msg_tick_id;

    };
}