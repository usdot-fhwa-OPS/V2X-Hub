#include <gtest/gtest.h>
#include <tmx/messages/TmxJ2735.hpp>
#include <tmx/messages/TmxJ2735Codec.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/shared_ptr.hpp>
#include <sstream> // Required for std::ostringstream
#include <memory>
#include <tuple>
#include <vector>
#include <MessageLogger.hpp>
#include <boost/core/null_deleter.hpp>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;


namespace JSONMessageLoggerPlugin {
    class TestMessageLoggerFixture : public ::testing::Test {
        public:
            std::ostringstream oss;


    protected:
        void SetUp() override {
            // Create a text stream backend
            boost::shared_ptr<sinks::text_ostream_backend> backend =
                boost::make_shared<sinks::text_ostream_backend>();
            // Add the ostringstream to the backend
            // Note: boost::shared_ptr is used to manage the stream lifetime,
            // and boost::null_deleter is used if you don't want the backend to delete the stream.
            // In this case, since oss is on the stack, it's safer to manage its lifetime manually.
            backend->add_stream(boost::shared_ptr<std::ostream>(&oss, boost::null_deleter()));

            // Create a sink front-end and attach the backend
            typedef sinks::synchronous_sink<sinks::text_ostream_backend> ostream_sink;
            boost::shared_ptr<ostream_sink> sink(new ostream_sink(backend));
            // Register the sink with the logging core
            logging::core::get()->add_sink(sink);

            // Add common attributes (like LineID, TimeStamp)
            logging::add_common_attributes();

        }

        void TearDown() override {
            // Cleanup code if needed
            boost::shared_ptr<logging::core> core = logging::core::get();

            core->flush();
            core->remove_all_sinks();


        }
    };

