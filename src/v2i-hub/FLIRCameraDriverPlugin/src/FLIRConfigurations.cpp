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
#include "FLIRConfigurations.hpp"

namespace FLIRCameraDriverPlugin
{
    FLIRConfiguration FLIRConfigurations::parseFLIRConfig(const pt::ptree& configJson) const{
        FLIRConfiguration config;
        config.socketIp = configJson.get<string>("WebSocketHost");
        config.socketPort = configJson.get<string>("WebSocketPort");
        config.apiSubscription = configJson.get<string>("HostString");
        config.cameraRotation = configJson.get<float>("CameraRotation");
        config.sensorId = configJson.get<string>("SensorId");
        return config;
    }

    void FLIRConfigurations::parseFLIRConfigs(const string& configsStr){        
        istringstream iss(configsStr);
        configsTree.clear();
        pt::read_json(iss, configsTree);
        for(const auto& [key, value]: configsTree){
            auto config = parseFLIRConfig(value);
            _configs.push_back(config);
        }     
    }

    std::vector<FLIRConfiguration> FLIRConfigurations::getConfigs() const{
        return _configs;
    }

}
