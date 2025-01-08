#include <gtest/gtest.h>
#include "TravelerInformationMessageHelper.hpp"
#include <regex>
using TIMHelper = PedestrianPlugin::TravelerInformationMessageHelper;
namespace unit_test {
    TEST(TravelerInformationMessage, calculateMinuteOfYear){
        int year = 2025;
        int month = 1;
        int day = 7;
        int hour = 16;
        int minute = 48;
        int second = 37;
        int expected = 9648;
        int actual = TIMHelper::calculateMinuteOfYear(year, month, day, hour, minute, second);
        EXPECT_EQ(expected, actual);

        hour = 16;
        minute = 49;
        expected = 9649;
        actual = TIMHelper::calculateMinuteOfYear(year, month, day, hour, minute, second);
        EXPECT_EQ(expected, actual);
    }

    TEST(TraveleInformationMessage, calculateMinutesOfCurrentYear){
        int notExpected = 0;
        int actual = TIMHelper::calculateMinuteOfCurrentYear();
        EXPECT_NE(notExpected, actual);
    }

    TEST(TravelerInformationMessage, calculateCurrentYear){
        int expected = 2025;
        int actual = TIMHelper::calculateCurrentYear();
        EXPECT_EQ(expected, actual);
    }

    TEST(TravelerInformationMessage, increaseMsgCount){
        int16_t msgCount = 0;
        int16_t expected = 1;
        int16_t actual = TIMHelper::increaseMsgCount(msgCount);
        EXPECT_EQ(expected, actual);

        msgCount = 127;
        expected = 0;
        actual = TIMHelper::increaseMsgCount(msgCount);
        EXPECT_EQ(expected, actual);
    }

    TEST(TravelerInformationMessage, updateTimTree){
        std::string staticTimXMLIn = "<TravelerInformation><msgCnt>1</msgCnt><timeStamp>3960</timeStamp><packetID>8D442EF003FC6B1B01</packetID><urlB>null</urlB><dataFrames><TravelerDataFrame><notUsed>0</notUsed><frameType><advisory/></frameType><msgId><roadSignID><position><lat>389550358</lat><long>-771495007</long></position><viewAngle>1111111111111111</viewAngle><mutcdCode><warning/></mutcdCode></roadSignID></msgId><startYear>2025</startYear><startTime>3960</startTime><durationTime>32000</durationTime><priority>5</priority><notUsed1>0</notUsed1><regions><GeographicalPath><name>WesternPedestrianCrossing</name><id><region>0</region><id>0</id></id><anchor><lat>389550358</lat><long>-771495007</long></anchor><laneWidth>600</laneWidth><directionality><reverse/></directionality><closedPath><false/></closedPath><direction>0000000000111100</direction><description><path><scale>0</scale><offset><ll><nodes><NodeLL><delta><node-LL1><lon>1556</lon><lat>-454</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>1676</lon><lat>-254</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL2><lon>5293</lon><lat>-531</lat></node-LL2></delta></NodeLL></nodes></ll></offset></path></description></GeographicalPath><GeographicalPath><name>NorthernPedestrianCrossing</name><id><region>0</region><id>0</id></id><anchor><lat>389551020</lat><long>-771492660</long></anchor><laneWidth>600</laneWidth><directionality><reverse/></directionality><closedPath><false/></closedPath><direction>1110000000000011</direction><description><path><scale>0</scale><offset><ll><nodes><NodeLL><delta><node-LL1><lon>-791</lon><lat>-1103</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-610</lon><lat>-876</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-3</lon><lat>2</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-1149</lon><lat>-1750</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-133</lon><lat>-257</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-121</lon><lat>-538</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>25</lon><lat>-360</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>241</lon><lat>-469</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>260</lon><lat>-232</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>159</lon><lat>-84</lat></node-LL1></delta></NodeLL></nodes></ll></offset></path></description></GeographicalPath><GeographicalPath><name>SouthernPedestrianCrossing</name><id><region>0</region><id>0</id></id><anchor><lat>389549055</lat><long>-771494061</long></anchor><laneWidth>600</laneWidth><directionality><reverse/></directionality><closedPath><false/></closedPath><direction>0000001111000000</direction><description><path><scale>0</scale><offset><ll><nodes><NodeLL><delta><node-LL1><lon>602</lon><lat>849</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>834</lon><lat>1109</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>353</lon><lat>584</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL2><lon>1646</lon><lat>3238</lat></node-LL2></delta></NodeLL></nodes></ll></offset></path></description></GeographicalPath><GeographicalPath><name>EasternPedestrianCrossing</name><id><region>0</region><id>0</id></id><anchor><lat>389549656</lat><long>-771491766</long></anchor><laneWidth>600</laneWidth><directionality><reverse/></directionality><closedPath><false/></closedPath><direction>0011110000000000</direction><description><path><scale>0</scale><offset><ll><nodes><NodeLL><delta><node-LL1><lon>-1693</lon><lat>241</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL2><lon>-4063</lon><lat>1160</lat></node-LL2></delta></NodeLL><NodeLL><delta><node-LL2><lon>-2311</lon><lat>981</lat></node-LL2></delta></NodeLL></nodes></ll></offset></path></description></GeographicalPath></regions><notUsed2>0</notUsed2><notUsed3>0</notUsed3><content><advisory><SEQUENCE><item><itis>9486</itis></item></SEQUENCE><SEQUENCE><item><itis>13585</itis></item></SEQUENCE></advisory></content><url>null</url></TravelerDataFrame></dataFrames></TravelerInformation>";
        int msgCount = 1;
        int startYear = 2026;
        int startTime = 9648;
        int durationTime = 1;
        boost::property_tree::ptree timTree;
        std::istringstream iss(staticTimXMLIn);
        boost::property_tree::read_xml(iss, timTree);
        TIMHelper::updateTimTree(timTree, msgCount, startYear, startTime, durationTime);
        int expectedMsgCount = 1;
        int actualMsgCount = timTree.get<int>("TravelerInformation.msgCnt");
        EXPECT_EQ(expectedMsgCount, actualMsgCount);

        int expectedStartYear = 2026;
        int actualStartYear = timTree.get<int>("TravelerInformation.dataFrames.TravelerDataFrame.startYear");
        EXPECT_EQ(expectedStartYear, actualStartYear);

        int expectedStartTime = 9648;
        int actualStartTime = timTree.get<int>("TravelerInformation.dataFrames.TravelerDataFrame.startTime");
        EXPECT_EQ(expectedStartTime, actualStartTime);

        int expectedDurationTime = 1;
        int actualDurationTime = timTree.get<int>("TravelerInformation.dataFrames.TravelerDataFrame.durationTime");
        EXPECT_EQ(expectedDurationTime, actualDurationTime);
    }

