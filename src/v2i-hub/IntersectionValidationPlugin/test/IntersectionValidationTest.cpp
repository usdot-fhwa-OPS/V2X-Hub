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

#include <tmx/j2735_messages/SpatMessage.hpp>
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <tmx/j2735_messages/TravelerInformationMessage.hpp>

#include "IntersectionValidationPlugin.h"
#include "MessageIntervalValidator.h"
#include "FieldValidation.h"
 
using namespace tmx::messages;
using namespace IntersectionValidation;
 
namespace {

    const std::string SPAT_SCHEMA_PATH = "/workspace/src/v2i-hub/IntersectionValidationPlugin/json/spat.schema.json";
 
TEST(MessageTypeTest, SpatMessageCanBeInstantiated) {
    SpatMessage msg;
    SUCCEED();
}
 
TEST(MessageTypeTest, MapDataMessageCanBeInstantiated) {
    MapDataMessage msg;
    SUCCEED();
}
 
TEST(MessageTypeTest, TimMessageCanBeInstantiated) {
    TimMessage msg;
    SUCCEED();
}

// Frequency Validation Tests
 
TEST(FrequencyValidationTest, InitialMessageIntervalIsZero) {
    auto result = calculateMessageInterval(0, 1000, SPAT_INTERVAL_MAX_THRESHOLD_MS);
    EXPECT_EQ(0u, result);
}

TEST(FrequencyValidationTest, SpatIntervalWithinThreshold) {
    auto result = calculateMessageInterval(1000, 1100, SPAT_INTERVAL_MAX_THRESHOLD_MS);
    EXPECT_EQ(100u, result);
}
 
TEST(FrequencyValidationTest, SpatIntervalExceedsThreshold) {
    EXPECT_THROW(
        calculateMessageInterval(1000, 1301, SPAT_INTERVAL_MAX_THRESHOLD_MS),
        tmx::TmxException);
}

TEST(FrequencyValidationTest, SpatIntervalCurrentTimestampEarlierThanLastTimestamp) {
    EXPECT_THROW(
        calculateMessageInterval(1001, 1000, SPAT_INTERVAL_MAX_THRESHOLD_MS),
        tmx::TmxException);
}
 
TEST(FrequencyValidationTest, MapIntervalWithinThreshold) {
    auto result = calculateMessageInterval(1000, 1050, MAP_INTERVAL_MAX_THRESHOLD_MS);
    EXPECT_EQ(50u, result);
}
 
TEST(FrequencyValidationTest, MapIntervalExceedsThreshold) {
    EXPECT_THROW(
        calculateMessageInterval(1000, 1101, MAP_INTERVAL_MAX_THRESHOLD_MS),
        tmx::TmxException);
}

TEST(FrequencyValidationTest, MapIntervalCurrentTimestampEarlierThanLastTimestamp) {
    EXPECT_THROW(
        calculateMessageInterval(1001, 1000, MAP_INTERVAL_MAX_THRESHOLD_MS),
        tmx::TmxException);
}
 
TEST(SpatFieldValidationTest, ValidSpatPasses) {
    std::string json = R"({
        "messageId": 19,
        "value": {
            "SPAT": {
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "status": "0000",
                    "timeStamp": 35176,
                    "states": [{
                        "signalGroup": 2,
                        "state-time-speed": [{
                            "eventState": "protected-Movement-Allowed",
                            "timing": {
                                "minEndTime": 22120,
                                "maxEndTime": 22121
                            }
                        }]
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, SPAT_SCHEMA_PATH);
    EXPECT_TRUE(result.valid) << (result.errors.empty() ? "" : result.errors[0]);
}
 
TEST(SpatFieldValidationTest, MissingMessageIdFails) {
    std::string json = R"({
        "value": {
            "SPAT": {
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "status": "0000",
                    "states": [{
                        "signalGroup": 2,
                        "state-time-speed": [{
                            "eventState": "stop-And-Remain",
                            "timing": {"minEndTime": 22120}
                        }]
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, SPAT_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
}
 
TEST(SpatFieldValidationTest, MissingIntersectionsFails) {
    std::string json = R"({
        "messageId": 19,
        "value": {
            "SPAT": {}
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, SPAT_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
}
 
TEST(SpatFieldValidationTest, MissingIntersectionIdFails) {
    std::string json = R"({
        "messageId": 19,
        "value": {
            "SPAT": {
                "intersections": [{
                    "revision": 0,
                    "status": "0000",
                    "states": [{
                        "signalGroup": 2,
                        "state-time-speed": [{
                            "eventState": "stop-And-Remain",
                            "timing": {"minEndTime": 22120}
                        }]
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, SPAT_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
}
 
TEST(SpatFieldValidationTest, MissingRevisionFails) {
    std::string json = R"({
        "messageId": 19,
        "value": {
            "SPAT": {
                "intersections": [{
                    "id": {"id": 12111},
                    "status": "0000",
                    "states": [{
                        "signalGroup": 2,
                        "state-time-speed": [{
                            "eventState": "stop-And-Remain",
                            "timing": {"minEndTime": 22120}
                        }]
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, SPAT_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
}
 
TEST(SpatFieldValidationTest, MissingStatusFails) {
    std::string json = R"({
        "messageId": 19,
        "value": {
            "SPAT": {
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "states": [{
                        "signalGroup": 2,
                        "state-time-speed": [{
                            "eventState": "stop-And-Remain",
                            "timing": {"minEndTime": 22120}
                        }]
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, SPAT_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
}
 
TEST(MapValidationTest, DISABLED_ValidMapPassesValidation) {
    // TODO: Construct a MAP with all required fields and verify validation passes
}

  
TEST(FrequencyValidationTest, DISABLED_SpatFrequencyWithinExpectedFrequency) {
    // TODO: Simulate SPaT messages arriving at expected frequency
}
 
TEST(FrequencyValidationTest, DISABLED_MapFrequencyWithinExpectedFrequency) {
    // TODO: Simulate MAP messages arriving at expected frequency
}
 
} // namespace