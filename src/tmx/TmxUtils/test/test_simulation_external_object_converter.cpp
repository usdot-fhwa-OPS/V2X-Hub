#include <simulation/SimulationExternalObjectConverter.h>
#include <gtest/gtest.h>
#include <algorithm>

using namespace tmx::utils::sim;
using namespace tmx::messages;

TEST(SimulationExternalObjectConverter, jsonToSimExternalObj)
{
    SimulationExternalObjectConverter converter;
    simulation::ExternalObject simExternalObj;
    std::string json_string = "{\"metadata\":{\"is_simulation\":false,\"datum\":\"\",\"proj_string\":\"\",\"sensor_x\":0.0,\"sensor_y\":0.0,\"sensor_z\":0.0,\"infrastructure_id\":\"\",\"sensor_id\":\"\"},\"header\":{\"seq\":0,\"stamp\":{\"secs\":0,\"nsecs\":0}},\"id\":0,\"pose\":{\"pose\":{\"position\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"orientation\":{\"x\":0.0,\"y\":0.0,\"z\":0.0,\"w\":0.0}},\"covariance\":[12.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0]},\"velocity\":{\"twist\":{\"linear\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"angular\":{\"x\":0.0,\"y\":0.0,\"z\":0.0}},\"covariance\":[0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0]},\"size\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"confidence\":0.0,\"object_type\":\"\",\"dynamic_obj\":false}";
    converter.jsonToSimExternalObj(json_string, simExternalObj);
    ASSERT_EQ(0, simExternalObj.get_Id());
    ASSERT_EQ(false, simExternalObj.get_MetadataIsSimulation());
    ASSERT_EQ("", simExternalObj.get_MetadataDatum());
    ASSERT_EQ("", simExternalObj.get_MetadataSensorId());
    ASSERT_EQ("", simExternalObj.get_MetadataInfrastructureId());
    ASSERT_EQ(0, simExternalObj.get_MetadataSensorX());
    ASSERT_EQ(0, simExternalObj.get_MetadataSensorY());
    ASSERT_EQ(0, simExternalObj.get_MetadataSensorZ());
    ASSERT_EQ(0, simExternalObj.get_HeaderSeq());
    ASSERT_EQ(0, simExternalObj.get_HeaderStampSecs());
    ASSERT_EQ(0, simExternalObj.get_HeaderStampNSecs());
    ASSERT_EQ(0, simExternalObj.get_PosePosePositionX());
    ASSERT_EQ(0, simExternalObj.get_PosePoseOrientationX());
    ASSERT_EQ(36, simExternalObj.get_PoseCovariance().size());
    ASSERT_EQ(0, simExternalObj.get_VelocityInstTwistLinearX());
    ASSERT_EQ(0, simExternalObj.get_VelocityInstTwistAngularX());
    ASSERT_EQ(36, simExternalObj.get_VelocityCovariance().size());
}

TEST(SimulationExternalObjectConverter, simExternalObjToJsonStr)
{
    SimulationExternalObjectConverter converter;
    simulation::ExternalObject simExternalObj;
    std::string json_string = "{\"metadata\":{\"is_simulation\":false,\"datum\":\"\",\"proj_string\":\"\",\"sensor_x\":0.0,\"sensor_y\":0.0,\"sensor_z\":0.0,\"infrastructure_id\":\"\",\"sensor_id\":\"\"},\"header\":{\"seq\":0,\"stamp\":{\"secs\":0,\"nsecs\":0}},\"id\":0,\"pose\":{\"pose\":{\"position\":{\"x\":34.0,\"y\":0.0,\"z\":0.0},\"orientation\":{\"x\":23.0,\"y\":0.0,\"z\":0.0,\"w\":0.0}},\"covariance\":[12.1,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,11.0]},\"velocity\":{\"twist\":{\"linear\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"angular\":{\"x\":0.0,\"y\":0.0,\"z\":0.0}},\"covariance\":[12.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0]},\"size\":{\"x\":12.0,\"y\":23.0,\"z\":12.0},\"confidence\":1.0,\"object_type\":\"128\",\"dynamic_obj\":false}";
    converter.jsonToSimExternalObj(json_string, simExternalObj);
    std::string output = converter.simExternalObjToJsonStr(simExternalObj);
    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    JSONCPP_STRING err;
    reader->parse(output.c_str(), output.c_str() + static_cast<int>(output.length()), &root, &err);
    ASSERT_FALSE(root["metadata"]["is_simulation"].asBool());
    ASSERT_EQ(12.1, root["pose"]["covariance"].begin()->asDouble());
    ASSERT_EQ(23, root["pose"]["pose"]["orientation"]["x"].asDouble());
}