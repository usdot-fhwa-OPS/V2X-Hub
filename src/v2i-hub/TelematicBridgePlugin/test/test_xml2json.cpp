#include <gtest/gtest.h>
#include "xml2json.h"
#include "stdio.h"

using namespace std;

class test_xml2json : public ::testing::Test
{
};

TEST_F(test_xml2json, xml2json)
{
    string expectedXMLStr = "<MessageFrame><messageId>20</messageId><value><BasicSafetyMessage><coreData><msgCnt>117</msgCnt><id>67458B6B</id><secMark>24440</secMark><lat>389565434</lat><long>-771500475</long><elev>745</elev><accuracy><semiMajor>255</semiMajor><semiMinor>255</semiMinor><orientation>65535</orientation></accuracy><transmission><neutral/></transmission><speed>8191</speed><heading>28800</heading><angle>127</angle><accelSet><long>2001</long><lat>2001</lat><vert>-127</vert><yaw>0</yaw></accelSet><brakes><wheelBrakes>00000</wheelBrakes><traction><unavailable/></traction><abs><unavailable/></abs><scs><unavailable/></scs><brakeBoost><unavailable/></brakeBoost><auxBrakes><unavailable/></auxBrakes></brakes><size><width>200</width><length>500</length></size></coreData></BasicSafetyMessage></value></MessageFrame>";
    auto json = xml2json(expectedXMLStr.c_str());
    string expectedJSONStr = "{\"MessageFrame\":{\"messageId\":\"20\",\"value\":{\"BasicSafetyMessage\":{\"coreData\":{\"msgCnt\":\"117\",\"id\":\"67458B6B\",\"secMark\":\"24440\",\"lat\":\"389565434\",\"long\":\"-771500475\",\"elev\":\"745\",\"accuracy\":{\"semiMajor\":\"255\",\"semiMinor\":\"255\",\"orientation\":\"65535\"},\"transmission\":{\"neutral\":null},\"speed\":\"8191\",\"heading\":\"28800\",\"angle\":\"127\",\"accelSet\":{\"long\":\"2001\",\"lat\":\"2001\",\"vert\":\"-127\",\"yaw\":\"0\"},\"brakes\":{\"wheelBrakes\":\"00000\",\"traction\":{\"unavailable\":null},\"abs\":{\"unavailable\":null},\"scs\":{\"unavailable\":null},\"brakeBoost\":{\"unavailable\":null},\"auxBrakes\":{\"unavailable\":null}},\"size\":{\"width\":\"200\",\"length\":\"500\"}}}}}}";
    ASSERT_EQ(expectedJSONStr, json);
}