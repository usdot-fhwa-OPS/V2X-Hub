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
#include <gtest/gtest.h>


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
         * @brief Method to give the next available id of infrastructure
         * @return next id to be assigned
         */
        static int get_next_id() { return infrastructure_id++; }
        
    private:
        static int infrastructure_id;
        std::string simulation_ip;
        uint simulation_registration_port;
        uint infrastructure_id_adapter;
        std::string local_ip;
        uint time_sync_port;
        uint v2x_port;
        tmx::utils::WGS84Point location;
        std::shared_ptr<tmx::utils::kafka_producer_worker> time_producer;
        std::unique_ptr<CDASimConnection> connection;
        std::mutex _lock;

        FRIEND_TEST(TestCARMASimulationConnection, carma_simulation_handshake);
    };
}