    // Add tests for MessageLogger functions here
    TEST(TestMessageLogger, routeableMessageToJsonStringValidMessage) {
        // Create a mock routeable_message containing a valid J2735 MessageFrame
        tmx::byte_stream bytes = tmx::byte_stream_decode("0012822138713020311b27c535a4e8ac6b49c0c5109602dc2522f100001e480cc800000002b5962bc282900000000000000e22f62aff9080a400000000000000ac0600018096068000c06481cc800000002b5d282c282a800000000000000000000000000388bd91bfe4202a8000000000000000000000000000ac26000180561e4000c04200aa400000002b575dc809c45eea9fd405050806a900000000ad65600027387bb17f512022200000002a8bd20502711501421805052e00a60302828560f0000802b014000402240c4400000005541a42604e72e02c5ed02828505a1a02b0f8000401586900020108019900000002a7481f602711700032a814282804072100b32000000054bf03ec04e22dfd46da02850501e04c4804480000000a89f9db4282800000e22ccb6fe50a0a000003ecd4c157fc80a0a000004142b058000201583a00010112051200000002a26f6210a0a00000388b2f8bfa8202800000d4cb37fb4080a000000ac36000080561640004042002240000000a89fa2e809c4599a200004fb353001ff840a0a1009120000000544ef3f004e22cd49007027d9a97a6ffe0050512044200000002a736d7302738700fca20dff3c951581c0004012c15000200c904210000000152f36c8050510000000007397016c8e040510000000005bfc78c41014400000000056170001002b09200080210077200000005510dae604e70e01f8dc1c0039b0201ee40000000aa7675e809ce1c0531aab7f572b009f6");
        tmx::messages::MapDataEncodedMessage encodedMap;
        encodedMap.set_data(bytes);
        tmx::routeable_message *rMsg = dynamic_cast<tmx::routeable_message *>(&encodedMap);

        // Call function under test
        std::string jsonStr = routeableMessageToJsonString(*rMsg);
        EXPECT_FALSE(jsonStr.empty());
        std::string expectJson = "{\"messageId\":\"18\",\"value\":{\"MapData\":{\"msgIssueRevision\":\"113\",\"layerType\":\"intersectionData\",\"layerID\":\"1\",\"intersections\":[{\"id\":{\"id\":\"36243\"},\"revision\":\"113\",\"refPoint\":{\"lat\":\"-84\",\"long\":\"-4410\",\"elevation\":\"150\"},\"laneWidth\":\"366\",\"speedLimits\":[{\"type\":\"vehicleMaxSpeed\",\"speed\":\"1118\"},{\"type\":\"vehicleMinSpeed\",\"speed\":\"0\"}],\"laneSet\":[{\"laneID\":\"6\",\"ingressApproach\":\"6\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"1381\",\"y\":\"175\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"2837\",\"y\":\"-7\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}]}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"1\",\"maneuver\":\"8000\"},\"signalGroup\":\"6\",\"connectionID\":\"2\"},{\"connectingLane\":{\"lane\":\"3\",\"maneuver\":\"4000\"},\"signalGroup\":\"6\",\"connectionID\":\"3\"}]},{\"laneID\":\"14\",\"ingressApproach\":\"6\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"1396\",\"y\":\"523\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"2851\",\"y\":\"-7\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}]}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"9\",\"maneuver\":\"8000\"},\"signalGroup\":\"6\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"15\",\"maneuver\":\"2000\"},\"signalGroup\":\"6\",\"connectionID\":\"2\"}]},{\"laneID\":\"5\",\"egressApproach\":\"5\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"1373\",\"y\":\"-142\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"2986\",\"y\":\"-22\"}},\"attributes\":{\"dElevation\":\"10\"}}]}},{\"laneID\":\"13\",\"egressApproach\":\"5\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"1381\",\"y\":\"-512\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"2993\",\"y\":\"-22\"}}}]}},{\"laneID\":\"4\",\"ingressApproach\":\"4\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"189\",\"y\":\"-1531\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY3\":{\"x\":\"10\",\"y\":\"-1780\"}},\"attributes\":{\"dElevation\":\"10\"}},{\"delta\":{\"node-XY4\":{\"x\":\"5\",\"y\":\"-2557\"}},\"attributes\":{\"dElevation\":\"10\"}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"7\",\"maneuver\":\"8000\"},\"signalGroup\":\"4\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"1\",\"maneuver\":\"4000\"},\"signalGroup\":\"4\",\"connectionID\":\"2\"}]},{\"laneID\":\"12\",\"ingressApproach\":\"4\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"525\",\"y\":\"-1517\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"22\",\"y\":\"-2579\"}},\"attributes\":{\"dElevation\":\"10\"}},{\"delta\":{\"node-XY3\":{\"x\":\"45\",\"y\":\"-1840\"}}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"15\",\"maneuver\":\"8000\"},\"signalGroup\":\"4\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"13\",\"maneuver\":\"2000\"},\"signalGroup\":\"4\",\"connectionID\":\"2\"}]},{\"laneID\":\"3\",\"egressApproach\":\"3\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-184\",\"y\":\"-1546\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"0\",\"y\":\"-2475\"}},\"attributes\":{\"dElevation\":\"20\"}},{\"delta\":{\"node-XY3\":{\"x\":\"4\",\"y\":\"-1934\"}}}]}},{\"laneID\":\"11\",\"egressApproach\":\"3\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-520\",\"y\":\"-1546\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-22\",\"y\":\"-2342\"}},\"attributes\":{\"dElevation\":\"20\"}},{\"delta\":{\"node-XY3\":{\"x\":\"15\",\"y\":\"-2010\"}}}]}},{\"laneID\":\"2\",\"ingressApproach\":\"2\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-1410\",\"y\":\"-147\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-2469\",\"y\":\"-27\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-10\"}},{\"delta\":{\"node-XY6\":{\"x\":\"-13291\",\"y\":\"-56\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"10\"}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"5\",\"maneuver\":\"8000\"},\"signalGroup\":\"2\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"7\",\"maneuver\":\"4000\"},\"signalGroup\":\"2\",\"connectionID\":\"2\"}]},{\"laneID\":\"10\",\"ingressApproach\":\"2\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-1425\",\"y\":\"-479\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-2575\",\"y\":\"-22\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}]}},{\"delta\":{\"node-XY6\":{\"x\":\"-13133\",\"y\":\"-76\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}]}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"13\",\"maneuver\":\"8000\"},\"signalGroup\":\"2\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"11\",\"maneuver\":\"2000\"},\"signalGroup\":\"2\",\"connectionID\":\"2\"}]},{\"laneID\":\"1\",\"egressApproach\":\"1\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-1410\",\"y\":\"186\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-2456\",\"y\":\"0\"}},\"attributes\":{\"dElevation\":\"-10\"}},{\"delta\":{\"node-XY6\":{\"x\":\"-13312\",\"y\":\"-31\"}},\"attributes\":{\"dElevation\":\"10\"}}]}},{\"laneID\":\"9\",\"egressApproach\":\"1\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-1417\",\"y\":\"504\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-2396\",\"y\":\"7\"}},\"attributes\":{\"dElevation\":\"-10\"}},{\"delta\":{\"node-XY6\":{\"x\":\"-13357\",\"y\":\"-16\"}},\"attributes\":{\"dElevation\":\"10\"}}]}},{\"laneID\":\"8\",\"ingressApproach\":\"8\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-202\",\"y\":\"1395\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"15\",\"y\":\"2372\"}}},{\"delta\":{\"node-XY4\":{\"x\":\"-7\",\"y\":\"3221\"}}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"3\",\"maneuver\":\"8000\"},\"signalGroup\":\"8\",\"connectionID\":\"2\"},{\"connectingLane\":{\"lane\":\"5\",\"maneuver\":\"4000\"},\"signalGroup\":\"8\",\"connectionID\":\"3\"}]},{\"laneID\":\"16\",\"ingressApproach\":\"8\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-538\",\"y\":\"1424\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"22\",\"y\":\"2332\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}]}},{\"delta\":{\"node-XY4\":{\"x\":\"-15\",\"y\":\"3170\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}]}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"11\",\"maneuver\":\"8000\"},\"signalGroup\":\"8\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"9\",\"maneuver\":\"2000\"},\"signalGroup\":\"8\",\"connectionID\":\"2\"}]},{\"laneID\":\"7\",\"egressApproach\":\"7\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"134\",\"y\":\"1395\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"15\",\"y\":\"2268\"}}},{\"delta\":{\"node-XY4\":{\"x\":\"0\",\"y\":\"3288\"}}}]}},{\"laneID\":\"15\",\"egressApproach\":\"7\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"473\",\"y\":\"1402\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"20\",\"y\":\"2261\"}}},{\"delta\":{\"node-XY4\":{\"x\":\"-22\",\"y\":\"3244\"}},\"attributes\":{\"dElevation\":\"-10\"}}]}}]}]}}}\n";
        std::size_t delimiterPos = jsonStr.find(" : ");
        EXPECT_NE(delimiterPos, std::string::npos) << "JSON output missing delimiter ' : '";
        // Extract timestamp and JSON
        std::string timestampStr = jsonStr.substr(0, delimiterPos);
        std::string jsonOnly = jsonStr.substr(delimiterPos + 3);
        // Check that timestamp is numeric
        EXPECT_TRUE(!timestampStr.empty()) << "Timestamp missing";
        for (char c : timestampStr)
        {
            EXPECT_TRUE(std::isdigit(c)) << "Timestamp contains non-digit characters";
        }
        // Compare only JSON portion
        EXPECT_EQ(expectJson, jsonOnly);
    }

