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
