/*
 * Main.cpp
 *
 *  Created on: May 10, 2016
 *      Author: ivp
 */

#include <gtest/gtest.h>
#include "KafkaTestEnvironment.cpp"

int main(int argc, char **argv)
{
    ::testing::AddGlobalTestEnvironment(new KafkaTestEnvironment());
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