    TEST(TestMessageLogger, routableMessageToJsonStringInvalidMessage) {
        // Create a mock routeable_message containing an invalid message with first 2 bytes removed
        tmx::byte_stream bytes = tmx::byte_stream_decode("822138713020311b27c535a4e8ac6b49c0c5109602dc2522f100001e480cc800000002b5962bc282900000000000000e22f62aff9080a400000000000000ac0600018096068000c06481cc800000002b5d282c282a800000000000000000000000000388bd91bfe4202a8000000000000000000000000000ac26000180561e4000c04200aa400000002b575dc809c45eea9fd405050806a900000000ad65600027387bb17f512022200000002a8bd20502711501421805052e00a60302828560f0000802b014000402240c4400000005541a42604e72e02c5ed02828505a1a02b0f8000401586900020108019900000002a7481f602711700032a814282804072100b32000000054bf03ec04e22dfd46da02850501e04c4804480000000a89f9db4282800000e22ccb6fe50a0a000003ecd4c157fc80a0a000004142b058000201583a00010112051200000002a26f6210a0a00000388b2f8bfa8202800000d4cb37fb4080a000000ac36000080561640004042002240000000a89fa2e809c4599a200004fb353001ff840a0a1009120000000544ef3f004e22cd49007027d9a97a6ffe0050512044200000002a736d7302738700fca20dff3c951581c0004012c15000200c904210000000152f36c8050510000000007397016c8e040510000000005bfc78c41014400000000056170001002b09200080210077200000005510dae604e70e01f8dc1c0039b0201ee40000000aa7675e809ce1c0531aab7f572b009f6");
        tmx::messages::MapDataEncodedMessage encodedMap;
        encodedMap.set_data(bytes);
        tmx::routeable_message *rMsg = dynamic_cast<tmx::routeable_message *>(&encodedMap);

        // Call function under test
        EXPECT_THROW(routeableMessageToJsonString(*rMsg), boost::wrapexcept<tmx::J2735Exception>);

    }

