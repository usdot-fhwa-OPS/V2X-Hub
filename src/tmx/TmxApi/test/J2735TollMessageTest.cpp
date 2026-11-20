/**
 * Unit tests for the J2735 2024 tolling messages (SAE J3217): TAM (37), TUM (38) and TUMack (39).
 */
#include <gtest/gtest.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

using namespace tmx;
using namespace tmx::messages;

namespace unit_test {
#if SAEJ2735_SPEC >= 2024
	// Tolling messages (J2735 2024 / SAE J3217). Test data: a TAM for a Suntrax toll zone and a TUM/TUMack with
	// made-up values, plus sample TAMs (A, B), TUM and TUMack from another encoder. The "cut" versions are the first
	// half of the same bytes, or a sample truncated TAM, so not valid ASN.1.
	static const std::vector<std::tuple<std::string, std::string, std::string>> TOLL_MESSAGES = {
		{"TAM", "TollAdvertisementMessage", "0025808e44000404101b62c20010221584060000005405cff7ea16a44000468000802333e4049d41f72f1461c05b8020008f06850a53e0c3905bd0000018ea1025c6333b9a018ea0fb97a333e40480400122a4f993105a9f3a292000000063a83ee5e8ccf901263a83ecbf8ccf9a4000000000000000000000000000000000000000000000000000000000000000009ab000"},
		{"TUM", "TollUsageMessage", "00262c0015017202040608040007c004488cd115599de2266aaef3377bbfc004488cd115599de2266aaef3377bbfc0"},
		{"TUMack", "TollUsageAckMessage", "0027080001020304024c90"},
		// Sample TAM A, TAM B, TUM and TUMack
		{"TAM", "TollAdvertisementMessage", "0025808e44000404101b62c20010221584060000005405cff7ea16a4400046800080266e880f9ea6f7d3940000fa0020008f06850a53e0c3905bd0000018f538148e66e84f018f537be9e66e880f80400122a4f993105a9f3a292000000063d4defa799ba203e63d4dea9499ba24f000000000000000000000000000000000000000000000000000000000000000009ab000"},
		{"TAM", "TollAdvertisementMessage", "0025808e4400040408110ac2002036c584060000005405c7f7ea16a4400046800080266e7ac833584f9c940000fa0020008f06850a53e0c3900000000018f537144e66e7c5818f537590e66e7ac800400122a4f993105a9f3a280080000063d4dd64399b9eb2063d4de2c399b9e99000000000000000000000000000000000000000000000000000000000000000009ab000"},
		{"TUM", "TollUsageMessage", "002640001501714b8528ba00000cc517fbf5451fcd4c8b48015074797e47e1db10d1ab6247c4dfe4b1b705066170e2c6c59334005e30a7041800005dc1000221584000"},
		{"TUMack", "TollUsageAckMessage", "00270800f9d88687004c90"}
	};
	static const std::vector<std::pair<std::string, std::string>> CUT_TOLL_MESSAGES = {
		{"TAM", "0025808e44000404101b62c20010221584060000005405cff7ea16a44000468000802333e4049d41f72f1461c05b8020008f06850a53e0c3905bd0000018ea1025c6333b9a018ea0fb"},
		{"TUM", "00262c0015017202040608040007c004488cd115599de2"},
		{"TUMack", "0027080001"},
		// Sample truncated TAM (from TAM B), and the first half of sample TAM A, TUM and TUMack
		{"TAM", "0025808e4400040408110ac2002036c584060000005405c7f7ea16a4400046800080266e7ac833584f9c940000fa0020008f06850a53e0"},
		{"TAM", "0025808e44000404101b62c20010221584060000005405cff7ea16a4400046800080266e880f9ea6f7d3940000fa0020008f06850a53e0c3905bd0000018f538148e66e84f018f537b"},
		{"TUM", "002640001501714b8528ba00000cc517fbf5451fcd4c8b48015074797e47e1db10"},
		{"TUMack", "00270800f9"}
	};

	TEST(J2735TollMessageTest, DecodeTollMessages) {
		J2735MessageFactory factory;
		for (const auto &[subtype, valueName, hex] : TOLL_MESSAGES) {
			tmx::byte_stream bytes = tmx::byte_stream_decode(hex);
			std::shared_ptr<MessageFrameEncodedMessage> msg(static_cast<MessageFrameEncodedMessage*>(factory.NewMessage(bytes)));
			ASSERT_NE(msg, nullptr) << subtype << ": " << factory.get_event().what();
			EXPECT_EQ(msg->get_subtype(), subtype);
			auto decoded = msg->decode_j2735_message();
			EXPECT_NE(decoded.to_string().find("<" + valueName + ">"), std::string::npos) << subtype;
		}
	}

	TEST(J2735TollMessageTest, CutTollMessagesFailToDecode) {
		J2735MessageFactory factory;
		for (const auto &[subtype, hex] : CUT_TOLL_MESSAGES) {
			tmx::byte_stream bytes = tmx::byte_stream_decode(hex);
			// The message ID is still readable, so the factory still creates the right type ...
			std::shared_ptr<MessageFrameEncodedMessage> msg(static_cast<MessageFrameEncodedMessage*>(factory.NewMessage(bytes)));
			ASSERT_NE(msg, nullptr) << subtype;
			EXPECT_EQ(msg->get_subtype(), subtype);
			// ... but the rest isn't valid ASN.1, so decoding fails.
			EXPECT_ANY_THROW(msg->decode_j2735_message()) << subtype;
		}
	}
#endif
}
