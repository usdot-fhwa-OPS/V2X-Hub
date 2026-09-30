/**
 * Copyright (C) 2026 LEIDOS.
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
#include <cmath>
#include <boost/property_tree/json_parser.hpp>
#include "PriorityConfiguration.hpp"

using namespace PriorityPlugin;

TEST(PriorityConfigurationTest, ParseTscConfigurationList) {
    const std::string json = R"([{"IntersectionID": 9709, "IP": "192.168.55.92", "Port": 161}])";
    auto configs = parseTscConfigurationList(json);
    EXPECT_EQ(configs.size(), 1u);
    EXPECT_EQ(configs[0].intersectionID, 9709);
    EXPECT_EQ(configs[0].ip, "192.168.55.92");
    EXPECT_EQ(configs[0].port, 161);
}

TEST(PriorityConfigurationTest, ParseTscConfigurationListEmptyStringArray) {
    // An empty string is a configuration error; an empty JSON array is valid
    EXPECT_THROW(parseTscConfigurationList(""), PriorityConfigurationException);
    auto configs = parseTscConfigurationList("[]");
    EXPECT_TRUE(configs.empty());
}

TEST(PriorityConfigurationTest, ParseTscConfigurationListMalformedJson) {
    EXPECT_THROW(parseTscConfigurationList("not json"),
                 boost::property_tree::json_parser_error);
    EXPECT_THROW(parseTscConfigurationList(R"([{"IntersectionID": 1,)"),
                 boost::property_tree::json_parser_error);
}

TEST(PriorityConfigurationTest, ParseTscConfigurationListMissingField) {
    // IP missing
    const std::string json = R"([{"IntersectionID": 1, "Port": 161}])";
    EXPECT_THROW(parseTscConfigurationList(json), boost::property_tree::ptree_error);
}

TEST(PriorityConfigurationTest, ParseLaneStrategyMapping) {
    const std::string json = R"([{"IntersectionID": 9709, "Lane": 2, "Strategy": 1}])";
    auto entries = parseLaneStrategyMapping(json);
    EXPECT_EQ(entries.size(), 1u);
    EXPECT_EQ(entries[0].intersectionID, 9709);
    EXPECT_EQ(entries[0].lane, 2);
    EXPECT_EQ(entries[0].strategy, 1);
}

TEST(PriorityConfigurationTest, ParseLaneStrategyMappingEmptyString) {
    EXPECT_THROW(parseLaneStrategyMapping(""), PriorityConfigurationException);
}

TEST(PriorityConfigurationTest, ParseLaneStrategyMappingStrategyBoundaries) {
    // Strategy values at the edges of the [1, 255] range are accepted.
    const std::string json = R"([
        {"IntersectionID": 1, "Lane": 1, "Strategy": 1},
        {"IntersectionID": 2, "Lane": 2, "Strategy": 255}
    ])";
    auto entries = parseLaneStrategyMapping(json);
    EXPECT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0].strategy, 1);
    EXPECT_EQ(entries[1].strategy, 255);
}

TEST(PriorityConfigurationTest, ParseLaneStrategyMappingStrategyOutOfRangeSkipped) {
    // Entries with Strategy outside [1, 255] are skipped.
    const std::string json = R"([
        {"IntersectionID": 1, "Lane": 1, "Strategy": 0},
        {"IntersectionID": 2, "Lane": 2, "Strategy": 50},
        {"IntersectionID": 3, "Lane": 3, "Strategy": 256},
        {"IntersectionID": 4, "Lane": 4, "Strategy": -1}
    ])";
    auto entries = parseLaneStrategyMapping(json);
    EXPECT_EQ(entries.size(), 1u);
    EXPECT_EQ(entries[0].intersectionID, 2);
    EXPECT_EQ(entries[0].lane, 2);
    EXPECT_EQ(entries[0].strategy, 50);
}

TEST(PriorityConfigurationTest, ParseLaneStrategyMappingMalformedJsonThrows) {
    EXPECT_THROW(parseLaneStrategyMapping("{not json"),
                 boost::property_tree::json_parser_error);
}

TEST(PriorityConfigurationTest, ParseLaneStrategyMappingMissingFieldThrows) {
    // Strategy missing - ptree throws on node.get<>()
    const std::string json = R"([{"IntersectionID": 1, "Lane": 2}])";
    EXPECT_THROW(parseLaneStrategyMapping(json), boost::property_tree::ptree_error);
}

TEST(PriorityConfigurationTest, ParseReserviceClassTimesEmptyString) {
    // An empty string is a configuration error
    EXPECT_THROW(parseReserviceClassTimes(""), PriorityConfigurationException);
}

TEST(PriorityConfigurationTest, ParseReserviceClassTimesAllSlotsPopulated) {
    auto result = parseReserviceClassTimes("10,20,30,40,50,60,70,80,90,100");
    EXPECT_EQ(result[0], 10u);
    EXPECT_EQ(result[1], 20u);
    EXPECT_EQ(result[2], 30u);
    EXPECT_EQ(result[3], 40u);
    EXPECT_EQ(result[4], 50u);
    EXPECT_EQ(result[5], 60u);
    EXPECT_EQ(result[6], 70u);
    EXPECT_EQ(result[7], 80u);
    EXPECT_EQ(result[8], 90u);
    EXPECT_EQ(result[9], 100u);
}

TEST(PriorityConfigurationTest, ParseReserviceClassTimesPartialFillsRemainingZeros) {
    auto result = parseReserviceClassTimes("5,15,25");
    EXPECT_EQ(result[0], 5u);
    EXPECT_EQ(result[1], 15u);
    EXPECT_EQ(result[2], 25u);
    for (size_t i = 3; i < ReserviceClassTimesSize; i++) {
        EXPECT_EQ(result[i], 0u) << "index " << i;
    }
}

TEST(PriorityConfigurationTest, ParseReserviceClassTimesIgnoresPastTen) {
    // 12 values; only first 10 should be read.
    auto result = parseReserviceClassTimes("1,2,3,4,5,6,7,8,9,10,11,12");
    EXPECT_EQ(result[0], 1u);
    EXPECT_EQ(result[9], 10u);
}

TEST(PriorityConfigurationTest, ParseReserviceClassTimesInvalidValue) {
    // Logs a warning and leaves index 1 at 0; Surrounding values still parse.
    auto result = parseReserviceClassTimes("7,abc,42");
    EXPECT_EQ(result[0], 7u);
    EXPECT_EQ(result[1], 0u);
    EXPECT_EQ(result[2], 42u);
    for (size_t i = 3; i < ReserviceClassTimesSize; i++) {
        EXPECT_EQ(result[i], 0u) << "index " << i;
    }
}

TEST(PriorityConfigurationTest, ParsePluginRoleValidValues) {
    EXPECT_EQ(parsePluginRole("PRS"), PluginRole::PRS);
    EXPECT_EQ(parsePluginRole("PRG"), PluginRole::PRG);
}

TEST(PriorityConfigurationTest, ParsePluginRoleInvalidValuesThrow) {
    // Matching is strict: anything but exactly "PRS" or "PRG" is a configuration error
    EXPECT_THROW(parsePluginRole(""), PriorityConfigurationException);
    EXPECT_THROW(parsePluginRole("prs"), PriorityConfigurationException);
    EXPECT_THROW(parsePluginRole("PRS "), PriorityConfigurationException);
    EXPECT_THROW(parsePluginRole("PSR"), PriorityConfigurationException);
}

TEST(PriorityConfigurationTest, PluginRoleToString) {
    EXPECT_STREQ(pluginRoleToString(PluginRole::PRS), "PRS");
    EXPECT_STREQ(pluginRoleToString(PluginRole::PRG), "PRG");
}

TEST(PriorityConfigurationTest, ParseReserviceClassTimesOutOfRangeValue) {
    // 2^64 exceeds unsigned long; std::stoul throws out_of_range, value stays 0.
    auto oor = std::pow(2, 64);
    auto classTimes = "3," + std::to_string(oor) + ",9";
    auto result = parseReserviceClassTimes(classTimes);
    EXPECT_EQ(result[0], 3u);
    EXPECT_EQ(result[1], 0u);
    EXPECT_EQ(result[2], 9u);
}
