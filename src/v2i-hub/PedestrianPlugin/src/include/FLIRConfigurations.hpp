#pragma once

#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/beast/core.hpp>
#include <iostream>


using std::string;
using std::vector;
using std::istringstream;
using std::ostringstream;
namespace pt = boost::property_tree;
namespace beast = boost::beast; 

namespace PedestrianPlugin{
    struct FLIRConfiguration
    {
        // IP address of the camera to connect to for data
        string socketIp;
        //Extension/port for FLIR camera subscription
        string socketPort;
        //Host HTTP header during the WebSocket handshake
        string apiSubscription;
        //Measured camera rotation from true north (in degrees) used for heading conversion
        float FLIRCameraRotation;
        //A brief name to describe FLIR field of view which defines a region to detect pedestrains in
        string FLIRCameraViewName;
    };
    
    class FLIRConfigurations{
        private:
            std::vector<FLIRConfiguration> _configs;
            pt::ptree configsTree;
            FLIRConfiguration parseFLIRConfig(const pt::ptree& config_json);
        public:
            void parseFLIRConfigs(const string& configs_str);
            std::vector<FLIRConfiguration> getConfigs();
            string toString();
    };

}