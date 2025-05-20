#include "FLIRConfigurations.hpp"

namespace FLIRCameraDriverPlugin
{
    FLIRConfiguration FLIRConfigurations::parseFLIRConfig(const pt::ptree& configJson) const{
        FLIRConfiguration config;
        config.socketIp = configJson.get<string>("WebSocketHost");
        config.socketPort = configJson.get<string>("WebSocketPort");
        config.apiSubscription = configJson.get<string>("HostString");
        config.FLIRCameraRotation = configJson.get<float>("FLIRCameraRotation");
        config.FLIRCameraViewName = configJson.get<string>("FLIRCameraViewName");
        return config;
    }

    void FLIRConfigurations::parseFLIRConfigs(const string& configsStr){        
        istringstream iss(configsStr);
        configsTree.clear();
        pt::read_json(iss, configsTree);
        for(const auto& treeElement: configsTree){
            auto config = parseFLIRConfig(treeElement.second);
            _configs.push_back(config);
        }     
    }

    std::vector<FLIRConfiguration> FLIRConfigurations::getConfigs() const{
        return _configs;
    }

}
