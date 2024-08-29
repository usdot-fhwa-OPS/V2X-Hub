#include <gtest/gtest.h>
#include <MUSTSensorDetection.h>
#include <chrono>

using namespace MUSTSensorDriverPlugin;

TEST(TestMUSTSensorDetection, fromStringToDetectionSize)
{
    EXPECT_EQ(DetectionSize::SMALL, fromStringToDetectionSize("small"));
    EXPECT_EQ(DetectionSize::MEDIUM, fromStringToDetectionSize("medium"));
    EXPECT_EQ(DetectionSize::LARGE, fromStringToDetectionSize("large"));
    EXPECT_EQ(DetectionSize::NA, fromStringToDetectionSize("not_a_size"));

}

TEST(TestMUSTSensorDetection, fromStringToDetectionClassification)
{
    EXPECT_EQ(DetectionClassification::SEDAN, fromStringToDetectionClassification("sedan"));
    EXPECT_EQ(DetectionClassification::VAN, fromStringToDetectionClassification("van"));
    EXPECT_EQ(DetectionClassification::TRUCK, fromStringToDetectionClassification("truck"));
    EXPECT_EQ(DetectionClassification::BUS, fromStringToDetectionClassification("bus"));
    EXPECT_EQ(DetectionClassification::PICKUP_TRUCK, fromStringToDetectionClassification("pickup truck"));
    EXPECT_EQ(DetectionClassification::PEDESTRIAN, fromStringToDetectionClassification("pedestrian"));
    EXPECT_EQ(DetectionClassification::NA, fromStringToDetectionClassification("not_a_classification"));

}

TEST(TestMUSTSensorDetection, csvToDetection ){
    std::string valid_csv_data = "truck,13.3,22.4,30.5,35.8,large,95.1,1,1714374738";
    auto detection = csvToDetection(valid_csv_data);
    EXPECT_EQ(detection.cl, DetectionClassification::TRUCK);
    EXPECT_DOUBLE_EQ(detection.position_x, 13.3);
    EXPECT_DOUBLE_EQ(detection.position_y, 22.4);
    EXPECT_DOUBLE_EQ(detection.heading, 30.5);
    EXPECT_DOUBLE_EQ(detection.speed, 35.8);
    EXPECT_EQ(detection.size, DetectionSize::LARGE);
    EXPECT_DOUBLE_EQ(detection.confidence, 95.1);
    EXPECT_EQ(detection.trackID, 1);
    EXPECT_EQ(detection.timestamp, 1714374738);
}

TEST(TestMUSTSensorDetection, csvToDectectionInvalidCSV ){
    std::string invalid_csv_data = "truck,13.3,22.4,30.5,35.8,large,95.1,1,1714374738,20";
    EXPECT_THROW(csvToDetection(invalid_csv_data), tmx::TmxException);
}

TEST(TestMUSTSensorDetection, csvToDectectionEmptyString ){
    std::string empty_string = "";
    EXPECT_THROW(csvToDetection(empty_string), tmx::TmxException);
}

TEST(TestMUSTSensorDetection, mustDetectionToSensorDetectedObject ) {
    using namespace std::chrono;

    MUSTSensorDetection detection;
    detection.cl = DetectionClassification::SEDAN;
    detection.confidence = 95.5;
    detection.heading = 330;
    detection.position_x = 10.5;
    detection.position_y = -20.3;
    detection.size = DetectionSize::SMALL;
    detection.timestamp = 1719506355.4;
    detection.trackID = 324;
    detection.speed = 5;
    // 0.0625 variance corresponds to 0.25 std. Assuming Normal distribution and a 95% confidence interval corresponds to +/- 0.5m or m/s respectively. 
    auto sensorDetectedObject = mustDetectionToSensorDetectedObject(detection, "MUSTSensor1", "PROJ String", 0.0625, 0.0625);

    EXPECT_EQ(detection.trackID, sensorDetectedObject.get_objectId());
    EXPECT_DOUBLE_EQ(detection.confidence/100.0, sensorDetectedObject.get_confidence());
    EXPECT_DOUBLE_EQ(detection.position_x, sensorDetectedObject.get_position().x);
    EXPECT_DOUBLE_EQ(detection.position_y, sensorDetectedObject.get_position().y);
    EXPECT_NEAR(4.33, sensorDetectedObject.get_velocity().y, 0.001);
    EXPECT_NEAR(2.5, sensorDetectedObject.get_velocity().x, 0.001);
    EXPECT_STRCASEEQ("SEDAN", sensorDetectedObject.get_type().c_str());
    EXPECT_EQ(1719506355400, sensorDetectedObject.get_timestamp());
    EXPECT_EQ("MUSTSensor1", sensorDetectedObject.get_sensorId());
    EXPECT_EQ("PROJ String", sensorDetectedObject.get_projString());
    EXPECT_DOUBLE_EQ( 0.0625, sensorDetectedObject.get_positionCovariance()[0][0].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_positionCovariance()[0][1].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_positionCovariance()[0][2].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_positionCovariance()[1][0].value);
    EXPECT_DOUBLE_EQ( 0.0625, sensorDetectedObject.get_positionCovariance()[1][1].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_positionCovariance()[1][2].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_positionCovariance()[2][0].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_positionCovariance()[2][1].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_positionCovariance()[2][2].value);

    EXPECT_DOUBLE_EQ( 0.0625, sensorDetectedObject.get_velocityCovariance()[0][0].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_velocityCovariance()[0][1].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_velocityCovariance()[0][2].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_velocityCovariance()[1][0].value);
    EXPECT_DOUBLE_EQ( 0.0625, sensorDetectedObject.get_velocityCovariance()[1][1].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_velocityCovariance()[1][2].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_velocityCovariance()[2][0].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_velocityCovariance()[2][1].value);
    EXPECT_DOUBLE_EQ( 0.0, sensorDetectedObject.get_velocityCovariance()[2][2].value);

}

TEST(TestMUSTSensorDetection, detectionClassificationToSensorDetectedObjectType ) {
    EXPECT_STRCASEEQ("SEDAN", detectionClassificationToSensorDetectedObjectType(DetectionClassification::SEDAN).c_str());
    EXPECT_STRCASEEQ("VAN", detectionClassificationToSensorDetectedObjectType(DetectionClassification::VAN).c_str());
    EXPECT_STRCASEEQ("TRUCK", detectionClassificationToSensorDetectedObjectType(DetectionClassification::TRUCK).c_str());
    EXPECT_STRCASEEQ("BUS", detectionClassificationToSensorDetectedObjectType(DetectionClassification::BUS).c_str());
    EXPECT_STRCASEEQ("PICKUP TRUCK", detectionClassificationToSensorDetectedObjectType(DetectionClassification::PICKUP_TRUCK).c_str());
    EXPECT_STRCASEEQ("PEDESTRIAN", detectionClassificationToSensorDetectedObjectType(DetectionClassification::PEDESTRIAN).c_str());
    EXPECT_THROW(detectionClassificationToSensorDetectedObjectType(DetectionClassification::NA).c_str(), std::runtime_error);

}

TEST(TestMUSTSensorDetection, headingSpeedToVelocity ) {
    auto velocity = headingSpeedToVelocity(30, 5);
    EXPECT_NEAR(4.33, velocity.y, 0.001);
    EXPECT_NEAR(-2.5, velocity.x, 0.001);
}