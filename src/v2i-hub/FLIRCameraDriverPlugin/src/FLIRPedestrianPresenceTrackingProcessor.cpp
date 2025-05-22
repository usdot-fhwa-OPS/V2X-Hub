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
#include "FLIRPedestrianPresenceTrackingProcessor.hpp"

namespace FLIRCameraDriverPlugin
{
    tmx::messages::SensorDetectedObject processPedestrianPresenceTrackingObject(const boost::property_tree::ptree& pr, uint64_t timestamp, double cameraRotation, const std::string& cameraViewName)
    {
        double angle = 0;
        double ned_heading = 0.0;
        double convertedCameraRotation =  360 - cameraRotation;

        double lat = 0.0;
        double lon = 0.0;
        double speed = 0.0;
        double velocityX = 0.0;
        double velocityY = 0.0;
        double offsetX = 0.0;
        double offsetY = 0.0;
        double correctOffsetX = 0.0;
        double correctOffsetY = 0.0;
        int id = 0;
        // Parse angle
        if (!pr.get_child("angle").data().empty())
        {
            // Angle is in degrees in camera coordinates
            angle = std::stod(pr.get_child("angle").data());
            // Convert camera reference frame angle
            // Assume camera rotation is NED (negative from true north)
            // Convert to ENU (positive from true east)
            // +90 for considering angle from east, subtract 90 for FLIR camera axis rotation.
            // Subtract camera rotation from 360 since FLIR camera rotation is in NED and ENU is
            // opposite direction
            ned_heading = angle + convertedCameraRotation;
            if (ned_heading < 0 )
            {
                ned_heading = std::fmod(ned_heading, 360.0f) + 360.0f;
            }
            else if (ned_heading > 360)
            {
                ned_heading = std::fmod(ned_heading, 360.0f);
            }
            
        }
        else {
            throw FLIRCameraDriverException("angle not found in JSON");
        }

        // Parse ID
        if (!pr.get_child("iD").data().empty()) 
        {
            id = std::stoi(pr.get_child("iD").data());
            if (id > 65535)
            {
                auto old_id = id;
                id = id%65535;
                FILE_LOG(tmx::utils::LogLevel::logWARNING) << "ID " << old_id << " out of range. Assigning new ID " << id;
            }
        }
        else {
            throw FLIRCameraDriverException("iD not found in JSON");
        }

        // Parse latitude
        if (!pr.get_child("latitude").data().empty())
        {
            // Latitude is in degrees
            lat = std::stod(pr.get_child("latitude").data());
        } 
        else {
            throw FLIRCameraDriverException("latitude not found in JSON");
        }

        // Parse longitude
        if (!pr.get_child("longitude").data().empty())
        {
            // Longitude is in degrees
            lon = std::stod(pr.get_child("longitude").data());
        }
        else {
            throw FLIRCameraDriverException("longitude not found in JSON");
        }
        
        if (!pr.get_child("x").data().empty())
        {
            // Offset in meters camera coordinates
            offsetX = std::stod(pr.get_child("x").data());
            
            
        }
        else {
            throw FLIRCameraDriverException("x not found in JSON");
        }
        if (!pr.get_child("y").data().empty())
        {
            // Offset in meters camera coordinates
            offsetY = std::stod(pr.get_child("y").data());
            
        }
        else {
            throw FLIRCameraDriverException("y not found in JSON");
        }
        // Calculate ENU y offset using converted camera rotation
        correctOffsetY = offsetX * std::sin( convertedCameraRotation* M_PI / 180.0) +
            offsetY * std::cos(convertedCameraRotation * M_PI / 180.0);
        correctOffsetX = offsetX * std::cos( convertedCameraRotation* M_PI / 180.0) -
            offsetY * std::sin(convertedCameraRotation * M_PI / 180.0);

        // Parse speed
        if (!pr.get_child("speed").data().empty())
        {
            // Speed is in m/s
            speed = std::stod(pr.get_child("speed").data());
            // Get velocity from speed and angle
            velocityX = speed * std::cos(ned_heading * M_PI / 180.0);
            velocityY = speed * std::sin(ned_heading * M_PI / 180.0);
        }
        else {
            throw FLIRCameraDriverException("speed not found in JSON");
        }                  

        tmx::messages::SensorDetectedObject obj;
        obj.set_timestamp(timestamp);
        obj.set_objectId(id);
        obj.set_type("PEDESTRIAN");
        obj.set_confidence(1.0);
        obj.set_sensorId(cameraViewName);
        obj.set_projString("+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs +axis=enu");
        obj.set_wgs84_position( tmx::messages::WGS84Position(lat, lon, 0.0));
        obj.set_position(tmx::messages::Position(correctOffsetX, correctOffsetY, 0.0));
        // Convert angle to orientation
        obj.set_orientation(tmx::messages::Orientation(std::cos(ned_heading * M_PI / 180.0), std::sin(ned_heading * M_PI / 180.0), 0.0));
        // Convert angle and speed to velocity
        obj.set_velocity(tmx::messages::Velocity(velocityX, velocityY, 0.0));
        // Average pedestrian size standing is 0.5m x 0.6m (https://www.fhwa.dot.gov/publications/research/safety/pedbike/05085/chapt8.cfm)
        obj.set_size(tmx::messages::Size(0.5, 0.6, 0.0));
        // FLIR Sensor position accuracy is :
        // Distance , Accuracy
        // 10m , +/- 0.010m
        // 20m , +/- 0.023m
        // 40m , +/- 0.045m
        // Line of best fit is 0.0115 * distance - 0.015
        double posXAccuracy = std::abs(0.0115* std::abs(correctOffsetX) - 0.015);
        double posYAccuracy = std::abs(0.0115* std::abs(correctOffsetY) - 0.015);
        // Convert Accuracy to variance
        // Variance is the square of the standard deviation
        // Considering normal distribution +/- 2 std deviations is 95% of the data
        double varianceX =  std::pow(posXAccuracy/2, 2);
        double varianceY =  std::pow(posYAccuracy/2, 2);
        std::vector<std::vector< tmx::messages::Covariance>> positionCov(3, std::vector<tmx::messages::Covariance>(3,tmx::messages::Covariance(0.0) ));
        positionCov[0][0] = tmx::messages::Covariance(varianceX); // x
        positionCov[1][1] = tmx::messages::Covariance(varianceY); // y
        positionCov[2][2] = tmx::messages::Covariance(0); // z
        obj.set_positionCovariance(positionCov);
        // FLIR Documentation says speed accuracy is +/- 10% of the speed
        double speedAccuracy = 0.1 * speed;
        // Convert Accuracy to variance
        // Variance is the square of the standard deviation
        // Considering normal distribution +/- 2 std deviations is 95% of the data
        double varianceSpeed =  std::pow(speedAccuracy/2, 2);
        std::vector<std::vector< tmx::messages::Covariance>> velocityCov(3, std::vector<tmx::messages::Covariance>(3,tmx::messages::Covariance(0.0) ));
        velocityCov[0][0] = tmx::messages::Covariance(varianceSpeed); // x
        velocityCov[1][1] = tmx::messages::Covariance(varianceSpeed); // y
        velocityCov[2][2] = tmx::messages::Covariance(1); // z
        obj.set_velocityCovariance(velocityCov);

        return obj;
    }

