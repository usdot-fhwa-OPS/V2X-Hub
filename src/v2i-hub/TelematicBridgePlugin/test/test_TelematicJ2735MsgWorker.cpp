#include <gtest/gtest.h>
#include "TelematicBridgeJ2735MsgWorker.h"
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
    auto success = TelematicBridgeJ2735MsgWorker::HexToBytes(bsmHex, byteBuff);

    ASSERT_TRUE(success);
    ASSERT_EQ(bsmHex.size() / 2, byteBuff.size());
}

TEST_F(test_TelematicJ2735MsgWorker, DecodeJ2735Msg)
{
    auto messageFrame = (MessageFrame_t *)malloc(sizeof(MessageFrame_t));
    string bsmHex = "0014251d59d162dad7de266e9a7d1ea6d4220974ffffffff8ffff080fdfa1fa1007fff0000640fa0";
    TelematicBridgeJ2735MsgWorker::DecodeJ2735Msg(bsmHex, messageFrame);
    ASSERT_EQ(20, messageFrame->messageId);
    ASN_STRUCT_FREE(asn_DEF_MessageFrame, messageFrame);
}

TEST_F(test_TelematicJ2735MsgWorker, ConvertJ2735FrameToXML)
{
    auto messageFrame = (MessageFrame_t *)malloc(sizeof(MessageFrame_t));
    string bsmHex = "0014251d59d162dad7de266e9a7d1ea6d4220974ffffffff8ffff080fdfa1fa1007fff0000640fa0";
    TelematicBridgeJ2735MsgWorker::DecodeJ2735Msg(bsmHex, messageFrame);
    auto xmlStr = TelematicBridgeJ2735MsgWorker::ConvertJ2735FrameToXML(messageFrame);
    ASN_STRUCT_FREE(asn_DEF_MessageFrame, messageFrame);
    string expectedXMLStr = "<MessageFrame><messageId>20</messageId><value><BasicSafetyMessage><coreData><msgCnt>117</msgCnt><id>67458B6B</id><secMark>24440</secMark><lat>389565434</lat><long>-771500475</long><elev>745</elev><accuracy><semiMajor>255</semiMajor><semiMinor>255</semiMinor><orientation>65535</orientation></accuracy><transmission><neutral/></transmission><speed>8191</speed><heading>28800</heading><angle>127</angle><accelSet><long>2001</long><lat>2001</lat><vert>-127</vert><yaw>0</yaw></accelSet><brakes><wheelBrakes>00000</wheelBrakes><traction><unavailable/></traction><abs><unavailable/></abs><scs><unavailable/></scs><brakeBoost><unavailable/></brakeBoost><auxBrakes><unavailable/></auxBrakes></brakes><size><width>200</width><length>500</length></size></coreData></BasicSafetyMessage></value></MessageFrame>";
    ASSERT_EQ(expectedXMLStr, xmlStr);
}