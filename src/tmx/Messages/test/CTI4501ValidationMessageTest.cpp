#include <gtest/gtest.h>
#include "CTI4501ValidationMessage.h"

namespace tmx::messages{
    class CTI4501ValidationMessageTest : public testing::Test{
        protected:  
        std::shared_ptr<CTI4501ValidationMessage> tmxCti4501Ptr; 
        CTI4501ValidationMessageTest(){
            tmxCti4501Ptr = std::make_shared<CTI4501ValidationMessage>();
        }         
        void SetUp() override {
            tmxCti4501Ptr->set_eventGeneratedAt(12222222222);
            tmxCti4501Ptr->set_eventType("SpatMinimumData");
            tmxCti4501Ptr->set_intersectionID(123);
            tmxCti4501Ptr->set_roadRegulatorID(456);
            tmxCti4501Ptr->set_source("RSU");
            ProcessingTimePeriod timePeriod(11111111111, 12222222222);
            tmxCti4501Ptr->set_timePeriod(timePeriod);
            std::vector<MissingDataElement> elements { MissingDataElement("id"), MissingDataElement("minEndTime") };
            tmxCti4501Ptr->set_missingDataElements(elements);
        }

        private:
            
    };

    TEST_F(CTI4501ValidationMessageTest, attributes){    
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
    }

    TEST_F(CTI4501ValidationMessageTest, to_string){
        std::string expectedStr = "{\"eventGeneratedAt\":\"12222222222\",\"eventType\":\"SpatMinimumData\",\"intersectionID\":\"123\",\"roadRegulatorID\":\"456\",\"source\":\"RSU\",\"timePeriod\":{\"beginTimestamp\":\"11111111111\",\"endTimestamp\":\"12222222222\"},\"missingDataElements\":[\"id\",\"minEndTime\"]}\n";
        EXPECT_EQ(expectedStr, tmxCti4501Ptr->to_string());
    }

    TEST_F(CTI4501ValidationMessageTest, deserialize){
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
                                    ]
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
    }
}