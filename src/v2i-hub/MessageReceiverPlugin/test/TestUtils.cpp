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
TEST(TestUtils, testDecodeSpduSigned) {
    tmx::byte_stream spdu = byte_stream_decode("0381004003806d00136a003842be5e7d1049ddd32f2e7971f4d3bf7097b4080000e8c4513f05800820809c7c00810d0508e508e02c086028470030410138f80202320a0a4a0a406010c04e3e00a0820271f00604341423942380d02180a11c01c10d052e652e601008c82829282901c0430138f850018200027df99e1f5dfa1738128fd203e9ae11f3810101000301801631afb5fc255d0f50820838ca0cf8a2890d5c5e6f5b000329cf3cb58400a9830101800348010680032040978007008101205ff4100001828001838005008001f0400001270001870002bfee81821212260959f3b3abf3ad90388bf779fec9d2c8b44078bd32ad9eb4e7feaaa9b88083741bc528c5c7ece6f1f0f2d6ddd01f61517e717402eb08609741285557cc65054f51ef089c3e267d71e23d56ac088ef0b01d95161b21a90465cbfd9d0efa798b");
    /*
    Ieee1609Dot2Data ::= {
        protocolVersion: 3
        content: signedData: SignedData ::= {
            hashId: 0 (sha256)
            tbsData: ToBeSignedData ::= {
                payload: SignedDataPayload ::= {
                    data: Ieee1609Dot2Data ::= {
                        protocolVersion: 3
                        content: unsecuredData: 
                            00 13 6A 00 38 42 BE 5E 7D 10 49 DD D3 2F 2E 79 
                            71 F4 D3 BF 70 97 B4 08 00 00 E8 C4 51 3F 05 80 
                            08 20 80 9C 7C 00 81 0D 05 08 E5 08 E0 2C 08 60 
                            28 47 00 30 41 01 38 F8 02 02 32 0A 0A 4A 0A 40 
                            60 10 C0 4E 3E 00 A0 82 02 71 F0 06 04 34 14 23 
                            94 23 80 D0 21 80 A1 1C 01 C1 0D 05 2E 65 2E 60 
                            10 08 C8 28 29 28 29 01 C0 43 01 38 F8
                    }
                }
                headerInfo: HeaderInfo ::= {
                    psid: 130
                    generationTime: 701461006605818
                    generationLocation: ThreeDLocation ::= {
                        latitude: 389550735
                        longitude: -771495506
                        elevation: 4595
                    }
                }
            }
            signer: certificate: SequenceOfCertificate ::= {
                Certificate ::= {
                    version: 3
                    type: 1 (implicit)
                    issuer: sha256AndDigest: 16 31 AF B5 FC 25 5D 0F
                    toBeSigned: ToBeSignedCertificate ::= {
                        id: binaryId: 38 CA 0C F8 A2 89 0D 5C
                        cracaId: 5E 6F 5B
                        crlSeries: 3
                        validityPeriod: ValidityPeriod ::= {
                            start: 701447349
                            duration: hours: 169
                        }
                        region: identifiedRegion: SequenceOfIdentifiedRegion ::= {
                            countryOnly: 840
                        }
                        appPermissions: SequenceOfPsidSsp ::= {
                            PsidSsp ::= {
                                psid: 2113687
                                ssp: opaque: 00 81 01 20 5F F4 10
                            }
                            PsidSsp ::= {
                                psid: 130
                            }
                            PsidSsp ::= {
                                psid: 131
                                ssp: opaque: 00 80 01 F0 40
                            }
                            PsidSsp ::= {
                                psid: 39
                            }
                            PsidSsp ::= {
                                psid: 135
                            }
                            PsidSsp ::= {
                                psid: 49134
                            }
                        }
                        verifyKeyIndicator: reconstructionValue: compressed-y-0: 
                            12 12 26 09 59 F3 B3 AB F3 AD 90 38 8B F7 79 FE 
                            C9 D2 C8 B4 40 78 BD 32 AD 9E B4 E7 FE AA A9 B8
                    }
                }
            }
            signature: ecdsaNistP256Signature: EcdsaP256Signature ::= {
                rSig: compressed-y-1: 
                    74 1B C5 28 C5 C7 EC E6 F1 F0 F2 D6 DD D0 1F 61 
                    51 7E 71 74 02 EB 08 60 97 41 28 55 57 CC 65 05
                sSig: 
                    4F 51 EF 08 9C 3E 26 7D 71 E2 3D 56 AC 08 8E F0 
                    B0 1D 95 16 1B 21 A9 04 65 CB FD 9D 0E FA 79 8B
            }
        }
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
    std::string expectedJson = R"({"protocolVersion":3,"content":{"signedData":{"hashId":"sha256","tbsData":{"payload":{"data":{"protocolVersion":3,"content":{"unsecuredData":"00136A003842BE5E7D1049DDD32F2E7971F4D3BF7097B4080000E8C4513F05800820809C7C00810D0508E508E02C086028470030410138F80202320A0A4A0A406010C04E3E00A0820271F00604341423942380D02180A11C01C10D052E652E601008C82829282901C0430138F8"}}},"headerInfo":{"psid":130,"generationTime":701461006605818,"generationLocation":{"latitude":389550735,"longitude":-771495506,"elevation":4595}}},"signer":{"certificate":[{"version":3,"type":"implicit","issuer":{"sha256AndDigest":"1631AFB5FC255D0F"},"toBeSigned":{"id":{"binaryId":"38CA0CF8A2890D5C"},"cracaId":"5E6F5B","crlSeries":3,"validityPeriod":{"start":701447349,"duration":{"hours":169}},"region":{"identifiedRegion":[{"countryOnly":840}]},"appPermissions":[{"psid":2113687,"ssp":{"opaque":"008101205FF410"}},{"psid":130},{"psid":131,"ssp":{"opaque":"008001F040"}},{"psid":39},{"psid":135},{"psid":49134}],"verifyKeyIndicator":{"reconstructionValue":{"compressed-y-0":"1212260959F3B3ABF3AD90388BF779FEC9D2C8B44078BD32AD9EB4E7FEAAA9B8"}}}}]},"signature":{"ecdsaNistP256Signature":{"rSig":{"compressed-y-1":"741BC528C5C7ECE6F1F0F2D6DDD01F61517E717402EB08609741285557CC6505"},"sSig":"4F51EF089C3E267D71E23D56AC088EF0B01D95161B21A90465CBFD9D0EFA798B"}}}}})";
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
    tmx::byte_stream spdu = byte_stream_decode("0381004003806d00136a003842be5e7d1049ddd32f2e7971f4d3bf7097b4080000e8c4513f05800820809c7c00810d0508e508e02c086028470030410138f80202320a0a4a0a406010c04e3e00a0820271f00604341423942380d02180a11c01c10d052e652e601008c82829282901c0430138f850018200027df99e1f5dfa1738128fd203e9ae11f3810101000301801631afb5fc255d0f50820838ca0cf8a2890d5c5e6f5b000329cf3cb58400a9830101800348010680032040978007008101205ff4100001828001838005008001f0400001270001870002bfee81821212260959f3b3abf3ad90388bf779fec9d2c8b44078bd32ad9eb4e7feaaa9b88083741bc528c5c7ece6f1f0f2d6ddd01f61517e717402eb08609741285557cc65054f51ef089c3e267d71e23d56ac088ef0b01d95161b21a90465cbfd9d0efa798b");
    auto decoded = MessageReceiver::decodeSpdu(spdu, spdu.size());
    tmx::byte_stream payload;
    uint psid = 0;
    bool psidSet = false;
    bool result = MessageReceiver::unwrapSpdu(decoded.get(), payload, psid, psidSet, 0);
    EXPECT_TRUE(result);
    EXPECT_TRUE(psidSet);
    std::string expectedPayloadHex = "00136a003842be5e7d1049ddd32f2e7971f4d3bf7097b4080000e8c4513f05800820809c7c00810d0508e508e02c086028470030410138f80202320a0a4a0a406010c04e3e00a0820271f00604341423942380d02180a11c01c10d052e652e601008c82829282901c0430138f8";
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