    std::queue<tmx::messages::SensorDetectedObject> processPedestrianPresenceTrackingObjects(const boost::property_tree::ptree& pr, double cameraRotation, const std::string& cameraViewName)
    {
        std::queue<tmx::messages::SensorDetectedObject> msgQueue;
        uint64_t timestamp = timeStringParser(pr.get_child("time").data());
        for (const auto& [key, value] : pr.get_child("track"))
        {
            try {
            // Process the pedestrian presence tracking object
                tmx::messages::SensorDetectedObject obj = processPedestrianPresenceTrackingObject(value, timestamp, cameraRotation, cameraViewName);
                msgQueue.push(obj);
            } 
            catch (const FLIRCameraDriverException& e) {
                FILE_LOG(tmx::utils::LogLevel::logERROR) << "Skipping track! Error processing pedestrian presence tracking object: " << e.what();
            }
            
        }
        return msgQueue;
    }
    uint64_t timeStringParser(const std::string& dateTimeStr)
    {
        //"time": "2022-04-20T15:25:51.001-04:00"
        std::regex re(R"((\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})\.(\d+)([+-])(\d{2}):(\d{2}))");
        std::smatch match;

        if (!std::regex_match(dateTimeStr, match, re)) {
            throw std::invalid_argument("Invalid datetime format");
        }

        std::tm t = {};
        t.tm_year = std::stoi(match[1]) - 1900;
        t.tm_mon  = std::stoi(match[2]) - 1;
        t.tm_mday = std::stoi(match[3]);
        t.tm_hour = std::stoi(match[4]);
        t.tm_min  = std::stoi(match[5]);
        t.tm_sec  = std::stoi(match[6]);
        // Daylight Saving Time flag. The value is positive if DST is in effect, zero if not and negative if no information is available.
        // Setting to -1 will allow mktime to determine if DST is in effect.
        t.tm_isdst = -1;
        int milliseconds = std::stoi(match[7]);
        std::string offset_sign = match[8];
        int offset_hours = std::stoi(match[9]);
        int offset_minutes = std::stoi(match[10]);

        // Calculate the UTC offset in seconds
        int offset_seconds = (offset_hours * 3600) + (offset_minutes * 60);
        if (offset_sign == "-") {
            offset_seconds *= -1;
        }
        // Interpret the time as UTC to avoid environment-dependent behavior. Using std::mktime
        // will consider local time zone of device, which may change behavior.
        std::time_t local_time_t = timegm(&t);
        // Account for timeoffset
        std::time_t utc_time_t = local_time_t - offset_seconds;
        // Convert to time_point
        auto utc_time_point = std::chrono::system_clock::from_time_t(utc_time_t);
        // Add milliseconds to the time_point
        utc_time_point = utc_time_point + std::chrono::milliseconds(milliseconds);

        // Convert to milliseconds since epoch
        return std::chrono::duration_cast<std::chrono::milliseconds>(utc_time_point.time_since_epoch()).count();
    }

    bool processSubscriptionMessage(const boost::property_tree::ptree& pr)
    {
        std::string subscrStatus = pr.get_child("subscription").get_child("returnValue").get_value<std::string>();

        if (subscrStatus == "OK")
        {
            return true;
        }
        FILE_LOG(tmx::utils::LogLevel::logWARNING) << "Ped presence data subscription status: " << subscrStatus;
        return false;
    }
}