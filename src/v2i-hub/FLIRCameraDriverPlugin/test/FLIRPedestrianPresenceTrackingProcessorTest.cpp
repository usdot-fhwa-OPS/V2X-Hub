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
#include <gtest/gtest.h>
#include <WGS84Point.h>
#include "FLIRPedestrianPresenceTrackingProcessor.hpp"
using std::string;


TEST(FLIRPedestrianPresenceTrackingProcessorTest, processPedestrianPresenceTrackingObject)
{
    std::string json = R"(
            {
                "angle": "263.00000000",
                "class": "Pedestrian",
                "iD": "15968646",
                "latitude": "38.95499217",
                "longitude": "-77.14920953",
                "speed": "1.41873741",
                "x": "0.09458912",
                "y": "14.80903757"
            }
        )";
    std::stringstream ss(json);
    boost::property_tree::ptree pr;
    try {
        boost::property_tree::read_json(ss, pr);
    } catch(const boost::property_tree::ptree_error &e) {
        GTEST_FAIL() << "Error converting json to p tree: " << e.what();
    }
    uint64_t timestamp = 1234567890;
    double cameraRotation = 90.0;
    string cameraViewName = "North";

    tmx::messages::SensorDetectedObject obj = FLIRCameraDriverPlugin::processPedestrianPresenceTrackingObject(pr, timestamp, cameraRotation, cameraViewName, tmx::utils::WGS84Point(38.95499217, -77.14920953, 0.0));

    EXPECT_EQ(obj.get_timestamp(), timestamp);
    EXPECT_EQ(obj.get_objectId(), 43641);
    EXPECT_EQ(obj.get_type(), "PEDESTRIAN");
    EXPECT_EQ(obj.get_confidence(), 1.0);
    EXPECT_EQ(obj.get_sensorId(), cameraViewName);
    std::string proj_string = "+proj=tmerc +lat_0=38.9549921700 +lon_0=-77.1492095300 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs +axis=enu";
    EXPECT_EQ( obj.get_projString(), proj_string);
    EXPECT_NEAR(obj.get_wgs84Position().Latitude, 38.95499217, 0.001);
    EXPECT_NEAR(obj.get_wgs84Position().Longitude, -77.14920953, 0.001);
    EXPECT_NEAR(obj.get_wgs84Position().Elevation, 0.0, 0.001);
    EXPECT_NEAR(obj.get_position().x, 14.80903757, 0.001);
    EXPECT_NEAR(obj.get_position().y, -0.09458912, 0.001);
    EXPECT_NEAR(obj.get_position().z, 0.0, 0.001);
    EXPECT_NEAR(obj.get_velocity().x, -1.40816, 0.001);
    EXPECT_NEAR(obj.get_velocity().y, 0.1729, 0.001);
    EXPECT_NEAR(obj.get_velocity().z, 0.0, 0.001);
    EXPECT_NEAR(obj.get_orientation().x, -0.992546, 0.001);
    EXPECT_NEAR(obj.get_orientation().y, 0.121869, 0.001);
    EXPECT_NEAR(obj.get_orientation().z, 0.0, 0.001);
    EXPECT_NEAR(obj.get_size().width, 0.6, 0.001);
    EXPECT_NEAR(obj.get_size().length, 0.5, 0.001);
    EXPECT_NEAR(obj.get_size().height, 0.0, 0.001);
}

TEST(FLIRPedestrianPresenceTrackingProcessorTest, processPedestrianPresenceTrackingObjectNoAngle)
{
    std::string json = R"(
            {
            "class": "Pedestrian",
            "iD": "4432844",
            "latitude": "38.95500282",
            "longitude": "-77.14917125",
            "speed": "0.00000000",
            "x": "1.85333057",
            "y": "18.45879336"
            }
        )";
    std::stringstream ss(json);
    boost::property_tree::ptree pr;
    try {
        boost::property_tree::read_json(ss, pr);
    } catch(const boost::property_tree::ptree_error &e) {
        GTEST_FAIL() << "Error converting json to p tree: " << e.what();
    }
    uint64_t timestamp = 1234567890;
    double cameraRotation = 355.0;
    string cameraViewName = "FLIRSensor1";

    tmx::messages::SensorDetectedObject obj = FLIRCameraDriverPlugin::processPedestrianPresenceTrackingObject(pr, timestamp, cameraRotation, cameraViewName, tmx::utils::WGS84Point(38.95499217, -77.14920953, 0.0));

    EXPECT_EQ(obj.get_timestamp(), timestamp);
    EXPECT_EQ(obj.get_objectId(), 41999);
    EXPECT_EQ(obj.get_type(), "PEDESTRIAN");
    EXPECT_EQ(obj.get_confidence(), 1.0);
    EXPECT_EQ(obj.get_sensorId(), cameraViewName);
    std::string proj_string = "+proj=tmerc +lat_0=38.9549921700 +lon_0=-77.1492095300 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs +axis=enu";
    EXPECT_EQ( obj.get_projString(), proj_string);
    EXPECT_NEAR(obj.get_wgs84Position().Latitude, 38.95500282, 0.001);
    EXPECT_NEAR(obj.get_wgs84Position().Longitude, -77.14917125, 0.001);
    EXPECT_NEAR(obj.get_wgs84Position().Elevation, 0.0, 0.001);
    EXPECT_NEAR(obj.get_position().x, 0.23748824212901187, 0.001);
    EXPECT_NEAR(obj.get_position().y, 18.550080480788409, 0.001);
    EXPECT_NEAR(obj.get_position().z, 0.0, 0.001);
    EXPECT_NEAR(obj.get_velocity().x, 0.0, 0.001);
    EXPECT_NEAR(obj.get_velocity().y, 0.0, 0.001);
    EXPECT_NEAR(obj.get_velocity().z, 0.0, 0.001);
    EXPECT_NEAR(obj.get_orientation().x, 1.0, 0.001);
    EXPECT_NEAR(obj.get_orientation().y, 0.0, 0.001);
    EXPECT_NEAR(obj.get_orientation().z, 0.0, 0.001);
    EXPECT_NEAR(obj.get_size().width, 0.6, 0.001);
    EXPECT_NEAR(obj.get_size().length, 0.5, 0.001);
    EXPECT_NEAR(obj.get_size().height, 0.0, 0.001);
}

