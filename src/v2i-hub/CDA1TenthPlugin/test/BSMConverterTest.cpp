#include <gtest/gtest.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include "BSMConverter.h"

TEST(BSMConverterTest, toTransmissionString)
{
    TransmissionState_t transmission = TransmissionState::TransmissionState_park;
    string result = CDA1TenthPlugin::BSMConverter::toTransmissionString(transmission);
    EXPECT_EQ("park", result);

    transmission = TransmissionState::TransmissionState_neutral;
    result = CDA1TenthPlugin::BSMConverter::toTransmissionString(transmission);
    EXPECT_EQ("neutral", result);

    transmission = TransmissionState::TransmissionState_forwardGears;
    result = CDA1TenthPlugin::BSMConverter::toTransmissionString(transmission);
    EXPECT_EQ("forwardGears", result);

    transmission = TransmissionState::TransmissionState_reverseGears;
    result = CDA1TenthPlugin::BSMConverter::toTransmissionString(transmission);
    EXPECT_EQ("reverseGears", result);

    transmission = TransmissionState::TransmissionState_reserved1;
    result = CDA1TenthPlugin::BSMConverter::toTransmissionString(transmission);
    EXPECT_EQ("reserved1", result);

    transmission = TransmissionState::TransmissionState_reserved2;
    result = CDA1TenthPlugin::BSMConverter::toTransmissionString(transmission);
    EXPECT_EQ("reserved2", result);

    transmission = TransmissionState::TransmissionState_reserved3;
    result = CDA1TenthPlugin::BSMConverter::toTransmissionString(transmission);
    EXPECT_EQ("reserved3", result);
}

TEST(BSMConverterTest, toTractionString)
{
    TractionControlStatus_t traction = TractionControlStatus::TractionControlStatus_off;
    string result = CDA1TenthPlugin::BSMConverter::toTractionString(traction);
    EXPECT_EQ("off", result);
    traction = TractionControlStatus::TractionControlStatus_on;
    result = CDA1TenthPlugin::BSMConverter::toTractionString(traction);
    EXPECT_EQ("on", result);

    traction = TractionControlStatus::TractionControlStatus_engaged;
    result = CDA1TenthPlugin::BSMConverter::toTractionString(traction);
    EXPECT_EQ("engaged", result);

    traction = TractionControlStatus::TractionControlStatus_unavailable;
    result = CDA1TenthPlugin::BSMConverter::toTractionString(traction);
    EXPECT_EQ("unavailable", result);
}

TEST(BSMConverterTest, toAntiLockBrakeStatusString)
{
    AntiLockBrakeStatus_t abs = AntiLockBrakeStatus::AntiLockBrakeStatus_on;
    string result = CDA1TenthPlugin::BSMConverter::toAntiLockBrakeStatusString(abs);
    EXPECT_EQ("on", result);
    abs = AntiLockBrakeStatus::AntiLockBrakeStatus_off;
    result = CDA1TenthPlugin::BSMConverter::toAntiLockBrakeStatusString(abs);
    EXPECT_EQ("off", result);

    abs = AntiLockBrakeStatus::AntiLockBrakeStatus_unavailable;
    result = CDA1TenthPlugin::BSMConverter::toAntiLockBrakeStatusString(abs);
    EXPECT_EQ("unavailable", result);

    abs = AntiLockBrakeStatus::AntiLockBrakeStatus_engaged;
    result = CDA1TenthPlugin::BSMConverter::toAntiLockBrakeStatusString(abs);
    EXPECT_EQ("engaged", result);
}

TEST(BSMConverterTest, toStabilityControlStatusString)
{
    StabilityControlStatus_t scs = StabilityControlStatus::StabilityControlStatus_engaged;
    string result = CDA1TenthPlugin::BSMConverter::toStabilityControlStatusString(scs);
    EXPECT_EQ("engaged", result);
    scs = StabilityControlStatus::StabilityControlStatus_off;
    result = CDA1TenthPlugin::BSMConverter::toStabilityControlStatusString(scs);
    EXPECT_EQ("off", result);

    scs = StabilityControlStatus::StabilityControlStatus_on;
    result = CDA1TenthPlugin::BSMConverter::toStabilityControlStatusString(scs);
    EXPECT_EQ("on", result);

    scs = StabilityControlStatus::StabilityControlStatus_unavailable;
    result = CDA1TenthPlugin::BSMConverter::toStabilityControlStatusString(scs);
    EXPECT_EQ("unavailable", result);
}

TEST(BSMConverterTest, toBrakeBoostAppliedString)
{
    BrakeBoostApplied_t brakeBoost = BrakeBoostApplied::BrakeBoostApplied_off;
    string result = CDA1TenthPlugin::BSMConverter::toBrakeBoostAppliedString(brakeBoost);
    EXPECT_EQ("off", result);
    brakeBoost = BrakeBoostApplied::BrakeBoostApplied_on;
    result = CDA1TenthPlugin::BSMConverter::toBrakeBoostAppliedString(brakeBoost);
    EXPECT_EQ("on", result);

    brakeBoost = BrakeBoostApplied::BrakeBoostApplied_unavailable;
    result = CDA1TenthPlugin::BSMConverter::toBrakeBoostAppliedString(brakeBoost);
    EXPECT_EQ("unavailable", result);
}

