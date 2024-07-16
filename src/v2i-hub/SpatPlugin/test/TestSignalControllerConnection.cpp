#include <gtest/gtest.h>
#include <SignalControllerConnection.h>
#include <MockSNMPClient.h>
#include <MockUdpServer.h>
#include <tmx/messages/J2735Exception.hpp>

using testing::_;
using testing::Action;
using testing::DoDefault;
using testing::Return;
using testing::SetArgReferee;
using testing::Throw;
namespace SpatPlugin {
    class TestSignalControllerConnection : public ::testing::Test
    {
        public:
            TestSignalControllerConnection() {

            }
            void SetUp() {
                std::string signalGroupMapping = R"(
                    {\"SignalGroups\":
                        [
                            {\"SignalGroupId\":1,\"Phase\":1,\"Type\":\"vehicle\"},
                            {\"SignalGroupId\":2,\"Phase\":2,\"Type\":\"vehicle\"},
                            {\"SignalGroupId\":3,\"Phase\":3,\"Type\":\"vehicle\"},
                            {\"SignalGroupId\":4,\"Phase\":4,\"Type\":\"vehicle\"},
                            {\"SignalGroupId\":5,\"Phase\":5,\"Type\":\"vehicle\"},
                            {\"SignalGroupId\":6,\"Phase\":6,\"Type\":\"vehicle\"},
                            {\"SignalGroupId\":7,\"Phase\":7,\"Type\":\"vehicle\"},
                            {\"SignalGroupId\":8,\"Phase\":8,\"Type\":\"vehicle\"},
                            {\"SignalGroupId\":22,\"Phase\":2,\"Type\":\"pedestrian\"},
                            {\"SignalGroupId\":24,\"Phase\":4,\"Type\":\"pedestrian\"},
                            {\"SignalGroupId\":26,\"Phase\":6,\"Type\":\"pedestrian\"},
                            {\"SignalGroupId\":28,\"Phase\":8,\"Type\":\"pedestrian\"}
                        ]
                    }
                )";
                signalControllerConnection = std::make_unique<SignalControllerConnection>("127.0.0.1", 5000, signalGroupMapping, "", 5020, "someIntersection", 9001);
                mockSnmpClient = std::make_shared<unit_test::mock_snmp_client>("127.0.0.1", 6045, "administrator", "", "", "");
                mockUdpServer = std::make_shared<tmx::utils::MockUpdServer>();
                signalControllerConnection->scSNMPClient = mockSnmpClient;
                signalControllerConnection->spatPacketReceiver = mockUdpServer;
            }

            std::shared_ptr<unit_test::mock_snmp_client> mockSnmpClient;
            std::shared_ptr<tmx::utils::MockUpdServer> mockUdpServer;

            std::unique_ptr<SignalControllerConnection> signalControllerConnection;
    };

    TEST_F(TestSignalControllerConnection, initialize) {
        tmx::utils::snmp_response_obj set_value;
        set_value.type = tmx::utils::snmp_response_obj::response_type::INTEGER;
        set_value.val_int = 2;
        EXPECT_CALL(*mockSnmpClient, process_snmp_request("1.3.6.1.4.1.1206.3.5.2.9.44.1.0", tmx::utils::request_type::SET, set_value)).WillOnce(testing::DoAll(SetArgReferee<2>(set_value), Return(true)));
        EXPECT_TRUE(signalControllerConnection->initializeSignalControllerConnection());
    }

    TEST_F(TestSignalControllerConnection, receiveBinarySPAT) {
    }

    TEST_F(TestSignalControllerConnection, receiveUPERSPAT) {
        std::string uper_hex = R"(
            Version=0.7
            Type=SPAT
            PSID=0x8002
            Priority=7
            TxMode=CONT
            TxChannel=172
            TxInterval=0
            DeliveryStart=
            DeliveryStop=
            Signature=True
            Encryption=False
            Payload=00136b4457f20180000000208457f2c7c20b0010434162bc650001022a0b0be328000c10d058af194000808682c578ca00050434162bc650003022a0b0be328001c10d058af194001008682c578ca000904341617c650005021a0b15e328002c10d0585f194001808682c578ca00
        )";
        EXPECT_CALL(*mockUdpServer, stringTimedReceive(1000)).WillOnce(testing::DoAll(Return(uper_hex)));
        
        /**
         * <SPAT>
                <timeStamp>284658</timeStamp>
                <intersections>
                    <IntersectionState>
                        <id>
                            <id>0</id>
                        </id>
                        <revision>0</revision>
                        <status>
                            0000001000001000
                        </status>
                        <moy>284658</moy>
                        <timeStamp>51138</timeStamp>
                        <states>
                            <MovementState>
                                <signalGroup>1</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><stop-And-Remain/></eventState>
                                        <timing>
                                            <minEndTime>11351</minEndTime>
                                            <maxEndTime>36000</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                            <MovementState>
                                <signalGroup>2</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><permissive-Movement-Allowed/></eventState>
                                        <timing>
                                            <minEndTime>11311</minEndTime>
                                            <maxEndTime>36000</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                            <MovementState>
                                <signalGroup>3</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><stop-And-Remain/></eventState>
                                        <timing>
                                            <minEndTime>11351</minEndTime>
                                            <maxEndTime>36000</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                            <MovementState>
                                <signalGroup>4</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><stop-And-Remain/></eventState>
                                        <timing>
                                            <minEndTime>11351</minEndTime>
                                            <maxEndTime>36000</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                            <MovementState>
                                <signalGroup>5</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><stop-And-Remain/></eventState>
                                        <timing>
                                            <minEndTime>11351</minEndTime>
                                            <maxEndTime>36000</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                            <MovementState>
                                <signalGroup>6</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><permissive-Movement-Allowed/></eventState>
                                        <timing>
                                            <minEndTime>11311</minEndTime>
                                            <maxEndTime>36000</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                            <MovementState>
                                <signalGroup>7</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><stop-And-Remain/></eventState>
                                        <timing>
                                            <minEndTime>11351</minEndTime>
                                            <maxEndTime>36000</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                            <MovementState>
                                <signalGroup>8</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><stop-And-Remain/></eventState>
                                        <timing>
                                            <minEndTime>11351</minEndTime>
                                            <maxEndTime>36000</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                            <MovementState>
                                <signalGroup>9</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><stop-And-Remain/></eventState>
                                        <timing>
                                            <minEndTime>11311</minEndTime>
                                            <maxEndTime>36000</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                            <MovementState>
                                <signalGroup>10</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><stop-And-Remain/></eventState>
                                        <timing>
                                            <minEndTime>11351</minEndTime>
                                            <maxEndTime>36000</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                            <MovementState>
                                <signalGroup>11</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><stop-And-Remain/></eventState>
                                        <timing>
                                            <minEndTime>11311</minEndTime>
                                            <maxEndTime>36000</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                            <MovementState>
                                <signalGroup>12</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><stop-And-Remain/></eventState>
                                        <timing>
                                            <minEndTime>11351</minEndTime>
                                            <maxEndTime>36000</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                        </states>
                    </IntersectionState>
                </intersections>
            </SPAT>
        */
        auto spatEncoded_ptr = std::make_shared<tmx::messages::SpatEncodedMessage>();
		signalControllerConnection->receiveUPERSPAT(spatEncoded_ptr);  
        auto spat = spatEncoded_ptr->decode_j2735_message().get_j2735_data();
        EXPECT_EQ(284658L, *spat->timeStamp);
        EXPECT_EQ(0, spat->intersections.list.array[0]->id.id);
        EXPECT_EQ(284658L, *spat->intersections.list.array[0]->moy);
        EXPECT_EQ(51138, *spat->intersections.list.array[0]->timeStamp);
        // Signal Group 1
        EXPECT_EQ(1, spat->intersections.list.array[0]->states.list.array[0]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[0]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(11351, spat->intersections.list.array[0]->states.list.array[0]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(36000, *spat->intersections.list.array[0]->states.list.array[0]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 2
        EXPECT_EQ(2, spat->intersections.list.array[0]->states.list.array[1]->signalGroup);
        EXPECT_EQ(5, spat->intersections.list.array[0]->states.list.array[1]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(11311, spat->intersections.list.array[0]->states.list.array[1]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(36000, *spat->intersections.list.array[0]->states.list.array[1]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 3
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[2]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[2]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(11351, spat->intersections.list.array[0]->states.list.array[2]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(36000, *spat->intersections.list.array[0]->states.list.array[2]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 4
        EXPECT_EQ(4, spat->intersections.list.array[0]->states.list.array[3]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[3]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(11351, spat->intersections.list.array[0]->states.list.array[3]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(36000, *spat->intersections.list.array[0]->states.list.array[3]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 5
        EXPECT_EQ(5, spat->intersections.list.array[0]->states.list.array[4]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[4]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(11351, spat->intersections.list.array[0]->states.list.array[4]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(36000, *spat->intersections.list.array[0]->states.list.array[4]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 6
        EXPECT_EQ(6, spat->intersections.list.array[0]->states.list.array[5]->signalGroup);
        EXPECT_EQ(5, spat->intersections.list.array[0]->states.list.array[5]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(11311, spat->intersections.list.array[0]->states.list.array[5]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(36000, *spat->intersections.list.array[0]->states.list.array[5]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 7
        EXPECT_EQ(7, spat->intersections.list.array[0]->states.list.array[6]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[6]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(11351, spat->intersections.list.array[0]->states.list.array[6]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(36000, *spat->intersections.list.array[0]->states.list.array[6]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 8
        EXPECT_EQ(8, spat->intersections.list.array[0]->states.list.array[7]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[7]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(11351, spat->intersections.list.array[0]->states.list.array[7]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(36000, *spat->intersections.list.array[0]->states.list.array[7]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 9
        EXPECT_EQ(9, spat->intersections.list.array[0]->states.list.array[8]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[8]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(11311, spat->intersections.list.array[0]->states.list.array[8]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(36000, *spat->intersections.list.array[0]->states.list.array[8]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 10
        EXPECT_EQ(10, spat->intersections.list.array[0]->states.list.array[9]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[9]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(11351, spat->intersections.list.array[0]->states.list.array[9]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(36000, *spat->intersections.list.array[0]->states.list.array[9]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 11
        EXPECT_EQ(11, spat->intersections.list.array[0]->states.list.array[10]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[10]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(11311, spat->intersections.list.array[0]->states.list.array[10]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(36000, *spat->intersections.list.array[0]->states.list.array[10]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 12
        EXPECT_EQ(12, spat->intersections.list.array[0]->states.list.array[11]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[11]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(11351, spat->intersections.list.array[0]->states.list.array[11]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(36000, *spat->intersections.list.array[0]->states.list.array[11]->state_time_speed.list.array[0]->timing->maxEndTime);
    }
}