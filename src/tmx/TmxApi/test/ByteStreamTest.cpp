#include <gtest/gtest.h>
#include <tmx/messages/byte_stream.hpp>
#include <string>

using namespace tmx;
TEST(ByteStreamTest, TestHexToBytes) {
    tmx::byte_stream bytes = byte_stream_decode("001f526011c35d000000000023667bac0407299b9ef9e7a9b9408230dfffe4386ba00078005a53373df3cf5372810461b90ffff53373df3cf53728104618129800010704a04c7d7976ca3501872e1bb66ad19b2620");
    std::string expectedBytes="031829617195930000035102123172474115515824923116918564130482232552285610716001200908355612432078311412949718515255245511152236024555401670241815201741607612512111820253113546271821062091553832";
    std::stringstream bytesToIntResult;
    for(const auto &byte: bytes){
        bytesToIntResult << (int)byte;
    }
    EXPECT_EQ(bytesToIntResult.str(), expectedBytes);
}

TEST(ByteStreamTest, TestBytesToHex) {
    std::string hex =  "001f526011c35d000000000023667bac0407299b9ef9e7a9b9408230dfffe4386ba00078005a53373df3cf5372810461b90ffff53373df3cf53728104618129800010704a04c7d7976ca3501872e1bb66ad19b2620";
    tmx::byte_stream bytes = byte_stream_decode(hex);
    std::string hexResult = byte_stream_encode(bytes);
    EXPECT_EQ(hexResult, hex);
}