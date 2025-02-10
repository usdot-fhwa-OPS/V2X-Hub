#include <gtest/gtest.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include "MobilityOperationConverter.h"


TEST(MobilityOperationConverterTest, fromTree)
{
    ptree test_tree;
	test_tree.put("cmv_id", "VEH_01");
	test_tree.put("cargo_id", "CARGO_01");
	ptree destination;
	destination.put<double>("latitude", 12222222);
	destination.put<double>("longitude", 13333333);
	test_tree.put_child("destination",destination);
	test_tree.put("operation", "TEST2");
	test_tree.put("action_id", 6);
}