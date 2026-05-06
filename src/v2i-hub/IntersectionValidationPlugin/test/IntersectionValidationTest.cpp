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

    const std::string SPAT_SCHEMA_PATH = "spat.schema.json";
 
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

// SPaT Field Validation Tests

TEST(FileLoadingTest, LoadExistingFile) {
    std::string path = "/tmp/test_schema.json";
    std::ofstream out(path);
    out << R"({"type": "object"})";
    out.close();
 
    std::string contents = loadFileContents(path);
    EXPECT_EQ(R"({"type": "object"})", contents);
    std::remove(path.c_str());
}
 
TEST(FileLoadingTest, LoadNonExistentFileThrows) {
    EXPECT_THROW(loadFileContents("/tmp/does_not_exist.json"), std::runtime_error);
}

TEST(SchemaValidationTest, InvalidJsonFails) {
    std::string schema = R"({"type": "object", "required": ["name"]})";
    auto result = validateJsonAgainstSchema("not json", schema);
    EXPECT_FALSE(result.valid);
    EXPECT_EQ("Failed to parse input JSON", result.errors[0]);
}
 
TEST(SchemaValidationTest, InvalidSchemaFails) {
    auto result = validateJsonAgainstSchema("{}", "not a schema");
    EXPECT_FALSE(result.valid);
    EXPECT_EQ("Failed to parse JSON schema", result.errors[0]);
}
 
TEST(SchemaValidationTest, NonExistentSchemaFileFails) {
    auto result = validateJsonAgainstSchemaFile(R"({})", "/tmp/missing_schema.json");
    EXPECT_FALSE(result.valid);
    EXPECT_EQ("Failed to open file: /tmp/missing_schema.json", result.errors[0]);
}

TEST(ConvertNumericStringsTest, ConvertsStringToIntWhenSchemaExpectsInteger) {
    // Schema declares "id" as integer
    std::string schemaStr = R"({"type": "object", "properties": {"id": {"type": "integer"}}})";
    std::string jsonStr = R"({"id": "42"})";
 
    rapidjson::Document schemaDoc;
    schemaDoc.Parse(schemaStr.c_str());
 
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());
 
    convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);
 
    EXPECT_TRUE(doc["id"].IsInt64());
    EXPECT_EQ(42, doc["id"].GetInt64());
}
 
TEST(ConvertNumericStringsTest, LeavesStringWhenSchemaExpectsString) {
    // Schema declares "status" as string (like BIT STRING hex fields)
    std::string schemaStr = R"({"type": "object", "properties": {"status": {"type": "string"}}})";
    std::string jsonStr = R"({"status": "0000"})";
 
    rapidjson::Document schemaDoc;
    schemaDoc.Parse(schemaStr.c_str());
 
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());
 
    convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);
 
    EXPECT_TRUE(doc["status"].IsString());
    EXPECT_STREQ("0000", doc["status"].GetString());
}
 
TEST(ConvertNumericStringsTest, LeavesNonNumericStringUnchanged) {
    // Schema declares "name" as integer but value is non-numeric — can't convert
    std::string schemaStr = R"({"type": "object", "properties": {"name": {"type": "integer"}}})";
    std::string jsonStr = R"({"name": "hello"})";
 
    rapidjson::Document schemaDoc;
    schemaDoc.Parse(schemaStr.c_str());
 
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());
 
    convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);
 
    // Should remain string since "hello" is not parseable as int
    EXPECT_TRUE(doc["name"].IsString());
    EXPECT_STREQ("hello", doc["name"].GetString());
}
 
TEST(ConvertNumericStringsTest, HandlesSchemaTypeArray) {
    // Schema declares type as array: ["integer", "string"]
    std::string schemaStr = R"({"type": "object", "properties": {"value": {"type": ["integer", "string"]}}})";
    std::string jsonStr = R"({"value": "99"})";
 
    rapidjson::Document schemaDoc;
    schemaDoc.Parse(schemaStr.c_str());
 
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());
 
    convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);
 
    EXPECT_TRUE(doc["value"].IsInt64());
    EXPECT_EQ(99, doc["value"].GetInt64());
}
 
TEST(ConvertNumericStringsTest, SkipsFieldNotInSchema) {
    // "extra" is not in schema properties — should be left alone
    std::string schemaStr = R"({"type": "object", "properties": {"id": {"type": "integer"}}})";
    std::string jsonStr = R"({"id": "10", "extra": "999"})";
 
    rapidjson::Document schemaDoc;
    schemaDoc.Parse(schemaStr.c_str());
 
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());
 
    convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);
 
    EXPECT_TRUE(doc["id"].IsInt64());
    EXPECT_EQ(10, doc["id"].GetInt64());
    EXPECT_TRUE(doc["extra"].IsString());
    EXPECT_STREQ("999", doc["extra"].GetString());
}
 
TEST(ConvertNumericStringsTest, RecursesIntoNestedObjects) {
    std::string schemaStr = R"({
        "type": "object",
        "properties": {
            "inner": {
                "type": "object",
                "properties": {
                    "count": {"type": "integer"}
                }
            }
        }
    })";
    std::string jsonStr = R"({"inner": {"count": "7"}})";
 
    rapidjson::Document schemaDoc;
    schemaDoc.Parse(schemaStr.c_str());
 
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());
 
    convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);
 
    EXPECT_TRUE(doc["inner"]["count"].IsInt64());
    EXPECT_EQ(7, doc["inner"]["count"].GetInt64());
}
 
TEST(ConvertNumericStringsTest, RecursesIntoArrayItems) {
    std::string schemaStr = R"({
        "type": "object",
        "properties": {
            "items": {
                "type": "array",
                "items": {
                    "type": "object",
                    "properties": {
                        "id": {"type": "integer"}
                    }
                }
            }
        }
    })";
    std::string jsonStr = R"({"items": [{"id": "1"}, {"id": "2"}]})";
 
    rapidjson::Document schemaDoc;
    schemaDoc.Parse(schemaStr.c_str());
 
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());
 
    convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);
 
    EXPECT_TRUE(doc["items"][0]["id"].IsInt64());
    EXPECT_EQ(1, doc["items"][0]["id"].GetInt64());
    EXPECT_TRUE(doc["items"][1]["id"].IsInt64());
    EXPECT_EQ(2, doc["items"][1]["id"].GetInt64());
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