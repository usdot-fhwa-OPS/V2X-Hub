#include <gtest/gtest.h>
#include <rsu/RSUSpec.h>
using namespace tmx::utils::rsu;
TEST(testRSUSpec, testRsuSpecToString) {
    EXPECT_EQ("NTCIP1218", rsuSpecToString(RSU_SPEC::NTCIP_1218));
    EXPECT_EQ("RSU4.1", rsuSpecToString(RSU_SPEC::RSU_4_1));

}

TEST(testRSUSpec, testStringToRSUSpec) {
    EXPECT_EQ(RSU_SPEC::NTCIP_1218, stringToRSUSpec("NTCIP1218"));
    EXPECT_EQ(RSU_SPEC::RSU_4_1, stringToRSUSpec("RSU4.1"));
    EXPECT_THROW(stringToRSUSpec("invalidSpec"), tmx::TmxException);

}