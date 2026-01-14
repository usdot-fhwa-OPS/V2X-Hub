#include <gtest/gtest.h>

#include <TelematicBridgeMsgWorker.h>
#include <TimeSyncMessage.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <tmx/messages/byte_stream.hpp>
#include <SensorDetectedObject.h>
#include <jsoncpp/json/json.h>

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
    EXPECT_EQ(jsonValueToString(json["payload"]), "{\"seq\":\"1622547800\",\"timestep\":\"12345\"}");
}

TEST(TestTelematicMsgWorker, routeableMessageToJsonValue_SensorDetectedObject)
{
    tmx::messages::SensorDetectedObject sensorMsg;
    sensorMsg.set_objectId(1);
    sensorMsg.set_type("Vehicle");
    sensorMsg.set_confidence(0.95);
    sensorMsg.set_sensorId("Sensor123");
    sensorMsg.set_position(tmx::messages::Position(100.0, 200.0, 0.0));
    sensorMsg.set_velocity(tmx::messages::Velocity(50.0, 0.0, 0.0));
    sensorMsg.set_angularVelocity(tmx::messages::Velocity(0.0, 0.0, 0.1));
    sensorMsg.set_orientation(tmx::messages::Orientation(90.0, 0.0, 0.0));
    sensorMsg.set_projString("EPSG:4326");
    sensorMsg.set_isSimulated(false);
    sensorMsg.set_isModified(false);
    sensorMsg.set_timestamp(1622547800);
    sensorMsg.set_size(tmx::messages::Size(4.5, 2.0, 1.5));
    std::vector<tmx::messages::Covariance> covs {
        tmx::messages::Covariance(12),
        tmx::messages::Covariance(11),
        tmx::messages::Covariance(13),
        tmx::messages::Covariance(14),
        tmx::messages::Covariance(15)
    };
    int covarianceSize = 3;
    std::vector<std::vector<tmx::messages::Covariance>> covs2d;
    for(int i=0; i<covarianceSize; i++){
        covs2d.push_back(covs);
    }
    sensorMsg.set_positionCovariance(covs2d);
    sensorMsg.set_velocityCovariance(covs2d);
    sensorMsg.set_orientationCovariance(covs2d);
    sensorMsg.set_wgs84Position(tmx::messages::WGS84Position(37.7749, -122.4194, 10.0));

    tmx::routeable_message msg;
    msg.set_type(tmx::messages::MSGTYPE_APPLICATION_STRING);
    msg.set_subtype(tmx::messages::MSGSUBTYPE_SENSOR_DETECTED_OBJECT_STRING);
    msg.set_source("SensorSource");
    msg.set_sourceId(67890);
    msg.set_flags(IvpMsgFlags_RouteDSRC);
    msg.set_timestamp(1622547900);
    msg.set_dsrcChannel(180);
    msg.set_dsrcPsid(5678);
    msg.set_payload(sensorMsg);

    Json::Value json = routeableMessageToJsonValue(msg);

    EXPECT_EQ(json["type"].asString(), tmx::messages::MSGTYPE_APPLICATION_STRING);
    EXPECT_EQ(json["subType"].asString(), tmx::messages::MSGSUBTYPE_SENSOR_DETECTED_OBJECT_STRING);
    EXPECT_EQ(json["source"].asString(), "SensorSource");
    EXPECT_EQ(json["sourceId"].asInt(), 67890);
    EXPECT_EQ(json["flags"].asInt(), IvpMsgFlags_RouteDSRC);
    EXPECT_EQ(json["timestamp"].asInt64(), 1622547900);
    EXPECT_EQ(json["channel"].asInt(), 180);
    EXPECT_EQ(json["psid"].asInt(), 5678);
    EXPECT_EQ(json["encoding"].asString(), "json");
    EXPECT_TRUE(json["payload"].isObject());
    EXPECT_EQ(json["payload"]["angularVelocity"]["x"].asString(), "0");
    EXPECT_EQ(json["payload"]["wgs84Position"]["latitude"].asString(), "37.774900000000002");
    EXPECT_EQ(json["payload"]["wgs84Position"]["longitude"].asString(), "-122.4194");
    EXPECT_EQ(json["payload"]["wgs84Position"]["elevation"].asString(), "10");
    EXPECT_EQ(jsonValueToString(json["payload"]), "{\"angularVelocity\":{\"x\":\"0\",\"y\":\"0\",\"z\":\"0.10000000000000001\"},\"confidence\":\"0.94999999999999996\",\"isModified\":\"0\",\"isSimulated\":\"0\",\"objectId\":\"1\",\"orientation\":{\"x\":\"90\",\"y\":\"0\",\"z\":\"0\"},\"orientationCovariance\":[[\"12\",\"11\",\"13\",\"14\",\"15\"],[\"12\",\"11\",\"13\",\"14\",\"15\"],[\"12\",\"11\",\"13\",\"14\",\"15\"]],\"position\":{\"x\":\"100\",\"y\":\"200\",\"z\":\"0\"},\"positionCovariance\":[[\"12\",\"11\",\"13\",\"14\",\"15\"],[\"12\",\"11\",\"13\",\"14\",\"15\"],[\"12\",\"11\",\"13\",\"14\",\"15\"]],\"projString\":\"EPSG:4326\",\"sensorId\":\"Sensor123\",\"size\":{\"height\":\"1.5\",\"length\":\"4.5\",\"width\":\"2\"},\"timestamp\":\"1622547800\",\"type\":\"Vehicle\",\"velocity\":{\"x\":\"50\",\"y\":\"0\",\"z\":\"0\"},\"velocityCovariance\":[[\"12\",\"11\",\"13\",\"14\",\"15\"],[\"12\",\"11\",\"13\",\"14\",\"15\"],[\"12\",\"11\",\"13\",\"14\",\"15\"]],\"wgs84Position\":{\"elevation\":\"10\",\"latitude\":\"37.774900000000002\",\"longitude\":\"-122.4194\"}}");
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
    EXPECT_EQ(jsonValue["payload"].asString(), "0012822138713020311b27c535a4e8ac6b49c0c5109602dc2522f100001e480cc800000002b5962bc282900000000000000e22f62aff9080a400000000000000ac0600018096068000c06481cc800000002b5d282c282a800000000000000000000000000388bd91bfe4202a8000000000000000000000000000ac26000180561e4000c04200aa400000002b575dc809c45eea9fd405050806a900000000ad65600027387bb17f512022200000002a8bd20502711501421805052e00a60302828560f0000802b014000402240c4400000005541a42604e72e02c5ed02828505a1a02b0f8000401586900020108019900000002a7481f602711700032a814282804072100b32000000054bf03ec04e22dfd46da02850501e04c4804480000000a89f9db4282800000e22ccb6fe50a0a000003ecd4c157fc80a0a000004142b058000201583a00010112051200000002a26f6210a0a00000388b2f8bfa8202800000d4cb37fb4080a000000ac36000080561640004042002240000000a89fa2e809c4599a200004fb353001ff840a0a1009120000000544ef3f004e22cd49007027d9a97a6ffe0050512044200000002a736d7302738700fca20dff3c951581c0004012c15000200c904210000000152f36c8050510000000007397016c8e040510000000005bfc78c41014400000000056170001002b09200080210077200000005510dae604e70e01f8dc1c0039b0201ee40000000aa7675e809ce1c0531aab7f572b009f6");

}

