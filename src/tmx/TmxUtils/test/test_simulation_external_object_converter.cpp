#include <simulation/SimulationExternalObjectConverter.h>
#include <gtest/gtest.h>
#include <algorithm>

using namespace tmx::utils::sim;
using namespace tmx::messages;

TEST(SimulationExternalObjectConverter, jsonToSimExternalObjInvalidJson)
{
    simulation::ExternalObject simExternalObj;
    std::string invalidJsonStr = "Invalid";
    ASSERT_THROW(SimulationExternalObjectConverter::jsonToSimExternalObj(invalidJsonStr, simExternalObj), std::runtime_error);
}

TEST(SimulationExternalObjectConverter, jsonToSimExternalObjValidJson)
{
    simulation::ExternalObject simExternalObj;
    std::string jsonStr = "{\"metadata\":{\"is_simulation\":false,\"datum\":\"\",\"proj_string\":\"\",\"sensor_x\":0.0,\"sensor_y\":0.0,\"sensor_z\":0.0,\"infrastructure_id\":\"\",\"sensor_id\":\"\"},\"header\":{\"seq\":0,\"stamp\":{\"secs\":0,\"nsecs\":0}},\"id\":0,\"pose\":{\"pose\":{\"position\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"orientation\":{\"x\":0.0,\"y\":0.0,\"z\":0.0,\"w\":0.0}},\"covariance\":[12.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0]},\"velocity\":{\"twist\":{\"linear\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"angular\":{\"x\":0.0,\"y\":0.0,\"z\":0.0}},\"covariance\":[0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0]},\"size\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"confidence\":0.0,\"object_type\":\"\",\"dynamic_obj\":false}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
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

TEST(SimulationExternalObjectConverter, jsonToSimExternalObjPresenceVector)
{
    simulation::ExternalObject simExternalObj;
    ASSERT_TRUE(simExternalObj.is_empty());
    ASSERT_EQ(simulation::PRESENCE_VECTOR_TYPES::UNAVAILABLE, simExternalObj.get_PresenceVector());
    ASSERT_FALSE(simExternalObj.is_empty());

    std::string jsonStr = "{\"presence_vector\":1}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::PRESENCE_VECTOR_TYPES::ID_PRESENCE_VECTOR, simExternalObj.get_PresenceVector());

    jsonStr = "{\"presence_vector\":2}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::PRESENCE_VECTOR_TYPES::POSE_PRESENCE_VECTOR, simExternalObj.get_PresenceVector());

    jsonStr = "{\"presence_vector\":4}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::PRESENCE_VECTOR_TYPES::VELOCITY_PRESENCE_VECTOR, simExternalObj.get_PresenceVector());

    jsonStr = "{\"presence_vector\":8}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::PRESENCE_VECTOR_TYPES::VELOCITY_INST_PRESENCE_VECTOR, simExternalObj.get_PresenceVector());

    jsonStr = "{\"presence_vector\":16}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::PRESENCE_VECTOR_TYPES::SIZE_PRESENCE_VECTOR, simExternalObj.get_PresenceVector());

    jsonStr = "{\"presence_vector\":32}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::PRESENCE_VECTOR_TYPES::CONFIDENCE_PRESENCE_VECTOR, simExternalObj.get_PresenceVector());

    jsonStr = "{\"presence_vector\":64}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::PRESENCE_VECTOR_TYPES::OBJECT_TYPE_PRESENCE_VECTOR, simExternalObj.get_PresenceVector());

    jsonStr = "{\"presence_vector\":128}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::PRESENCE_VECTOR_TYPES::BSM_ID_PRESENCE_VECTOR, simExternalObj.get_PresenceVector());

    jsonStr = "{\"presence_vector\":256}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::PRESENCE_VECTOR_TYPES::DYNAMIC_OBJ_PRESENCE, simExternalObj.get_PresenceVector());

    jsonStr = "{\"presence_vector\":512}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::PRESENCE_VECTOR_TYPES::PREDICTION_PRESENCE_VECTOR, simExternalObj.get_PresenceVector());

    jsonStr = "{\"presence_vector\":212121}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::PRESENCE_VECTOR_TYPES::UNAVAILABLE, simExternalObj.get_PresenceVector());
}

TEST(SimulationExternalObjectConverter, jsonToSimExternalObjObjectTypes)
{
    simulation::ExternalObject simExternalObj;
    ASSERT_TRUE(simExternalObj.is_empty());
    ASSERT_EQ(simulation::OBJECT_TYPES::UNKNOWN, simExternalObj.get_ObjectType());
    ASSERT_FALSE(simExternalObj.is_empty());

    std::string jsonStr = "{\"object_type\":\"1\"}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::OBJECT_TYPES::SMALL_VEHICLE, simExternalObj.get_ObjectType());

    jsonStr = "{\"object_type\":\"2\"}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::OBJECT_TYPES::LARGE_VEHICLE, simExternalObj.get_ObjectType());

    jsonStr = "{\"object_type\":\"3\"}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::OBJECT_TYPES::MOTORCYCLE, simExternalObj.get_ObjectType());

    jsonStr = "{\"object_type\":\"5\"}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::OBJECT_TYPES::UNKNOWN, simExternalObj.get_ObjectType());

    jsonStr = "{\"object_type\":\"4\"}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::OBJECT_TYPES::PEDESTRIAN, simExternalObj.get_ObjectType());

    jsonStr = "{\"object_type\":\"invalid\"}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    ASSERT_EQ(simulation::OBJECT_TYPES::UNKNOWN, simExternalObj.get_ObjectType());
}

TEST(SimulationExternalObjectConverter, simExternalObjToJsonStr)
{
    simulation::ExternalObject simExternalObj;
    std::string jsonStr = "{\"metadata\":{\"is_simulation\":false,\"datum\":\"\",\"proj_string\":\"\",\"sensor_x\":0.0,\"sensor_y\":0.0,\"sensor_z\":0.0,\"infrastructure_id\":\"\",\"sensor_id\":\"\"},\"header\":{\"seq\":0,\"stamp\":{\"secs\":0,\"nsecs\":0}},\"id\":0,\"pose\":{\"pose\":{\"position\":{\"x\":34.0,\"y\":0.0,\"z\":0.0},\"orientation\":{\"x\":23.0,\"y\":0.0,\"z\":0.0,\"w\":0.0}},\"covariance\":[12.1,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,11.0]},\"velocity\":{\"twist\":{\"linear\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"angular\":{\"x\":0.0,\"y\":0.0,\"z\":0.0}},\"covariance\":[12.0,17.33333333,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,20.3333]},\"size\":{\"x\":12.0,\"y\":23.0,\"z\":12.0},\"confidence\":1.0,\"object_type\":\"128\",\"dynamic_obj\":false}";
    SimulationExternalObjectConverter::jsonToSimExternalObj(jsonStr, simExternalObj);
    std::string output = SimulationExternalObjectConverter::simExternalObjToJsonStr(simExternalObj);
    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    JSONCPP_STRING err;
    reader->parse(output.c_str(), output.c_str() + static_cast<int>(output.length()), &root, &err);
    ASSERT_FALSE(root["metadata"]["is_simulation"].asBool());
    ASSERT_EQ(12.1, root["pose"]["covariance"].begin()->asDouble());
    ASSERT_EQ(23, root["pose"]["pose"]["orientation"]["x"].asDouble());
    ASSERT_EQ(12, root["velocity"]["covariance"].begin()->asDouble());
    ASSERT_EQ(17.33333333, root["velocity"]["covariance"][1].asDouble());
}