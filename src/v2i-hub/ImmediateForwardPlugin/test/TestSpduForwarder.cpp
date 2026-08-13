#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>
#include <string>

#include <RawSpdu.h>
#include <SpduForwarder.h>
#include <tmx/messages/byte_stream.hpp>
#include <tmx/messages/routeable_message.hpp>
#include "ImmediateForwardPlugin.h"

using namespace ImmediateForward;

namespace {

    // Unsecured BSM payload
    const std::string bsmHex =
        "001425067c0eb5842562e66e8a2b9ea6c96408b97fffffff900027d9637d07d0007fff8000640fa0";

    // Stands in for a secured BSM: the BSM payload with bytes around it
    const std::string spduHex =
        "3b996de867c7419ad2ec5652d133c1fb" + bsmHex + "8cfae61d57e2bfba0b68242e11d641c2";

    const std::string uuidHex = "0a303030303fdfa7";

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

    /**
     * Publish a RawSpdu into a routeable message the way MessageReceiverPlugin does, so that tests
     * read it back over the same path the plugin takes on receipt.
     */
    void publish(const tmx::messages::RawSpdu &rawSpdu, tmx::routeable_message &rMsg)
    {
        tmx::messages::RawSpdu payload = rawSpdu;
        rMsg.initialize<tmx::messages::RawSpdu>(payload, "", 0, IvpMsgFlags_RouteDSRC);
        rMsg.addDsrcMetadata(payload.get_psid());
    }

    std::string upper(const std::string &str)
    {
        std::string out = str;
        std::transform(out.begin(), out.end(), out.begin(),
                [](unsigned char c){ return std::toupper(c); });
        return out;
    }
}

TEST(TestSpduForwarder, readsBackFullSpduBytesAndPsid) {
    tmx::routeable_message rMsg;
    publish(buildSpdu(spduHex, "BSM", 0x20), rMsg);
    const tmx::routeable_message &constMsg = rMsg;

    tmx::messages::RawSpdu rawSpdu = getRawSpdu(constMsg.get_message());

    // The bytes forwarded to the radio must be the SPDU exactly as it was received
    EXPECT_EQ(upper(spduHex), toUpperHex(rawSpdu.get_fullByteData()));
    EXPECT_EQ(0x20, rawSpdu.get_psid());
    EXPECT_EQ("BSM", rawSpdu.get_messageType());
}

TEST(TestSpduForwarder, throwsWhenFullByteDataIsEmpty) {
    tmx::routeable_message rMsg;
    publish(buildSpdu("", "BSM", 0x20), rMsg);
    const tmx::routeable_message &constMsg = rMsg;

    EXPECT_THROW(getRawSpdu(constMsg.get_message()), tmx::TmxException);
}

TEST(TestSpduForwarder, throwsWhenMessageTypeIsUnidentified) {
    // getJ2735SubType records UNKNOWN_SUBTYPE when it cannot identify the unsecured payload, and
    // there is no tmxType such a message could be routed to
    tmx::routeable_message rMsg;
    publish(buildSpdu(spduHex, UNKNOWN_SUBTYPE, 0x20), rMsg);
    const tmx::routeable_message &constMsg = rMsg;

    EXPECT_THROW(getRawSpdu(constMsg.get_message()), tmx::TmxException);
}

TEST(TestSpduForwarder, throwsOnNullMessage) {
    EXPECT_THROW(getRawSpdu(nullptr), tmx::TmxException);
}

TEST(TestSpduForwarder, toUpperHexMatchesImmediateForwardPayloadFormat) {
    EXPECT_EQ("00A1FF", toUpperHex(tmx::byte_stream_decode("00a1ff")));
    EXPECT_EQ("", toUpperHex(tmx::byte_stream()));
}

TEST(TestSpduForwarder, formatsPsidUsingConfigurationConvention) {
    EXPECT_EQ("0x8002", toPsidHex(0x8002));
    EXPECT_EQ("0x20", toPsidHex(0x20));
    EXPECT_EQ("0xBFEE", toPsidHex(0xBFEE));
}

TEST(TestSpduForwarder, testIsSPDU){
    tmx::messages::RawSpdu rawSpdu;
    tmx::routeable_message rMsg;
    rMsg.initialize<tmx::messages::RawSpdu>(rawSpdu);
    auto ivpMsg = rMsg.get_message();
    EXPECT_TRUE(IsSPDU(ivpMsg));
}