TEST(TestTelematicMsgWorker, j2735MessageToJson) {
    // Create a TmxJ2735Message with factory
    tmx::messages::J2735MessageFactory myFactory;
    auto bytes = tmx::byte_stream_decode("0012822138713020311b27c535a4e8ac6b49c0c5109602dc2522f100001e480cc800000002b5962bc282900000000000000e22f62aff9080a400000000000000ac0600018096068000c06481cc800000002b5d282c282a800000000000000000000000000388bd91bfe4202a8000000000000000000000000000ac26000180561e4000c04200aa400000002b575dc809c45eea9fd405050806a900000000ad65600027387bb17f512022200000002a8bd20502711501421805052e00a60302828560f0000802b014000402240c4400000005541a42604e72e02c5ed02828505a1a02b0f8000401586900020108019900000002a7481f602711700032a814282804072100b32000000054bf03ec04e22dfd46da02850501e04c4804480000000a89f9db4282800000e22ccb6fe50a0a000003ecd4c157fc80a0a000004142b058000201583a00010112051200000002a26f6210a0a00000388b2f8bfa8202800000d4cb37fb4080a000000ac36000080561640004042002240000000a89fa2e809c4599a200004fb353001ff840a0a1009120000000544ef3f004e22cd49007027d9a97a6ffe0050512044200000002a736d7302738700fca20dff3c951581c0004012c15000200c904210000000152f36c8050510000000007397016c8e040510000000005bfc78c41014400000000056170001002b09200080210077200000005510dae604e70e01f8dc1c0039b0201ee40000000aa7675e809ce1c0531aab7f572b009f6");
    std::unique_ptr<tmx::routeable_message> routeableMsg(myFactory.NewMessage(bytes));
    // Convert to JSON string
    std::string jsonPayloadString = j2735MessageToJson(*routeableMsg.get());
    std::string expectedJson = "{\"messageId\":\"18\",\"value\":{\"MapData\":{\"msgIssueRevision\":\"113\",\"layerType\":\"intersectionData\",\"layerID\":\"1\",\"intersections\":[{\"id\":{\"id\":\"36243\"},\"revision\":\"113\",\"refPoint\":{\"lat\":\"-84\",\"long\":\"-4410\",\"elevation\":\"150\"},\"laneWidth\":\"366\",\"speedLimits\":[{\"type\":\"vehicleMaxSpeed\",\"speed\":\"1118\"},{\"type\":\"vehicleMinSpeed\",\"speed\":\"0\"}],\"laneSet\":[{\"laneID\":\"6\",\"ingressApproach\":\"6\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"1381\",\"y\":\"175\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"2837\",\"y\":\"-7\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}]}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"1\",\"maneuver\":\"8000\"},\"signalGroup\":\"6\",\"connectionID\":\"2\"},{\"connectingLane\":{\"lane\":\"3\",\"maneuver\":\"4000\"},\"signalGroup\":\"6\",\"connectionID\":\"3\"}]},{\"laneID\":\"14\",\"ingressApproach\":\"6\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"1396\",\"y\":\"523\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"2851\",\"y\":\"-7\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}]}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"9\",\"maneuver\":\"8000\"},\"signalGroup\":\"6\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"15\",\"maneuver\":\"2000\"},\"signalGroup\":\"6\",\"connectionID\":\"2\"}]},{\"laneID\":\"5\",\"egressApproach\":\"5\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"1373\",\"y\":\"-142\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"2986\",\"y\":\"-22\"}},\"attributes\":{\"dElevation\":\"10\"}}]}},{\"laneID\":\"13\",\"egressApproach\":\"5\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"1381\",\"y\":\"-512\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"2993\",\"y\":\"-22\"}}}]}},{\"laneID\":\"4\",\"ingressApproach\":\"4\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"189\",\"y\":\"-1531\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY3\":{\"x\":\"10\",\"y\":\"-1780\"}},\"attributes\":{\"dElevation\":\"10\"}},{\"delta\":{\"node-XY4\":{\"x\":\"5\",\"y\":\"-2557\"}},\"attributes\":{\"dElevation\":\"10\"}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"7\",\"maneuver\":\"8000\"},\"signalGroup\":\"4\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"1\",\"maneuver\":\"4000\"},\"signalGroup\":\"4\",\"connectionID\":\"2\"}]},{\"laneID\":\"12\",\"ingressApproach\":\"4\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"525\",\"y\":\"-1517\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"22\",\"y\":\"-2579\"}},\"attributes\":{\"dElevation\":\"10\"}},{\"delta\":{\"node-XY3\":{\"x\":\"45\",\"y\":\"-1840\"}}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"15\",\"maneuver\":\"8000\"},\"signalGroup\":\"4\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"13\",\"maneuver\":\"2000\"},\"signalGroup\":\"4\",\"connectionID\":\"2\"}]},{\"laneID\":\"3\",\"egressApproach\":\"3\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-184\",\"y\":\"-1546\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"0\",\"y\":\"-2475\"}},\"attributes\":{\"dElevation\":\"20\"}},{\"delta\":{\"node-XY3\":{\"x\":\"4\",\"y\":\"-1934\"}}}]}},{\"laneID\":\"11\",\"egressApproach\":\"3\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-520\",\"y\":\"-1546\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-22\",\"y\":\"-2342\"}},\"attributes\":{\"dElevation\":\"20\"}},{\"delta\":{\"node-XY3\":{\"x\":\"15\",\"y\":\"-2010\"}}}]}},{\"laneID\":\"2\",\"ingressApproach\":\"2\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-1410\",\"y\":\"-147\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-2469\",\"y\":\"-27\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-10\"}},{\"delta\":{\"node-XY6\":{\"x\":\"-13291\",\"y\":\"-56\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"10\"}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"5\",\"maneuver\":\"8000\"},\"signalGroup\":\"2\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"7\",\"maneuver\":\"4000\"},\"signalGroup\":\"2\",\"connectionID\":\"2\"}]},{\"laneID\":\"10\",\"ingressApproach\":\"2\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-1425\",\"y\":\"-479\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-2575\",\"y\":\"-22\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}]}},{\"delta\":{\"node-XY6\":{\"x\":\"-13133\",\"y\":\"-76\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}]}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"13\",\"maneuver\":\"8000\"},\"signalGroup\":\"2\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"11\",\"maneuver\":\"2000\"},\"signalGroup\":\"2\",\"connectionID\":\"2\"}]},{\"laneID\":\"1\",\"egressApproach\":\"1\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-1410\",\"y\":\"186\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-2456\",\"y\":\"0\"}},\"attributes\":{\"dElevation\":\"-10\"}},{\"delta\":{\"node-XY6\":{\"x\":\"-13312\",\"y\":\"-31\"}},\"attributes\":{\"dElevation\":\"10\"}}]}},{\"laneID\":\"9\",\"egressApproach\":\"1\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-1417\",\"y\":\"504\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-2396\",\"y\":\"7\"}},\"attributes\":{\"dElevation\":\"-10\"}},{\"delta\":{\"node-XY6\":{\"x\":\"-13357\",\"y\":\"-16\"}},\"attributes\":{\"dElevation\":\"10\"}}]}},{\"laneID\":\"8\",\"ingressApproach\":\"8\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-202\",\"y\":\"1395\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"15\",\"y\":\"2372\"}}},{\"delta\":{\"node-XY4\":{\"x\":\"-7\",\"y\":\"3221\"}}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"3\",\"maneuver\":\"8000\"},\"signalGroup\":\"8\",\"connectionID\":\"2\"},{\"connectingLane\":{\"lane\":\"5\",\"maneuver\":\"4000\"},\"signalGroup\":\"8\",\"connectionID\":\"3\"}]},{\"laneID\":\"16\",\"ingressApproach\":\"8\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-538\",\"y\":\"1424\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"22\",\"y\":\"2332\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}]}},{\"delta\":{\"node-XY4\":{\"x\":\"-15\",\"y\":\"3170\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}]}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"11\",\"maneuver\":\"8000\"},\"signalGroup\":\"8\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"9\",\"maneuver\":\"2000\"},\"signalGroup\":\"8\",\"connectionID\":\"2\"}]},{\"laneID\":\"7\",\"egressApproach\":\"7\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"134\",\"y\":\"1395\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"15\",\"y\":\"2268\"}}},{\"delta\":{\"node-XY4\":{\"x\":\"0\",\"y\":\"3288\"}}}]}},{\"laneID\":\"15\",\"egressApproach\":\"7\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"473\",\"y\":\"1402\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"20\",\"y\":\"2261\"}}},{\"delta\":{\"node-XY4\":{\"x\":\"-22\",\"y\":\"3244\"}},\"attributes\":{\"dElevation\":\"-10\"}}]}}]}]}}}\n";
    EXPECT_EQ(expectedJson, jsonPayloadString);
}