TEST(BSMConverterTest, toAuxiliaryBrakeStatusString)
{
    AuxiliaryBrakeStatus_t auxBrakes = AuxiliaryBrakeStatus::AuxiliaryBrakeStatus_reserved;
    string result = CDA1TenthPlugin::BSMConverter::toAuxiliaryBrakeStatusString(auxBrakes);
    EXPECT_EQ("reserved", result);
    auxBrakes = AuxiliaryBrakeStatus::AuxiliaryBrakeStatus_unavailable;
    result = CDA1TenthPlugin::BSMConverter::toAuxiliaryBrakeStatusString(auxBrakes);
    EXPECT_EQ("unavailable", result);

    auxBrakes = AuxiliaryBrakeStatus::AuxiliaryBrakeStatus_off;
    result = CDA1TenthPlugin::BSMConverter::toAuxiliaryBrakeStatusString(auxBrakes);
    EXPECT_EQ("off", result);

    auxBrakes = AuxiliaryBrakeStatus::AuxiliaryBrakeStatus_on;
    result = CDA1TenthPlugin::BSMConverter::toAuxiliaryBrakeStatusString(auxBrakes);
    EXPECT_EQ("on", result);
}

TEST(BSMConverterTest, toBrakeAppliedStatusString)
{
    BrakeAppliedStatus_t wheel;
    uint8_t my_bytes_brakes[] = {8};
    wheel.bits_unused = 3;
    wheel.size = sizeof(my_bytes_brakes);
    wheel.buf = my_bytes_brakes;
    string result = CDA1TenthPlugin::BSMConverter::toBrakeAppliedStatusString(wheel);
    EXPECT_EQ("00001", result);
}

TEST(BSMConverterTest, toTemporaryIdString)
{
    TemporaryID_t id;
    uint8_t my_bytes_id[] = {0x12, 0x34, 0x56, 0xf8}; // Initialize with hex values
    id.buf = my_bytes_id;
    id.size = sizeof(my_bytes_id);
    string result = CDA1TenthPlugin::BSMConverter::toTemporaryIdString(id);
    EXPECT_EQ("123456f8", result);
}

TEST(BSMConverterTest, toTree)
{
    BasicSafetyMessage_t *bsm = (BasicSafetyMessage_t *)calloc(1, sizeof(BasicSafetyMessage_t));

    /**
     * Populate BSMcoreData
     */
    bsm->coreData.msgCnt = 1;
    uint8_t my_bytes_id[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10}; // Initialize with decimal values
    bsm->coreData.id.buf = my_bytes_id;
    bsm->coreData.id.size = sizeof(my_bytes_id);
    bsm->coreData.secMark = 1023;
    bsm->coreData.lat = 38954961;
    bsm->coreData.Long = -77149303;
    bsm->coreData.elev = 72;
    bsm->coreData.speed = 100;
    bsm->coreData.heading = 12;
    bsm->coreData.angle = 10;
    bsm->coreData.transmission = 0; // allow 0...7

    // position accuracy
    bsm->coreData.accuracy.orientation = 100;
    bsm->coreData.accuracy.semiMajor = 200;
    bsm->coreData.accuracy.semiMinor = 200;

    // Acceleration set
    bsm->coreData.accelSet.lat = 100;
    bsm->coreData.accelSet.Long = 300;
    bsm->coreData.accelSet.vert = 100;
    bsm->coreData.accelSet.yaw = 0;

    // populate brakes
    bsm->coreData.brakes.abs = 1;        // allow 0,1,2,3
    bsm->coreData.brakes.scs = 1;        // allow 0,1,2,3
    bsm->coreData.brakes.traction = 1;   // allow 0,1,2,3
    bsm->coreData.brakes.brakeBoost = 1; // allow 0,1,2
    bsm->coreData.brakes.auxBrakes = 1;  // allow 0,1,2,3
    uint8_t my_bytes_brakes[1] = {8};
    bsm->coreData.brakes.wheelBrakes.buf = my_bytes_brakes;          // allow 0,1,2,3,4
    bsm->coreData.brakes.wheelBrakes.size = sizeof(my_bytes_brakes); // allow 0,1,2,3,4
    bsm->coreData.brakes.wheelBrakes.bits_unused = 3;                // allow 0,1,2,3,4

    // vehicle size
    bsm->coreData.size.length = 500;
    bsm->coreData.size.width = 300;

    boost::property_tree::ptree tree = CDA1TenthPlugin::BSMConverter::toTree(*bsm);

    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.msgCnt"), 1);
    EXPECT_EQ(tree.get<std::string>("BasicSafetyMessage.coreData.id"), "010c0c0a");
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.secMark"), 1023);
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.lat"), 38954961);
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.long"), -77149303);
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.elev"), 72);
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.speed"), 100);
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.heading"), 12);
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.angle"), 10);
    EXPECT_EQ(tree.get<string>("BasicSafetyMessage.coreData.transmission.neutral"), "");
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.accuracy.orientation"), 100);
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.accuracy.semiMajor"), 200);
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.accuracy.semiMinor"), 200);
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.accelSet.lat"), 100);
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.accelSet.long"), 300);
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.accelSet.vert"), 100);
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.accelSet.yaw"), 0);
    EXPECT_EQ(tree.get<string>("BasicSafetyMessage.coreData.brakes.abs.off"), "");
    EXPECT_EQ(tree.get<string>("BasicSafetyMessage.coreData.brakes.scs.off"), "");
    EXPECT_EQ(tree.get<string>("BasicSafetyMessage.coreData.brakes.traction.off"), "");
    EXPECT_EQ(tree.get<string>("BasicSafetyMessage.coreData.brakes.brakeBoost.off"), "");
    EXPECT_EQ(tree.get<string>("BasicSafetyMessage.coreData.brakes.auxBrakes.off"), "");
    EXPECT_EQ(tree.get<std::string>("BasicSafetyMessage.coreData.brakes.wheelBrakes"), "00001");
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.size.length"), 500);
    EXPECT_EQ(tree.get<int>("BasicSafetyMessage.coreData.size.width"), 300);
}