TEST(FLIRPedestrianPresenceTrackingProcessorTest, processPedestrianPresenceTrackingObjectNoAngleAndNonZeroSpeed)
{
    std::string json = R"(
            {
            "class": "Pedestrian",
            "iD": "4432844",
            "latitude": "38.95500282",
            "longitude": "-77.14917125",
            "speed": "1.00000000",
            "x": "1.85333057",
            "y": "18.45879336"
            }
        )";
    std::stringstream ss(json);
    boost::property_tree::ptree pr;
    try {
        boost::property_tree::read_json(ss, pr);
    } catch(const boost::property_tree::ptree_error &e) {
        GTEST_FAIL() << "Error converting json to p tree: " << e.what();
    }
    uint64_t timestamp = 1234567890;
    double cameraRotation = 355.0;
    string cameraViewName = "FLIRSensor1";
    // Confirm that when angle is not provided and speed is non-zero, the velocity is set to zero and heading is set to 0 degrees or (1.0, 0.0, 0.0) orientation.
    auto detection = FLIRCameraDriverPlugin::processPedestrianPresenceTrackingObject(pr, timestamp, cameraRotation, cameraViewName, tmx::utils::WGS84Point(38.95499217, -77.14920953, 0.0));
    EXPECT_TRUE(detection.get_isModified());
    EXPECT_EQ(detection.get_velocity().x, 0.0);
    EXPECT_EQ(detection.get_velocity().y, 0.0);
    EXPECT_EQ(detection.get_velocity().z, 0.0);
    EXPECT_EQ(detection.get_orientation().x, 1.0);
    EXPECT_EQ(detection.get_orientation().y, 0.0);
    EXPECT_EQ(detection.get_orientation().z, 0.0);
}

TEST(FLIRPedestrianPresenceTrackingProcessorTest, testDoublePreprocessing) {
    std::string json = R"(
        {
            "angle": "263.00000000",
            "class": "Pedestrian",
            "iD": "15968646",
            "latitude": "38.95499217",
            "longitude": "-77.14920953",
            "speed": "0.0000141873741",
            "x": "0.000009458912",
            "y": "-0.0000001480903757"
        }
    )";
    std::stringstream ss(json);
    boost::property_tree::ptree pr;
    try {
        boost::property_tree::read_json(ss, pr);
    } catch(const boost::property_tree::ptree_error &e) {
        GTEST_FAIL() << "Error converting json to p tree: " << e.what();
    }
    uint64_t timestamp = 1234567890;
    double cameraRotation = 90.0;
    string cameraViewName = "North";

    tmx::messages::SensorDetectedObject obj = FLIRCameraDriverPlugin::processPedestrianPresenceTrackingObject(pr, timestamp, cameraRotation, cameraViewName, tmx::utils::WGS84Point(38.95499217, -77.14920953, 0.0));

    EXPECT_EQ(obj.get_timestamp(), timestamp);
    EXPECT_EQ(obj.get_objectId(), 43641);
    EXPECT_EQ(obj.get_type(), "PEDESTRIAN");
    EXPECT_EQ(obj.get_confidence(), 1.0);
    EXPECT_EQ(obj.get_sensorId(), cameraViewName);
    std::string proj_string = "+proj=tmerc +lat_0=38.9549921700 +lon_0=-77.1492095300 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs +axis=enu";
    EXPECT_EQ( obj.get_projString(), proj_string);
    EXPECT_NEAR(obj.get_wgs84Position().Latitude, 38.95499217, 0.001);
    EXPECT_NEAR(obj.get_wgs84Position().Longitude, -77.14920953, 0.001);
    EXPECT_NEAR(obj.get_wgs84Position().Elevation, 0.0, 0.001);
    EXPECT_EQ(obj.get_position().x, 0.0);
    EXPECT_EQ(obj.get_position().y, 0.0);
    EXPECT_EQ(obj.get_position().z, 0.0);
    EXPECT_EQ(obj.get_velocity().x, 0.0);
    EXPECT_EQ(obj.get_velocity().y, 0.0);
    EXPECT_NEAR(obj.get_velocity().z, 0.0, 0.001);
    EXPECT_NEAR(obj.get_orientation().x, -0.992546, 0.001);
    EXPECT_NEAR(obj.get_orientation().y, 0.121869, 0.001);
    EXPECT_NEAR(obj.get_orientation().z, 0.0, 0.001);
    EXPECT_NEAR(obj.get_size().width, 0.6, 0.001);
    EXPECT_NEAR(obj.get_size().length, 0.5, 0.001);
    EXPECT_NEAR(obj.get_size().height, 0.0, 0.001);
}