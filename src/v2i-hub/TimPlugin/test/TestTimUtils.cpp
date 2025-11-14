#include <gtest/gtest.h>

#include <TimUtils.hpp>

namespace TimPlugin {

    TEST(TestTimUtils, isTimActiveFalse ) {
        // Test expired TIM 
        auto tim = readTimFile("../../TimPlugin/test/test_files/tim_2024.xml");
        EXPECT_FALSE(isTimActive(tim));
        // Start time 2025 May 14 5:36 PM (UTC)
        // Duration is 5760 miutes -> 4 days

    }

      TEST(TestTimUtils, isTimActiveTrue ) {
        // Test expired TIM 
        auto tim = readTimFile("../../TimPlugin/test/test_files/tim_2024.xml");
        auto timPtr = tim->get_j2735_data();
        // Setting duration time to max value 32000 should indicate indefinite broadcast of TIM
        timPtr->dataFrames.list.array[0]->durationTime = 32000;
        EXPECT_TRUE(isTimActive(tim));
        // Start time 2025 May 14 5:36 PM (UTC)
        // Duration is 32000 miutes -> indefinite

    }

    TEST(TestTimUtils, isTimActiveInvalid) {

    }

    TEST(TestTimUtils, convertTimTime) {
        // Convert start year and minuteOfTheYear to correct time 
        auto year = 2025;
        auto moy = 445437;
        // Corresponds to November 5 2025 7:57 AM (UTC)

        time_t convertedTime = convertTimTime(2025, moy);
        // Convert to UTC tm 
        struct tm *tmConvertedTime = gmtime(&convertedTime);

        EXPECT_EQ(2025-1900, tmConvertedTime->tm_year);
        EXPECT_EQ(10, tmConvertedTime->tm_mon);
        EXPECT_EQ(5, tmConvertedTime->tm_mday);
        EXPECT_EQ(7, tmConvertedTime->tm_hour);
        EXPECT_EQ(57, tmConvertedTime->tm_min);

    }

    TEST(TestTimUtils, readTimXml) {
        std::string timXml = R"(
            <TravelerInformation>
                <msgCnt>1</msgCnt>
                <packetID>0000000000087FAA72</packetID>
                <dataFrames>
                    <TravelerDataFrame>
                    <doNotUse1>0</doNotUse1>
                    <frameType>
                        <roadSignage/>
                    </frameType>
                    <msgId>
                        <roadSignID>
                        <position>
                            <lat>281185423</lat>
                            <long>-818311882</long>
                            <elevation>220</elevation>
                        </position>
                        <viewAngle>1100000000000001</viewAngle>
                        <mutcdCode>
                            <maintenance/>
                        </mutcdCode>
                        </roadSignID>
                    </msgId>
                    <startYear>2025</startYear>
                    <startTime>181181</startTime>
                    <durationTime>5760</durationTime>
                    <priority>5</priority>
                    <doNotUse2>0</doNotUse2>
                    <regions>
                        <GeographicalPath>
                        <anchor>
                            <lat>281185423</lat>
                            <long>-818311882</long>
                            <elevation>220</elevation>
                        </anchor>
                        <laneWidth>366</laneWidth>
                        <directionality>
                            <forward/>
                        </directionality>
                        <closedPath>
                            <true/>
                        </closedPath>
                        <direction>1100000000000001</direction>
                        <description>
                            <path>
                            <offset>
                                <xy>
                                <nodes>
                                    <NodeXY>
                                    <delta>
                                        <node-XY3>
                                        <x>277</x>
                                        <y>-1815</y>
                                        </node-XY3>
                                    </delta>
                                    </NodeXY>
                                    <NodeXY>
                                    <delta>
                                        <node-XY2>
                                        <x>20</x>
                                        <y>885</y>
                                        </node-XY2>
                                    </delta>
                                    </NodeXY>
                                    <NodeXY>
                                    <delta>
                                        <node-XY4>
                                        <x>-296</x>
                                        <y>2261</y>
                                        </node-XY4>
                                    </delta>
                                    </NodeXY>
                                    <NodeXY>
                                    <delta>
                                        <node-XY3>
                                        <x>-527</x>
                                        <y>1756</y>
                                        </node-XY3>
                                    </delta>
                                    <attributes>
                                        <dElevation>-10</dElevation>
                                    </attributes>
                                    </NodeXY>
                                    <NodeXY>
                                    <delta>
                                        <node-XY1>
                                        <x>-408</x>
                                        <y>-66</y>
                                        </node-XY1>
                                    </delta>
                                    </NodeXY>
                                    <NodeXY>
                                    <delta>
                                        <node-XY3>
                                        <x>527</x>
                                        <y>-1652</y>
                                        </node-XY3>
                                    </delta>
                                    </NodeXY>
                                    <NodeXY>
                                    <delta>
                                        <node-XY2>
                                        <x>26</x>
                                        <y>-826</y>
                                        </node-XY2>
                                    </delta>
                                    <attributes>
                                        <dElevation>10</dElevation>
                                    </attributes>
                                    </NodeXY>
                                    <NodeXY>
                                    <delta>
                                        <node-XY3>
                                        <x>329</x>
                                        <y>-1186</y>
                                        </node-XY3>
                                    </delta>
                                    </NodeXY>
                                </nodes>
                                </xy>
                            </offset>
                            </path>
                        </description>
                        </GeographicalPath>
                    </regions>
                    <doNotUse3>0</doNotUse3>
                    <doNotUse4>0</doNotUse4>
                    <content>
                        <workZone>
                        <SEQUENCE>
                            <item>
                            <itis>769</itis>
                            </item>
                        </SEQUENCE>
                        </workZone>
                    </content>
                    </TravelerDataFrame>
                </dataFrames>
            </TravelerInformation>
        )";

        auto tim = readTimXml(timXml);
        auto timPtr = tim->get_j2735_data();
        EXPECT_EQ(2025, *(timPtr->dataFrames.list.array[0]->startYear));
        EXPECT_EQ(181181, timPtr->dataFrames.list.array[0]->startTime);
        EXPECT_EQ(5760, timPtr->dataFrames.list.array[0]->durationTime);
        EXPECT_EQ(5, timPtr->dataFrames.list.array[0]->priority);


    }

    TEST(TestTimUtils, readTimFile) {
        auto tim = readTimFile("../../TimPlugin/test/test_files/tim_2024.xml");
        auto timPtr = tim->get_j2735_data();
        EXPECT_EQ(2025, *(timPtr->dataFrames.list.array[0]->startYear));
        EXPECT_EQ(181181, timPtr->dataFrames.list.array[0]->startTime);
        EXPECT_EQ(5760, timPtr->dataFrames.list.array[0]->durationTime);
        EXPECT_EQ(5, timPtr->dataFrames.list.array[0]->priority);
    }
}