    TEST(TravelerInformationMessage, updateTimXML){
    std::string staticTimXMLIn = "<TravelerInformation><msgCnt>1</msgCnt><timeStamp>3960</timeStamp><packetID>8D442EF003FC6B1B01</packetID><urlB>null</urlB><dataFrames><TravelerDataFrame><notUsed>0</notUsed><frameType><advisory/></frameType><msgId><roadSignID><position><lat>389550358</lat><long>-771495007</long></position><viewAngle>1111111111111111</viewAngle><mutcdCode><warning/></mutcdCode></roadSignID></msgId><startYear>2025</startYear><startTime>3960</startTime><durationTime>32000</durationTime><priority>5</priority><notUsed1>0</notUsed1><regions><GeographicalPath><name>WesternPedestrianCrossing</name><id><region>0</region><id>0</id></id><anchor><lat>389550358</lat><long>-771495007</long></anchor><laneWidth>600</laneWidth><directionality><reverse/></directionality><closedPath><false/></closedPath><direction>0000000000111100</direction><description><path><scale>0</scale><offset><ll><nodes><NodeLL><delta><node-LL1><lon>1556</lon><lat>-454</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>1676</lon><lat>-254</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL2><lon>5293</lon><lat>-531</lat></node-LL2></delta></NodeLL></nodes></ll></offset></path></description></GeographicalPath><GeographicalPath><name>NorthernPedestrianCrossing</name><id><region>0</region><id>0</id></id><anchor><lat>389551020</lat><long>-771492660</long></anchor><laneWidth>600</laneWidth><directionality><reverse/></directionality><closedPath><false/></closedPath><direction>1110000000000011</direction><description><path><scale>0</scale><offset><ll><nodes><NodeLL><delta><node-LL1><lon>-791</lon><lat>-1103</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-610</lon><lat>-876</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-3</lon><lat>2</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-1149</lon><lat>-1750</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-133</lon><lat>-257</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-121</lon><lat>-538</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>25</lon><lat>-360</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>241</lon><lat>-469</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>260</lon><lat>-232</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>159</lon><lat>-84</lat></node-LL1></delta></NodeLL></nodes></ll></offset></path></description></GeographicalPath><GeographicalPath><name>SouthernPedestrianCrossing</name><id><region>0</region><id>0</id></id><anchor><lat>389549055</lat><long>-771494061</long></anchor><laneWidth>600</laneWidth><directionality><reverse/></directionality><closedPath><false/></closedPath><direction>0000001111000000</direction><description><path><scale>0</scale><offset><ll><nodes><NodeLL><delta><node-LL1><lon>602</lon><lat>849</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>834</lon><lat>1109</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>353</lon><lat>584</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL2><lon>1646</lon><lat>3238</lat></node-LL2></delta></NodeLL></nodes></ll></offset></path></description></GeographicalPath><GeographicalPath><name>EasternPedestrianCrossing</name><id><region>0</region><id>0</id></id><anchor><lat>389549656</lat><long>-771491766</long></anchor><laneWidth>600</laneWidth><directionality><reverse/></directionality><closedPath><false/></closedPath><direction>0011110000000000</direction><description><path><scale>0</scale><offset><ll><nodes><NodeLL><delta><node-LL1><lon>-1693</lon><lat>241</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL2><lon>-4063</lon><lat>1160</lat></node-LL2></delta></NodeLL><NodeLL><delta><node-LL2><lon>-2311</lon><lat>981</lat></node-LL2></delta></NodeLL></nodes></ll></offset></path></description></GeographicalPath></regions><notUsed2>0</notUsed2><notUsed3>0</notUsed3><content><advisory><SEQUENCE><item><itis>9486</itis></item></SEQUENCE><SEQUENCE><item><itis>13585</itis></item></SEQUENCE></advisory></content><url>null</url></TravelerDataFrame></dataFrames></TravelerInformation>";
        int msgCount = 1;
        int startYear = 2026;
        int startTime = 9648;
        int durationTime = 1;
        std::string expected = "<?xml version=\"1.0\" encoding=\"utf-8\"?><TravelerInformation><msgCnt>1</msgCnt><timeStamp>3960</timeStamp><packetID>8D442EF003FC6B1B01</packetID><urlB>null</urlB><dataFrames><TravelerDataFrame><notUsed>0</notUsed><frameType><advisory/></frameType><msgId><roadSignID><position><lat>389550358</lat><long>-771495007</long></position><viewAngle>1111111111111111</viewAngle><mutcdCode><warning/></mutcdCode></roadSignID></msgId><startYear>2026</startYear><startTime>9648</startTime><durationTime>1</durationTime><priority>5</priority><notUsed1>0</notUsed1><regions><GeographicalPath><name>WesternPedestrianCrossing</name><id><region>0</region><id>0</id></id><anchor><lat>389550358</lat><long>-771495007</long></anchor><laneWidth>600</laneWidth><directionality><reverse/></directionality><closedPath><false/></closedPath><direction>0000000000111100</direction><description><path><scale>0</scale><offset><ll><nodes><NodeLL><delta><node-LL1><lon>1556</lon><lat>-454</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>1676</lon><lat>-254</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL2><lon>5293</lon><lat>-531</lat></node-LL2></delta></NodeLL></nodes></ll></offset></path></description></GeographicalPath><GeographicalPath><name>NorthernPedestrianCrossing</name><id><region>0</region><id>0</id></id><anchor><lat>389551020</lat><long>-771492660</long></anchor><laneWidth>600</laneWidth><directionality><reverse/></directionality><closedPath><false/></closedPath><direction>1110000000000011</direction><description><path><scale>0</scale><offset><ll><nodes><NodeLL><delta><node-LL1><lon>-791</lon><lat>-1103</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-610</lon><lat>-876</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-3</lon><lat>2</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-1149</lon><lat>-1750</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-133</lon><lat>-257</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>-121</lon><lat>-538</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>25</lon><lat>-360</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>241</lon><lat>-469</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>260</lon><lat>-232</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>159</lon><lat>-84</lat></node-LL1></delta></NodeLL></nodes></ll></offset></path></description></GeographicalPath><GeographicalPath><name>SouthernPedestrianCrossing</name><id><region>0</region><id>0</id></id><anchor><lat>389549055</lat><long>-771494061</long></anchor><laneWidth>600</laneWidth><directionality><reverse/></directionality><closedPath><false/></closedPath><direction>0000001111000000</direction><description><path><scale>0</scale><offset><ll><nodes><NodeLL><delta><node-LL1><lon>602</lon><lat>849</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>834</lon><lat>1109</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL1><lon>353</lon><lat>584</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL2><lon>1646</lon><lat>3238</lat></node-LL2></delta></NodeLL></nodes></ll></offset></path></description></GeographicalPath><GeographicalPath><name>EasternPedestrianCrossing</name><id><region>0</region><id>0</id></id><anchor><lat>389549656</lat><long>-771491766</long></anchor><laneWidth>600</laneWidth><directionality><reverse/></directionality><closedPath><false/></closedPath><direction>0011110000000000</direction><description><path><scale>0</scale><offset><ll><nodes><NodeLL><delta><node-LL1><lon>-1693</lon><lat>241</lat></node-LL1></delta></NodeLL><NodeLL><delta><node-LL2><lon>-4063</lon><lat>1160</lat></node-LL2></delta></NodeLL><NodeLL><delta><node-LL2><lon>-2311</lon><lat>981</lat></node-LL2></delta></NodeLL></nodes></ll></offset></path></description></GeographicalPath></regions><notUsed2>0</notUsed2><notUsed3>0</notUsed3><content><advisory><SEQUENCE><item><itis>9486</itis></item></SEQUENCE><SEQUENCE><item><itis>13585</itis></item></SEQUENCE></advisory></content><url>null</url></TravelerDataFrame></dataFrames></TravelerInformation>";
        std::string actual = TIMHelper::updateTimXML(staticTimXMLIn, msgCount, startYear, startTime, durationTime);
        actual = std::regex_replace(actual, std::regex("\n"), "");
        EXPECT_EQ(expected, actual);
    }

}