#include <gtest/gtest.h>
#include <MUSTSensorDetection.h>

using namespace MUSTSensorDriverPlugin;

TEST(TestMUSTSensorDetection, fromStringToDetectionSize)
{
    ASSERT_EQ(DetectionSize::SMALL, fromStringToDetectionSize("small"));
    ASSERT_EQ(DetectionSize::MEDIUM, fromStringToDetectionSize("medium"));
    ASSERT_EQ(DetectionSize::LARGE, fromStringToDetectionSize("large"));
    ASSERT_EQ(DetectionSize::NA, fromStringToDetectionSize("not_a_size"));

}

TEST(TestMUSTSensorDetection, fromStringToDetectionClassification)
{
    ASSERT_EQ(DetectionClassification::SEDAN, fromStringToDetectionClassification("sedan"));
    ASSERT_EQ(DetectionClassification::VAN, fromStringToDetectionClassification("van"));
    ASSERT_EQ(DetectionClassification::TRUCK, fromStringToDetectionClassification("truck"));
    ASSERT_EQ(DetectionClassification::NA, fromStringToDetectionClassification("not_a_classification"));

}