#include <gtest/gtest.h>

#include "TelematicBridgeMsgWorker.h"
#include <TimeSyncMessage.h>
using namespace TelematicBridge;
using namespace std;

TEST(TestTelematicMsgWorker, IvpMessageToJson)
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

    Json::Value json = IvpMessageToJson(msg);

    EXPECT_EQ(json["type"].asString(), "J2735");
    EXPECT_EQ(json["subType"].asString(), "SignalStatusMessage");
    EXPECT_EQ(json["source"].asString(), "TestSource");
    EXPECT_EQ(json["sourceId"].asString(), "12345");
    EXPECT_EQ(json["flags"].asInt(), IvpMsgFlags_RouteDSRC);
    EXPECT_EQ(json["timestamp"].asInt64(), 1622547800);
    EXPECT_EQ(json["channel"].asInt(), 172);
    EXPECT_EQ(json["psid"].asInt(), 1234);
    EXPECT_EQ(json["encoding"].asString(), "json");
    EXPECT_EQ(json["payload"].asString(), "{\"timestep\":\"12345\",\"seq\":\"1622547800\"}");
}
TEST(TestTelematicMsgWorker, IvpMessageToJsonJ2735EncodedMsg) {
    // Generate a TmxRouteableMessage with J2735 encoded data
    
}