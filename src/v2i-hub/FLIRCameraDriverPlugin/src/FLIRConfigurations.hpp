/**
 * Copyright (C) 2025 LEIDOS.
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

#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/beast/core.hpp>
#include <iostream>
#include <regex>
#include <WGS84Point.h>


using std::string;
using std::vector;
using std::istringstream;
using std::ostringstream;
using std::regex_replace;
using std::regex;
namespace pt = boost::property_tree;
namespace beast = boost::beast; 

namespace FLIRCameraDriverPlugin{
    struct FLIRConfiguration
    {
        // IP address of the camera to connect to for data
        string socketIp;
        //Extension/port for FLIR camera subscription
        string socketPort;
        //Host HTTP header during the WebSocket handshake
        string apiSubscription;
        //Measured camera rotation from true north (in degrees) used for heading conversion
        double cameraRotation;
        //A brief name to describe FLIR field of view which defines a region to detect pedestrains in
        string sensorId;

        tmx::utils::WGS84Point cameraRefPoint; // Camera reference point in WGS84 coordinates
    };
    
    class FLIRConfigurations{
        private:
            //Vector of FLIR configurations
            std::vector<FLIRConfiguration> _configs;
            //Property tree to store the parsed JSON configurations
            pt::ptree configsTree;
            /**
             * @brief Parse a single FLIR configuration from a JSON object.
             */
            FLIRConfiguration parseFLIRConfig(const pt::ptree& config_json) const;
        public:
            /***
             * @brief Parse FLIR configurations from a JSON string.
             */
            void parseFLIRConfigs(const string& configs_str);
            /***
             * @brief Get the parsed FLIR configurations.
             */
            std::vector<FLIRConfiguration> getConfigs() const;
    };

}