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


TEST(MobilityOperationConverterTest, fromTree)
{
    string strat = "carma/port_drayage";
    ptree test_tree;
	test_tree.put("cmv_id", "VEH_01");
	test_tree.put("cargo_id", "CARGO_01");
	ptree destination;
	destination.put<double>("latitude", 12222222);
	destination.put<double>("longitude", 13333333);
	test_tree.put_child("destination", destination);
	test_tree.put("operation", "TEST2");
	test_tree.put("action_id", 6);

    ptree result = CDA1TenthPlugin::MobilityOperationConverter::toXML(test_tree, strat);

    std::string expectedJson = R"({
                "cmv_id": "VEH_01",
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