    TEST_F(TestMessageLoggerFixture, logRouteableMessage) {
        // Create a mock routeable_message containing a valid J2735 MessageFrame
        tmx::byte_stream bytes = tmx::byte_stream_decode("0012822138713020311b27c535a4e8ac6b49c0c5109602dc2522f100001e480cc800000002b5962bc282900000000000000e22f62aff9080a400000000000000ac0600018096068000c06481cc800000002b5d282c282a800000000000000000000000000388bd91bfe4202a8000000000000000000000000000ac26000180561e4000c04200aa400000002b575dc809c45eea9fd405050806a900000000ad65600027387bb17f512022200000002a8bd20502711501421805052e00a60302828560f0000802b014000402240c4400000005541a42604e72e02c5ed02828505a1a02b0f8000401586900020108019900000002a7481f602711700032a814282804072100b32000000054bf03ec04e22dfd46da02850501e04c4804480000000a89f9db4282800000e22ccb6fe50a0a000003ecd4c157fc80a0a000004142b058000201583a00010112051200000002a26f6210a0a00000388b2f8bfa8202800000d4cb37fb4080a000000ac36000080561640004042002240000000a89fa2e809c4599a200004fb353001ff840a0a1009120000000544ef3f004e22cd49007027d9a97a6ffe0050512044200000002a736d7302738700fca20dff3c951581c0004012c15000200c904210000000152f36c8050510000000007397016c8e040510000000005bfc78c41014400000000056170001002b09200080210077200000005510dae604e70e01f8dc1c0039b0201ee40000000aa7675e809ce1c0531aab7f572b009f6");
        tmx::messages::MapDataEncodedMessage encodedMap;
        encodedMap.set_data(bytes);
        tmx::routeable_message *rMsg = dynamic_cast<tmx::routeable_message *>(&encodedMap);
        boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level, std::string> lg(boost::log::keywords::channel = "test");
        logRouteableMessage(*rMsg, lg);

        oss.flush();
        std::string logOutput = oss.str();
        EXPECT_FALSE(logOutput.empty());
        std::string expectJson = "{\"messageId\":\"18\",\"value\":{\"MapData\":{\"msgIssueRevision\":\"113\",\"layerType\":\"intersectionData\",\"layerID\":\"1\",\"intersections\":[{\"id\":{\"id\":\"36243\"},\"revision\":\"113\",\"refPoint\":{\"lat\":\"-84\",\"long\":\"-4410\",\"elevation\":\"150\"},\"laneWidth\":\"366\",\"speedLimits\":[{\"type\":\"vehicleMaxSpeed\",\"speed\":\"1118\"},{\"type\":\"vehicleMinSpeed\",\"speed\":\"0\"}],\"laneSet\":[{\"laneID\":\"6\",\"ingressApproach\":\"6\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"1381\",\"y\":\"175\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"2837\",\"y\":\"-7\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}]}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"1\",\"maneuver\":\"8000\"},\"signalGroup\":\"6\",\"connectionID\":\"2\"},{\"connectingLane\":{\"lane\":\"3\",\"maneuver\":\"4000\"},\"signalGroup\":\"6\",\"connectionID\":\"3\"}]},{\"laneID\":\"14\",\"ingressApproach\":\"6\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"1396\",\"y\":\"523\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"2851\",\"y\":\"-7\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}]}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"9\",\"maneuver\":\"8000\"},\"signalGroup\":\"6\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"15\",\"maneuver\":\"2000\"},\"signalGroup\":\"6\",\"connectionID\":\"2\"}]},{\"laneID\":\"5\",\"egressApproach\":\"5\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"1373\",\"y\":\"-142\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"2986\",\"y\":\"-22\"}},\"attributes\":{\"dElevation\":\"10\"}}]}},{\"laneID\":\"13\",\"egressApproach\":\"5\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"1381\",\"y\":\"-512\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"2993\",\"y\":\"-22\"}}}]}},{\"laneID\":\"4\",\"ingressApproach\":\"4\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"189\",\"y\":\"-1531\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY3\":{\"x\":\"10\",\"y\":\"-1780\"}},\"attributes\":{\"dElevation\":\"10\"}},{\"delta\":{\"node-XY4\":{\"x\":\"5\",\"y\":\"-2557\"}},\"attributes\":{\"dElevation\":\"10\"}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"7\",\"maneuver\":\"8000\"},\"signalGroup\":\"4\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"1\",\"maneuver\":\"4000\"},\"signalGroup\":\"4\",\"connectionID\":\"2\"}]},{\"laneID\":\"12\",\"ingressApproach\":\"4\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"525\",\"y\":\"-1517\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"22\",\"y\":\"-2579\"}},\"attributes\":{\"dElevation\":\"10\"}},{\"delta\":{\"node-XY3\":{\"x\":\"45\",\"y\":\"-1840\"}}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"15\",\"maneuver\":\"8000\"},\"signalGroup\":\"4\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"13\",\"maneuver\":\"2000\"},\"signalGroup\":\"4\",\"connectionID\":\"2\"}]},{\"laneID\":\"3\",\"egressApproach\":\"3\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-184\",\"y\":\"-1546\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"0\",\"y\":\"-2475\"}},\"attributes\":{\"dElevation\":\"20\"}},{\"delta\":{\"node-XY3\":{\"x\":\"4\",\"y\":\"-1934\"}}}]}},{\"laneID\":\"11\",\"egressApproach\":\"3\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-520\",\"y\":\"-1546\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-22\",\"y\":\"-2342\"}},\"attributes\":{\"dElevation\":\"20\"}},{\"delta\":{\"node-XY3\":{\"x\":\"15\",\"y\":\"-2010\"}}}]}},{\"laneID\":\"2\",\"ingressApproach\":\"2\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-1410\",\"y\":\"-147\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-2469\",\"y\":\"-27\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-10\"}},{\"delta\":{\"node-XY6\":{\"x\":\"-13291\",\"y\":\"-56\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"10\"}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"5\",\"maneuver\":\"8000\"},\"signalGroup\":\"2\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"7\",\"maneuver\":\"4000\"},\"signalGroup\":\"2\",\"connectionID\":\"2\"}]},{\"laneID\":\"10\",\"ingressApproach\":\"2\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-1425\",\"y\":\"-479\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-2575\",\"y\":\"-22\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}]}},{\"delta\":{\"node-XY6\":{\"x\":\"-13133\",\"y\":\"-76\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"}]}]}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"13\",\"maneuver\":\"8000\"},\"signalGroup\":\"2\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"11\",\"maneuver\":\"2000\"},\"signalGroup\":\"2\",\"connectionID\":\"2\"}]},{\"laneID\":\"1\",\"egressApproach\":\"1\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-1410\",\"y\":\"186\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-2456\",\"y\":\"0\"}},\"attributes\":{\"dElevation\":\"-10\"}},{\"delta\":{\"node-XY6\":{\"x\":\"-13312\",\"y\":\"-31\"}},\"attributes\":{\"dElevation\":\"10\"}}]}},{\"laneID\":\"9\",\"egressApproach\":\"1\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-1417\",\"y\":\"504\"}},\"attributes\":{\"dElevation\":\"-60\"}},{\"delta\":{\"node-XY4\":{\"x\":\"-2396\",\"y\":\"7\"}},\"attributes\":{\"dElevation\":\"-10\"}},{\"delta\":{\"node-XY6\":{\"x\":\"-13357\",\"y\":\"-16\"}},\"attributes\":{\"dElevation\":\"10\"}}]}},{\"laneID\":\"8\",\"ingressApproach\":\"8\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-202\",\"y\":\"1395\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"15\",\"y\":\"2372\"}}},{\"delta\":{\"node-XY4\":{\"x\":\"-7\",\"y\":\"3221\"}}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"3\",\"maneuver\":\"8000\"},\"signalGroup\":\"8\",\"connectionID\":\"2\"},{\"connectingLane\":{\"lane\":\"5\",\"maneuver\":\"4000\"},\"signalGroup\":\"8\",\"connectionID\":\"3\"}]},{\"laneID\":\"16\",\"ingressApproach\":\"8\",\"laneAttributes\":{\"directionalUse\":\"80\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"-538\",\"y\":\"1424\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}],\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"22\",\"y\":\"2332\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}]}},{\"delta\":{\"node-XY4\":{\"x\":\"-15\",\"y\":\"3170\"}},\"attributes\":{\"data\":[{\"speedLimits\":[{\"type\":\"unknown\",\"speed\":\"0\"},{\"type\":\"unknown\",\"speed\":\"0\"}]}]}}]},\"connectsTo\":[{\"connectingLane\":{\"lane\":\"11\",\"maneuver\":\"8000\"},\"signalGroup\":\"8\",\"connectionID\":\"1\"},{\"connectingLane\":{\"lane\":\"9\",\"maneuver\":\"2000\"},\"signalGroup\":\"8\",\"connectionID\":\"2\"}]},{\"laneID\":\"7\",\"egressApproach\":\"7\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"134\",\"y\":\"1395\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"15\",\"y\":\"2268\"}}},{\"delta\":{\"node-XY4\":{\"x\":\"0\",\"y\":\"3288\"}}}]}},{\"laneID\":\"15\",\"egressApproach\":\"7\",\"laneAttributes\":{\"directionalUse\":\"40\",\"sharedWith\":\"0000\",\"laneType\":{\"vehicle\":\"00\"}},\"nodeList\":{\"nodes\":[{\"delta\":{\"node-XY3\":{\"x\":\"473\",\"y\":\"1402\"}},\"attributes\":{\"dElevation\":\"-50\"}},{\"delta\":{\"node-XY4\":{\"x\":\"20\",\"y\":\"2261\"}}},{\"delta\":{\"node-XY4\":{\"x\":\"-22\",\"y\":\"3244\"}},\"attributes\":{\"dElevation\":\"-10\"}}]}}]}]}}}\n";
        std::size_t delimiterPos = logOutput.find(" : ");
        EXPECT_NE(delimiterPos, std::string::npos) << "JSON output missing delimiter ' : '";
        // Extract timestamp and JSON
        std::string timestampStr = logOutput.substr(0, delimiterPos);
        std::string jsonOnly = logOutput.substr(delimiterPos + 3);
        // Check that timestamp is numeric
        EXPECT_TRUE(!timestampStr.empty()) << "Timestamp missing";
        for (char c : timestampStr)
        {
            EXPECT_TRUE(std::isdigit(c)) << "Timestamp contains non-digit characters";
        }
        // Compare only JSON portion
        EXPECT_EQ(expectJson, jsonOnly);
    }

