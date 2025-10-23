#include <SpatMode.h>
#include <gtest/gtest.h>

using namespace SpatPlugin;

TEST(TestSpatMode, TestSpatModeToString) {
    EXPECT_EQ("UNKNOWN", spat_mode_to_string(SPAT_MODE::UNKNOWN));
    EXPECT_EQ("TSCBM", spat_mode_to_string(SPAT_MODE::TSCBM));
    EXPECT_EQ("SPAT", spat_mode_to_string(SPAT_MODE::SPAT));
}

TEST(TestSpatMode, TestStringToSpatMode) {
    EXPECT_EQ(SPAT_MODE::SPAT, spat_mode_from_string("SPAT"));
    EXPECT_EQ(SPAT_MODE::TSCBM, spat_mode_from_string("TSCBM"));
    EXPECT_EQ(SPAT_MODE::UNKNOWN, spat_mode_from_string("UNKOWN"));
    EXPECT_EQ(SPAT_MODE::UNKNOWN, spat_mode_from_string("INVALID"));

}