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

#include <boost/property_tree/ptree.hpp>
#include <queue>
#include <queue>
#include <ctime>
#include <chrono>
#include <cmath>
#include <regex>
#include <PluginLog.h>
#include <SensorDetectedObject.h>
#include "FLIRCameraDriverException.hpp"


namespace FLIRCameraDriverPlugin
{
    /**
     * @brief Processes a single pedestrian presence tracking object from the FLIR camera.
     * @param pr The property tree containing the pedestrian presence tracking object.
     * @param timestamp The timestamp of the object in milliseconds since epoch.
     * @param cameraRotation The rotation of the camera in degrees clockwise from North (NED).
     * @param cameraViewName The name of the camera view.
     * @return A SensorDetectedObject message representing the processed object.
     * @throws FLIRCameraDriverException if fails to parse the track object.
     */
    tmx::messages::SensorDetectedObject processPedestrianPresenceTrackingObject(const boost::property_tree::ptree& pr, uint64_t timestamp, double cameraRotation, const std::string& cameraViewName);
    /**
     * @brief Processes an array of pedestrian presence tracking objects from the FLIR camera.
     * @param pr The property tree containing the pedestrian presence tracking objects.
     * @param cameraRotation The rotation of the camera in degrees clockwise from North (NED).
     * @param cameraViewName The name of the camera view.
     * @return A queue of processed SensorDetectedObject messages.
     * @throws FLIRCameraDriverException if fails to parse the tracking objects.
     */
    std::queue<tmx::messages::SensorDetectedObject> processPedestrianPresenceTrackingObjects(const boost::property_tree::ptree& pr, double cameraRotation, const std::string& cameraViewName);
    /**
     * @brief Parses a time string in the format "2022-04-20T15:25:51.001-04:00" and converts it to a epoch timestamp.
     * @param dateTimeStr The time string to parse.
     * @return The epoch timestamp in milliseconds.
     * @throws std::invalid_argument if the time string is not in the expected format.
     */
    uint64_t timeStringParser(const std::string& dateTimeStr);
    
    /**
     * @brief Processes a subscription message from the FLIR camera.
     * @param pr The property tree containing the subscription message.
     * @return True if the subscription was successful, false otherwise.
     */
    bool processSubscriptionMessage(const boost::property_tree::ptree& pr);
}