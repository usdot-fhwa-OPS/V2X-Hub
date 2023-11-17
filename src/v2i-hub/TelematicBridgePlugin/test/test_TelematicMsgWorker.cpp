#include <gtest/gtest.h>
#include "TelematicBridgeMsgWorker.h"
#include "stdio.h"

using namespace TelematicBridge;
using namespace std;

class test_TelematicJ2735MsgWorker : public ::testing::Test
{
};

TEST_F(test_TelematicJ2735MsgWorker, HexToBytes)
{
    vector<char> byteBuff;
    string bsmHex = "0014251d59d162dad7de266e9a7d1ea6d4220974ffffffff8ffff080fdfa1fa1007fff0000640fa0";
    auto success = TelematicBridgeMsgWorker::HexToBytes(bsmHex, byteBuff);
    ASSERT_TRUE(success);
    ASSERT_EQ(bsmHex.size() / 2, byteBuff.size());
}

TEST_F(test_TelematicJ2735MsgWorker, DecodeJ2735Msg)
{
    auto messageFrame = (MessageFrame_t *)malloc(sizeof(MessageFrame_t));
    string bsmHex = "0014251d59d162dad7de266e9a7d1ea6d4220974ffffffff8ffff080fdfa1fa1007fff0000640fa0";
    ASSERT_NO_THROW(TelematicBridgeMsgWorker::DecodeJ2735Msg(bsmHex, messageFrame));
    ASSERT_EQ(20, messageFrame->messageId);
    ASN_STRUCT_FREE(asn_DEF_MessageFrame, messageFrame);
}

TEST_F(test_TelematicJ2735MsgWorker, DecodeJ2735MsgFailure)
{
    auto messageFrame = (MessageFrame_t *)malloc(sizeof(MessageFrame_t));
    string badHex = "0014251d59d162dad7de266e9a7d1ea6d4220974ffffffff8ffff080fdfa1fa1007fff0000640fG0";
    ASSERT_THROW(TelematicBridgeMsgWorker::DecodeJ2735Msg(badHex, messageFrame), TelematicBridgeException);

    badHex = "0014251d59d162dad7de266e9a";
    ASSERT_THROW(TelematicBridgeMsgWorker::DecodeJ2735Msg(badHex, messageFrame), TelematicBridgeException);
    ASN_STRUCT_FREE(asn_DEF_MessageFrame, messageFrame);
}

TEST_F(test_TelematicJ2735MsgWorker, ConvertJ2735FrameToXML)
{
    auto messageFrame = (MessageFrame_t *)malloc(sizeof(MessageFrame_t));
    string bsmHex = "0014251d59d162dad7de266e9a7d1ea6d4220974ffffffff8ffff080fdfa1fa1007fff0000640fA0";
    ASSERT_NO_THROW(TelematicBridgeMsgWorker::DecodeJ2735Msg(bsmHex, messageFrame));
    auto xmlStr = TelematicBridgeMsgWorker::ConvertJ2735FrameToXML(messageFrame);
    ASN_STRUCT_FREE(asn_DEF_MessageFrame, messageFrame);
    string expectedXMLStr = "<MessageFrame><messageId>20</messageId><value><BasicSafetyMessage><coreData><msgCnt>117</msgCnt><id>67458B6B</id><secMark>24440</secMark><lat>389565434</lat><long>-771500475</long><elev>745</elev><accuracy><semiMajor>255</semiMajor><semiMinor>255</semiMinor><orientation>65535</orientation></accuracy><transmission><neutral/></transmission><speed>8191</speed><heading>28800</heading><angle>127</angle><accelSet><long>2001</long><lat>2001</lat><vert>-127</vert><yaw>0</yaw></accelSet><brakes><wheelBrakes>00000</wheelBrakes><traction><unavailable/></traction><abs><unavailable/></abs><scs><unavailable/></scs><brakeBoost><unavailable/></brakeBoost><auxBrakes><unavailable/></auxBrakes></brakes><size><width>200</width><length>500</length></size></coreData></BasicSafetyMessage></value></MessageFrame>";
    ASSERT_EQ(expectedXMLStr, xmlStr);
}

TEST_F(test_TelematicJ2735MsgWorker, constructTelematicPayload)
{
    IvpMessage msg;
    auto payload = cJSON_CreateObject();
    payload->valuedouble = 12;
    payload->type = cJSON_Number;
    msg.payload = payload;
    msg.source = "Plugin";
    msg.encoding = "json";
    msg.type = "Application";
    msg.subtype = "alive";
    msg.sourceId = 123;
    msg.flags = 10;
    msg.timestamp = 10;
    IvpDsrcMetadata metadata;
    metadata.channel = 12;
    metadata.psid = 120;
    msg.dsrcMetadata = &metadata;
    auto json = TelematicBridgeMsgWorker::ivpMessageToJson(&msg);
    auto str = TelematicBridgeMsgWorker::JsonToString(json);
    string expectedStr = "{\"channel\":12,\"encoding\":\"json\",\"flags\":10,\"payload\":12.0,\"psid\":120,\"source\":\"Plugin\",\"sourceId\":123,\"subType\":\"alive\",\"timestamp\":10,\"type\":\"Application\"}";
    ASSERT_EQ(expectedStr, str);

    payload->valueint = 13;
    payload->type = cJSON_Number;
    msg.payload = payload;
    json = TelematicBridgeMsgWorker::ivpMessageToJson(&msg);
    str = TelematicBridgeMsgWorker::JsonToString(json);
    expectedStr = "{\"channel\":12,\"encoding\":\"json\",\"flags\":10,\"payload\":13.0,\"psid\":120,\"source\":\"Plugin\",\"sourceId\":123,\"subType\":\"alive\",\"timestamp\":10,\"type\":\"Application\"}";
    ASSERT_EQ(expectedStr, str);

    payload->valuestring = "test";
    payload->type = cJSON_String;
    msg.payload = payload;
    json = TelematicBridgeMsgWorker::ivpMessageToJson(&msg);
    str = TelematicBridgeMsgWorker::JsonToString(json);
    expectedStr = "{\"channel\":12,\"encoding\":\"json\",\"flags\":10,\"payload\":\"test\",\"psid\":120,\"source\":\"Plugin\",\"sourceId\":123,\"subType\":\"alive\",\"timestamp\":10,\"type\":\"Application\"}";
    ASSERT_EQ(expectedStr, str);

    msg.payload = cJSON_Parse("[{\"test\":12}]");
    json = TelematicBridgeMsgWorker::ivpMessageToJson(&msg);
    str = TelematicBridgeMsgWorker::JsonToString(json);
    expectedStr = "{\"channel\":12,\"encoding\":\"json\",\"flags\":10,\"payload\":[{\"test\":12}],\"psid\":120,\"source\":\"Plugin\",\"sourceId\":123,\"subType\":\"alive\",\"timestamp\":10,\"type\":\"Application\"}";
    ASSERT_EQ(expectedStr, str);

    json = TelematicBridgeMsgWorker::StringToJson("{\"test\":12}");
    ASSERT_EQ(12,json["test"].asInt64());
}