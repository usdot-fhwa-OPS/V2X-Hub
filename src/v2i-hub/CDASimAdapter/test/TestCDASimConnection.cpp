#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "include/CDASimConnection.hpp"
#include "include/CDASimAdapter.hpp"
#include <WGS84Point.h>
#include <MockUdpClient.h>
#include <MockUdpServer.h>
#include <filesystem>


using testing::_;
using testing::Return;
using testing::Throw;
using testing::DoDefault;
using testing::SetArgPointee;
using testing::SetArrayArgument;
using namespace tmx::utils;


namespace CDASimAdapter {

    class TestCDASimConnection : public ::testing::Test {
        protected:
            void SetUp() override {
                // Initialize CARMA Simulation connection with (0,0,0) location.
                Point location; 
                connection = std::make_shared<CDASimConnection>("127.0.0.1", "1212", 4567, 4678, "127.0.0.1", 1213, 1214, 1215, location, sensors_file_path);
            }
            void TearDown() override {

            }
        public:
            std::shared_ptr<CDASimConnection> connection;
            std::string sensors_file_path = "../../CDASimAdapter/test/sensors.json";
        

    };
    TEST_F( TestCDASimConnection, initialize) {
        ASSERT_FALSE(connection->is_connected());
    }

    TEST_F( TestCDASimConnection, forward_message) {
        std::shared_ptr<MockUpdClient> client = std::make_shared<MockUpdClient>();
        std::string test_message = "message";
        EXPECT_CALL( *client, Send(test_message) ).Times(2).WillOnce(testing::DoAll(Return(-1))).WillRepeatedly(testing::DoAll(Return(test_message.size())));
        ASSERT_THROW(connection->forward_message(test_message, client), UdpClientRuntimeError);
        connection->forward_message(test_message, client);
    }

    TEST_F( TestCDASimConnection, forward_message_invalid ) {
        std::shared_ptr<MockUpdClient> client = std::make_shared<MockUpdClient>();
        std::string test_message = "";
        // ASSERT that we never call Send message.
        EXPECT_CALL( *client, Send(test_message) ).Times(0);
        // sent empty message
        connection->forward_message(test_message,client);
        test_message = "message";
        client = nullptr;
        connection->forward_message(test_message, client);
    } 

    TEST_F( TestCDASimConnection, setup_upd_connection) {
        ASSERT_TRUE(connection->setup_udp_connection("127.0.0.1", "127.0.0.1", 4567, 4568, 4569, 4570));
    }

    TEST_F( TestCDASimConnection, get_handshake_json) {
        Point location;
        location.X = 1000;
        location.Y = 38.955; 
        location.Z = -77.149;
        std::ifstream in_strm;
        in_strm.open(sensors_file_path, std::ifstream::binary);
        if(in_strm.is_open())
        {
            EXPECT_EQ(connection->get_handshake_json("4566", "127.0.0.1", 4567, 4568, 4569, location), 
            "{\n   \"infrastructureId\" : \"4566\",\n   \"location\" : {\n      \"x\" : 1000.0,\n      \"y\" : 38.954999999999998,\n      \"z\" : -77.149000000000001\n   },\n   \"rxMessageIpAddress\" : \"127.0.0.1\",\n   \"rxMessagePort\" : 4569,\n   \"sensors\" : [\n      {\n         \"location\" : {\n            \"x\" : 0.0,\n            \"y\" : 0.0,\n            \"z\" : 0.0\n         },\n         \"orientation\" : {\n            \"pitch\" : 0.0,\n            \"roll\" : 0.0,\n            \"yaw\" : 0.0\n         },\n         \"sensorId\" : \"SomeID\",\n         \"type\" : \"SemanticLidar\"\n      },\n      {\n         \"location\" : {\n            \"x\" : 1.0,\n            \"y\" : 2.0,\n            \"z\" : 0.0\n         },\n         \"orientation\" : {\n            \"pitch\" : 0.0,\n            \"roll\" : 0.0,\n            \"yaw\" : 23.0\n         },\n         \"sensorId\" : \"SomeID2\",\n         \"type\" : \"SemanticLidar\"\n      }\n   ],\n   \"simulatedInteractionPort\" : 4568,\n   \"timeSyncPort\" : 4567\n}\n");
        }
    }

    TEST_F( TestCDASimConnection, get_handshake_json_no_sensor_config) {
        Point location;
        location.X = 1000;
        location.Y = 38.955; 
        location.Z = -77.149;
        connection.reset(new CDASimConnection("127.0.0.1", "1212", 4567, 4678, "127.0.0.1", 1213, 1214, 1215, location));
        // Test method when sensors_file_path is empty
        EXPECT_EQ(connection->get_handshake_json("4566", "127.0.0.1", 4567, 4568, 4569, location), 
            "{\n   \"infrastructureId\" : \"4566\",\n   \"location\" : {\n      \"x\" : 1000.0,\n      \"y\" : 38.954999999999998,\n      \"z\" : -77.149000000000001\n   },\n   \"rxMessageIpAddress\" : \"127.0.0.1\",\n   \"rxMessagePort\" : 4569,\n   \"simulatedInteractionPort\" : 4568,\n   \"timeSyncPort\" : 4567\n}\n");
    }

    TEST_F( TestCDASimConnection, carma_simulation_handshake) {
        Point location;
        // UDP creation error
        EXPECT_FALSE(connection->carma_simulation_handshake("", "45", 0, 
                                "",  45, 45, 45, location));
    }

    TEST_F(TestCDASimConnection, connect) {
        EXPECT_TRUE(connection->connect());
    }

    TEST_F(TestCDASimConnection, read_json_file)
    {        
        EXPECT_THROW(connection->read_json_file("Invalid_file_path" ), std::invalid_argument);
        std::ifstream in_strm;
        in_strm.open(sensors_file_path, std::ifstream::binary);
        if(in_strm.is_open())
        {
            auto sensorJsonV = connection->read_json_file(sensors_file_path );
            ASSERT_FALSE(sensorJsonV.empty());
        }        
    }

    TEST_F(TestCDASimConnection, string_to_json)
    {        
        EXPECT_THROW(connection->string_to_json("Invalid Json"), std::invalid_argument);
    }

    TEST_F(TestCDASimConnection, read_sensor_configuration_file) {
        auto sensor_registration = connection->read_sensor_configuration_file("../../CDASimAdapter/test/sensors_including_invalid_entries.json");
        EXPECT_EQ(2, sensor_registration.size());
        EXPECT_EQ("SomeID", sensor_registration[0]["sensorId"].asString());
        EXPECT_EQ("SomeID2", sensor_registration[1]["sensorId"].asString());

    }
}