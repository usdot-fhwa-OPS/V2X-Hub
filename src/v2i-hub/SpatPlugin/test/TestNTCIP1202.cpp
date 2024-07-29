/**
 * Copyright (C) 2024 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
#include <gtest/gtest.h>
#include <tmx/j2735_messages/SpatMessage.hpp>
#include <NTCIP1202.h>
#include <carma-clock/carma_clock.h>

using namespace fwha_stol::lib::time;

TEST(NTCIP1202Test, copyBytesIntoNtcip1202)
{
    uint64_t tsMsec = 1677775434400;

    auto ntcip1202_p = std::make_shared<Ntcip1202>();
    unsigned int raw_data[] =  {4294967245, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 118, 0, 118, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 4294967208, 0, 4294967208, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 118, 0, 118, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 4294967208, 0, 4294967208, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4294967295, 4294967261, 0, 0, 0, 34, 4294967295, 4294967295, 0, 0, 0, 0, 4294967295, 4294967295, 0, 0, 0, 0, 0, 0, 0, 0, 4294967168, 0, 8, 103, 1, 10, 4294967237, 0, 0};
    int numBytes = sizeof(raw_data)/sizeof(unsigned int);
    char buf[ numBytes] = {};
    for(int i = 0; i< numBytes;i++)
    {
        buf[i]=static_cast<char>( raw_data[i] );
    }
    string json = "{\"SignalGroups\":[{\"SignalGroupId\":1,\"Phase\":1,\"Type\":\"vehicle\"},{\"SignalGroupId\":2,\"Phase\":2,\"Type\":\"vehicle\"},{\"SignalGroupId\":3,\"Phase\":3,\"Type\":\"vehicle\"},{\"SignalGroupId\":4,\"Phase\":4,\"Type\":\"vehicle\"},{\"SignalGroupId\":5,\"Phase\":5,\"Type\":\"vehicle\"},{\"SignalGroupId\":6,\"Phase\":6,\"Type\":\"vehicle\"},{\"SignalGroupId\":7,\"Phase\":7,\"Type\":\"vehicle\"},{\"SignalGroupId\":8,\"Phase\":8,\"Type\":\"vehicle\"},{\"SignalGroupId\":22,\"Phase\":2,\"Type\":\"pedestrian\"},{\"SignalGroupId\":24,\"Phase\":4,\"Type\":\"pedestrian\"},{\"SignalGroupId\":26,\"Phase\":6,\"Type\":\"pedestrian\"},{\"SignalGroupId\":28,\"Phase\":8,\"Type\":\"pedestrian\"}]}";
    ntcip1202_p->setSignalGroupMappingList(json);
    ntcip1202_p->copyBytesIntoNtcip1202(buf, numBytes);

    SPAT *spat_ptr = (SPAT *)calloc(1, sizeof(SPAT));
    ntcip1202_p->ToJ2735SPAT(spat_ptr,tsMsec, "test intersection name", 9012);

    ASSERT_EQ(3,  spat_ptr->intersections.list.array[0]->states.list.array[0]->state_time_speed.list.array[0]->eventState);

    free(spat_ptr);
}

TEST(NTCIP1202Test, ToJ2735SPAT)
{
    uint64_t tsMsec = 1677775434400;

    auto ntcip1202_p = std::make_shared<Ntcip1202>();
    SPAT *spat_ptr = (SPAT *)calloc(1, sizeof(SPAT));
    ntcip1202_p->ToJ2735SPAT(spat_ptr, tsMsec, "test intersection name", 9012);
    auto _spatMessage = std::make_shared<tmx::messages::SpatMessage>(spat_ptr);
    auto spat = _spatMessage->get_j2735_data();
}

TEST(NTCIP1202Test, TestAdjustedTime)
{
    // 1677775434400 = 2023-02-03 16:43:54.400
    uint64_t tsMsec = 1677775434400;
    auto baseTenthsOfSeconds = 43 * 600 + 54 * 10 + 4;
    auto ntcip1202_p = std::make_shared<Ntcip1202>();
    auto result = ntcip1202_p->getAdjustedTime(0, tsMsec);
    EXPECT_EQ(baseTenthsOfSeconds, result);
    result = ntcip1202_p->getAdjustedTime(46, tsMsec);
    EXPECT_EQ(baseTenthsOfSeconds + 46, result);
    // cross minute boundary
    result = ntcip1202_p->getAdjustedTime(200, tsMsec);
    EXPECT_EQ(baseTenthsOfSeconds + 200, result);
    // cross hour boundary
    result = ntcip1202_p->getAdjustedTime(10200, tsMsec);
    EXPECT_EQ((baseTenthsOfSeconds + 10200) % 36000, result);
}