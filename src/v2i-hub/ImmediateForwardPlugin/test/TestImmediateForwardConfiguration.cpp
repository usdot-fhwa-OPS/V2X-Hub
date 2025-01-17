#include <gtest/gtest.h>
#include <ImmediateForwardConfiguration.h>
#include <rsu/RSUSpec.h>

using namespace ImmediateForward;

TEST(TestImmediateForwardConfiguration, txModeToString ) {
    EXPECT_EQ("CONT", txModeToString(TxMode::CONT));
    EXPECT_EQ("ALT", txModeToString(TxMode::ALT));
}

TEST(TestImmediateForwardConfiguration, stringToTxMode) {
    EXPECT_EQ(TxMode::CONT, stringToTxMode("CONT"));
    EXPECT_EQ(TxMode::ALT, stringToTxMode("ALT"));
    EXPECT_THROW(stringToTxMode("INVALID"), std::out_of_range);
}

// TEST(TestImmediateForwardConfiguration, rsuSpecToString ) {
//     EXPECT_EQ("RSU4.1", rsuSpecToString(RSU_SPEC::RSU_4_1));
//     EXPECT_EQ("NTCIP1218", rsuSpecToString(RSU_SPEC::NTCIP_1218));
// }

// TEST(TestImmediateForwardConfiguration, stringToRSUSpec) {
//     EXPECT_EQ(RSU_SPEC::RSU_4_1, stringToRSUSpec("RSU4.1"));
//     EXPECT_EQ(RSU_SPEC::NTCIP_1218, stringToRSUSpec("NTCIP1218"));
//     EXPECT_THROW(stringToRSUSpec("INVALID"), std::out_of_range);
// }

TEST(TestImmediateForwardConfiguration, parseImmediateForwardConfiguration ) {
    std::string jsonConfiguration = R"(
        [
            {
                "name": "East Intersection Cohda",
                "rsuSpec": "RSU4.1",
                "address": "127.0.0.1",
                "port": 3745,
                "txMode": "CONT",
                "signMessage": false,
                "messages": [
                    { "tmxType": "SPAT-P", "sendType": "SPAT", "psid": "0x8002", "channel": 183 },
                    { "tmxType": "MAP-P", "sendType": "MAP", "psid": "0x8002", "channel": 183 },
                    { "tmxType": "PSM-P", "sendType": "PSM", "psid": "0x27", "channel": 183 }, 
                    { "tmxType": "TIM", "sendType": "TIM", "psid": "0x8003", "channel": 183 },
                    { "tmxType": "TMSG07-P", "sendType": "TMSG07", "psid": "0x8002", "channel": 183 },
                    { "tmxType": "TMSG03-P", "sendType": "TMSG03", "psid": "0xBFEE", "channel": 183 },
                    { "tmxType": "TMSG05-P", "sendType": "TMSG05", "psid": "0x8003", "channel": 183 },
                    { "tmxType": "SSM-P", "sendType": "SSM", "psid": "0x8002", "channel": 183 },
                    { "tmxType": "SDSM", "sendType": "SDSM", "psid": "0x8010", "channel": 183 }
                ]
            }
        ]    
        )";
    auto immediateForwardConfigs =  parseImmediateForwardConfiguration(jsonConfiguration);

    EXPECT_EQ( immediateForwardConfigs.size(), 1);

    auto firstImfConfig = immediateForwardConfigs[0];

    EXPECT_EQ(firstImfConfig.name, "East Intersection Cohda");
    EXPECT_EQ(firstImfConfig.spec, tmx::utils::rsu::RSU_SPEC::RSU_4_1);
    EXPECT_EQ(firstImfConfig.address, "127.0.0.1");
    EXPECT_EQ(firstImfConfig.port, 3745);
    EXPECT_EQ(firstImfConfig.mode, TxMode::CONT);
    EXPECT_EQ(firstImfConfig.signMessage, false);
    EXPECT_EQ(firstImfConfig.messages.size(), 9);
    EXPECT_FALSE(firstImfConfig.enableHsm.has_value());
    EXPECT_FALSE(firstImfConfig.hsmUrl.has_value());

    auto firstMessage = firstImfConfig.messages[0];
    EXPECT_EQ(firstMessage.tmxType, "SPAT-P");
    EXPECT_EQ(firstMessage.sendType, "SPAT");
    EXPECT_EQ(firstMessage.psid, "0x8002");
    EXPECT_EQ(firstMessage.channel, 183);

}