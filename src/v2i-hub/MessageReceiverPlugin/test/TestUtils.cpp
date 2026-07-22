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
#include <Utils.h>
#include <boost/uuid/uuid.hpp>
/**
 * @brief Helper static function to write buffer to a stringstream.  Used for JER encoding of SPDU data.
 * @param buffer The buffer to write to the stringstream.
 * @param size The size of the buffer to write.
 * @param app_key A pointer to a stringstream object to write the buffer to.
 * @return 0 on success, -1 on failure.
 */
static int writeToStringStream(const void *buffer, size_t size, void *app_key)
{
    // Cast app_key as stringstream
    std::stringstream *ss = static_cast<std::stringstream *>(app_key);
    if (!ss || !ss->good()) {
        return -1;
    }
    ss->write(static_cast<const char *>(buffer), size);
    return 0;
}
TEST(TestUtils, testDecodeSpdu){
    tmx::byte_stream spdu = byte_stream_decode("03806b0013680038422e1e7d2fc9ddd32f2e7971f4d3bf709b640800020d766174858008208214c8"
       "01011910c110c1002c0860853200304104299001021a2189a189806010c10c4c00a0820853200804"
       "644304430400d0218214c801c10410a6400c08688626862601c043043130");
    /* spdu content
        Ieee1609Dot2Data ::= {
        protocolVersion: 3
        content: unsecuredData: 
            00 13 68 00 38 42 2E 1E 7D 2F C9 DD D3 2F 2E 79 
            71 F4 D3 BF 70 9B 64 08 00 02 0D 76 61 74 85 80 
            08 20 82 14 C8 01 01 19 10 C1 10 C1 00 2C 08 60 
            85 32 00 30 41 04 29 90 01 02 1A 21 89 A1 89 80 
            60 10 C1 0C 4C 00 A0 82 08 53 20 08 04 64 43 04 
            43 04 00 D0 21 82 14 C8 01 C1 04 10 A6 40 0C 08 
            68 86 26 86 26 01 C0 43 04 31 30
        }
    */
    auto decoded = MessageReceiver::decodeSpdu(spdu, spdu.size());
    asn_fprint(stdout, &asn_DEF_Ieee1609Dot2Data, decoded.get());
    // All follow on assertions required decoded pointer
    ASSERT_TRUE(decoded != nullptr);
    // Convert to JSON and confirm message content
    std::stringstream ss;
    jer_encode(&asn_DEF_Ieee1609Dot2Data, decoded.get(), jer_encoder_flags_e::JER_F_MINIFIED,
				writeToStringStream,
				static_cast<void *>(&ss));
    std::string json=ss.str();
    std::string expectedJson = R"({"protocolVersion":3,"content":{"unsecuredData":"0013680038422E1E7D2FC9DDD32F2E7971F4D3BF709B640800020D766174858008208214C801011910C110C1002C0860853200304104299001021A2189A189806010C10C4C00A0820853200804644304430400D0218214C801C10410A6400C08688626862601C043043130"}})";
    EXPECT_EQ(json, expectedJson);
}
TEST(TestUtils, testDecodeSpduSignedWsmp) {
    tmx::byte_stream spdu = byte_stream_decode("0300e000001783380381004003808202630012825f38013020304bda054cdcf8a03d4dc408118602dc05117862c00913a1208b9f965e7cc4800000800005581e530239cbb717fc009ec6c02269c24127767e5979f3240000038000153cfb1808e22edfdf66027d8960b0000802b032000402d83a0009b64100d600c8af09045cfcb2f3e6640000040001289ba95c11ce11c74ea089da88047034b4d8084c584824eecfcb2f3e6880000070001a853257411d8080db2a10bd5f822eeb904695ac2d83c0009b64081160a400040ab0140002065805229c241173f2cbcf9a90000010001891721081a788284b267aa03e03e023018817a2380f60ea095c2504f44103630dc1a89f80e5d4d0f4b602ac336f0027d83cb99ab603133a12093bb3f2cbcf9b2000001c0014a5fc2ca069ba0a0a2f14e03d45401de04015727c0da13e08a48805225282ee3081c01ec1da2b4d78b86f3013ec4b018000807581a0004046c1c8004db21012b00e44584822e7e5979f372000002000115aa6c6808e20b3adc35592ce8e05050b5fddd55acef8c05050b4e605db0209af09049dd9f965e7ce1000000e00085732b938239c2c147521656baf2b541dd40a0a565cbdd814142e2d8171679c28ab44a1e809f60fb39381fad31496070000814b05400040b580900020620049539043e5bf9f3ef8766b000080000027ba7e004739251e5c8239c8014545410f96fe7cfbe1d9ac0002000000a353ad0047392c74c48239c8016553410f96fe7cfbe1d9ac0002000000a3d95630678a0a4fb34d608f1000600004000001590ab2e08f1264eeab04789004919ba7262d9bf1eb410f96fe7cfbe1d9ac0002000000bff2aec419e282930d358833c4fb00600320409700027df9a357e61500027dfcfda67615810101000301801631afb5fc255d0f50820838ca0cf8a2890d5c5e6f5b000329cf3cb58400a9830101800348010680032040978007008101205ff4100001828001838005008001f0400001270001870002bfee81821212260959f3b3abf3ad90388bf779fec9d2c8b44078bd32ad9eb4e7feaaa9b880827a382363f013f9ebd9ee7424aed03435c6f721dcd6d43578d27d85a429ff14c7ceaaa81559c27bc5f450e82d50de4c4b28bea5c361420402d231a58731aa2b96");

    auto decoded = MessageReceiver::decodeSpdu(spdu, spdu.size());
    asn_fprint(stdout, &asn_DEF_Ieee1609Dot2Data, decoded.get());
    // All follow on assertions required decoded pointer
    ASSERT_TRUE(decoded != nullptr);
    // Convert to JSON and confirm message content
    std::stringstream ss;
    jer_encode(&asn_DEF_Ieee1609Dot2Data, decoded.get(), jer_encoder_flags_e::JER_F_MINIFIED,
				writeToStringStream,
				static_cast<void *>(&ss));
    std::string json=ss.str();
    std::string expectedJson = R"({"protocolVersion":3,"content":{"unsecuredData":"0013680038422E1E7D2FC9DDD32F2E7971F4D3BF709B640800020D766174858008208214C801011910C110C1002C0860853200304104299001021A2189A189806010C10C4C00A0820853200804644304430400D0218214C801C10410A6400C08688626862601C043043130"}})";
    EXPECT_EQ(json, expectedJson);
}

