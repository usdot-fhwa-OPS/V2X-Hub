#include <gtest/gtest.h>
#include "SecuredV2XMessage.h"
#include <vector>
#include <iostream>

using namespace std;
using namespace tmx;
using namespace tmx::messages;

// Function to convert hex string to byte array
std::vector<uint8_t> hexStringToByteArray(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = (uint8_t) strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    return bytes;
}


static const std::string BsmHex =
    "001425067c0eb5842562e66e8a2b9ea6c96408b97fffffff900027d9637d07d0007fff8000640fa0";

// Real PSM payload from the same test (psmMsg1)
static const std::string PsmHex =
    "00201c000002a5158048d159e14cdd338f3d4da420101effffffff00000000";


static const std::string UuidHex = "0a303030303fdfa7";

TEST(SecuredV2XMessageTest, SetAndGetAttributesBsm) {
    SecuredV2XMessage msg;

    std::vector<uint8_t> spdu = hexStringToByteArray(BsmHex);
    std::vector<uint8_t> uuid = hexStringToByteArray(UuidHex);

    msg.set_spdu_data_bytes(std::make_shared<std::vector<uint8_t>>(spdu));
    msg.set_uuid_bytes(std::make_shared<std::vector<uint8_t>>(uuid));
    msg.set_MessageType("BSM"); // BasicSafetyMessage
    msg.set_timestamp_ms(1718900000000);

    DsrcMetadata meta;
    meta.set_psid(8002);
    meta.set_channel(183);
    msg.set_dsrc_metadata(meta);

    std::cout << msg.to_string() << std::endl;

    EXPECT_EQ("BSM", msg.get_MessageType());
    EXPECT_EQ(1718900000000, msg.get_timestamp_ms());
    EXPECT_EQ(8002, msg.get_dsrc_metadata().get_psid());
    EXPECT_EQ(183, msg.get_dsrc_metadata().get_channel());

    auto spduOut = msg.get_spdu_data_bytes();
    EXPECT_EQ(spdu, *spduOut);

    auto uuidOut = msg.get_uuid_bytes();
    EXPECT_EQ(uuid, *uuidOut);
}

TEST(SecuredV2XMessageTest, SetAndGetAttributesPsm) {
    SecuredV2XMessage msg;

    std::vector<uint8_t> spdu = hexStringToByteArray(PsmHex);
    std::vector<uint8_t> uuid = hexStringToByteArray(UuidHex);

    msg.set_spdu_data_bytes(std::make_shared<std::vector<uint8_t>>(spdu));
    msg.set_uuid_bytes(std::make_shared<std::vector<uint8_t>>(uuid));
    msg.set_MessageType("PSM"); // PersonalSafetyMessage
    msg.set_timestamp_ms(1718900123456);

    DsrcMetadata meta;
    meta.set_psid(0x27);
    meta.set_channel(183);
    msg.set_dsrc_metadata(meta);

    std::cout << msg.to_string() << std::endl;

    EXPECT_EQ("PSM", msg.get_MessageType());
    EXPECT_EQ(1718900123456, msg.get_timestamp_ms());
    EXPECT_EQ(0x27, msg.get_dsrc_metadata().get_psid());
    EXPECT_EQ(183, msg.get_dsrc_metadata().get_channel());

    auto spduOut = msg.get_spdu_data_bytes();
    EXPECT_EQ(spdu, *spduOut);

    auto uuidOut = msg.get_uuid_bytes();
    EXPECT_EQ(uuid, *uuidOut);
}

TEST(SecuredV2XMessageTest, SpduDataRoundTripBsm) {
    SecuredV2XMessage msg;
    std::vector<uint8_t> expectedSpdu = hexStringToByteArray(BsmHex);

    msg.set_spdu_data_bytes(std::make_shared<std::vector<uint8_t>>(expectedSpdu));

    auto spduOut = msg.get_spdu_data_bytes();
    EXPECT_EQ(expectedSpdu, *spduOut);
}

TEST(SecuredV2XMessageTest, SpduDataRoundTripPsm) {
    SecuredV2XMessage msg;
    std::vector<uint8_t> expectedSpdu = hexStringToByteArray(PsmHex);

    msg.set_spdu_data_bytes(std::make_shared<std::vector<uint8_t>>(expectedSpdu));

    auto spduOut = msg.get_spdu_data_bytes();
    EXPECT_EQ(expectedSpdu, *spduOut);
}