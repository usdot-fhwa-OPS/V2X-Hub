//============================================================================
// Name        : CDASimAdapter.cpp
// Author      : Paul Bourelly
// Version     : 7.5.1
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
#include <PluginClient.h>
#include "CDASimConnection.hpp"
#include <simulation/SimulationEnvUtils.h>
#include "ThreadWorker.h"

namespace CDASimAdapter
{

    /**
     * @brief V2X-Hub Plugin that acts as a adapter for integration with CARMA-Simulation. Plugin used
     * environment variable to be installed and enabled by default.
     */
    class CDASimAdapter : public tmx::utils::PluginClient
    {
    public:
        /**
         * @brief CARMA-Simulation Infrastructure Adapter constructor.
         * @param name name of plugin.
         */
        explicit CDASimAdapter(const std::string &name);

        int Main() override;

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
         * @brief Method to attempt to establish connection between CARMA Simulation and Infrastructure Software (V2X-Hub).
         * @return true if successful and false if unsuccessful.
         */
        bool connect();

        /**
         * @brief Method to start thread timer for processing msg from v2xhub
         */
        void start_immediate_forward_thread();

        /**
         * @brief Method to start thread timer for processing msg from CDASimConnection
         */
        void start_message_receiver_thread();

        /**
         * @brief Method to consume msg in amf fromat from V2Xhub and forward to CDASimConnection
         */
        void attempt_message_from_v2xhub() const;

        /**
         * @brief Method to consume ans1 binary msg from CDASimConnection and forward to V2Xhub
         */
        void attempt_message_from_simulation() const;
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
        // Simulated location of RSU
        tmx::utils::Point location;
        // Stores configurable MaxConnectionAttempts. Any value > 1 will result in infinite connection attempts.
        int max_connection_attempts;
        // Time in seconds between connection attempts. Most be greater than zero!
        uint connection_sleep_time;
        // Kafka producer for sending time_sync messages to carma-streets
        std::shared_ptr<tmx::utils::kafka_producer_worker> time_producer;
        // CDASim connection
        std::unique_ptr<CDASimConnection> connection;
        // Mutex for configuration parameter thread safety
        std::mutex _lock;
        // Time sync thread to forward time sync messages to PluginClientClockAware V2X-Hub plugins.
        std::unique_ptr<tmx::utils::ThreadTimer> time_sync_timer;
        // Time sync thread id
        int time_sync_tick_id;
        // Immediate forward thread to consume messages from the immediate forward plugin and send to CDASim
        std::unique_ptr<tmx::utils::ThreadTimer> immediate_forward_timer;
        // Immediate forward thread id
        int immediate_forward_tick_id;
        // Message receiver thread to consume messages from CDASim and forward them to the message receiver.
        std::unique_ptr<tmx::utils::ThreadTimer> message_receiver_timer;
        // Message receiver thread id
        int message_receiver_tick_id;
    };
}