#include <gtest/gtest.h>
#include "RawSpdu.h"
#include <vector>
#include <iostream>

using namespace std;
using namespace tmx;
using namespace tmx::messages;

static const std::string bsmHex =
    "001425067c0eb5842562e66e8a2b9ea6c96408b97fffffff900027d9637d07d0007fff8000640fa0";

// PSM Payload
static const std::string psmHex =
    "00201c000002a5158048d159e14cdd338f3d4da420101effffffff00000000";


static const std::string uuidHex = "0a303030303fdfa7";


TEST(RawSpduTest, SetAndGetAttributesBsm) {
    RawSpdu msg;

    tmx::byte_stream spdu = tmx::byte_stream_decode(bsmHex);
    tmx::byte_stream uuid = tmx::byte_stream_decode(uuidHex);

    msg.set_spduData(spdu);
    msg.set_uuid(uuid);
    msg.set_messageType("BSM"); // BasicSafetyMessage
    msg.set_timestampMs(1718900000000);
    msg.set_psid(8002);



    std::cout << msg.to_string() << std::endl;

    EXPECT_EQ("BSM", msg.get_messageType());
    EXPECT_EQ(1718900000000, msg.get_timestampMs());


    auto spduOut = msg.get_spduData();
    EXPECT_EQ(spdu, spduOut);

    auto uuidOut = msg.get_uuid();
    EXPECT_EQ(uuid, uuidOut);
    EXPECT_EQ(8002, msg.get_psid());
}

TEST(RawSpduTest, ToStringSerializesByteStreamAsHexString)
{
    RawSpdu msg;

    tmx::byte_stream spdu = tmx::byte_stream_decode(bsmHex);
    tmx::byte_stream uuid = tmx::byte_stream_decode(uuidHex);

    msg.set_spduData(spdu);
    msg.set_uuid(uuid);
    msg.set_messageType("BSM");
    msg.set_timestampMs(1718900000000);
    msg.set_psid(8002);

    std::string json = msg.to_string();

    EXPECT_NE(json.find("\"spduData\":\"" + bsmHex + "\""), std::string::npos);
    EXPECT_NE(json.find("\"uuid\":\"" + uuidHex + "\""), std::string::npos);
    EXPECT_NE(json.find("\"messageType\":\"BSM\""), std::string::npos);
    EXPECT_NE(json.find("\"timestampMs\":\"1718900000000\""), std::string::npos);
    EXPECT_NE(json.find("\"psid\":\"8002\""), std::string::npos);
}

TEST(RawSpduTest, SetAndGetAttributesPsm) {
    RawSpdu msg;

    tmx::byte_stream spdu = tmx::byte_stream_decode(psmHex);
    tmx::byte_stream uuid = tmx::byte_stream_decode(uuidHex);

    msg.set_spduData(spdu);
    msg.set_uuid(uuid);
    msg.set_messageType("PSM"); // PersonalSafetyMessage
    msg.set_timestampMs(1718900123456);

    msg.set_psid(8001);
    std::cout << msg.to_string() << std::endl;

    EXPECT_EQ("PSM", msg.get_messageType());
    EXPECT_EQ(1718900123456, msg.get_timestampMs());

    auto spduOut = msg.get_spduData();
    EXPECT_EQ(spdu, spduOut);

    auto uuidOut = msg.get_uuid();
    EXPECT_EQ(uuid, uuidOut);
    EXPECT_EQ(8001, msg.get_psid());
}

