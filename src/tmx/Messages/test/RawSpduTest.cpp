#include <gtest/gtest.h>
#include "RawSpdu.h"
#include <vector>
#include <iostream>
#include "tmx/messages/routeable_message.hpp"
#include "tmx/json/cJSON.h"

using namespace std;
using namespace tmx;
using namespace tmx::messages;

// BSM Payload
static const std::string bsmHex =
    "001425067c0eb5842562e66e8a2b9ea6c96408b97fffffff900027d9637d07d0007fff8000640fa0";

// faking secured bsm raw bytes (random bytes added to the front and back of the actual bsm payload)
static const std::string rawBytesBsmHex = "3b996de867c7419ad2ec5652d133c1fb" + bsmHex + "8cfae61d57e2bfba0b68242e11d641c2";

// PSM Payload
static const std::string psmHex =
    "00201c000002a5158048d159e14cdd338f3d4da420101effffffff00000000";

// fake secured psm raw bytes (random bytes added to the front and back of the actual psm payload)
static const std::string rawBytesPsmHex = "837c4ce3696aa2bccc4bec4e33f7c3b0" + psmHex + "b49483fd31d5dd38eb60f6f7ef93b842";

static const std::string uuidHex = "0a303030303fdfa7";


TEST(RawSpduTest, SetAndGetAttributesBsm) {
    RawSpdu rawSpdu;
    
    tmx::byte_stream spdu = tmx::byte_stream_decode(rawBytesBsmHex);
    tmx::byte_stream msg = tmx::byte_stream_decode(bsmHex);
    tmx::byte_stream uuid = tmx::byte_stream_decode(uuidHex);

    rawSpdu.set_fullByteData(spdu);
    rawSpdu.set_msgByteData(msg);
    rawSpdu.set_uuid(uuid);
    rawSpdu.set_messageType("BSM"); // BasicSafetyMessage
    long ts = 1718900000000;
    rawSpdu.set_timestampMs(ts);
    rawSpdu.set_psid(8002);


    std::cout << rawSpdu.to_string() << std::endl;

    EXPECT_EQ("BSM", rawSpdu.get_messageType());
    EXPECT_EQ(ts, rawSpdu.get_timestampMs());
    EXPECT_EQ(spdu, rawSpdu.get_fullByteData());
    EXPECT_EQ(msg, rawSpdu.get_msgByteData());
    EXPECT_EQ(uuid, rawSpdu.get_uuid());
    EXPECT_EQ(8002, rawSpdu.get_psid());
}

TEST(RawSpduTest, ToStringSerializesByteStreamAsHexString)
{
    RawSpdu rawSpdu;

    tmx::byte_stream spdu = tmx::byte_stream_decode(rawBytesBsmHex);
    tmx::byte_stream msg = tmx::byte_stream_decode(bsmHex);
    tmx::byte_stream uuid = tmx::byte_stream_decode(uuidHex);

    rawSpdu.set_fullByteData(spdu);
    rawSpdu.set_msgByteData(msg);
    rawSpdu.set_uuid(uuid);
    rawSpdu.set_messageType("BSM");
    rawSpdu.set_timestampMs(1718900000000);
    rawSpdu.set_psid(8002);

    std::string json = rawSpdu.to_string();

    EXPECT_NE(json.find("\"fullByteData\":\"" + rawBytesBsmHex + "\""), std::string::npos);
    EXPECT_NE(json.find("\"msgByteData\":\"" + bsmHex + "\""), std::string::npos);
    EXPECT_NE(json.find("\"uuid\":\"" + uuidHex + "\""), std::string::npos);
    EXPECT_NE(json.find("\"messageType\":\"BSM\""), std::string::npos);
    EXPECT_NE(json.find("\"timestampMs\":\"1718900000000\""), std::string::npos);
    EXPECT_NE(json.find("\"psid\":\"8002\""), std::string::npos);
}

TEST(RawSpduTest, SetAndGetAttributesPsm) {
    RawSpdu rawSpdu;

    tmx::byte_stream spdu = tmx::byte_stream_decode(rawBytesPsmHex);
    tmx::byte_stream msg = tmx::byte_stream_decode(psmHex);
    tmx::byte_stream uuid = tmx::byte_stream_decode(uuidHex);

    rawSpdu.set_fullByteData(spdu);
    rawSpdu.set_msgByteData(msg);
    rawSpdu.set_uuid(uuid);
    rawSpdu.set_messageType("PSM"); // PersonalSafetyMessage
    long ts = 1718900123456;
    rawSpdu.set_timestampMs(ts);

    rawSpdu.set_psid(8001);
    std::cout << rawSpdu.to_string() << std::endl;

    EXPECT_EQ("PSM", rawSpdu.get_messageType());
    EXPECT_EQ(ts, rawSpdu.get_timestampMs());
    EXPECT_EQ(spdu, rawSpdu.get_fullByteData());
    EXPECT_EQ(msg, rawSpdu.get_msgByteData());
    EXPECT_EQ(uuid, rawSpdu.get_uuid());
    EXPECT_EQ(8001, rawSpdu.get_psid());
}