#include <gtest/gtest.h>
#include "SensorDetectedObject.h"

namespace tmx::messages{
    class SensorDetectedObjectTest : public testing::Test{
        protected:
                std::shared_ptr<SensorDetectedObject> tmxSdsmPtr;   
        SensorDetectedObjectTest(){
            tmxSdsmPtr = std::make_shared<SensorDetectedObject>();
        }         
        void SetUp() override {
            tmxSdsmPtr->set_ISSimulated(false);
            Position pos(1.0, 2.3, 2.0);
            tmxSdsmPtr->set_Position(pos);
            tmxSdsmPtr->set_ProjString("+proj=tmerc +lat_0=38.95197911150576 +lon_0=-77.14835128349988 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs");
            tmxSdsmPtr->set_Timestamp(12222222222);
            tmxSdsmPtr->set_SensorId("SomeID");
            tmxSdsmPtr->set_Type("Car");
            tmxSdsmPtr->set_Confidence(0.7);
            tmxSdsmPtr->set_ObjectId(123);

            Velocity vel(1.0, 0.3, 2.0);
            tmxSdsmPtr->set_Velocity(vel);
            tmxSdsmPtr->set_AngularVelocity(vel);

            std::vector<Covariance> covs { 
                Covariance("a11"), 
                Covariance("a12"),
                Covariance("a13"), 
                Covariance("a21"),
                Covariance("a22"), 
                Covariance("a23"),
                Covariance("a31"), 
                Covariance("a32"),
                Covariance("a33"), 
                Covariance("a41")};
            tmxSdsmPtr->set_PositionCovariance(covs);
            tmxSdsmPtr->set_VelocityCovariance(covs);
            tmxSdsmPtr->set_AngularVelocityCovariance(covs);
        }
    };

    TEST_F(SensorDetectedObjectTest, attributes){    
        EXPECT_EQ(false, tmxSdsmPtr->get_ISSimulated());
        EXPECT_EQ(0.7, tmxSdsmPtr->get_Confidence());
        EXPECT_EQ("SomeID", tmxSdsmPtr->get_SensorId());
        EXPECT_EQ(12222222222, tmxSdsmPtr->get_Timestamp());
        EXPECT_EQ(123, tmxSdsmPtr->get_ObjectId());
        EXPECT_EQ("+proj=tmerc +lat_0=38.95197911150576 +lon_0=-77.14835128349988 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs", tmxSdsmPtr->get_ProjString());
        EXPECT_EQ(1.0, tmxSdsmPtr->get_Position().x);
        EXPECT_NEAR(2.3, tmxSdsmPtr->get_Position().y, 0.01);
        EXPECT_EQ(2.0, tmxSdsmPtr->get_Position().z);
        EXPECT_EQ(1.0, tmxSdsmPtr->get_Velocity().x);
        EXPECT_NEAR(0.3, tmxSdsmPtr->get_Velocity().y, 0.01);
        EXPECT_EQ(2.0, tmxSdsmPtr->get_Velocity().z);
        EXPECT_EQ(1.0, tmxSdsmPtr->get_AngularVelocity().x);
        EXPECT_NEAR(0.3, tmxSdsmPtr->get_AngularVelocity().y, 0.01);
        EXPECT_EQ(2.0, tmxSdsmPtr->get_AngularVelocity().z);
        EXPECT_EQ(10, tmxSdsmPtr->get_PositionCovariance().size());
        EXPECT_EQ(10, tmxSdsmPtr->get_AngularVelocityCovariance().size());
        EXPECT_EQ(10, tmxSdsmPtr->get_VelocityCovariance().size());
    }   
     
    TEST_F(SensorDetectedObjectTest, to_string){
        std::string expectedStr = "{\"ISSimulated\":\"0\",\"Position\":{\"x\":\"1\",\"y\":\"2.2999999999999998\",\"z\":\"2\"},\"ProjString\":\"+proj=tmerc +lat_0=38.95197911150576 +lon_0=-77.14835128349988 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs\",\"Timestamp\":\"12222222222\",\"SensorId\":\"SomeID\",\"Type\":\"Car\",\"Confidence\":\"0.69999999999999996\",\"ObjectId\":\"123\",\"Velocity\":{\"x\":\"1\",\"y\":\"0.29999999999999999\",\"z\":\"2\"},\"AngularVelocity\":{\"x\":\"1\",\"y\":\"0.29999999999999999\",\"z\":\"2\"},\"PositionCovariance\":[\"a11\",\"a12\",\"a13\",\"a21\",\"a22\",\"a23\",\"a31\",\"a32\",\"a33\",\"a41\"],\"VelocityCovariance\":[\"a11\",\"a12\",\"a13\",\"a21\",\"a22\",\"a23\",\"a31\",\"a32\",\"a33\",\"a41\"],\"AngularVelocityCovariance\":[\"a11\",\"a12\",\"a13\",\"a21\",\"a22\",\"a23\",\"a31\",\"a32\",\"a33\",\"a41\"]}\n";
        EXPECT_EQ(expectedStr, tmxSdsmPtr->to_string());
    }

    TEST_F(SensorDetectedObjectTest, deserialize){
        auto tmxSdsmPtr2 = std::make_shared<SensorDetectedObject>();
        std::string expectedStr = "{\"ISSimulated\":\"1\",\"Position\":{\"x\":\"1\",\"y\":\"2.5\",\"z\":\"2\"},\"ProjString\":\"+proj=tmerc +lat_0=38.95197911150576 +lon_0=-77.14835128349988 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs\",\"Timestamp\":\"12222222222\",\"SensorId\":\"SomeID\",\"Type\":\"Car\",\"Confidence\":\"0.7\",\"ObjectId\":\"123\",\"Velocity\":{\"x\":\"1\",\"y\":\"0.5\",\"z\":\"2\"},\"AngularVelocity\":{\"x\":\"1\",\"y\":\"0.3\",\"z\":\"2\"},\"PositionCovariance\":[\"a11\",\"a12\",\"a13\",\"a21\",\"a22\",\"a23\",\"a31\",\"a32\",\"a33\",\"a41\"],\"VelocityCovariance\":[\"a11\",\"a12\",\"a13\",\"a21\",\"a22\",\"a23\",\"a31\",\"a32\",\"a33\",\"a41\"],\"AngularVelocityCovariance\":[\"a11\",\"a12\",\"a13\",\"a21\",\"a22\",\"a23\",\"a31\",\"a32\",\"a33\",\"a41\"]}\n";
        tmxSdsmPtr2->set_contents(expectedStr);
        EXPECT_EQ(expectedStr, tmxSdsmPtr2->to_string());
        EXPECT_EQ(true, tmxSdsmPtr2->get_ISSimulated());
        EXPECT_EQ(1.0, tmxSdsmPtr2->get_Position().x);
        EXPECT_NEAR(2.5, tmxSdsmPtr2->get_Position().y, 0.01);
        EXPECT_EQ(2.0, tmxSdsmPtr2->get_Position().z);
    }
}