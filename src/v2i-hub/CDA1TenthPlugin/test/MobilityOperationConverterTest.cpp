/**
 * Copyright (C) 2025 LEIDOS.
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
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include "MobilityOperationConverter.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional/optional.hpp>
#include <tmx/j2735_messages/testMessage03.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>

using namespace tmx::messages;
using namespace boost::property_tree;
using namespace tmx;

TEST(MobilityOperationConverterTest, fromTree)
{
    string strat = "carma/port_drayage";
    ptree test_tree;
	test_tree.put("cmv_id", "DOT-101");
	test_tree.put("cargo_id", "CARGO_01");
	ptree destination;
	destination.put<double>("latitude", 12222222);
	destination.put<double>("longitude", 13333333);
	test_tree.put_child("destination", destination);
	test_tree.put("operation", "TEST2");
	test_tree.put("action_id", 6);

    ptree result = CDA1TenthPlugin::MobilityOperationConverter::toXML(test_tree, strat);

    std::string expectedJson = R"({
                "cmv_id": "DOT-101",
                "cargo_id": "CARGO_01",
                "destination": {
                    "latitude": "12222222",
                    "longitude": "13333333"
                },
                "operation": "TEST2",
                "action_id": "6"
            })";
    boost::algorithm::erase_all(expectedJson, "\n");
    boost::algorithm::erase_all(expectedJson, " ");

    std::string resultPayload = result.get_child("TestMessage03").get_child("body").get_child("operationParams").get_value<string>();
    boost::algorithm::erase_all(resultPayload, "\n");
    boost::algorithm::erase_all(resultPayload, " ");

    EXPECT_EQ(result.get_child("TestMessage03").get_child("body").get_child("strategy").get_value<string>(), strat);
    EXPECT_EQ(resultPayload, expectedJson);
    EXPECT_EQ(result.get_child("TestMessage03").get_child("header").get_child("hostStaticId").get_value<string>(), "UNSET");
    EXPECT_EQ(result.get_child("TestMessage03").get_child("header").get_child("targetStaticId").get_value<string>(), "UNSET");
    EXPECT_EQ(result.get_child("TestMessage03").get_child("header").get_child("hostBSMId").get_value<string>(), "00000000");
    EXPECT_EQ(result.get_child("TestMessage03").get_child("header").get_child("planId").get_value<string>(), "00000000-0000-0000-0000-000000000000");
    EXPECT_EQ(result.get_child("TestMessage03").get_child("header").get_child("timestamp").get_value<string>(), "0000000000000000000");

}

TEST(MobilityOperationConverterTest, fromTest_firstAction){
    //When there is no action id in mobilityOperation message operationParams, it is considered first action for the vehicle
    string strat = "carma/port_drayage";
    ptree test_tree;
	test_tree.put("cmv_id", "DOT-101");
	test_tree.put("cargo_id", "CARGO_01");
	ptree destination;
	destination.put<double>("latitude", 12222222);
	destination.put<double>("longitude", 13333333);
	test_tree.put_child("destination", destination);
	test_tree.put("operation", "TEST2");

    ptree result = CDA1TenthPlugin::MobilityOperationConverter::toXML(test_tree, strat);
    //Convert ActionObject to MobilityOperationMessage and encode it
    tsm3Message mob_msg;
    tsm3EncodedMessage mobilityENC;
    tmx::message_container_type container;
    std::unique_ptr<tsm3EncodedMessage> msg;
    std::stringstream content;
    write_xml(content, result);
    std::string expectedEncodedMOM = "00f3812b1d59d4e2d43ab3a9c5a8c183060c183060c183060c183060b583060c16b060c182d60c18305ac183060c183060c183060c183060c183060c183060c183060c1830431e1e5b70afe1bf974bf93961f3873e53d7da04dc7aefe8ef1eded7f4e44dc7aefe8edd204dc7aefe8ee24fa8b58b0629b8f5dfd1dac409b8f5dfd1de3c3cb3efbfa7226e3d77f476e9026e3d77f4770e0d28f3efb0629b8f5dfd1dac409b8f5dfd1de4cbcfa69dd87a69dfb9371ebbfa3b7483da04dc7aefe8ef661e9a7a75c995371ebbfa3b7481371ebbfa3b62c993264c99324dc7aefe8ed6204dc7aefe8ef66fdd9f4f4eb932a6e3d77f476e9026e3d77f476c59b366cd9b3669b8f5dfd1da0fab1026e3d77f477bf865e587a69dfb9371ebbfa3b7481371ebbfa3ba9169d4649b8f5dfd1da0fa";
    try {
        // Uper encode message 
        container.load<XML>(content);
        mob_msg.set_contents(container.get_storage().get_tree());
        mobilityENC.encode_j2735_message( mob_msg);
        msg.reset();
        J2735MessageFactory factory;
        msg.reset(dynamic_cast<tsm3EncodedMessage*>(factory.NewMessage(api::MSGSUBTYPE_TESTMESSAGE03_STRING)));
        string enc = mobilityENC.get_encoding();
        EXPECT_EQ(mobilityENC.get_payload_str(), expectedEncodedMOM);
    }
    catch ( const J2735Exception &e) {
        std::cout << "Error occurred during message encoding " << std::endl << e.what() << std::endl;
    }
}