TEST(TestUtils, testDecodeSpduInvalid){
    tmx::byte_stream spdu = byte_stream_decode("03806b0013680038422e1e7d2fc9ddd32f2e7971f4d3bf709b640800020d766174858008208214c8"
       "01011910c110c1002c0860853200304104299001021a2189a189806010c10c4c00a0820853200804"
       "644304430400d0218214c801c10410a6400c08688626862601c043043130");
    // Corrupt the SPDU data to make it invalid
    spdu[1] = 0xFF;
    EXPECT_THROW(MessageReceiver::decodeSpdu(spdu, spdu.size()), tmx::TmxException);
}

TEST(TestUtils, testDecodeSpduEmpty){
    tmx::byte_stream spdu;
    EXPECT_THROW({
        auto decoded = MessageReceiver::decodeSpdu(spdu, spdu.size());
    }, tmx::TmxException);
}

TEST(TestUtils, testUnwrapSpdu){
    tmx::byte_stream spdu = byte_stream_decode("03806b0013680038422e1e7d2fc9ddd32f2e7971f4d3bf709b640800020d766174858008208214c8"
       "01011910c110c1002c0860853200304104299001021a2189a189806010c10c4c00a0820853200804"
       "644304430400d0218214c801c10410a6400c08688626862601c043043130");
    auto decoded = MessageReceiver::decodeSpdu(spdu, spdu.size());
    tmx::byte_stream payload;
    uint psid = 0;
    bool psidSet = false;
    bool result = MessageReceiver::unwrapSpdu(decoded.get(), payload, psid, psidSet, 0);
    EXPECT_TRUE(result);
    EXPECT_FALSE(psidSet);
    std::string expectedPayloadHex = "0013680038422e1e7d2fc9ddd32f2e7971f4d3bf709b640800020d766174858008208214c801011910c110c1002c0860853200304104299001021a2189a189806010c10c4c00a0820853200804644304430400d0218214c801c10410a6400c08688626862601c043043130";
    std::string actualPayloadHex = byte_stream_encode(payload);
    EXPECT_EQ(actualPayloadHex, expectedPayloadHex);
}

TEST(TestUtils, testBuildRawSpdu){
    tmx::byte_stream payload = byte_stream_decode("0013680038422e1e7d2fc9ddd32f2e7971f4d3bf709b640800020d766174858008208214c8"
       "01011910c110c1002c0860853200304104299001021a2189a189806010c10c4c00a0820853200804"
       "644304430400d0218214c801c10410a6400c08688626862601c043043130");
    uint psid = 123;
    auto uuid = boost::uuids::random_generator()();
    // Convert UUID to byte stream for comparison
    tmx::byte_stream uuidBytes(uuid.begin(), uuid.end());
    auto spduMsg = MessageReceiver::buildRawSpdu(psid, payload, 1234567890, uuid);
    EXPECT_EQ(spduMsg.get_psid(), psid);
    EXPECT_EQ(spduMsg.get_spduData(), payload);
    EXPECT_EQ(spduMsg.get_timestampMs(), 1234567890);
    EXPECT_EQ(spduMsg.get_uuid(), uuidBytes);
    EXPECT_EQ(spduMsg.get_messageType(), "Missing ID");
}
