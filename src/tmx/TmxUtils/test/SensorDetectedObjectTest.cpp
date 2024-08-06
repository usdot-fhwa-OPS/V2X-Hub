#include <gtest/gtest.h>
#include "SensorDetectedObject.h"
namespace tmx::messages{
    TEST(SensorDetectedObjectTest, attributes){
        SensorDetectedObject tmxSdsm;
        tmxSdsm.set_ISSimulated(false);
        SensorDetectedObject::Position pos(1.0, 2.3, 2.0);
        tmxSdsm.set_Position(pos);

        tmxSdsm.set_ProjString("+proj=tmerc +lat_0=38.95197911150576 +lon_0=-77.14835128349988 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs");
        tmxSdsm.set_Timestamp(12222222222);
        tmxSdsm.set_SensorId("SomeID");
        tmxSdsm.set_Type("Car");
        tmxSdsm.set_Confidence(0.7);
        tmxSdsm.set_ObjectId(123);

        SensorDetectedObject::Velocity vel(1.0, 0.3, 2.0);
        tmxSdsm.set_Velocity(vel);
        tmxSdsm.set_AngularVelocity(vel);

        std::vector<SensorDetectedObject::Covariance> covs { 
            SensorDetectedObject::Covariance("a11"), 
            SensorDetectedObject::Covariance("a12"),
            SensorDetectedObject::Covariance("a13"), 
            SensorDetectedObject::Covariance("a21"),
            SensorDetectedObject::Covariance("a22"), 
            SensorDetectedObject::Covariance("a23"),
            SensorDetectedObject::Covariance("a31"), 
            SensorDetectedObject::Covariance("a32"),
            SensorDetectedObject::Covariance("a33"), 
            SensorDetectedObject::Covariance("a41")};
        tmxSdsm.set_PositionCovariance(covs);
        tmxSdsm.set_VelocityCovariance(covs);
        tmxSdsm.set_AngularVelocityCovariance(covs);

        EXPECT_EQ(false, tmxSdsm.get_ISSimulated());
        EXPECT_EQ(0.7, tmxSdsm.get_Confidence());
        EXPECT_EQ("SomeID", tmxSdsm.get_SensorId());
        EXPECT_EQ(12222222222, tmxSdsm.get_Timestamp());
        EXPECT_EQ(123, tmxSdsm.get_ObjectId());
        EXPECT_EQ("+proj=tmerc +lat_0=38.95197911150576 +lon_0=-77.14835128349988 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +geoidgrids=egm96_15.gtx +vunits=m +no_defs", tmxSdsm.get_ProjString());
        EXPECT_EQ(1.0, tmxSdsm.get_Velocity().x);
        EXPECT_NEAR(0.3, tmxSdsm.get_Velocity().y, 0.01);
        EXPECT_EQ(2.0, tmxSdsm.get_Velocity().z);
        EXPECT_EQ(1.0, tmxSdsm.get_AngularVelocity().x);
        EXPECT_NEAR(0.3, tmxSdsm.get_AngularVelocity().y, 0.01);
        EXPECT_EQ(2.0, tmxSdsm.get_AngularVelocity().z);
        EXPECT_EQ(10, tmxSdsm.get_PositionCovariance().size());
        EXPECT_EQ(10, tmxSdsm.get_AngularVelocityCovariance().size());
        EXPECT_EQ(10, tmxSdsm.get_VelocityCovariance().size());
    }
}