#include <gtest/gtest.h>
#include <simulation/ExternalObject.h>
#include <simulation/SimulationExternalObjectConverter.h>
#include <tmx/messages/routeable_message.hpp>
#include <boost/chrono.hpp>

using namespace std;
using namespace tmx;
using namespace tmx::messages;

TEST(SimulationMessages, ExternalObjectToRoutableMessage)
{
    simulation::ExternalObject externalObj;
    string expectedStr = "{\"metadata\":{\"is_simulation\":false,\"datum\":\"\",\"proj_string\":\"\",\"sensor_x\":0.0,\"sensor_y\":0.0,\"sensor_z\":0.0,\"infrastructure_id\":\"\",\"sensor_id\":\"\"},\"header\":{\"seq\":0,\"stamp\":{\"secs\":0,\"nsecs\":0}},\"id\":0,\"pose\":{\"pose\":{\"position\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"orientation\":{\"x\":0.0,\"y\":0.0,\"z\":0.0,\"w\":0.0}},\"covariance\":[0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0]},\"velocity\":{\"twist\":{\"linear\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"angular\":{\"x\":0.0,\"y\":0.0,\"z\":0.0}},\"covariance\":[0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0]},\"size\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"confidence\":0.0,\"object_type\":\"\",\"dynamic_obj\":false}";
    tmx::utils::sim::SimulationExternalObjectConverter converter;
    converter.jsonToSimExternalObj(expectedStr, externalObj);
    ASSERT_EQ("ExternalObject", std::string(simulation::ExternalObject::MessageSubType));
    ASSERT_EQ("Simulation", std::string(simulation::ExternalObject::MessageType));
    tmx::routeable_message routeableMsg;
    routeableMsg.initialize(externalObj, "CDASimAdapter", 0, IvpMsgFlags_None);
    ASSERT_EQ("json", routeableMsg.get_encoding());
    ASSERT_EQ(0, routeableMsg.get_flags());
    auto current_time_mill = boost::chrono::duration_cast<boost::chrono::milliseconds>(boost::chrono::system_clock::now().time_since_epoch()).count();
    ASSERT_NEAR(current_time_mill, routeableMsg.get_timestamp(), 1000);
    ASSERT_NEAR(current_time_mill, routeableMsg.get_millisecondsSinceEpoch(), 1000);
    ASSERT_EQ(0, routeableMsg.get_sourceId());
    ASSERT_EQ("CDASimAdapter", routeableMsg.get_source());
    ASSERT_EQ("ExternalObject",routeableMsg.get_subtype());
    ASSERT_EQ("Simulation", routeableMsg.get_type());
}