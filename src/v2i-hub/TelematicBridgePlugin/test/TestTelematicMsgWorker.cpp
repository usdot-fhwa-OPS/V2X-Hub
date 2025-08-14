#include <gtest/gtest.h>

#include <TelematicBridgeMsgWorker.h>
#include <TimeSyncMessage.h>
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <tmx/messages/byte_stream.hpp>
using namespace TelematicBridge;
using namespace std;

TEST(TestTelematicMsgWorker, routeableMessageToJsonValue)
{
    tmx::messages::TimeSyncMessage timeSyncMsg( 12345, 1622547800 );
    tmx::routeable_message msg;
    msg.set_type(tmx::messages::MSGTYPE_APPLICATION_STRING);
    msg.set_subtype(tmx::messages::MSGSUBTYPE_TIMESYNC_STRING);
    msg.set_source("TestSource");
    msg.set_sourceId(12345);
    msg.set_flags(IvpMsgFlags_RouteDSRC);
    msg.set_timestamp(1622547800);
    msg.set_dsrcChannel(172);
    msg.set_dsrcPsid(1234);
    msg.set_payload(timeSyncMsg); // Simulating a payload string

    Json::Value json = routeableMessageToJsonValue(msg);

    EXPECT_EQ(json["type"].asString(), "Application");
    EXPECT_EQ(json["subType"].asString(), "TimeSync");
    EXPECT_EQ(json["source"].asString(), "TestSource");
    EXPECT_EQ(json["sourceId"].asString(), "12345");
    EXPECT_EQ(json["flags"].asInt(), IvpMsgFlags_RouteDSRC);
    EXPECT_EQ(json["timestamp"].asInt64(), 1622547800);
    EXPECT_EQ(json["channel"].asInt(), 172);
    EXPECT_EQ(json["psid"].asInt(), 1234);
    EXPECT_EQ(json["encoding"].asString(), "json");
    EXPECT_EQ(json["payload"].asString(), "{\"timestep\":\"12345\",\"seq\":\"1622547800\"}");
}
TEST(TestTelematicMsgWorker, routeableMessageToJsonValueJ2735EncodedMsg) {
    // Create an EncodedMapDataMessage
    tmx::messages::MapDataEncodedMessage mapDataMsg;
    auto bytes = tmx::byte_stream_decode("0012822138713020311b27c535a4e8ac6b49c0c5109602dc2522f100001e480cc800000002b5962bc282900000000000000e22f62aff9080a400000000000000ac0600018096068000c06481cc800000002b5d282c282a800000000000000000000000000388bd91bfe4202a8000000000000000000000000000ac26000180561e4000c04200aa400000002b575dc809c45eea9fd405050806a900000000ad65600027387bb17f512022200000002a8bd20502711501421805052e00a60302828560f0000802b014000402240c4400000005541a42604e72e02c5ed02828505a1a02b0f8000401586900020108019900000002a7481f602711700032a814282804072100b32000000054bf03ec04e22dfd46da02850501e04c4804480000000a89f9db4282800000e22ccb6fe50a0a000003ecd4c157fc80a0a000004142b058000201583a00010112051200000002a26f6210a0a00000388b2f8bfa8202800000d4cb37fb4080a000000ac36000080561640004042002240000000a89fa2e809c4599a200004fb353001ff840a0a1009120000000544ef3f004e22cd49007027d9a97a6ffe0050512044200000002a736d7302738700fca20dff3c951581c0004012c15000200c904210000000152f36c8050510000000007397016c8e040510000000005bfc78c41014400000000056170001002b09200080210077200000005510dae604e70e01f8dc1c0039b0201ee40000000aa7675e809ce1c0531aab7f572b009f6");
    mapDataMsg.set_payload_bytes(bytes);
    mapDataMsg.set_dsrcChannel(183);
    mapDataMsg.set_dsrcPsid(32770);
    mapDataMsg.set_flags(IvpMsgFlags_RouteDSRC);
    mapDataMsg.set_source("TelematicBridge");
    mapDataMsg.set_sourceId(1);
    tmx::routeable_message msg(mapDataMsg);
    // Get Current time in milliseconds
    auto currentTime = std::chrono::system_clock::now();
    auto timeSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();
    auto jsonValue = routeableMessageToJsonValue(msg);
    EXPECT_EQ(jsonValue["type"].asString(), "J2735");
    EXPECT_EQ(jsonValue["subType"].asString(), "MAP-P");
    EXPECT_EQ(jsonValue["source"].asString(), "TelematicBridge");
    EXPECT_EQ(jsonValue["sourceId"].asInt(), 1);
    EXPECT_EQ(jsonValue["flags"].asInt(), IvpMsgFlags_RouteDSRC);
    EXPECT_NEAR(jsonValue["timestamp"].asInt64(), timeSinceEpoch, 1000);
    EXPECT_EQ(jsonValue["channel"].asInt(), 183);
    EXPECT_EQ(jsonValue["psid"].asInt(), 32770);
    EXPECT_EQ(jsonValue["encoding"].asString(), "bytearray/hexstring");
}

TEST(TestTelematicMsgWork, stringToJsonValue) {
    std::string jsonString = R"({"key1": "value1", "key2": 2, "key3": true})";
    Json::Value jsonValue = stringToJsonValue(jsonString);
    
    EXPECT_EQ(jsonValue["key1"].asString(), "value1");
    EXPECT_EQ(jsonValue["key2"].asInt(), 2);
    EXPECT_EQ(jsonValue["key3"].asBool(), true);
}

TEST(TestTelematicMsgWorker, stringToJsonValueException) {
    std::string invalidJsonString = R"({"key1": "value1", "key2": 2, "awdasd)";
    EXPECT_THROW(stringToJsonValue(invalidJsonString), TelematicBridgeException);
}