#if SAEJ2735_SPEC >= 2024
    // Tolling messages (J2735 2024 / SAE J3217): a Suntrax TAM and a TUM/TUMack with made-up values, plus sample TAMs
    // (A, B), TUM and TUMack from another encoder. The cut versions are the first half of the same bytes (or, for
    // TAM B, a sample truncated TAM), so not valid ASN.1.
    static const std::vector<std::tuple<std::string, std::string, std::string, std::string>> TOLL_MESSAGES = {
        // subtype, value name in the JSON, valid hex, cut hex
        {"TAM", "TollAdvertisementMessage", "0025808e44000404101b62c20010221584060000005405cff7ea16a44000468000802333e4049d41f72f1461c05b8020008f06850a53e0c3905bd0000018ea1025c6333b9a018ea0fb97a333e40480400122a4f993105a9f3a292000000063a83ee5e8ccf901263a83ecbf8ccf9a4000000000000000000000000000000000000000000000000000000000000000009ab000", "0025808e44000404101b62c20010221584060000005405cff7ea16a44000468000802333e4049d41f72f1461c05b8020008f06850a53e0c3905bd0000018ea1025c6333b9a018ea0fb"},
        {"TUM", "TollUsageMessage", "00262c0015017202040608040007c004488cd115599de2266aaef3377bbfc004488cd115599de2266aaef3377bbfc0", "00262c0015017202040608040007c004488cd115599de2"},
        {"TUMack", "TollUsageAckMessage", "0027080001020304024c90", "0027080001"},
        // Sample TAM A, TAM B (with the sample truncated TAM), TUM and TUMack
        {"TAM", "TollAdvertisementMessage", "0025808e44000404101b62c20010221584060000005405cff7ea16a4400046800080266e880f9ea6f7d3940000fa0020008f06850a53e0c3905bd0000018f538148e66e84f018f537be9e66e880f80400122a4f993105a9f3a292000000063d4defa799ba203e63d4dea9499ba24f000000000000000000000000000000000000000000000000000000000000000009ab000", "0025808e44000404101b62c20010221584060000005405cff7ea16a4400046800080266e880f9ea6f7d3940000fa0020008f06850a53e0c3905bd0000018f538148e66e84f018f537b"},
        {"TAM", "TollAdvertisementMessage", "0025808e4400040408110ac2002036c584060000005405c7f7ea16a4400046800080266e7ac833584f9c940000fa0020008f06850a53e0c3900000000018f537144e66e7c5818f537590e66e7ac800400122a4f993105a9f3a280080000063d4dd64399b9eb2063d4de2c399b9e99000000000000000000000000000000000000000000000000000000000000000009ab000", "0025808e4400040408110ac2002036c584060000005405c7f7ea16a4400046800080266e7ac833584f9c940000fa0020008f06850a53e0"},
        {"TUM", "TollUsageMessage", "002640001501714b8528ba00000cc517fbf5451fcd4c8b48015074797e47e1db10d1ab6247c4dfe4b1b705066170e2c6c59334005e30a7041800005dc1000221584000", "002640001501714b8528ba00000cc517fbf5451fcd4c8b48015074797e47e1db10"},
        {"TUMack", "TollUsageAckMessage", "00270800f9d88687004c90", "00270800f9"}
    };

    static std::unique_ptr<tmx::routeable_message> tollMessageFromHex(const std::string &hex) {
        tmx::messages::J2735MessageFactory factory;
        tmx::byte_stream bytes = tmx::byte_stream_decode(hex);
        return std::unique_ptr<tmx::routeable_message>(factory.NewMessage(bytes));
    }

    TEST(TestMessageLogger, validTollMessagesLogAsJson) {
        for (const auto &[subtype, valueName, hex, cutHex] : TOLL_MESSAGES) {
            auto msg = tollMessageFromHex(hex);
            ASSERT_NE(msg, nullptr) << subtype;
            EXPECT_TRUE(isTollMessage(*msg)) << subtype;
            std::string jsonStr = routeableMessageToJsonString(*msg);
            EXPECT_NE(jsonStr.find("\"" + valueName + "\""), std::string::npos) << subtype << ": " << jsonStr;
        }
    }

    TEST(TestMessageLogger, invalidTollMessagesFailWithWarningText) {
        for (const auto &[subtype, valueName, hex, cutHex] : TOLL_MESSAGES) {
            auto msg = tollMessageFromHex(cutHex);
            ASSERT_NE(msg, nullptr) << subtype;
            EXPECT_TRUE(isTollMessage(*msg)) << subtype;
            // Not valid ASN.1: the conversion throws, and the plugin reports it at WARN level with this text.
            EXPECT_ANY_THROW(routeableMessageToJsonString(*msg)) << subtype;
            std::string warning = invalidTollMessageWarning(*msg, "test reason");
            EXPECT_EQ(warning.rfind("Invalid " + subtype + " message (ASN.1 UPER decoding failed: test reason). Payload: ", 0), 0u) << warning;
            EXPECT_NE(warning.find(cutHex), std::string::npos) << warning;
        }
    }
#endif

    TEST(TestMessageLogger, otherMessagesAreNotTollMessages) {
        tmx::messages::BsmEncodedMessage bsm;
        tmx::routeable_message *rMsg = dynamic_cast<tmx::routeable_message *>(&bsm);
        EXPECT_FALSE(isTollMessage(*rMsg));
    }
}
