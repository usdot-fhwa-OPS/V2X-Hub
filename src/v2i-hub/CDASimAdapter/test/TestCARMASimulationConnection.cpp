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

    class TestCARMASimulationConnection : public ::testing::Test {
        protected:
            void SetUp() override {
                // Initialize CARMA Simulation connection with (0,0,0) location and mock kafka producer.
                Point location; 
                connection = std::make_shared<CDASimConnection>("127.0.0.1", "1212", 4567, 4678, "127.0.0.1", 1213, 1214, location, sensors_file_path);
            }
            void TearDown() override {

            }
        public:
            std::shared_ptr<CDASimConnection> connection;
            std::string sensors_file_path = "./sensors.json";
        

    };
    TEST_F( TestCARMASimulationConnection, initialize) {
        ASSERT_FALSE(connection->is_connected());
    }

    TEST_F( TestCARMASimulationConnection, forward_message) {
        std::shared_ptr<MockUpdClient> client = std::make_shared<MockUpdClient>();
        std::string test_message = "message";
        EXPECT_CALL( *client, Send(test_message) ).Times(2).WillOnce(testing::DoAll(Return(-1))).WillRepeatedly(testing::DoAll(Return(test_message.size())));
        ASSERT_THROW(connection->forward_message(test_message, client), UdpClientRuntimeError);
        connection->forward_message(test_message, client);
    }

    TEST_F( TestCARMASimulationConnection, forward_message_invalid ) {
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

    TEST_F( TestCARMASimulationConnection, consume_msg){

        std::shared_ptr<MockUpdServer> server = std::make_shared<MockUpdServer>();
        char *msg_data  = new char();
        char *test_string = "Test Message";
        EXPECT_CALL( *server, TimedReceive(_, _, _) ).Times(2).
            WillOnce(testing::DoAll(Return(-1))).
            WillRepeatedly( testing::DoAll( SetArrayArgument<0>(test_string, test_string + strlen(test_string) + 1),Return(10)));
        ASSERT_THROW(connection->consume_server_message(server), UdpServerRuntimeError);

        std::string msg = connection->consume_server_message(server);

        std::string compare_str;
        compare_str = test_string;
        ASSERT_EQ(compare_str.compare( msg ) , 0);
        delete msg_data;

    }

    TEST_F( TestCARMASimulationConnection, setup_upd_connection) {
        ASSERT_TRUE(connection->setup_udp_connection("127.0.0.1", "127.0.0.1", 4567, 4568, 4569));
    }

    TEST_F( TestCARMASimulationConnection, get_handshake_json) {
        Point location;
        location.X = 1000;
        location.Y = 38.955; 
        location.Z = -77.149;
        Json::Value sensorsJsonV;
        ASSERT_EQ(connection->get_handshake_json("4566", "127.0.0.1", 4567, 4568, location, sensorsJsonV), 
        "{\n   \"infrastructureId\" : \"4566\",\n   \"location\" : {\n      \"x\" : 1000.0,\n      \"y\" : 38.954999999999998,\n      \"z\" : -77.149000000000001\n   },\n   \"rxMessageIpAddress\" : \"127.0.0.1\",\n   \"rxMessagePort\" : 4568,\n   \"sensors\" : null,\n   \"timeSyncPort\" : 4567\n}\n");
    }

    TEST_F( TestCARMASimulationConnection, carma_simulation_handshake) {
        Point location;
        Json::Value sensorsJsonV;
        // UDP creation error
        ASSERT_FALSE(connection->carma_simulation_handshake("", "45", NULL, 
                                "",  45, 45, location, sensorsJsonV));
    }

    TEST_F(TestCARMASimulationConnection, connect) {
        ASSERT_TRUE(connection->connect());
    }

    TEST_F(TestCARMASimulationConnection, populate_sensors_with_file)
    {
        Json::Value sensorJsonV;
        ASSERT_TRUE(sensorJsonV.empty());
        std::ifstream in_strm;
        in_strm.open(sensors_file_path, std::ifstream::binary);
        if(in_strm.is_open())
        {
            connection->populate_sensors_with_file(sensors_file_path, sensorJsonV );
            ASSERT_FALSE(sensorJsonV.empty());
        }        
    }

    TEST_F(TestCARMASimulationConnection, get_sensor_by_id)
    {
        //Populate connection with sensors upon connect call.        
        connection->connect();
        std::ifstream in_strm;
        in_strm.open(sensors_file_path, std::ifstream::binary);
        if(in_strm.is_open())
        {
            std::string sensor_id = "SomeID";
            auto sensor = connection->get_sensor_by_id(sensor_id);
            ASSERT_FALSE(sensor.empty());
            ASSERT_EQ("SematicLidar", sensor["type"].asString());

            sensor_id = "not_found";
            sensor = connection->get_sensor_by_id(sensor_id);
            ASSERT_TRUE(sensor.empty());      
        }
         
    }
    
}