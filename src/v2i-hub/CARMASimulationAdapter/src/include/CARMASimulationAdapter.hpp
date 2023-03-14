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
#include <CARMASimulationConnection.hpp>
#include <kafka_producer_worker.hpp>

using namespace std;
using namespace tmx;
using namespace tmx::utils;

namespace CARMASimulationAdapter {
    /**
     * @brief V2X-Hub Plugin that acts as a adapter for integration with CARMA-Simulation. Plugin used 
     * environment variable to be installed and enabled by default.
     */
    class CARMASimulationAdapter: public PluginClientClockAware {
    public:
        /**
         * @brief CCARMA-Simulation Infrastucture Adapter constructor.
         * @param name name of plugin.
         */
        CARMASimulationAdapter(std::string name);
        /**
         * @brief CARMA-Simulation Infrastucture Adapter destructor.
         */
        virtual ~CARMASimulationAdapter();
        int Main();
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
        void OnConfigChanged(const char *key, const char *value);
        /**
         * @brief Overrides PluginClient OnStateChange(IvpPluginState state) method.
         * @param state new state of the plugin.
         */
        void OnStateChange(IvpPluginState state);
        // Virtual method overrides END.
        
        /**
         * @brief Get Kafka Connection string from environment variable KAFKA_BROKER_ADDRESS and time sync topic name from
         * CARMA_INFRASTRUCTURE_TIME_SYNC_TOPIC and initialize a Kafka producer to forward time synchronization messages to
         * all infrastructure services.
         * @return true if initialization is successful and false if initialization fails.
         */
        bool initialize_time_producer();

        bool connect();
        
    private:

        std::string simulation_ip;
        uint simulation_registration_port;
        std::string local_ip;
        uint time_sync_port;
        uint v2x_port;
        std::shared_ptr<kafka_clients::kafka_producer_worker> time_producer;
        std::unique_ptr<CARMASimulationConnection> connection;
        std::mutex _lock;
        inline static const char *KAFKA_BROKER_ADDRESS_ENV = "KAFKA_BROKER_ADDRESS";
        inline static const char *TIME_SYNC_TOPIC_ENV = "TIME_SYNC_TOPIC";
        inline static const char *SIMULATION_IP = "SIMULATION_IP";
        inline static const char *SIMULATION_REGISTRATION_PORT_ENV = "SIMULATION_REGISTRATION_PORT";
        inline static const char *TIME_SYNC_PORT_ENV = "TIME_SYNC_PORT";
        inline static const char *V2X_PORT_ENV = "V2X_PORT";



    };
}