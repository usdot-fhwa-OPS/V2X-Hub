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
    EXPECT_THROW(stringToTxMode("INVALID"), tmx::TmxException);
}

TEST(TestImmediateForwardConfiguration, stringToSecurityLevel) {
    EXPECT_EQ(SecurityLevel::AUTH_PRIV, stringToSecurityLevel("authPriv"));
    EXPECT_EQ(SecurityLevel::AUTH_NO_PRIV, stringToSecurityLevel("authNoPriv"));
    EXPECT_EQ(SecurityLevel::NO_AUTH_NO_PRIV, stringToSecurityLevel("noAuthNoPriv"));
    EXPECT_THROW(stringToSecurityLevel("INVALID"), tmx::TmxException);
}
TEST(TestImmediateFowardConfiguration, securityLevelToString) {
    EXPECT_EQ("authPriv", securityLevelToString(SecurityLevel::AUTH_PRIV));
    EXPECT_EQ("authNoPriv", securityLevelToString(SecurityLevel::AUTH_NO_PRIV));
    EXPECT_EQ("noAuthNoPriv", securityLevelToString(SecurityLevel::NO_AUTH_NO_PRIV));
}

TEST(TestImmediateForwardConfiguration, parseImmediateForwardConfiguration ) {
    std::string jsonConfiguration = R"(
        [
            {
                "name": "East Intersection Cohda",
                "rsuSpec": "RSU4.1",
                "address": "127.0.0.1",
                "port": 3745,
                "txMode": "CONT",
                "signMessages": false,
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
TEST(TestImmediateForwardConfiguration, parseImmediateForwardConfigurationHsm ) {
    std::string jsonConfiguration = R"(
        [
            {
                "name": "East Intersection Cohda with HSM signing", 
                "rsuSpec": "RSU4.1", 
                "address": "127.0.0.1", 
                "port": 3745, 
                "txMode": "CONT", 
                "signMessages": true, 
                "enableHsm": true, 
                "hsmUrl": "http://<softhsm raspberrypi IP>:3000/v1/scms/", 
                "messages": 
                [ 
                    { 
                        "tmxType": "SPAT-P", 
                        "sendType": "SPAT",
                        "psid": "0x8002", 
                        "channel": 183 
                    },
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

    EXPECT_EQ(firstImfConfig.name, "East Intersection Cohda with HSM signing");
    EXPECT_EQ(firstImfConfig.spec, tmx::utils::rsu::RSU_SPEC::RSU_4_1);
    EXPECT_EQ(firstImfConfig.address, "127.0.0.1");
    EXPECT_EQ(firstImfConfig.port, 3745);
    EXPECT_EQ(firstImfConfig.mode, TxMode::CONT);
    EXPECT_EQ(firstImfConfig.signMessage, true);
    EXPECT_EQ(firstImfConfig.messages.size(), 9);
    ASSERT_TRUE(firstImfConfig.enableHsm.has_value());
    ASSERT_TRUE(firstImfConfig.hsmUrl.has_value());
    EXPECT_EQ(firstImfConfig.enableHsm.value(), true);
    EXPECT_EQ(firstImfConfig.hsmUrl.value(), "http://<softhsm raspberrypi IP>:3000/v1/scms/");

    auto firstMessage = firstImfConfig.messages[0];
    EXPECT_EQ(firstMessage.tmxType, "SPAT-P");
    EXPECT_EQ(firstMessage.sendType, "SPAT");
    EXPECT_EQ(firstMessage.psid, "0x8002");
    EXPECT_EQ(firstMessage.channel, 183);

}
TEST(TestImmediateForwardConfiguration, parseImmediateForwardConfigurationInvalidJson ) {
    std::string jsonConfiguration = R"(
        [
            invalidjson
            {
                "name": "East Intersection Cohda with HSM signing", 
                "rsuSpec": "RSU4.1", 
                "address": "127.0.0.1", 
                "port": 3745, 
                "txMode": "CONT", 
                "signMessages": true, 
                "enableHsm": true, 
                "hsmUrl": "http://<softhsm raspberrypi IP>:3000/v1/scms/", 
                "messages": 
                [ 
                    { 
                        "tmxType": "SPAT-P", 
                        "sendType": "SPAT",
                        "psid": "0x8002", 
                        "channel": 183 
                    },
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
   EXPECT_THROW(parseImmediateForwardConfiguration(jsonConfiguration), tmx::TmxException );
}


TEST(TestImmediateForwardConfiguration, parseImmediateForwardConfigurationNotArray ) {
    std::string jsonConfiguration = R"(
            {
                "name": "East Intersection Cohda with HSM signing", 
                "rsuSpec": "RSU4.1", 
                "address": "127.0.0.1", 
                "port": 3745, 
                "txMode": "CONT", 
                "signMessages": true, 
                "enableHsm": true, 
                "hsmUrl": "http://<softhsm raspberrypi IP>:3000/v1/scms/", 
                "messages": 
                [ 
                    { 
                        "tmxType": "SPAT-P", 
                        "sendType": "SPAT",
                        "psid": "0x8002", 
                        "channel": 183 
                    },
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
        )";
   EXPECT_THROW(parseImmediateForwardConfiguration(jsonConfiguration), tmx::TmxException );
}

TEST(TestImmediateForwardConfiguration, parseImmediateForwardConfigurationNTCIP1218 ) {
  

    std::string jsonConfiguration = R"(
        [
            {
                "name": "East Intersection Cohda",
                "rsuSpec": "NTCIP1218",
                "snmpAuth": {
                    "user": "authOnlyUser",
                    "securityLevel": "authPriv",
                    "community": "public",
                    "authProtocol": "SHA-512",
                    "authPassPhrase": "dummy123",
                    "privacyProtocol": "AES-256",
                    "privacyPassPhrase": "dummy123"
                },
                "address": "127.0.0.1",
                "port": 3745,
                "txMode": "CONT",
                "signMessages": false,
                "messages": [
                    { "tmxType": "SPAT-P", "sendType": "SPAT", "psid": "0x8002", "channel": 183 },
                    { "tmxType": "MAP-P", "sendType": "MAP", "psid": "0x8002", "channel": 183 }
                ]
            }
        ]    
        )";
    auto immediateForwardConfigs =  parseImmediateForwardConfiguration(jsonConfiguration);

    EXPECT_EQ( immediateForwardConfigs.size(), 1);

    auto firstImfConfig = immediateForwardConfigs[0];

    EXPECT_EQ(firstImfConfig.name, "East Intersection Cohda");
    EXPECT_EQ(firstImfConfig.spec, tmx::utils::rsu::RSU_SPEC::NTCIP_1218);
    EXPECT_EQ(firstImfConfig.address, "127.0.0.1");
    EXPECT_EQ(firstImfConfig.port, 3745);
    EXPECT_EQ(firstImfConfig.mode, TxMode::CONT);
    EXPECT_EQ(firstImfConfig.signMessage, false);
    EXPECT_EQ(firstImfConfig.messages.size(), 2);
    EXPECT_FALSE(firstImfConfig.enableHsm.has_value());
    EXPECT_FALSE(firstImfConfig.hsmUrl.has_value());

    ASSERT_TRUE(firstImfConfig.snmpAuth.has_value());
    auto snmpAuth = firstImfConfig.snmpAuth.value();
    EXPECT_EQ(snmpAuth.user, "authOnlyUser"  );
    EXPECT_EQ(snmpAuth.securityLevel, SecurityLevel::AUTH_PRIV);
    EXPECT_EQ(snmpAuth.community, "public");

    EXPECT_EQ(snmpAuth.authProtocol, "SHA-512");
    EXPECT_EQ(snmpAuth.authPassPhrase, "dummy123");
    EXPECT_EQ(snmpAuth.privProtocol, "AES-256");
    EXPECT_EQ(snmpAuth.privPassPhrase, "dummy123");

    auto firstMessage = firstImfConfig.messages[0];
    EXPECT_EQ(firstMessage.tmxType, "SPAT-P");
    EXPECT_EQ(firstMessage.sendType, "SPAT");
    EXPECT_EQ(firstMessage.psid, "0x8002");
    EXPECT_EQ(firstMessage.channel, 183);
}