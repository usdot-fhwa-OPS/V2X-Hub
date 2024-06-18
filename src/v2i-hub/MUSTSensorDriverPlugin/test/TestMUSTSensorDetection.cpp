#include <gtest/gtest.h>
#include <MUSTSensorDetection.h>

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
    EXPECT_EQ(DetectionClassification::NA, fromStringToDetectionClassification("not_a_classification"));

}

TEST(TestMUSTSensorDetection, csvToDectection ){
    std::string valid_csv_data = "truck,13.3,22.4,30.5,35.8,large,95.1,1,1714374738";
    auto detection = csvToDectection(valid_csv_data);
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