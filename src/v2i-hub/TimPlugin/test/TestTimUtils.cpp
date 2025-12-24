#include <gtest/gtest.h>

#include <TimUtils.hpp>

namespace TimPlugin {

    TEST(TestTimUtils, isTimActiveFalse ) {
        // Test expired TIM 
#if SAEJ2735_SPEC >= 2024
        auto tim = readTimFile("../../TimPlugin/test/test_files/tim_2024.xml");
#else
        auto tim = readTimFile("../../TimPlugin/test/test_files/tim_2016.xml");
#endif

        EXPECT_FALSE(isTimActive(tim));
        // Start time 2025 May 14 5:36 PM (UTC)
        // Duration is 5760 miutes -> 4 days

    }

      TEST(TestTimUtils, isTimActivePersistTrue ) {
        // Test Persist TIM 
#if SAEJ2735_SPEC >= 2024
        auto tim = readTimFile("../../TimPlugin/test/test_files/tim_2024.xml");
#else
        auto tim = readTimFile("../../TimPlugin/test/test_files/tim_2016.xml");
#endif
        auto timPtr = tim->get_j2735_data();
        // Setting duration time to max value 32000 should indicate indefinite broadcast of TIM
        timPtr->dataFrames.list.array[0]->durationTime = 32000;
        EXPECT_TRUE(isTimActive(tim));
        // Start time 2025 May 14 5:36 PM (UTC)
        // Duration is 32000 miutes -> indefinite

    }

    TEST(TestTimUtils, isTimActiveEdgeCases) {
        // Test TIM when start time == current time 
#if SAEJ2735_SPEC >= 2024
        auto tim = readTimFile("../../TimPlugin/test/test_files/tim_2024.xml");
#else
        auto tim = readTimFile("../../TimPlugin/test/test_files/tim_2016.xml");
#endif
        auto timPtr = tim->get_j2735_data();
        // Set start time to current time 
        // 1) Get current time 
        time_t curTimeUTC =  std::time(nullptr);
        // Convert to utc tm struct
        auto tmUtc = gmtime(&curTimeUTC); 
        std::cout << asctime(tmUtc);
        // 2) Get minute of the year
        auto minuteOfYear = tmUtc->tm_yday * 24 * 60 +
                            tmUtc->tm_hour * 60 +
                            tmUtc->tm_min;
        // 3) Get current year
        long int* year =new long int ( tmUtc->tm_year + 1900);
        timPtr->dataFrames.list.array[0]->startYear = year;
        timPtr->dataFrames.list.array[0]->startTime = minuteOfYear;
        EXPECT_TRUE(isTimActive(tim));

        // What if start time is 2 minutes after current time
        timPtr->dataFrames.list.array[0]->startTime = minuteOfYear + 2;
        EXPECT_FALSE(isTimActive(tim));

        // What if start time is current time - duration + 1 (to account for seconds of current minute)
        timPtr->dataFrames.list.array[0]->startTime = minuteOfYear - 5760 + 1;
        EXPECT_TRUE(isTimActive(tim));

        // What if start time is current time - duration -2
        timPtr->dataFrames.list.array[0]->startTime = minuteOfYear - 5760 - 2;
        EXPECT_FALSE(isTimActive(tim));
    }

    TEST(TestTimUtils, convertTimTime) {
        // Convert start year and minuteOfTheYear to correct time 
        auto year = 2025;
        auto moy = 445437;
        // Corresponds to November 6 2025 7:57 AM (UTC)

        time_t convertedTime = convertTimTime(2025, moy);
        // Convert to UTC tm 
        struct tm *tmConvertedTime = gmtime(&convertedTime);

        EXPECT_EQ(2025-1900, tmConvertedTime->tm_year);
        EXPECT_EQ(10, tmConvertedTime->tm_mon);
        EXPECT_EQ(6, tmConvertedTime->tm_mday);
        EXPECT_EQ(7, tmConvertedTime->tm_hour);
        EXPECT_EQ(57, tmConvertedTime->tm_min);

    }

    TEST(TestTimUtils, readTimXml) {
#if SAEJ2735_SPEC >= 2024
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
#else
        std::string timXml = R"(
            <TravelerInformation>
                <msgCnt>1</msgCnt>
                <packetID>0000000000087FAA72</packetID>
                <dataFrames>
                    <TravelerDataFrame>
                    <sspTimRights>0</sspTimRights>
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
                    <startTime>5760</startTime>
                    <duratonTime>5760</duratonTime>
                    <priority>5</priority>
                    <sspLocationRights>0</sspLocationRights>
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
                    <sspMsgRights1>0</sspMsgRights1>
                    <sspMsgRights2>0</sspMsgRights2>
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
        )"
#endif
        auto tim = readTimXml(timXml);
        auto timPtr = tim->get_j2735_data();
        EXPECT_EQ(2025, *(timPtr->dataFrames.list.array[0]->startYear));
        EXPECT_EQ(181181, timPtr->dataFrames.list.array[0]->startTime);
        EXPECT_EQ(5760, timPtr->dataFrames.list.array[0]->durationTime);
        EXPECT_EQ(5, timPtr->dataFrames.list.array[0]->priority);


    }

    TEST(TestTimUtils, readTimFile) {
#if SAEJ2735_SPEC >= 2024
        auto tim = readTimFile("../../TimPlugin/test/test_files/tim_2024.xml");
#else
        auto tim = readTimFile("../../TimPlugin/test/test_files/tim_2016.xml");
#endif
        auto timPtr = tim->get_j2735_data();
        EXPECT_EQ(2025, *(timPtr->dataFrames.list.array[0]->startYear));
        EXPECT_EQ(181181, timPtr->dataFrames.list.array[0]->startTime);
        EXPECT_EQ(5760, timPtr->dataFrames.list.array[0]->durationTime);
        EXPECT_EQ(5, timPtr->dataFrames.list.array[0]->priority);
        // Attempt to read from file that does not exist
        EXPECT_THROW(readTimFile("../../TimPlugin/test/test_files/non-existant-file.xml"), tmx::TmxException);

        // Attempt to read from file that has the incorrect file extension
#if SAEJ2735_SPEC >= 2024
        EXPECT_NO_THROW(readTimFile("../../TimPlugin/test/test_files/tim_2024.bad_ext"));
#else
        EXPECT_NO_THROW(readTimFile("../../TimPlugin/test/test_files/tim_2016.bad_ext"));
#endif
    }
}

