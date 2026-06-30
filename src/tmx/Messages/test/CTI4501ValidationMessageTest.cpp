#include <gtest/gtest.h>
#include "CTI4501ValidationMessage.h"

namespace tmx::messages
{
    class CTI4501ValidationMessageTest : public testing::Test
    {
    protected:
        std::shared_ptr<CTI4501ValidationMessage> tmxCti4501Ptr;
        CTI4501ValidationMessageTest()
        {
            tmxCti4501Ptr = std::make_shared<CTI4501ValidationMessage>();
        }
        void SetUp() override
        {
            tmxCti4501Ptr->set_eventGeneratedAt(12222222222);
            tmxCti4501Ptr->set_eventType("SpatMinimumData");
            tmxCti4501Ptr->set_intersectionID(123);
            tmxCti4501Ptr->set_roadRegulatorID(456);
            tmxCti4501Ptr->set_source("RSU");
            tmxCti4501Ptr->set_timestampA("2023-01-01T00:00:00Z");
            tmxCti4501Ptr->set_timestampB("2023-01-01T00:00:00Z");
            tmxCti4501Ptr->set_messageCountA(1);
            tmxCti4501Ptr->set_messageCountB(2);
            tmxCti4501Ptr->set_topicName("topic.CmSpatBroadcastRateEvents");
            tmxCti4501Ptr->set_numberOfMessages(5);
            tmxCti4501Ptr->set_messageType("SPAT");
            ProcessingTimePeriod timePeriod(11111111111, 12222222222);
            tmxCti4501Ptr->set_timePeriod(timePeriod);
            std::vector<std::string> elements{"id", "minEndTime"};
            tmxCti4501Ptr->set_missingDataElements(elements);
        }
    };

    TEST_F(CTI4501ValidationMessageTest, attributes)
    {
        EXPECT_EQ(12222222222, tmxCti4501Ptr->get_eventGeneratedAt());
        EXPECT_EQ("SpatMinimumData", tmxCti4501Ptr->get_eventType());
        EXPECT_EQ(123, tmxCti4501Ptr->get_intersectionID());
        EXPECT_EQ(456, tmxCti4501Ptr->get_roadRegulatorID());
        EXPECT_EQ("RSU", tmxCti4501Ptr->get_source());
        EXPECT_EQ(11111111111, tmxCti4501Ptr->get_timePeriod().beginTimestamp);
        EXPECT_EQ(12222222222, tmxCti4501Ptr->get_timePeriod().endTimestamp);
        ASSERT_EQ(2, tmxCti4501Ptr->get_missingDataElements().size());
        EXPECT_EQ("id", tmxCti4501Ptr->get_missingDataElements()[0].value);
        EXPECT_EQ("minEndTime", tmxCti4501Ptr->get_missingDataElements()[1].value);
        EXPECT_EQ("2023-01-01T00:00:00Z", tmxCti4501Ptr->get_timestampA());
        EXPECT_EQ("2023-01-01T00:00:00Z", tmxCti4501Ptr->get_timestampB());
        EXPECT_EQ(1, tmxCti4501Ptr->get_messageCountA());
        EXPECT_EQ(2, tmxCti4501Ptr->get_messageCountB());
        EXPECT_EQ("topic.CmSpatBroadcastRateEvents", tmxCti4501Ptr->get_topicName());
        EXPECT_EQ(5, tmxCti4501Ptr->get_numberOfMessages());
        EXPECT_EQ("SPAT", tmxCti4501Ptr->get_messageType());
    }

    TEST_F(CTI4501ValidationMessageTest, to_string)
    {
        std::string expectedStr = "{\"eventGeneratedAt\":\"12222222222\",\"eventType\":\"SpatMinimumData\",\"intersectionID\":\"123\",\"roadRegulatorID\":\"456\",\"source\":\"RSU\",\"timestampA\":\"2023-01-01T00:00:00Z\",\"timestampB\":\"2023-01-01T00:00:00Z\",\"messageCountA\":\"1\",\"messageCountB\":\"2\",\"topicName\":\"topic.CmSpatBroadcastRateEvents\",\"numberOfMessages\":\"5\",\"messageType\":\"SPAT\",\"timePeriod\":{\"beginTimestamp\":\"11111111111\",\"endTimestamp\":\"12222222222\"},\"missingDataElements\":[\"id\",\"minEndTime\"]}\n";
        EXPECT_EQ(expectedStr, tmxCti4501Ptr->to_string());
    }

