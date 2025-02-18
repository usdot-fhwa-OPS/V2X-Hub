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
#include "ActionConverter.h"
#include "ActionObject.h"

TEST(ActionConverterTest, toTree)
{
    std::unique_ptr<Action_Object> test_obj( new Action_Object());

    test_obj->action_id = 2;
    test_obj->next_action = 3;
    test_obj->prev_action = 1;
    test_obj->area.name = "TEST";
    test_obj->area.longitude = 10000000;
    test_obj->area.latitude =  11111111;
    test_obj->area.status = true;
    test_obj->area.is_notify = true;
    test_obj->cargo.cargo_uuid = "CARGO_01";
    test_obj->cargo.name = "TEST_CARGO";
    test_obj->vehicle.veh_id = "VEH_01";
    test_obj->vehicle.name = "SOME_VEHICLE";

    ptree result = CDA1TenthPlugin::ActionConverter::toTree(*test_obj);

    EXPECT_EQ(result.get_child("action_id").get_value<int>(), 2);
    EXPECT_EQ(result.get_child("operation").get_value<string>(), "TEST");
    EXPECT_EQ(result.get_child("cmv_id").get_value<string>(), "VEH_01");
    EXPECT_EQ(result.get_child("cargo_id").get_value<string>(), "CARGO_01");
    EXPECT_EQ(result.get_child("destination").get_child("latitude").get_value<double>(), 11111111);
    EXPECT_EQ(result.get_child("destination").get_child("longitude").get_value<double>(), 10000000);
}

TEST(ActionConverterTest, toActionObject)
{
    ptree test_tree;
	test_tree.put("operation", "TEST2");
	test_tree.put("action_id", 5);
	test_tree.put("cmv_id", "VEH_02");
	test_tree.put("cargo_id", "CARGO_02");
    
	ptree destination;
	destination.put<double>("latitude", 12222222);
	destination.put<double>("longitude", 13333333);
	test_tree.put_child("destination", destination);

    Action_Object result_obj;
    result_obj = CDA1TenthPlugin::ActionConverter::fromTree( test_tree );

    EXPECT_EQ(result_obj.action_id, 5);
    EXPECT_EQ(result_obj.area.name, "TEST2");
    EXPECT_EQ(result_obj.vehicle.veh_id, "VEH_02");
    EXPECT_EQ(result_obj.cargo.cargo_uuid, "CARGO_02");
    EXPECT_EQ(result_obj.area.latitude, 12222222);
    EXPECT_EQ(result_obj.area.longitude, 13333333);

}

TEST(ActionConverterTest, toActionObject_InvalidActionId)
{
    ptree test_tree;
	test_tree.put("operation", "TEST2");
	test_tree.put("action_id", -1);
	test_tree.put("cmv_id", "VEH_02");
	test_tree.put("cargo_id", "CARGO_02");
    
	ptree destination;
	destination.put<double>("latitude", 12222222);
	destination.put<double>("longitude", 13333333);
	test_tree.put_child("destination", destination);

    Action_Object result_obj;
    result_obj = CDA1TenthPlugin::ActionConverter::fromTree( test_tree );
    EXPECT_EQ(result_obj.action_id, -1);
    EXPECT_EQ(result_obj.is_first_action, false);

}

TEST(ActionConverterTest, toActionObject_MissingActionId)
{
    ptree test_tree;
	test_tree.put("operation", "TEST2");
	test_tree.put("cmv_id", "VEH_02");
	test_tree.put("cargo_id", "CARGO_02");
    
	ptree destination;
	destination.put<double>("latitude", 12222222);
	destination.put<double>("longitude", 13333333);
	test_tree.put_child("destination", destination);

    Action_Object result_obj;
    result_obj = CDA1TenthPlugin::ActionConverter::fromTree( test_tree );
    EXPECT_EQ(result_obj.action_id, -1);
    EXPECT_EQ(result_obj.is_first_action, true);

}