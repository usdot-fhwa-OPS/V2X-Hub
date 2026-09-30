#include <gtest/gtest.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include "jsoncpp/json/json.h"
#include "J2735MapToJsonConverter.h"

class test_J2735MapToJsonConverter : public testing::Test
{
public:
    test_J2735MapToJsonConverter() = default;
    ~test_J2735MapToJsonConverter() = default;
};

namespace unit_test
{
   
    TEST_F(test_J2735MapToJsonConverter, convertJ2735MAPToMapJSON)
    {
        tmx::messages::J2735MessageFactory factory;
        tmx::byte_stream bytes = tmx::byte_stream_decode("0012822138713020311b27c535a4e8ac6b49c0c5109602dc2522f100001e480cc800000002b5962bc282900000000000000e22f62aff9080a400000000000000ac0600018096068000c06481cc800000002b5d282c282a800000000000000000000000000388bd91bfe4202a8000000000000000000000000000ac26000180561e4000c04200aa400000002b575dc809c45eea9fd405050806a900000000ad65600027387bb17f512022200000002a8bd20502711501421805052e00a60302828560f0000802b014000402240c4400000005541a42604e72e02c5ed02828505a1a02b0f8000401586900020108019900000002a7481f602711700032a814282804072100b32000000054bf03ec04e22dfd46da02850501e04c4804480000000a89f9db4282800000e22ccb6fe50a0a000003ecd4c157fc80a0a000004142b058000201583a00010112051200000002a26f6210a0a00000388b2f8bfa8202800000d4cb37fb4080a000000ac36000080561640004042002240000000a89fa2e809c4599a200004fb353001ff840a0a1009120000000544ef3f004e22cd49007027d9a97a6ffe0050512044200000002a736d7302738700fca20dff3c951581c0004012c15000200c904210000000152f36c8050510000000007397016c8e040510000000005bfc78c41014400000000056170001002b09200080210077200000005510dae604e70e01f8dc1c0039b0201ee40000000aa7675e809ce1c0531aab7f572b009f6");
        /**
         * MapData ::= {
            msgIssueRevision: 113
            layerType: 3 (intersectionData)
            layerID: 1
            intersections: IntersectionGeometryList ::= {
                IntersectionGeometry ::= {
                    id: IntersectionReferenceID ::= {
                        id: 36243
                    }
                    revision: 113
                    refPoint: Position3D ::= {
                        lat: -84
                        long: -4410
                        elevation: 150
                    }
                    laneWidth: 366
                    speedLimits: SpeedLimitList ::= {
                        RegulatorySpeedLimit ::= {
                            type: 5 (vehicleMaxSpeed)
                            speed: 1118
                        }
                        RegulatorySpeedLimit ::= {
                            type: 4 (vehicleMinSpeed)
                            speed: 0
                        }
                    }
                    laneSet: LaneList ::= {
                        GenericLane ::= {
                            laneID: 6
                            ingressApproach: 6
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 80 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: 1381
                                        y: 175
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        data: LaneDataAttributeList ::= {
                                            speedLimits: SpeedLimitList ::= {
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                            }
                                        }
                                        dElevation: -60
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: 2837
                                        y: -7
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        data: LaneDataAttributeList ::= {
                                            speedLimits: SpeedLimitList ::= {
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            connectsTo: ConnectsToList ::= {
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 1
                                        maneuver: 80 00 (4 bits unused)
                                    }
                                    signalGroup: 6
                                    connectionID: 2
                                }
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 3
                                        maneuver: 40 00 (4 bits unused)
                                    }
                                    signalGroup: 6
                                    connectionID: 3
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 14
                            ingressApproach: 6
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 80 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: 1396
                                        y: 523
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        data: LaneDataAttributeList ::= {
                                            speedLimits: SpeedLimitList ::= {
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                            }
                                        }
                                        dElevation: -60
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: 2851
                                        y: -7
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        data: LaneDataAttributeList ::= {
                                            speedLimits: SpeedLimitList ::= {
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            connectsTo: ConnectsToList ::= {
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 9
                                        maneuver: 80 00 (4 bits unused)
                                    }
                                    signalGroup: 6
                                    connectionID: 1
                                }
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 15
                                        maneuver: 20 00 (4 bits unused)
                                    }
                                    signalGroup: 6
                                    connectionID: 2
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 5
                            egressApproach: 5
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 40 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: 1373
                                        y: -142
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -60
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: 2986
                                        y: -22
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: 10
                                    }
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 13
                            egressApproach: 5
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 40 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: 1381
                                        y: -512
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -50
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: 2993
                                        y: -22
                                    }
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 4
                            ingressApproach: 4
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 80 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: 189
                                        y: -1531
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -60
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: 10
                                        y: -1780
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: 10
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: 5
                                        y: -2557
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: 10
                                    }
                                }
                            }
                            connectsTo: ConnectsToList ::= {
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 7
                                        maneuver: 80 00 (4 bits unused)
                                    }
                                    signalGroup: 4
                                    connectionID: 1
                                }
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 1
                                        maneuver: 40 00 (4 bits unused)
                                    }
                                    signalGroup: 4
                                    connectionID: 2
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 12
                            ingressApproach: 4
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 80 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: 525
                                        y: -1517
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -50
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: 22
                                        y: -2579
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: 10
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: 45
                                        y: -1840
                                    }
                                }
                            }
                            connectsTo: ConnectsToList ::= {
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 15
                                        maneuver: 80 00 (4 bits unused)
                                    }
                                    signalGroup: 4
                                    connectionID: 1
                                }
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 13
                                        maneuver: 20 00 (4 bits unused)
                                    }
                                    signalGroup: 4
                                    connectionID: 2
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 3
                            egressApproach: 3
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 40 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: -184
                                        y: -1546
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -60
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: 0
                                        y: -2475
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: 20
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: 4
                                        y: -1934
                                    }
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 11
                            egressApproach: 3
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 40 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: -520
                                        y: -1546
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -60
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: -22
                                        y: -2342
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: 20
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: 15
                                        y: -2010
                                    }
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 2
                            ingressApproach: 2
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 80 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: -1410
                                        y: -147
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        data: LaneDataAttributeList ::= {
                                            speedLimits: SpeedLimitList ::= {
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                            }
                                        }
                                        dElevation: -60
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: -2469
                                        y: -27
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        data: LaneDataAttributeList ::= {
                                            speedLimits: SpeedLimitList ::= {
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                            }
                                        }
                                        dElevation: -10
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY6: Node-XY-32b ::= {
                                        x: -13291
                                        y: -56
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        data: LaneDataAttributeList ::= {
                                            speedLimits: SpeedLimitList ::= {
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                            }
                                        }
                                        dElevation: 10
                                    }
                                }
                            }
                            connectsTo: ConnectsToList ::= {
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 5
                                        maneuver: 80 00 (4 bits unused)
                                    }
                                    signalGroup: 2
                                    connectionID: 1
                                }
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 7
                                        maneuver: 40 00 (4 bits unused)
                                    }
                                    signalGroup: 2
                                    connectionID: 2
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 10
                            ingressApproach: 2
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 80 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: -1425
                                        y: -479
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        data: LaneDataAttributeList ::= {
                                            speedLimits: SpeedLimitList ::= {
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                            }
                                        }
                                        dElevation: -60
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: -2575
                                        y: -22
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        data: LaneDataAttributeList ::= {
                                            speedLimits: SpeedLimitList ::= {
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                            }
                                        }
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY6: Node-XY-32b ::= {
                                        x: -13133
                                        y: -76
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        data: LaneDataAttributeList ::= {
                                            speedLimits: SpeedLimitList ::= {
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            connectsTo: ConnectsToList ::= {
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 13
                                        maneuver: 80 00 (4 bits unused)
                                    }
                                    signalGroup: 2
                                    connectionID: 1
                                }
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 11
                                        maneuver: 20 00 (4 bits unused)
                                    }
                                    signalGroup: 2
                                    connectionID: 2
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 1
                            egressApproach: 1
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 40 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: -1410
                                        y: 186
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -60
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: -2456
                                        y: 0
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -10
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY6: Node-XY-32b ::= {
                                        x: -13312
                                        y: -31
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: 10
                                    }
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 9
                            egressApproach: 1
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 40 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: -1417
                                        y: 504
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -60
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: -2396
                                        y: 7
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -10
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY6: Node-XY-32b ::= {
                                        x: -13357
                                        y: -16
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: 10
                                    }
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 8
                            ingressApproach: 8
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 80 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: -202
                                        y: 1395
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -50
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: 15
                                        y: 2372
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: -7
                                        y: 3221
                                    }
                                }
                            }
                            connectsTo: ConnectsToList ::= {
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 3
                                        maneuver: 80 00 (4 bits unused)
                                    }
                                    signalGroup: 8
                                    connectionID: 2
                                }
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 5
                                        maneuver: 40 00 (4 bits unused)
                                    }
                                    signalGroup: 8
                                    connectionID: 3
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 16
                            ingressApproach: 8
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 80 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: -538
                                        y: 1424
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        data: LaneDataAttributeList ::= {
                                            speedLimits: SpeedLimitList ::= {
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                            }
                                        }
                                        dElevation: -50
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: 22
                                        y: 2332
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        data: LaneDataAttributeList ::= {
                                            speedLimits: SpeedLimitList ::= {
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                            }
                                        }
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: -15
                                        y: 3170
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        data: LaneDataAttributeList ::= {
                                            speedLimits: SpeedLimitList ::= {
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                                RegulatorySpeedLimit ::= {
                                                    type: 0 (unknown)
                                                    speed: 0
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            connectsTo: ConnectsToList ::= {
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 11
                                        maneuver: 80 00 (4 bits unused)
                                    }
                                    signalGroup: 8
                                    connectionID: 1
                                }
                                Connection ::= {
                                    connectingLane: ConnectingLane ::= {
                                        lane: 9
                                        maneuver: 20 00 (4 bits unused)
                                    }
                                    signalGroup: 8
                                    connectionID: 2
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 7
                            egressApproach: 7
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 40 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: 134
                                        y: 1395
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -50
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: 15
                                        y: 2268
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: 0
                                        y: 3288
                                    }
                                }
                            }
                        }
                        GenericLane ::= {
                            laneID: 15
                            egressApproach: 7
                            laneAttributes: LaneAttributes ::= {
                                directionalUse: 40 (6 bits unused)
                                sharedWith: 00 00 (6 bits unused)
                                laneType: vehicle: 00
                            }
                            nodeList: nodes: NodeSetXY ::= {
                                NodeXY ::= {
                                    delta: node-XY3: Node-XY-24b ::= {
                                        x: 473
                                        y: 1402
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -50
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: 20
                                        y: 2261
                                    }
                                }
                                NodeXY ::= {
                                    delta: node-XY4: Node-XY-26b ::= {
                                        x: -22
                                        y: 3244
                                    }
                                    attributes: NodeAttributeSetXY ::= {
                                        dElevation: -10
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
         */
        std::shared_ptr<tmx::messages::MapDataEncodedMessage> msg = std::shared_ptr<tmx::messages::MapDataEncodedMessage>(static_cast<tmx::messages::MapDataEncodedMessage*>(factory.NewMessage(bytes)));
        auto decodedMap = msg->decode_j2735_message().get_j2735_data();
        asn_fprint(stdout, &asn_DEF_MapData, decodedMap.get());

        Json::Value mapJson;
        CARMAStreetsPlugin::J2735MapToJsonConverter converter;
        converter.convertJ2735MAPToMapJSON(decodedMap, mapJson);
        Json::Value mapDataJson = mapJson["map_data"];
        // Create the builder and remove indentation
        Json::StreamWriterBuilder builder;
        builder["indentation"] = ""; // Removes tabs and newlines
        std::string expectedMapJson = "{\"intersections\":{\"intersection_geometry\":{\"id\":{\"id\":\"36243\"},\"lane_set\":[{\"connects_to\":[{\"connecting_lane\":{\"lane\":\"1\"},\"signal_group\":\"6\"},{\"connecting_lane\":{\"lane\":\"3\"},\"signal_group\":\"6\"}],\"ingress_approach\":\"6\",\"lane_attributes\":{\"directional_use\":\"10\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"6\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"1381\",\"y\":\"175\"}}},{\"delta\":{\"node-xy\":{\"x\":\"2837\",\"y\":\"-7\"}}}]}},{\"connects_to\":[{\"connecting_lane\":{\"lane\":\"9\"},\"signal_group\":\"6\"},{\"connecting_lane\":{\"lane\":\"15\"},\"signal_group\":\"6\"}],\"ingress_approach\":\"6\",\"lane_attributes\":{\"directional_use\":\"10\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"14\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"1396\",\"y\":\"523\"}}},{\"delta\":{\"node-xy\":{\"x\":\"2851\",\"y\":\"-7\"}}}]}},{\"egressApproach\":\"5\",\"lane_attributes\":{\"directional_use\":\"01\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"5\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"1373\",\"y\":\"-142\"}}},{\"delta\":{\"node-xy\":{\"x\":\"2986\",\"y\":\"-22\"}}}]}},{\"egressApproach\":\"5\",\"lane_attributes\":{\"directional_use\":\"01\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"13\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"1381\",\"y\":\"-512\"}}},{\"delta\":{\"node-xy\":{\"x\":\"2993\",\"y\":\"-22\"}}}]}},{\"connects_to\":[{\"connecting_lane\":{\"lane\":\"7\"},\"signal_group\":\"4\"},{\"connecting_lane\":{\"lane\":\"1\"},\"signal_group\":\"4\"}],\"ingress_approach\":\"4\",\"lane_attributes\":{\"directional_use\":\"10\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"4\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"189\",\"y\":\"-1531\"}}},{\"delta\":{\"node-xy\":{\"x\":\"10\",\"y\":\"-1780\"}}},{\"delta\":{\"node-xy\":{\"x\":\"5\",\"y\":\"-2557\"}}}]}},{\"connects_to\":[{\"connecting_lane\":{\"lane\":\"15\"},\"signal_group\":\"4\"},{\"connecting_lane\":{\"lane\":\"13\"},\"signal_group\":\"4\"}],\"ingress_approach\":\"4\",\"lane_attributes\":{\"directional_use\":\"10\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"12\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"525\",\"y\":\"-1517\"}}},{\"delta\":{\"node-xy\":{\"x\":\"22\",\"y\":\"-2579\"}}},{\"delta\":{\"node-xy\":{\"x\":\"45\",\"y\":\"-1840\"}}}]}},{\"egressApproach\":\"3\",\"lane_attributes\":{\"directional_use\":\"01\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"3\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"-184\",\"y\":\"-1546\"}}},{\"delta\":{\"node-xy\":{\"x\":\"0\",\"y\":\"-2475\"}}},{\"delta\":{\"node-xy\":{\"x\":\"4\",\"y\":\"-1934\"}}}]}},{\"egressApproach\":\"3\",\"lane_attributes\":{\"directional_use\":\"01\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"11\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"-520\",\"y\":\"-1546\"}}},{\"delta\":{\"node-xy\":{\"x\":\"-22\",\"y\":\"-2342\"}}},{\"delta\":{\"node-xy\":{\"x\":\"15\",\"y\":\"-2010\"}}}]}},{\"connects_to\":[{\"connecting_lane\":{\"lane\":\"5\"},\"signal_group\":\"2\"},{\"connecting_lane\":{\"lane\":\"7\"},\"signal_group\":\"2\"}],\"ingress_approach\":\"2\",\"lane_attributes\":{\"directional_use\":\"10\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"2\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"-1410\",\"y\":\"-147\"}}},{\"delta\":{\"node-xy\":{\"x\":\"-2469\",\"y\":\"-27\"}}},{\"delta\":{\"node-xy\":{\"x\":\"-13291\",\"y\":\"-56\"}}}]}},{\"connects_to\":[{\"connecting_lane\":{\"lane\":\"13\"},\"signal_group\":\"2\"},{\"connecting_lane\":{\"lane\":\"11\"},\"signal_group\":\"2\"}],\"ingress_approach\":\"2\",\"lane_attributes\":{\"directional_use\":\"10\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"10\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"-1425\",\"y\":\"-479\"}}},{\"delta\":{\"node-xy\":{\"x\":\"-2575\",\"y\":\"-22\"}}},{\"delta\":{\"node-xy\":{\"x\":\"-13133\",\"y\":\"-76\"}}}]}},{\"egressApproach\":\"1\",\"lane_attributes\":{\"directional_use\":\"01\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"1\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"-1410\",\"y\":\"186\"}}},{\"delta\":{\"node-xy\":{\"x\":\"-2456\",\"y\":\"0\"}}},{\"delta\":{\"node-xy\":{\"x\":\"-13312\",\"y\":\"-31\"}}}]}},{\"egressApproach\":\"1\",\"lane_attributes\":{\"directional_use\":\"01\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"9\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"-1417\",\"y\":\"504\"}}},{\"delta\":{\"node-xy\":{\"x\":\"-2396\",\"y\":\"7\"}}},{\"delta\":{\"node-xy\":{\"x\":\"-13357\",\"y\":\"-16\"}}}]}},{\"connects_to\":[{\"connecting_lane\":{\"lane\":\"3\"},\"signal_group\":\"8\"},{\"connecting_lane\":{\"lane\":\"5\"},\"signal_group\":\"8\"}],\"ingress_approach\":\"8\",\"lane_attributes\":{\"directional_use\":\"10\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"8\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"-202\",\"y\":\"1395\"}}},{\"delta\":{\"node-xy\":{\"x\":\"15\",\"y\":\"2372\"}}},{\"delta\":{\"node-xy\":{\"x\":\"-7\",\"y\":\"3221\"}}}]}},{\"connects_to\":[{\"connecting_lane\":{\"lane\":\"11\"},\"signal_group\":\"8\"},{\"connecting_lane\":{\"lane\":\"9\"},\"signal_group\":\"8\"}],\"ingress_approach\":\"8\",\"lane_attributes\":{\"directional_use\":\"10\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"16\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"-538\",\"y\":\"1424\"}}},{\"delta\":{\"node-xy\":{\"x\":\"22\",\"y\":\"2332\"}}},{\"delta\":{\"node-xy\":{\"x\":\"-15\",\"y\":\"3170\"}}}]}},{\"egressApproach\":\"7\",\"lane_attributes\":{\"directional_use\":\"01\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"7\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"134\",\"y\":\"1395\"}}},{\"delta\":{\"node-xy\":{\"x\":\"15\",\"y\":\"2268\"}}},{\"delta\":{\"node-xy\":{\"x\":\"0\",\"y\":\"3288\"}}}]}},{\"egressApproach\":\"7\",\"lane_attributes\":{\"directional_use\":\"01\",\"lane_type\":{\"vehicle\":\"\"},\"shared_with\":\"\"},\"lane_id\":\"15\",\"node_list\":{\"nodes\":[{\"delta\":{\"node-xy\":{\"x\":\"473\",\"y\":\"1402\"}}},{\"delta\":{\"node-xy\":{\"x\":\"20\",\"y\":\"2261\"}}},{\"delta\":{\"node-xy\":{\"x\":\"-22\",\"y\":\"3244\"}}}]}}],\"lane_width\":\"366\",\"ref_point\":{\"elevation\":\"150\",\"lat\":\"-84\",\"long\":\"-4410\"},\"revision\":\"113\"}},\"layer_id\":\"1\",\"layer_type\":\"3\",\"msg_issue_revision\":\"113\"}";
        std::string returnedJson = Json::writeString(builder, mapDataJson);
        EXPECT_EQ(expectedMapJson, returnedJson);
    }
}