#include <gtest/gtest.h>
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

    tmx::messages::SensorDetectedObject obj = FLIRCameraDriverPlugin::processPedestrianPresenceTrackingObject(pr, timestamp, cameraRotation, cameraViewName);

    EXPECT_EQ(obj.get_timestamp(), timestamp);
    EXPECT_EQ(obj.get_objectId(), 43641);
    EXPECT_EQ(obj.get_type(), "PEDESTRIAN");
    EXPECT_EQ(obj.get_confidence(), 1.0);
    EXPECT_EQ(obj.get_sensorId(), cameraViewName);
    EXPECT_NEAR(obj.get_wgs84_position().Latitude, 38.95499217, 0.001);
    EXPECT_NEAR(obj.get_wgs84_position().Longitude, -77.14920953, 0.001);
    EXPECT_NEAR(obj.get_wgs84_position().Elevation, 0.0, 0.001);
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

TEST(FLIRPedestrianPresenceTrackingProcessorTest, processPedestrianPresenceTrackingObjects)
{
    std::string json = R"(
                {
            "dataNumber": "199262",
            "messageType": "Data",
            "time": "2025-05-20T13:47:35.092-04:00",
            "track": [
                {
                "angle": "255.00000000",
                "class": "Pedestrian",
                "iD": "2910604",
                "latitude": "38.95504354",
                "longitude": "-77.14934177",
                "speed": "1.12053359",
                "x": "-2.66856720",
                "y": "19.92332193"
                },
                {
                "angle": "246.00000000",
                "class": "Pedestrian",
                "iD": "2910927",
                "latitude": "38.95503376",
                "longitude": "-77.14937300",
                "speed": "4.96141529",
                "x": "-3.02072971",
                "y": "22.81371546"
                }
            ],
            "type": "PedestrianPresenceTracking"
            }
        )";
    std::stringstream ss(json);
    boost::property_tree::ptree pr;
    try {
        boost::property_tree::read_json(ss, pr);
    } catch(const boost::property_tree::ptree_error &e) {
        GTEST_FAIL() << "Error converting json to p tree: " << e.what();
    }
    double cameraRotation = 90.0;
    string cameraViewName = "North";

    std::queue<tmx::messages::SensorDetectedObject> msgQueue = FLIRCameraDriverPlugin::processPedestrianPresenceTrackingObjects(pr, cameraRotation, cameraViewName);

    EXPECT_EQ(msgQueue.size(), 2);
    tmx::messages::SensorDetectedObject obj = msgQueue.front();
 
    EXPECT_NEAR(obj.get_timestamp(), 1747763255092, 1);
}