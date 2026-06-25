#include <gtest/gtest.h>
#include "RawSpdu.h"
#include <vector>
#include <iostream>

using namespace std;
using namespace tmx;
using namespace tmx::messages;

// Function to convert hex string to byte array
tmx::byte_stream hexStringToByteArray(const std::string& hex) {
    tmx::byte_stream bytes;
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = (uint8_t) strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    return bytes;
}
static const std::string BsmHex =
    "001425067c0eb5842562e66e8a2b9ea6c96408b97fffffff900027d9637d07d0007fff8000640fa0";

// PSM Payload
static const std::string PsmHex =
    "00201c000002a5158048d159e14cdd338f3d4da420101effffffff00000000";


static const std::string UuidHex = "0a303030303fdfa7";


TEST(RawSpduTest, SetAndGetAttributesBsm) {
    RawSpdu msg;

    tmx::byte_stream spdu = hexStringToByteArray(BsmHex);
    tmx::byte_stream uuid = hexStringToByteArray(UuidHex);

    msg.set_spdu_data_bytes(std::make_shared<std::vector<uint8_t>>(spdu));
    msg.set_uuid(uuid);
    msg.set_MessageType("BSM"); // BasicSafetyMessage
    msg.set_timestamp_ms(1718900000000);
    msg.set_psid(8002);
    msg.set_channel(183);


    std::cout << msg.to_string() << std::endl;

    EXPECT_EQ("BSM", msg.get_MessageType());
    EXPECT_EQ(1718900000000, msg.get_timestamp_ms());


    auto spduOut = msg.get_spdu_data_bytes();
    EXPECT_EQ(spdu, *spduOut);

    auto uuidOut = msg.get_uuid();
    EXPECT_EQ(uuid, uuidOut);
    EXPECT_EQ(8002, msg.get_psid());
    EXPECT_EQ(183, msg.get_channel());
}

TEST(RawSpduTest, SetAndGetAttributesPsm) {
    RawSpdu msg;

    tmx::byte_stream spdu = hexStringToByteArray(BsmHex);
    tmx::byte_stream uuid = hexStringToByteArray(UuidHex);

    msg.set_spdu_data_bytes(std::make_shared<std::vector<uint8_t>>(spdu));
    msg.set_uuid(uuid);
    msg.set_MessageType("PSM"); // PersonalSafetyMessage
    msg.set_timestamp_ms(1718900123456);

    msg.set_psid(8001);
    msg.set_channel(183);
    std::cout << msg.to_string() << std::endl;

    EXPECT_EQ("PSM", msg.get_MessageType());
    EXPECT_EQ(1718900123456, msg.get_timestamp_ms());

    auto spduOut = msg.get_spdu_data_bytes();
    EXPECT_EQ(spdu, *spduOut);

    auto uuidOut = msg.get_uuid();
    EXPECT_EQ(uuid, uuidOut);
    EXPECT_EQ(8001, msg.get_psid());
    EXPECT_EQ(183, msg.get_channel());
}

