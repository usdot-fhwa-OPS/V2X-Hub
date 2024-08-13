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
            tmxSdsmPtr->set_isSimulated(false);
            Position pos(1.0, 2.3, 2.0);
            tmxSdsmPtr->set_position(pos);
            tmxSdsmPtr->set_projString("+proj=tmerc +lat_0=38.95197911150576 +lon_0=-77.14835128349988 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs");
            tmxSdsmPtr->set_timestamp(12222222222);
            tmxSdsmPtr->set_sensorId("SomeID");
            tmxSdsmPtr->set_type("Car");
            tmxSdsmPtr->set_confidence(0.7);
            tmxSdsmPtr->set_objectId(123);

            Velocity vel(1.0, 0.3, 2.0);
            tmxSdsmPtr->set_velocity(vel);
            tmxSdsmPtr->set_angularVelocity(vel);

            std::vector<Covariance> covs { 
                Covariance(12), 
                Covariance(11),
                Covariance(13), 
                Covariance(14),
                Covariance(15)
                };
            int covarianceSize = 3;
            std::vector<std::vector<Covariance>> covs2d;
            for(int i=0; i<covarianceSize; i++){
                covs2d.push_back(covs);
            }
            tmxSdsmPtr->set_positionCovariance(covs2d);
            tmxSdsmPtr->set_velocityCovariance(covs2d);
            tmxSdsmPtr->set_angularVelocityCovariance(covs2d);
        }
    };

    TEST_F(SensorDetectedObjectTest, attributes){    
        EXPECT_EQ(false, tmxSdsmPtr->get_isSimulated());
        EXPECT_EQ(0.7, tmxSdsmPtr->get_confidence());
        EXPECT_EQ("SomeID", tmxSdsmPtr->get_sensorId());
        EXPECT_EQ(12222222222, tmxSdsmPtr->get_timestamp());
        EXPECT_EQ(123, tmxSdsmPtr->get_objectId());
        EXPECT_EQ("+proj=tmerc +lat_0=38.95197911150576 +lon_0=-77.14835128349988 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs", tmxSdsmPtr->get_projString());
        EXPECT_EQ(1.0, tmxSdsmPtr->get_position().x);
        EXPECT_NEAR(2.3, tmxSdsmPtr->get_position().y, 0.01);
        EXPECT_EQ(2.0, tmxSdsmPtr->get_position().z);
        EXPECT_EQ(1.0, tmxSdsmPtr->get_velocity().x);
        EXPECT_NEAR(0.3, tmxSdsmPtr->get_velocity().y, 0.01);
        EXPECT_EQ(2.0, tmxSdsmPtr->get_velocity().z);
        EXPECT_EQ(1.0, tmxSdsmPtr->get_angularVelocity().x);
        EXPECT_NEAR(0.3, tmxSdsmPtr->get_angularVelocity().y, 0.01);
        EXPECT_EQ(2.0, tmxSdsmPtr->get_angularVelocity().z);
        EXPECT_EQ(3, tmxSdsmPtr->get_positionCovariance().size());
        EXPECT_EQ(3, tmxSdsmPtr->get_angularVelocityCovariance().size());
        EXPECT_EQ(3, tmxSdsmPtr->get_velocityCovariance().size());
    }   
     
    TEST_F(SensorDetectedObjectTest, to_string){
        std::string expectedStr = "{\"isSimulated\":\"0\",\"position\":{\"x\":\"1\",\"y\":\"2.2999999999999998\",\"z\":\"2\"},\"projString\":\"+proj=tmerc +lat_0=38.95197911150576 +lon_0=-77.14835128349988 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs\",\"timestamp\":\"12222222222\",\"sensorId\":\"SomeID\",\"type\":\"Car\",\"confidence\":\"0.69999999999999996\",\"objectId\":\"123\",\"velocity\":{\"x\":\"1\",\"y\":\"0.29999999999999999\",\"z\":\"2\"},\"angularVelocity\":{\"x\":\"1\",\"y\":\"0.29999999999999999\",\"z\":\"2\"},\"positionCovariance\":[[\"12\",\"11\",\"13\",\"14\",\"15\"],[\"12\",\"11\",\"13\",\"14\",\"15\"],[\"12\",\"11\",\"13\",\"14\",\"15\"]],\"velocityCovariance\":[[\"12\",\"11\",\"13\",\"14\",\"15\"],[\"12\",\"11\",\"13\",\"14\",\"15\"],[\"12\",\"11\",\"13\",\"14\",\"15\"]],\"angularVelocityCovariance\":[[\"12\",\"11\",\"13\",\"14\",\"15\"],[\"12\",\"11\",\"13\",\"14\",\"15\"],[\"12\",\"11\",\"13\",\"14\",\"15\"]]}\n";
        EXPECT_EQ(expectedStr, tmxSdsmPtr->to_string());
    }

    TEST_F(SensorDetectedObjectTest, deserialize){
        auto tmxSdsmPtr2 = std::make_shared<SensorDetectedObject>();
        std::string expectedStr = "{\"isSimulated\":1,\"type\":\"CAR\",\"confidence\":1.0,\"sensorId\":\"IntersectionLidar\",\"projString\":\"+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs\",\"objectId\":207,\"position\":{\"x\":-5.021,\"y\":64.234,\"z\":-10.297},\"positionCovariance\":[[0.04000000000000001,0.0,0.0],[0.0,0.04000000000000001,0.0],[0.0,0.0,0.04000000000000001]],\"velocity\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"velocityCovariance\":[[0.04000000000000001,0.0,0.0],[0.0,0.04000000000000001,0.0],[0.0,0.0,0.04000000000000001]],\"angularVelocity\":{\"x\":0.0,\"y\":-0.0,\"z\":-0.0},\"angularVelocityCovariance\":[[0.010000000000000002,0.0,0.0],[0.0,0.010000000000000002,0.0],[0.0,0.0,0.010000000000000002]],\"size\":{\"length\":2.257,\"height\":1.003,\"width\":0.762},\"timestamp\":110200}";
        tmxSdsmPtr2->set_contents(expectedStr);
        EXPECT_EQ(expectedStr, tmxSdsmPtr2->to_string());
        EXPECT_EQ(true, tmxSdsmPtr2->get_isSimulated());
        EXPECT_EQ(-5.021, tmxSdsmPtr2->get_position().x);
        EXPECT_NEAR(64.234, tmxSdsmPtr2->get_position().y, 0.01);
        EXPECT_EQ(-10.297, tmxSdsmPtr2->get_position().z);
        EXPECT_EQ("CAR", tmxSdsmPtr2->get_type());
        EXPECT_EQ(1.0, tmxSdsmPtr2->get_confidence());
        EXPECT_EQ("IntersectionLidar", tmxSdsmPtr2->get_sensorId());
        EXPECT_EQ("+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs", tmxSdsmPtr2->get_projString());
        EXPECT_EQ(207, tmxSdsmPtr2->get_objectId());
        EXPECT_EQ(0.0, tmxSdsmPtr2->get_velocity().x);
        EXPECT_EQ(0.0, tmxSdsmPtr2->get_velocity().y);
        EXPECT_EQ(0.0, tmxSdsmPtr2->get_velocity().z);
        EXPECT_EQ(0.0, tmxSdsmPtr2->get_angularVelocity().x);
        EXPECT_EQ(-0.0, tmxSdsmPtr2->get_angularVelocity().y);
        EXPECT_EQ(-0.0, tmxSdsmPtr2->get_angularVelocity().z);
        EXPECT_EQ(2.257, tmxSdsmPtr2->get_size().length);
        EXPECT_EQ(1.003, tmxSdsmPtr2->get_size().height);
        EXPECT_EQ(0.762, tmxSdsmPtr2->get_size().width);
        EXPECT_EQ(110200, tmxSdsmPtr2->get_timestamp());

        EXPECT_EQ(3, tmxSdsmPtr2->get_positionCovariance().size());
        EXPECT_EQ(3, tmxSdsmPtr2->get_positionCovariance().begin()->size());
        EXPECT_EQ(3, tmxSdsmPtr2->get_angularVelocityCovariance().size());
        EXPECT_EQ(3, tmxSdsmPtr2->get_angularVelocityCovariance().begin()->size());
        EXPECT_EQ(3, tmxSdsmPtr2->get_velocityCovariance().size());
        EXPECT_EQ(3, tmxSdsmPtr2->get_velocityCovariance().begin()->size());
        
        EXPECT_NEAR(0.04,tmxSdsmPtr2->get_positionCovariance().begin()->begin()->value, 0.0001);
        EXPECT_EQ(0.0, tmxSdsmPtr2->get_positionCovariance().begin()->back().value);

        EXPECT_NEAR(0.04, tmxSdsmPtr2->get_velocityCovariance().begin()->begin()->value, 0.0001);
        EXPECT_EQ(0.0, tmxSdsmPtr2->get_velocityCovariance().begin()->back().value);

        EXPECT_NEAR(0.01, tmxSdsmPtr2->get_angularVelocityCovariance().begin()->begin()->value, 0.0001);
        EXPECT_EQ(0.0, tmxSdsmPtr2->get_angularVelocityCovariance().begin()->back().value);
    }
}