TEST(TestTelematicMsgWork, stringToJsonValue) {
    std::string jsonString = R"({"key1": "value1", "key2": 2, "key3": true})";
    Json::Value jsonValue = stringToJsonValue(jsonString);

    EXPECT_EQ(jsonValue["key1"].asString(), "value1");
    EXPECT_EQ(jsonValue["key2"].asInt(), 2);
    EXPECT_EQ(jsonValue["key3"].asBool(), true);
}

TEST(TestTelematicMsgWork, jsonValueToString) {
    std::string expectedJsonStr = R"({"key1":"value1","key2":2,"key3":true})";
    Json::Value json;
    json["key1"] = "value1";
    json["key2"] = 2;
    json["key3"] = true;
    EXPECT_EQ(jsonValueToString(json), expectedJsonStr);
}

TEST(TestTelematicMsgWorker, stringToJsonValueException) {
    std::string invalidJsonString = R"({"key1": "value1", "key2": 2, "awdasd)";
    EXPECT_THROW(stringToJsonValue(invalidJsonString), TelematicBridgeException);
}

TEST(TestTelematicMsgWorker, trim) {
    std::string stringWithSpaces = "\r\n\t test \t\n\r";
    std::string expectedStr = "test";
    EXPECT_EQ(expectedStr, trim(stringWithSpaces));
}

TEST(testTelematicMsgWOrker, jsonValuetoRouteableMessage)
{
    Json::Value json;
    json["unit"]["unitId"] = "Unit123";
    json["unit"]["maxConnections"] = 10;
    json["rsuConfigs"] = Json::arrayValue;
    json["timestamp"] = 1234567890;

    tmx::messages::RSURegistrationConfigMessage msg;

    bool result = jsonValueToRouteableMessage(json, msg);

    ASSERT_TRUE(result);
    ASSERT_FALSE(msg.to_string().empty());
}