#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>
#include <string>

#include <RawSpdu.h>
#include <SpduForwarder.h>
#include <tmx/messages/byte_stream.hpp>
#include <tmx/messages/routeable_message.hpp>

using namespace ImmediateForward;

namespace {

    // Unsecured BSM payload
    const std::string bsmHex =
        "001425067c0eb5842562e66e8a2b9ea6c96408b97fffffff900027d9637d07d0007fff8000640fa0";

    // Stand in for a secured BSM: the BSM payload with bytes around it
    const std::string spduHex =
        "3b996de867c7419ad2ec5652d133c1fb" + bsmHex + "8cfae61d57e2bfba0b68242e11d641c2";

    const std::string uuidHex = "0a303030303fdfa7";

    /**
     * Build a RawSpdu, publish it into a routeable message the way MessageReceiverPlugin does, and
     * return the underlying IvpMessage so tests exercise the same path the plugin takes on receipt.
     */
    tmx::messages::RawSpdu buildSpdu(const std::string &fullByteDataHex, const std::string &messageType, int psid)
    {
        tmx::messages::RawSpdu rawSpdu;
        rawSpdu.set_fullByteData(tmx::byte_stream_decode(fullByteDataHex));
        rawSpdu.set_msgByteData(tmx::byte_stream_decode(bsmHex));
        rawSpdu.set_uuid(tmx::byte_stream_decode(uuidHex));
        rawSpdu.set_messageType(messageType);
        rawSpdu.set_timestampMs(1718900000000);
        rawSpdu.set_psid(psid);
        return rawSpdu;
    }
}

TEST(TestSpduForwarder, extractsFullSpduPayloadAsUppercaseHex) {
    tmx::messages::RawSpdu rawSpdu = buildSpdu(spduHex, "BSM", 0x20);

    tmx::routeable_message rMsg;
    rMsg.initialize<tmx::messages::RawSpdu>(rawSpdu, "", 0, IvpMsgFlags_RouteDSRC);
    const tmx::routeable_message &constMsg = rMsg;

    SpduForwardData data = extractSpduForwardData(constMsg.get_message());

    std::string expected = spduHex;
    std::transform(expected.begin(), expected.end(), expected.begin(),
            [](unsigned char c){ return std::toupper(c); });

    // The bytes forwarded to the radio must be the SPDU exactly as it was received
    EXPECT_EQ(expected, data.payloadHex);
    EXPECT_EQ("0x20", data.psidHex);
    EXPECT_EQ("BSM", data.messageType);
}

TEST(TestSpduForwarder, throwsWhenFullByteDataIsEmpty) {
    tmx::messages::RawSpdu rawSpdu = buildSpdu("", "BSM", 0x20);

    tmx::routeable_message rMsg;
    rMsg.initialize<tmx::messages::RawSpdu>(rawSpdu, "", 0, IvpMsgFlags_RouteDSRC);
    const tmx::routeable_message &constMsg = rMsg;

    EXPECT_THROW(extractSpduForwardData(constMsg.get_message()), tmx::TmxException);
}

TEST(TestSpduForwarder, throwsWhenMessageTypeIsUnidentified) {
    // getJ2735SubType records "Unknown" when it cannot identify the unsecured payload, and there is
    // no tmxType to route such a message to
    tmx::messages::RawSpdu rawSpdu = buildSpdu(spduHex, "Unknown", 0x20);

    tmx::routeable_message rMsg;
    rMsg.initialize<tmx::messages::RawSpdu>(rawSpdu, "", 0, IvpMsgFlags_RouteDSRC);
    const tmx::routeable_message &constMsg = rMsg;

    EXPECT_THROW(extractSpduForwardData(constMsg.get_message()), tmx::TmxException);
}

TEST(TestSpduForwarder, throwsOnNullMessage) {
    EXPECT_THROW(extractSpduForwardData(nullptr), tmx::TmxException);
}

TEST(TestSpduForwarder, formatsPsidUsingConfigurationConvention) {
    EXPECT_EQ("0x8002", toPsidHex(0x8002));
    EXPECT_EQ("0x20", toPsidHex(0x20));
    EXPECT_EQ("0xBFEE", toPsidHex(0xBFEE));
}

TEST(TestSpduForwarder, psidMatchesIgnoresPrefixCaseAndLeadingZeros) {
    EXPECT_TRUE(psidMatches("0x0027", "0x27"));
    EXPECT_TRUE(psidMatches("0xbfee", "0xBFEE"));
    EXPECT_TRUE(psidMatches("8002", "0x8002"));
    EXPECT_TRUE(psidMatches("0x0", "0x00"));

    EXPECT_FALSE(psidMatches("0x8002", "0x27"));
    EXPECT_FALSE(psidMatches("0x8002", "0x8003"));
}
