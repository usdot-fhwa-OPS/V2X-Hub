/**
 * Copyright (C) 2019 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
#pragma once

#include <PluginClientClockAware.h>
#include <UdpServer.h>
#include <ThreadWorker.h>
#include <mutex>
#include <SensorDetectedObject.h>

#include "MUSTSensorDetection.h"




namespace MUSTSensorDriverPlugin
{
    /**
     * @brief TODO Plugin description
     */
    class MUSTSensorDriverPlugin : public tmx::utils::PluginClientClockAware
    {
        private:
            std::mutex _configMutex;
            /**
             * @brief Status label simulation time to be displayed by each plugin.
             */
            const char* keySensorConnectionStatus = "Sensor Connection Status";

            std::unique_ptr<tmx::utils::UdpServer> mustSensorPacketReceiver;

            std::unique_ptr<tmx::utils::ThreadTimer> mustSensorPacketReceiverThread;

            std::string sensorId;

            std::string projString;
            /**
             * @brief Callback triggered on configuration updates
             */
            void UpdateConfigSettings();
            void OnConfigChanged(const char *key, const char *value) override;
            void createUdpServer(const std::string &address, unsigned int port);

            void processMUSTSensorDetection();

        public:
            /**
             * @brief Constructor
             * @param name Plugin Name
             */
            explicit MUSTSensorDriverPlugin(const std::string &name);
        protected:
            
    };

} 