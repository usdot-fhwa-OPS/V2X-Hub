#pragma once

#include <boost/property_tree/ptree.hpp>
#include <queue>
#include <queue>
#include <ctime>
#include <cmath>
#include <regex>
#include <PluginLog.h>
#include <SensorDetectedObject.h>


namespace FLIRCameraDriverPlugin
{
    tmx::messages::SensorDetectedObject processPedestrianPresenceTrackingObject(const boost::property_tree::ptree& pr, uint64_t timestamp, double cameraRotation, const std::string& cameraViewName);

    std::queue<tmx::messages::SensorDetectedObject> processPedestrianPresenceTrackingObjects(const boost::property_tree::ptree& pr, double cameraRotation, const std::string& cameraViewName);

    uint64_t timeStringParser(const std::string& dateTimeStr);
    
    bool processSubscriptionMessage(const boost::property_tree::ptree& pr);
}