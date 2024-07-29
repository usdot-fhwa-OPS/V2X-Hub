/**
 * Copyright (C) 2024 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#include <gtest/gtest.h>
#include <SignalControllerConnection.h>
#include <MockSNMPClient.h>
#include <MockUdpServer.h>
#include <tmx/messages/J2735Exception.hpp>
#include <NTCIP1202OIDs.h>

using testing::_;
using testing::Action;
using testing::ByRef;
using testing::DoDefault;
using testing::Return;
using testing::SetArgPointee;
using testing::SetArgReferee;
using testing::SetArrayArgument;
using testing::Throw;
namespace SpatPlugin {
    class TestSignalControllerConnection : public ::testing::Test
    {
        public:
            TestSignalControllerConnection() {

            }
            void SetUp() {
                std::string signalGroupMapping = R"(
                    {"SignalGroups":
                        [
                            {"SignalGroupId":1,"Phase":1,"Type":"vehicle"},
                            {"SignalGroupId":2,"Phase":2,"Type":"vehicle"},
                            {"SignalGroupId":3,"Phase":3,"Type":"vehicle"},
                            {"SignalGroupId":4,"Phase":4,"Type":"vehicle"},
                            {"SignalGroupId":5,"Phase":5,"Type":"vehicle"},
                            {"SignalGroupId":6,"Phase":6,"Type":"vehicle"},
                            {"SignalGroupId":7,"Phase":7,"Type":"vehicle"},
                            {"SignalGroupId":8,"Phase":8,"Type":"vehicle"},
                            {"SignalGroupId":9,"Phase":2,"Type":"pedestrian"},
                            {"SignalGroupId":10,"Phase":4,"Type":"pedestrian"},
                            {"SignalGroupId":11,"Phase":6,"Type":"pedestrian"},
                            {"SignalGroupId":12,"Phase":8,"Type":"pedestrian"}
                        ]
                    }
                )";
                signalControllerConnection = std::make_unique<SignalControllerConnection>("127.0.0.1", 5000, signalGroupMapping, "", 5020, "someIntersection", 9001);
                mockSnmpClient = std::make_shared<unit_test::mock_snmp_client>("127.0.0.1", 6045, "administrator", "", "", "");
                mockUdpServer = std::make_shared<tmx::utils::MockUpdServer>();
                signalControllerConnection->scSNMPClient = mockSnmpClient;
                signalControllerConnection->spatPacketReceiver = mockUdpServer;
            }

            std::vector<char> read_binary_file(std::string name) 
            {
                std::ifstream file(name.c_str(), std::ios::binary);
                std::vector<char> buf;

                if (!file.good()) 
                {
                    throw runtime_error("Could not open file " + name);
                }

                file.unsetf(std::ios::skipws);
                file.seekg(0, std::ios::end);
                const size_t size = file.tellg();

                file.seekg(0, std::ios::beg);
                buf.resize(size);
                file.read(buf.data(), size);
                file.close();

                return buf;
            }

            std::shared_ptr<unit_test::mock_snmp_client> mockSnmpClient;
            std::shared_ptr<tmx::utils::MockUpdServer> mockUdpServer;

            std::unique_ptr<SignalControllerConnection> signalControllerConnection;
    };

    TEST_F(TestSignalControllerConnection, initialize) {
        tmx::utils::snmp_response_obj enable_spat;
        enable_spat.type = tmx::utils::snmp_response_obj::response_type::INTEGER;
        enable_spat.val_int = 2;
        tmx::utils::snmp_response_obj intersection_id;
        intersection_id.type = tmx::utils::snmp_response_obj::response_type::INTEGER;
        intersection_id.val_int = 9001;
        EXPECT_CALL(*mockSnmpClient, process_snmp_request(NTCIP1202V2::ENABLE_SPAT_OID, tmx::utils::request_type::SET, enable_spat)).WillOnce(testing::DoAll(SetArgReferee<2>(enable_spat), Return(true)));
        EXPECT_CALL(*mockSnmpClient, process_snmp_request(NTCIP1202V3::INTERSECTION_ID, tmx::utils::request_type::SET, intersection_id)).WillOnce(testing::DoAll(SetArgReferee<2>(enable_spat), Return(true)));
        EXPECT_TRUE(signalControllerConnection->initializeSignalControllerConnection(true, true));
    }

    TEST_F(TestSignalControllerConnection, receiveBinarySPAT) {
        auto spat_binary_buf = read_binary_file("../../SpatPlugin/test/test_spat_binaries/spat_1721238398773.bin");
        EXPECT_CALL(*mockUdpServer, TimedReceive(_, 1000, 1000)).WillOnce(testing::DoAll(SetArrayArgument<0>(spat_binary_buf.begin(), spat_binary_buf.end()), Return(spat_binary_buf.size())));
        auto spat = std::make_shared<SPAT>();
		signalControllerConnection->receiveBinarySPAT(spat, 1721238398773);
        /**
         * <SPAT>
                <intersections>
                    <IntersectionState>
                        <name>someIntersection</name>
                        <id>
                            <id>9001</id>
                        </id>
                        <revision>1</revision>
                        <status>
                            0000000000000000
                        </status>
                        <moy>286186</moy>
                        <timeStamp>38773</timeStamp>
                        <states>
                            <MovementState>
                                <signalGroup>1</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><stop-And-Remain/></eventState>
                                        <timing>
                                            <minEndTime>28027</minEndTime>
                                            <maxEndTime>21522</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                            <MovementState>
                                <signalGroup>2</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><protected-Movement-Allowed/></eventState>
                                        <timing>
                                            <minEndTime>27987</minEndTime>
                                            <maxEndTime>21522</maxEndTime>
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
                                            <minEndTime>27987</minEndTime>
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
                                            <minEndTime>28027</minEndTime>
                                            <maxEndTime>21522</maxEndTime>
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
                                            <minEndTime>28027</minEndTime>
                                            <maxEndTime>21522</maxEndTime>
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
                                            <minEndTime>28027</minEndTime>
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
                                            <minEndTime>28027</minEndTime>
                                            <maxEndTime>21522</maxEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                            </MovementState>
                            <MovementState>
                                <signalGroup>6</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><protected-Movement-Allowed/></eventState>
                                        <timing>
                                            <minEndTime>27987</minEndTime>
                                            <maxEndTime>21522</maxEndTime>
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
                                            <minEndTime>27987</minEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                                <maneuverAssistList>
                                    <ConnectionManeuverAssist>
                                        <connectionID>0</connectionID>
                                        <pedBicycleDetect><true/></pedBicycleDetect>
                                    </ConnectionManeuverAssist>
                                </maneuverAssistList>
                            </MovementState>
                            <MovementState>
                                <signalGroup>7</signalGroup>
                                <state-time-speed>
                                    <MovementEvent>
                                        <eventState><stop-And-Remain/></eventState>
                                        <timing>
                                            <minEndTime>28027</minEndTime>
                                            <maxEndTime>21522</maxEndTime>
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
                                            <minEndTime>28027</minEndTime>
                                            <maxEndTime>21522</maxEndTime>
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
                                            <minEndTime>28027</minEndTime>
                                        </timing>
                                    </MovementEvent>
                                </state-time-speed>
                                <maneuverAssistList>
                                    <ConnectionManeuverAssist>
                                        <connectionID>0</connectionID>
                                        <pedBicycleDetect><true/></pedBicycleDetect>
                                    </ConnectionManeuverAssist>
                                </maneuverAssistList>
                            </MovementState>
                        </states>
                    </IntersectionState>
                </intersections>
            </SPAT>
        */
        EXPECT_EQ(9001, spat->intersections.list.array[0]->id.id);
        EXPECT_EQ(286186, *spat->intersections.list.array[0]->moy);
        EXPECT_EQ(38773, *spat->intersections.list.array[0]->timeStamp);
        // Signal Group 1
        EXPECT_EQ(1, spat->intersections.list.array[0]->states.list.array[0]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[0]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(28027, spat->intersections.list.array[0]->states.list.array[0]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(21522, *spat->intersections.list.array[0]->states.list.array[0]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 2
        EXPECT_EQ(2, spat->intersections.list.array[0]->states.list.array[1]->signalGroup);
        EXPECT_EQ(6, spat->intersections.list.array[0]->states.list.array[1]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(27987, spat->intersections.list.array[0]->states.list.array[1]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(21522, *spat->intersections.list.array[0]->states.list.array[1]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 9
        EXPECT_EQ(9, spat->intersections.list.array[0]->states.list.array[2]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[2]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(27987, spat->intersections.list.array[0]->states.list.array[2]->state_time_speed.list.array[0]->timing->minEndTime);
        // Signal Group 3
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[3]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[3]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(28027, spat->intersections.list.array[0]->states.list.array[3]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(21522, *spat->intersections.list.array[0]->states.list.array[3]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 4
        EXPECT_EQ(4, spat->intersections.list.array[0]->states.list.array[4]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[4]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(28027, spat->intersections.list.array[0]->states.list.array[4]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(21522, *spat->intersections.list.array[0]->states.list.array[4]->state_time_speed.list.array[0]->timing->maxEndTime);
        // // Signal Group 10
        EXPECT_EQ(10, spat->intersections.list.array[0]->states.list.array[5]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[5]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(28027, spat->intersections.list.array[0]->states.list.array[5]->state_time_speed.list.array[0]->timing->minEndTime);
        // // Signal Group 5
        EXPECT_EQ(5, spat->intersections.list.array[0]->states.list.array[6]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[6]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(28027, spat->intersections.list.array[0]->states.list.array[6]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(21522, *spat->intersections.list.array[0]->states.list.array[6]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 6
        EXPECT_EQ(6, spat->intersections.list.array[0]->states.list.array[7]->signalGroup);
        EXPECT_EQ(6, spat->intersections.list.array[0]->states.list.array[7]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(27987, spat->intersections.list.array[0]->states.list.array[7]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(21522, *spat->intersections.list.array[0]->states.list.array[7]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 11
        EXPECT_EQ(11, spat->intersections.list.array[0]->states.list.array[8]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[8]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(27987, spat->intersections.list.array[0]->states.list.array[8]->state_time_speed.list.array[0]->timing->minEndTime);
        // Signal Group 7
        EXPECT_EQ(7, spat->intersections.list.array[0]->states.list.array[9]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[9]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(28027, spat->intersections.list.array[0]->states.list.array[9]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(21522, *spat->intersections.list.array[0]->states.list.array[9]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 8
        EXPECT_EQ(8, spat->intersections.list.array[0]->states.list.array[10]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[10]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(28027, spat->intersections.list.array[0]->states.list.array[10]->state_time_speed.list.array[0]->timing->minEndTime);
        EXPECT_EQ(21522, *spat->intersections.list.array[0]->states.list.array[10]->state_time_speed.list.array[0]->timing->maxEndTime);
        // Signal Group 12
        EXPECT_EQ(12, spat->intersections.list.array[0]->states.list.array[11]->signalGroup);
        EXPECT_EQ(3, spat->intersections.list.array[0]->states.list.array[11]->state_time_speed.list.array[0]->eventState);
        EXPECT_EQ(28027, spat->intersections.list.array[0]->states.list.array[11]->state_time_speed.list.array[0]->timing->minEndTime);
    }
    TEST_F(TestSignalControllerConnection, receiveBinarySPATException) {
        EXPECT_CALL(*mockUdpServer, TimedReceive(_, 1000, 1000)).WillOnce(testing::DoAll( Return(0)));
        auto spat = std::make_shared<SPAT>();
		EXPECT_THROW(signalControllerConnection->receiveBinarySPAT(spat, 1721238398773), tmx::utils::UdpServerRuntimeError);
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

    TEST_F(TestSignalControllerConnection, receiveUPERSPATException) {
        std::string without_paylod = R"(
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
        )";
        EXPECT_CALL(*mockUdpServer, stringTimedReceive(1000)).WillOnce(testing::DoAll(Return(without_paylod)));

        auto spatEncoded_ptr = std::make_shared<tmx::messages::SpatEncodedMessage>();
		EXPECT_THROW(signalControllerConnection->receiveUPERSPAT(spatEncoded_ptr), tmx::TmxException);  
    }
}