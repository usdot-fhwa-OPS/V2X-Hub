#include <gtest/gtest.h>

#include "TelematicBridgeMsgWorker.h"
#include <TimeSyncMessage.h>
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
    // Generate a TmxRouteableMessage with J2735 encoded data
    
}

TEST(TestTelematicMsgWork, stringToJsonValue) {
    std::string jsonString = R"({"key1": "value1", "key2": 2, "key3": true})";
    Json::Value jsonValue = stringToJsonValue(jsonString);
    
    EXPECT_EQ(jsonValue["key1"].asString(), "value1");
    EXPECT_EQ(jsonValue["key2"].asInt(), 2);
    EXPECT_EQ(jsonValue["key3"].asBool(), true);
}