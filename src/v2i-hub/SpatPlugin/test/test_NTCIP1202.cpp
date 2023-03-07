
#include <gtest/gtest.h>
#include <tmx/j2735_messages/SpatMessage.hpp>
#include <NTCIP1202.h>

using namespace fwha_stol::lib::time;

TEST(NTCIP1202Test, copyBytesIntoNtcip1202)
{
    DescriptiveName_t *update_to_intersection_name = (DescriptiveName_t *)calloc(1, sizeof(DescriptiveName_t));
    char *my_string = (char*) "test intersection name";
    stringstream ss;
    update_to_intersection_name->buf = reinterpret_cast<uint8_t *>(my_string);
    ss << update_to_intersection_name->buf;
    ASSERT_EQ(ss.str(), "test intersection name");

    IntersectionReferenceID_t *update_to_intersection_id = (IntersectionReferenceID_t *)calloc(1, sizeof(IntersectionReferenceID_t));
    update_to_intersection_id->id = 9012;

    auto clock = std::make_shared<CarmaClock>();
    clock->wait_for_initialization();
    auto ntcip1202_p = std::make_shared<Ntcip1202>(clock);
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
    ntcip1202_p->ToJ2735r41SPAT(spat_ptr, reinterpret_cast<char *>(update_to_intersection_name->buf), update_to_intersection_id->id);
    ASSERT_EQ(3,  spat_ptr->intersections.list.array[0]->states.list.array[0]->state_time_speed.list.array[0]->eventState);
    free(spat_ptr);
}

TEST(NTCIP1202Test, ToJ2735r41SPAT)
{
    auto clock = std::make_shared<CarmaClock>();
    clock->wait_for_initialization();
    auto ntcip1202_p = std::make_shared<Ntcip1202>(clock);
    SPAT *spat_ptr = (SPAT *)calloc(1, sizeof(SPAT));

    char *my_string = (char*)"test intersection name";
    stringstream ss;
   
    DescriptiveName_t *update_to_intersection_name = (DescriptiveName_t *)calloc(1, sizeof(DescriptiveName_t));
    update_to_intersection_name->buf = reinterpret_cast<uint8_t *>(my_string);
    ss << update_to_intersection_name->buf;
    ASSERT_EQ(ss.str(), "test intersection name");

    IntersectionReferenceID_t *update_to_intersection_id = (IntersectionReferenceID_t *)calloc(1, sizeof(IntersectionReferenceID_t));
    update_to_intersection_id->id = 9012;


    bool transform_status = ntcip1202_p->ToJ2735r41SPAT(spat_ptr, reinterpret_cast<char *>(update_to_intersection_name->buf), update_to_intersection_id->id);
    auto _spatMessage = std::make_shared<tmx::messages::SpatMessage>(spat_ptr);
    auto spat = _spatMessage->get_j2735_data();
    ASSERT_EQ(transform_status, true);
}

TEST(NTCIP1202Test, TestAdjustedTime)
{
    auto clock = std::make_shared<CarmaClock>(true);
    // 1677775434 = 2023-02-03 16:43:54
    timeStampMilliseconds tsMsec = ((uint64_t)1677775434 * 1000) + 400;
    auto baseTenthsOfSeconds = 43 * 600 + 54 * 10 + 4;
    clock->update(tsMsec);
    auto ntcip1202_p = std::make_shared<Ntcip1202>(clock);
    auto result = ntcip1202_p->getAdjustedTime(0);
    EXPECT_EQ(baseTenthsOfSeconds, result);
    result = ntcip1202_p->getAdjustedTime(46);
    EXPECT_EQ(baseTenthsOfSeconds + 46, result);
    // cross minute boundary
    result = ntcip1202_p->getAdjustedTime(200);
    EXPECT_EQ(baseTenthsOfSeconds + 200, result);
    // cross hour boundary
    result = ntcip1202_p->getAdjustedTime(10200);
    EXPECT_EQ((baseTenthsOfSeconds + 10200) % 36000, result);
}