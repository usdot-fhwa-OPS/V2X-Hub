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
    tmx::messages::SensorDetectedObject processPedestrianPresenceTrackingObject(const boost::property_tree::ptree& pr, uint64_t timestamp, double cameraRotation, const std::string& sensorId, const tmx::utils::WGS84Point& sensorRefPosition)
    {
        // 2 dimensional orientation of detection measured by camera
        double angle = 0;
        double ned_heading = 0.0;
        // Convert camera reference frame angle
        // Assume camera rotation is NED (negative from true north)
        // Convert to ENU (positive from true east)
        // +90 for considering angle from east, subtract 90 for FLIR camera axis rotation.
        // Subtract camera rotation from 360 since FLIR camera rotation is in NED and ENU is
        // opposite direction        
        double convertedCameraRotation =  360 - cameraRotation;
        // GPS location of detection measure by camera
        double lat = 0.0;
        double lon = 0.0;
        // Speed of detection in m/s measures by camera
        double speed = 0.0;
        // Convert speed and angle and camera rotation into ENU velocity vector
        double velocityX = 0.0;
        double velocityY = 0.0;
        // Offset of detection in camera coordinates in meters
        double offsetX = 0.0;
        double offsetY = 0.0;
        // Corrected offset in ENU considering camera rotation
        double correctOffsetX = 0.0;
        double correctOffsetY = 0.0;
        // ID of detection
        int id = 0;

        bool heading_present;
        // Parse angle
        if (pr.get_optional<double>("angle"))
        {
            // Angle is in degrees in camera coordinates
            angle = pr.get_optional<double>("angle").get();
            
            ned_heading = angle + convertedCameraRotation;
            if (ned_heading < 0 )
            {
                ned_heading = std::fmod(ned_heading, 360.0f) + 360.0f;
            }
            else if (ned_heading > 360)
            {
                ned_heading = std::fmod(ned_heading, 360.0f);
            }
            heading_present = true;
            
        }
        else {
            ned_heading = 0.0; // If angle is not present, assuming 0 degree heading
            heading_present = false;
            FILE_LOG(tmx::utils::LogLevel::logWARNING) << "Angle not found in JSON, assuming 0 degree heading";
        }

        // Parse ID
        if (pr.get_optional<int>("iD")) 
        {
            id = pr.get_optional<int>("iD").get();
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
        if (pr.get_optional<double>("latitude"))
        {
            // Latitude is in degrees
            lat = pr.get_optional<double>("latitude").get();
        } 
        else {
            throw FLIRCameraDriverException("latitude not found in JSON");
        }

        // Parse longitude
        if (pr.get_optional<double>("longitude"))
        {
            // Longitude is in degrees
            lon = pr.get_optional<double>("longitude").get();
        }
        else {
            throw FLIRCameraDriverException("longitude not found in JSON");
        }
        
        if (pr.get_optional<double>("x"))
        {
            // Offset in meters camera coordinates
            offsetX = pr.get_optional<double>("x").get(); 
        }
        else {
            throw FLIRCameraDriverException("x not found in JSON");
        }
        if (pr.get_optional<double>("y"))
        {
            // Offset in meters camera coordinates
            offsetY = pr.get_optional<double>("y").get();
            
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
        if (pr.get_optional<double>("speed"))
        {
            // Speed is in m/s
            speed = pr.get_optional<double>("speed").get();
           
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
        obj.set_sensorId(sensorId);
        std::ostringstream proj_string;
        proj_string << "+proj=tmerc +lat_0=" << std::fixed << std::setprecision(10) << sensorRefPosition.Latitude <<" +lon_0=" << std::fixed << std::setprecision(10) << sensorRefPosition.Longitude <<" +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs +axis=enu";
        obj.set_projString(proj_string.str());
        obj.set_wgs84Position( tmx::messages::WGS84Position(lat, lon, 0.0));
        obj.set_position(tmx::messages::Position(
            roundNearZeroDoubles(correctOffsetX), 
            roundNearZeroDoubles(correctOffsetY)
            , 0.0));
        // Convert angle to orientation
        obj.set_orientation(tmx::messages::Orientation(
            roundNearZeroDoubles(std::cos(ned_heading * M_PI / 180.0)),
            roundNearZeroDoubles(std::sin(ned_heading * M_PI / 180.0))
            , 0.0));
        // FLIR Camera will occasionally report detections with no angle and non-zero speed.
        // In this case, we assume the speed is 0.0 m/s and the angle is 0 degrees since during
        // testing we have seen this to only happen when objects are stationary.
        if (speed > 0.001  && !heading_present) {
            FILE_LOG(tmx::utils::logWARNING) << "Invalid speed for detection " << id << ": Setting speed to 0.0 m/s since angle is not present";
            obj.set_velocity(tmx::messages::Velocity(0.0, 0.0, 0.0));
            obj.set_isModified(true);
        }   
        else {
            // Convert angle and speed to velocity
            obj.set_velocity(tmx::messages::Velocity(roundNearZeroDoubles(velocityX), roundNearZeroDoubles(velocityY), 0.0));
        }
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
        double varianceX =  roundNearZeroDoubles(std::pow(posXAccuracy/2, 2));
        double varianceY =  roundNearZeroDoubles(std::pow(posYAccuracy/2, 2));
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
        double varianceSpeed =  roundNearZeroDoubles(std::pow(speedAccuracy/2, 2));
        std::vector<std::vector< tmx::messages::Covariance>> velocityCov(3, std::vector<tmx::messages::Covariance>(3,tmx::messages::Covariance(0.0) ));
        velocityCov[0][0] = tmx::messages::Covariance(varianceSpeed); // x
        velocityCov[1][1] = tmx::messages::Covariance(varianceSpeed); // y
        velocityCov[2][2] = tmx::messages::Covariance(1); // z
        obj.set_velocityCovariance(velocityCov);

        return obj;
    }

    std::queue<tmx::messages::SensorDetectedObject> processPedestrianPresenceTrackingObjects(const boost::property_tree::ptree& pr, double cameraRotation, const std::string& sensorId, const tmx::utils::WGS84Point& sensorRefPosition)
    {
        std::queue<tmx::messages::SensorDetectedObject> msgQueue;
        uint64_t timestamp = timeStringParser(pr.get_child("time").data());
        for (const auto& [key, value] : pr.get_child("track"))
        {
            try {
            // Process the pedestrian presence tracking object
                tmx::messages::SensorDetectedObject obj = processPedestrianPresenceTrackingObject(value, timestamp, cameraRotation, sensorId, sensorRefPosition);
                msgQueue.push(obj);
            } 
            catch (const FLIRCameraDriverException& e) {
                FILE_LOG(tmx::utils::LogLevel::logERROR) << "Skipping track " << value.get_child("iD").data() <<"! Error processing pedestrian presence tracking object: " << e.what();
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

    double roundNearZeroDoubles(double value) {
        // Preprocess doubles to avoid scientific notation in JSON serailization
        if (std::abs(value) < 0.001) {
            return 0;
        }
        else  {
            return value;
        }

    }
}
