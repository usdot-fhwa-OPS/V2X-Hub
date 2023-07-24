#include <simulation/SimulationSensorDetectedObjectConverter.h>
#include <gtest/gtest.h>
#include <algorithm>

using namespace tmx::utils::sim;
using namespace tmx::messages;

TEST(SimulationSensorDetectedObjectConverter, jsonToSimExternalObjInvalidJson)
{
    simulation::SensorDetectedObject simExternalObj;
    std::string invalidJsonStr = "Invalid";
    ASSERT_THROW(SimulationSensorDetectedObjectConverter::jsonToSimExternalObj(invalidJsonStr, simExternalObj), std::runtime_error);
    std::string noContentJsonStr = "Invalid";
    ASSERT_THROW(SimulationSensorDetectedObjectConverter::jsonToSimExternalObj(noContentJsonStr, simExternalObj), std::runtime_error);
}

TEST(SimulationSensorDetectedObjectConverter, jsonToSimExternalObjValidJson)
{
    simulation::SensorDetectedObject simExternalObj;
    std::string jsonStr = "{\"type\":\"Application\",\"subtype\":\"SensorDetectedObject\",\"content\":{\"timestamp\":123,\"isSimulated\":true,\"sensor\":{\"id\":\"SomeID\",\"type\":\"SematicLidar\",\"location\":{\"x\":1.0,\"y\":1.0,\"z\":2.0},\"proj_string\":\"+proj=tmerc+lat_0=38.95197911150576+lon_0=-77.14835128349988+k=1+x_0=0+y_0=0+datum=WGS84+units=m+geoidgrids=egm96_15.gtx+vunits=m+no_defs\"},\"type\":\"Car\",\"confidence\":\"0.7\",\"objectId\":\"Object1\",\"position\":{\"x\":1.0,\"y\":2.5,\"z\":1.1},\"positionCovariance\":[12.0,17.33333333,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,20.3333],\"velocity\":{\"x\":1.0,\"y\":2.5,\"z\":1.1},\"velocityCovariance\":[12.0,17.33333333,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,20.3333],\"angularVelocity\":{\"x\":1.0,\"y\":2.5,\"z\":1.1},\"angularVelocityCovariance\":[12.0,17.33333333,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,20.3333],\"size\":{\"length\":0.1,\"width\":0.4,\"height\":1.5}}}";
    SimulationSensorDetectedObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ("Object1", simExternalObj.get_Id());
    ASSERT_EQ(true, simExternalObj.get_MetadataIsSimulation());
    ASSERT_EQ("SomeID", simExternalObj.get_SensorId());
    ASSERT_EQ("SematicLidar", simExternalObj.get_SensorType());
    ASSERT_EQ("+proj=tmerc+lat_0=38.95197911150576+lon_0=-77.14835128349988+k=1+x_0=0+y_0=0+datum=WGS84+units=m+geoidgrids=egm96_15.gtx+vunits=m+no_defs", simExternalObj.get_SensorProjString());
    ASSERT_EQ(1, simExternalObj.get_SensorLocationX());
    ASSERT_EQ(1, simExternalObj.get_SensorLocationY());
    ASSERT_EQ(2, simExternalObj.get_SensorLocationZ());
    ASSERT_EQ(1, simExternalObj.get_PositionX());
    ASSERT_EQ(2.5, simExternalObj.get_PositionY());
    ASSERT_EQ(1.1, simExternalObj.get_PositionZ());
    ASSERT_EQ(36, simExternalObj.get_PositionCovariance().size());
    ASSERT_EQ(36, simExternalObj.get_PositionCovariance().size());
}

TEST(SimulationSensorDetectedObjectConverter, simExternalObjToJsonStr)
{
    std::string jsonStr = "{\"type\":\"Application\",\"subtype\":\"SensorDetectedObject\",\"content\":{\"timestamp\":123,\"isSimulated\":true,\"sensor\":{\"id\":\"SomeID\",\"type\":\"SematicLidar\",\"location\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"proj_string\":\"+proj=tmerc+lat_0=38.95197911150576+lon_0=-77.14835128349988+k=1+x_0=0+y_0=0+datum=WGS84+units=m+geoidgrids=egm96_15.gtx+vunits=m+no_defs\"},\"type\":\"Car\",\"confidence\":\"0.7\",\"objectId\":\"Object1\",\"position\":{\"x\":1.0,\"y\":2.5,\"z\":1.1},\"positionCovariance\":[12.0,17.33333333,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,20.3333],\"velocity\":{\"x\":1.0,\"y\":2.5,\"z\":1.1},\"velocityCovariance\":[12.0,17.33333333,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,20.3333],\"angularVelocity\":{\"x\":1.0,\"y\":2.5,\"z\":1.1},\"angularVelocityCovariance\":[12.0,17.33333333,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,20.3333],\"size\":{\"length\":0.1,\"width\":0.4,\"height\":1.5}}}";
    simulation::SensorDetectedObject simExternalObj;
    SimulationSensorDetectedObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    std::string output = SimulationSensorDetectedObjectConverter::simExternalObjToJsonStr(simExternalObj);
    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    JSONCPP_STRING err;
    reader->parse(output.c_str(), output.c_str() + static_cast<int>(output.length()), &root, &err);
    ASSERT_FALSE(root["metadata"]["isSimulation"].asBool());
    ASSERT_EQ(12, root["payload"]["positionCovariance"].begin()->asDouble());
    ASSERT_EQ(12, root["payload"]["velocityCovariance"].begin()->asDouble());
    ASSERT_EQ(17.33333333, root["payload"]["velocityCovariance"][1].asDouble());
}