    TEST_F(CTI4501ValidationMessageTest, deserialize)
    {
        auto tmxCti4501Ptr2 = std::make_shared<CTI4501ValidationMessage>();
        std::string expectedStr = R"(
                                {
                                    "eventGeneratedAt": 12222222222,
                                    "eventType": "SpatMinimumData",
                                    "intersectionID": 123,
                                    "roadRegulatorID": 456,
                                    "source": "RSU",
                                    "timePeriod": {
                                        "beginTimestamp": 11111111111,
                                        "endTimestamp": 12222222222
                                    },
                                    "missingDataElements": [
                                        "id",
                                        "minEndTime"
                                    ],
                                    "timestampA": "2023-01-01T00:00:00Z",
                                    "timestampB": "2023-01-01T00:00:00Z",
                                    "messageCountA": 1,
                                    "messageCountB": 2,
                                    "topicName": "topic.CmSpatBroadcastRateEvents",
                                    "numberOfMessages": 5,
                                    "messageType": "SPAT"
                                }
                                )";
        tmxCti4501Ptr2->set_contents(expectedStr);
        EXPECT_EQ(expectedStr, tmxCti4501Ptr2->to_string());
        EXPECT_EQ(12222222222, tmxCti4501Ptr2->get_eventGeneratedAt());
        EXPECT_EQ("SpatMinimumData", tmxCti4501Ptr2->get_eventType());
        EXPECT_EQ(123, tmxCti4501Ptr2->get_intersectionID());
        EXPECT_EQ(456, tmxCti4501Ptr2->get_roadRegulatorID());
        EXPECT_EQ("RSU", tmxCti4501Ptr2->get_source());
        EXPECT_EQ(11111111111, tmxCti4501Ptr2->get_timePeriod().beginTimestamp);
        EXPECT_EQ(12222222222, tmxCti4501Ptr2->get_timePeriod().endTimestamp);
        ASSERT_EQ(2, tmxCti4501Ptr2->get_missingDataElements().size());
        EXPECT_EQ("id", tmxCti4501Ptr2->get_missingDataElements()[0].value);
        EXPECT_EQ("minEndTime", tmxCti4501Ptr2->get_missingDataElements()[1].value);
        EXPECT_EQ("2023-01-01T00:00:00Z", tmxCti4501Ptr2->get_timestampA());
        EXPECT_EQ("2023-01-01T00:00:00Z", tmxCti4501Ptr2->get_timestampB());
        EXPECT_EQ(1, tmxCti4501Ptr2->get_messageCountA());
        EXPECT_EQ(2, tmxCti4501Ptr2->get_messageCountB());
        EXPECT_EQ("topic.CmSpatBroadcastRateEvents", tmxCti4501Ptr2->get_topicName());
        EXPECT_EQ(5, tmxCti4501Ptr2->get_numberOfMessages());
        EXPECT_EQ("SPAT", tmxCti4501Ptr2->get_messageType());
    }

    // A BroadcastRate event carries topicName / numberOfMessages / timePeriod, but
    // no missingDataElements or message-count fields.
    class CTI4501BroadcastRateMessageTest : public testing::Test
    {
    protected:
        std::shared_ptr<CTI4501ValidationMessage> tmxCti4501Ptr;
        CTI4501BroadcastRateMessageTest()
        {
            tmxCti4501Ptr = std::make_shared<CTI4501ValidationMessage>();
        }
        void SetUp() override
        {
            tmxCti4501Ptr->set_eventGeneratedAt(12222222222);
            tmxCti4501Ptr->set_eventType("SpatBroadcastRate");
            tmxCti4501Ptr->set_intersectionID(123);
            tmxCti4501Ptr->set_roadRegulatorID(456);
            tmxCti4501Ptr->set_source("RSU");
            tmxCti4501Ptr->set_topicName("topic.CmSpatBroadcastRateEvents");
            tmxCti4501Ptr->set_numberOfMessages(2);
            ProcessingTimePeriod timePeriod(11111111111, 12222222222);
            tmxCti4501Ptr->set_timePeriod(timePeriod);
        }
    };

    TEST_F(CTI4501BroadcastRateMessageTest, attributes)
    {
        EXPECT_EQ(12222222222, tmxCti4501Ptr->get_eventGeneratedAt());
        EXPECT_EQ("SpatBroadcastRate", tmxCti4501Ptr->get_eventType());
        EXPECT_EQ(123, tmxCti4501Ptr->get_intersectionID());
        EXPECT_EQ(456, tmxCti4501Ptr->get_roadRegulatorID());
        EXPECT_EQ("RSU", tmxCti4501Ptr->get_source());
        EXPECT_EQ("topic.CmSpatBroadcastRateEvents", tmxCti4501Ptr->get_topicName());
        EXPECT_EQ(2, tmxCti4501Ptr->get_numberOfMessages());
        EXPECT_EQ(11111111111, tmxCti4501Ptr->get_timePeriod().beginTimestamp);
        EXPECT_EQ(12222222222, tmxCti4501Ptr->get_timePeriod().endTimestamp);
    }

    TEST_F(CTI4501BroadcastRateMessageTest, to_string)
    {
        std::string expectedStr = "{\"eventGeneratedAt\":\"12222222222\",\"eventType\":\"SpatBroadcastRate\",\"intersectionID\":\"123\",\"roadRegulatorID\":\"456\",\"source\":\"RSU\",\"topicName\":\"topic.CmSpatBroadcastRateEvents\",\"numberOfMessages\":\"2\",\"timePeriod\":{\"beginTimestamp\":\"11111111111\",\"endTimestamp\":\"12222222222\"}}\n";
        EXPECT_EQ(expectedStr, tmxCti4501Ptr->to_string());
    }

    // A MessageCountProgression event carries messageType / messageCountA-B /
    // timestampA-B, but no topicName, numberOfMessages, or missingDataElements.
    class CTI4501MessageCountProgressionMessageTest : public testing::Test
    {
    protected:
        std::shared_ptr<CTI4501ValidationMessage> tmxCti4501Ptr;
        CTI4501MessageCountProgressionMessageTest()
        {
            tmxCti4501Ptr = std::make_shared<CTI4501ValidationMessage>();
        }
        void SetUp() override
        {
            tmxCti4501Ptr->set_eventGeneratedAt(12222222222);
            tmxCti4501Ptr->set_eventType("SpatMessageCountProgression");
            tmxCti4501Ptr->set_intersectionID(123);
            tmxCti4501Ptr->set_roadRegulatorID(456);
            tmxCti4501Ptr->set_source("RSU");
            tmxCti4501Ptr->set_messageType("SPAT");
            tmxCti4501Ptr->set_messageCountA(7);
            tmxCti4501Ptr->set_messageCountB(9);
            tmxCti4501Ptr->set_timestampA("2023-01-01T00:00:00Z");
            tmxCti4501Ptr->set_timestampB("2023-01-01T00:00:01Z");
        }
    };

    TEST_F(CTI4501MessageCountProgressionMessageTest, attributes)
    {
        EXPECT_EQ(12222222222, tmxCti4501Ptr->get_eventGeneratedAt());
        EXPECT_EQ("SpatMessageCountProgression", tmxCti4501Ptr->get_eventType());
        EXPECT_EQ(123, tmxCti4501Ptr->get_intersectionID());
        EXPECT_EQ(456, tmxCti4501Ptr->get_roadRegulatorID());
        EXPECT_EQ("RSU", tmxCti4501Ptr->get_source());
        EXPECT_EQ("SPAT", tmxCti4501Ptr->get_messageType());
        EXPECT_EQ(7, tmxCti4501Ptr->get_messageCountA());
        EXPECT_EQ(9, tmxCti4501Ptr->get_messageCountB());
        EXPECT_EQ("2023-01-01T00:00:00Z", tmxCti4501Ptr->get_timestampA());
        EXPECT_EQ("2023-01-01T00:00:01Z", tmxCti4501Ptr->get_timestampB());
    }

    TEST_F(CTI4501MessageCountProgressionMessageTest, to_string)
    {
        std::string expectedStr = "{\"eventGeneratedAt\":\"12222222222\",\"eventType\":\"SpatMessageCountProgression\",\"intersectionID\":\"123\",\"roadRegulatorID\":\"456\",\"source\":\"RSU\",\"messageType\":\"SPAT\",\"messageCountA\":\"7\",\"messageCountB\":\"9\",\"timestampA\":\"2023-01-01T00:00:00Z\",\"timestampB\":\"2023-01-01T00:00:01Z\"}\n";
        EXPECT_EQ(expectedStr, tmxCti4501Ptr->to_string());
    }
}