TEST(BSMConverterTest, toJsonString)
{
    BasicSafetyMessage_t *bsm = (BasicSafetyMessage_t *)calloc(1, sizeof(BasicSafetyMessage_t));

    // Populate BSMcoreData
    bsm->coreData.msgCnt = 1;
    uint8_t my_bytes_id[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
    bsm->coreData.id.buf = my_bytes_id;
    bsm->coreData.id.size = sizeof(my_bytes_id);
    bsm->coreData.secMark = 1023;
    bsm->coreData.lat = 38954961;
    bsm->coreData.Long = -77149303;
    bsm->coreData.elev = 72;
    bsm->coreData.speed = 100;
    bsm->coreData.heading = 12;
    bsm->coreData.angle = 10;
    bsm->coreData.transmission = 0;

    // position accuracy
    bsm->coreData.accuracy.orientation = 100;
    bsm->coreData.accuracy.semiMajor = 200;
    bsm->coreData.accuracy.semiMinor = 200;

    // Acceleration set
    bsm->coreData.accelSet.lat = 100;
    bsm->coreData.accelSet.Long = 300;
    bsm->coreData.accelSet.vert = 100;
    bsm->coreData.accelSet.yaw = 0;

    // populate brakes
    bsm->coreData.brakes.abs = 1;
    bsm->coreData.brakes.scs = 1;
    bsm->coreData.brakes.traction = 1;
    bsm->coreData.brakes.brakeBoost = 1;
    bsm->coreData.brakes.auxBrakes = 1;
    uint8_t my_bytes_brakes[1] = {8};
    bsm->coreData.brakes.wheelBrakes.buf = my_bytes_brakes;
    bsm->coreData.brakes.wheelBrakes.size = sizeof(my_bytes_brakes);
    bsm->coreData.brakes.wheelBrakes.bits_unused = 3;

    // vehicle size
    bsm->coreData.size.length = 500;
    bsm->coreData.size.width = 300;

    boost::property_tree::ptree tree = CDA1TenthPlugin::BSMConverter::toTree(*bsm);
    std::string jsonString = CDA1TenthPlugin::BSMConverter::toJsonString(tree);

    std::string expectedJson = R"({
            "BasicSafetyMessage": {
                "coreData": {
                "id": "010c0c0a",
                "msgCnt": "1",
                "secMark": "1023",
                "lat": "38954961",
                "long": "-77149303",
                "elev": "72",
                "accuracy": {
                    "semiMajor": "200",
                    "semiMinor": "200",
                    "orientation": "100"
                },
                "transmission": {
                    "neutral": ""
                },
                "speed": "100",
                "heading": "12",
                "angle": "10",
                "accelSet": {
                    "yaw": "0",
                    "long": "300",
                    "lat": "100",
                    "vert": "100"
                },
                "brakes": {
                    "wheelBrakes": "00001",
                    "traction": {
                        "off": ""
                    },
                    "abs": {
                        "off": ""
                    },
                    "scs": {
                        "off": ""
                    },
                    "brakeBoost": {
                        "off": ""
                    },
                    "auxBrakes": {
                        "off": ""
                    }
                },
                "size": {
                    "width": "300",
                    "length": "500"
                    }
                }
            }
        })";
    boost::algorithm::erase_all(expectedJson, "\n");
    boost::algorithm::erase_all(expectedJson, " ");
    EXPECT_EQ(jsonString, expectedJson);
}