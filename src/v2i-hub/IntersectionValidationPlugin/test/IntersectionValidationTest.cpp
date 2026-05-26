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

#include "IntersectionValidationPlugin.h"
#include "MessageIntervalValidator.h"
#include "FieldValidation.h"
 
using namespace tmx::messages;
using namespace IntersectionValidation;
 
namespace {

const std::string SPAT_SCHEMA_PATH = "../../../v2i-hub/IntersectionValidationPlugin/resources/spat.schema.json";
const std::string MAP_SCHEMA_PATH = "../../../v2i-hub/IntersectionValidationPlugin/resources/map.schema.json";

 
TEST(MessageTypeTest, SpatMessageCanBeInstantiated) {
    SpatMessage msg;
    SUCCEED();
}
 
TEST(MessageTypeTest, MapDataMessageCanBeInstantiated) {
    MapDataMessage msg;
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

TEST(SpatFieldValidationTest, ValidSpatPasses)
{
    std::string json = R"({
  "messageId": 19,
  "value": {
    "SPAT": {
      "timeStamp": 336511,
      "name": "I",
      "intersections": [
        {
          "name": "IA5S",
          "id": {
            "region": 59982,
            "id": 59963
          },
          "revision": 34,
          "status": "0040",
          "moy": 255914,
          "timeStamp": 18202,
          "enabledLanes": [
            7,
            7,
            170,
            87,
            215
          ],
          "states": [
            {
              "movementName": "IA5",
              "signalGroup": 40,
              "state-time-speed": [
                {
                  "eventState": "unavailable",
                  "timing": {
                    "startTime": 30703,
                    "minEndTime": 10021,
                    "maxEndTime": 29929,
                    "likelyTime": 34789,
                    "confidence": 4,
                    "nextTime": 1352
                  },
                  "speeds": [
                    {
                      "type": "transit",
                      "speed": 88,
                      "confidence": "prec10ms",
                      "distance": 9203,
                      "class": 172
                    },
                    {
                      "type": "ecoDrive",
                      "speed": 134,
                      "confidence": "prec0-05ms",
                      "distance": 6975,
                      "class": 178
                    },
                    {
                      "type": "none",
                      "speed": 362,
                      "confidence": "prec100ms",
                      "distance": 6641,
                      "class": 138
                    },
                    {
                      "type": "greenwave",
                      "speed": 462,
                      "confidence": "prec1ms",
                      "distance": 3168,
                      "class": 249
                    }
                  ]
                },
                {
                  "eventState": "stop-And-Remain",
                  "timing": {
                    "startTime": 14107,
                    "minEndTime": 2153,
                    "maxEndTime": 1115,
                    "likelyTime": 1565,
                    "confidence": 12,
                    "nextTime": 12653
                  },
                  "speeds": [
                    {
                      "type": "none",
                      "speed": 252,
                      "confidence": "prec0-01ms",
                      "distance": 6362,
                      "class": 212
                    },
                    {
                      "type": "none",
                      "speed": 70,
                      "confidence": "prec0-01ms",
                      "distance": 3228,
                      "class": 189
                    },
                    {
                      "type": "ecoDrive",
                      "speed": 211,
                      "confidence": "prec0-01ms",
                      "distance": 2172,
                      "class": 81
                    },
                    {
                      "type": "ecoDrive",
                      "speed": 108,
                      "confidence": "prec0-1ms",
                      "distance": 2531,
                      "class": 35
                    },
                    {
                      "type": "none",
                      "speed": 304,
                      "confidence": "prec0-01ms",
                      "distance": 9620,
                      "class": 81
                    }
                  ]
                },
                {
                  "eventState": "stop-And-Remain",
                  "timing": {
                    "startTime": 28869,
                    "minEndTime": 28398,
                    "maxEndTime": 12108,
                    "likelyTime": 13463,
                    "confidence": 7,
                    "nextTime": 19608
                  },
                  "speeds": [
                    {
                      "type": "none",
                      "speed": 42,
                      "confidence": "prec10ms",
                      "distance": 6519,
                      "class": 31
                    }
                  ]
                },
                {
                  "eventState": "stop-And-Remain",
                  "timing": {
                    "startTime": 22460,
                    "minEndTime": 35943,
                    "maxEndTime": 23208,
                    "likelyTime": 27163,
                    "confidence": 2,
                    "nextTime": 21976
                  },
                  "speeds": [
                    {
                      "type": "greenwave",
                      "speed": 16,
                      "confidence": "prec0-01ms",
                      "distance": 1136,
                      "class": 208
                    },
                    {
                      "type": "ecoDrive",
                      "speed": 119,
                      "confidence": "prec10ms",
                      "distance": 1412,
                      "class": 66
                    },
                    {
                      "type": "none",
                      "speed": 90,
                      "confidence": "prec0-05ms",
                      "distance": 6941,
                      "class": 177
                    },
                    {
                      "type": "none",
                      "speed": 495,
                      "confidence": "unavailable",
                      "distance": 989,
                      "class": 115
                    }
                  ]
                },
                {
                  "eventState": "permissive-Movement-Allowed",
                  "timing": {
                    "startTime": 17582,
                    "minEndTime": 27124,
                    "maxEndTime": 17450,
                    "likelyTime": 7962,
                    "confidence": 12,
                    "nextTime": 10565
                  },
                  "speeds": [
                    {
                      "type": "none",
                      "speed": 309,
                      "confidence": "unavailable",
                      "distance": 6563,
                      "class": 175
                    },
                    {
                      "type": "none",
                      "speed": 321,
                      "confidence": "prec0-1ms",
                      "distance": 403,
                      "class": 178
                    },
                    {
                      "type": "greenwave",
                      "speed": 415,
                      "confidence": "prec0-05ms",
                      "distance": 2708,
                      "class": 210
                    }
                  ]
                }
              ],
              "maneuverAssistList": [
                {
                  "connectionID": 73,
                  "queueLength": 7134,
                  "availableStorageLength": 7803,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 34,
                  "queueLength": 2359,
                  "availableStorageLength": 6137,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                }
              ]
            },
            {
              "movementName": "IA5S",
              "signalGroup": 205,
              "state-time-speed": [
                {
                  "eventState": "pre-Movement",
                  "timing": {
                    "startTime": 8395,
                    "minEndTime": 24329,
                    "maxEndTime": 30030,
                    "likelyTime": 27954,
                    "confidence": 4,
                    "nextTime": 15861
                  },
                  "speeds": [
                    {
                      "type": "ecoDrive",
                      "speed": 190,
                      "confidence": "prec10ms",
                      "distance": 9958,
                      "class": 245
                    },
                    {
                      "type": "none",
                      "speed": 152,
                      "confidence": "prec0-01ms",
                      "distance": 7871,
                      "class": 10
                    },
                    {
                      "type": "none",
                      "speed": 48,
                      "confidence": "prec0-01ms",
                      "distance": 5887,
                      "class": 188
                    },
                    {
                      "type": "transit",
                      "speed": 5,
                      "confidence": "prec0-05ms",
                      "distance": 332,
                      "class": 47
                    }
                  ]
                }
              ],
              "maneuverAssistList": [
                {
                  "connectionID": 33,
                  "queueLength": 7556,
                  "availableStorageLength": 2283,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                }
              ]
            },
            {
              "movementName": "IA5",
              "signalGroup": 72,
              "state-time-speed": [
                {
                  "eventState": "protected-Movement-Allowed",
                  "timing": {
                    "startTime": 799,
                    "minEndTime": 7968,
                    "maxEndTime": 5482,
                    "likelyTime": 15905,
                    "confidence": 1,
                    "nextTime": 19581
                  },
                  "speeds": [
                    {
                      "type": "greenwave",
                      "speed": 18,
                      "confidence": "prec5ms",
                      "distance": 5265,
                      "class": 46
                    },
                    {
                      "type": "transit",
                      "speed": 375,
                      "confidence": "unavailable",
                      "distance": 8950,
                      "class": 205
                    }
                  ]
                },
                {
                  "eventState": "pre-Movement",
                  "timing": {
                    "startTime": 7750,
                    "minEndTime": 6460,
                    "maxEndTime": 29204,
                    "likelyTime": 21482,
                    "confidence": 13,
                    "nextTime": 26673
                  },
                  "speeds": [
                    {
                      "type": "ecoDrive",
                      "speed": 0,
                      "confidence": "prec0-05ms",
                      "distance": 3386,
                      "class": 74
                    }
                  ]
                },
                {
                  "eventState": "permissive-Movement-Allowed",
                  "timing": {
                    "startTime": 23179,
                    "minEndTime": 4261,
                    "maxEndTime": 17107,
                    "likelyTime": 8366,
                    "confidence": 0,
                    "nextTime": 21647
                  },
                  "speeds": [
                    {
                      "type": "ecoDrive",
                      "speed": 336,
                      "confidence": "prec5ms",
                      "distance": 7881,
                      "class": 27
                    },
                    {
                      "type": "greenwave",
                      "speed": 329,
                      "confidence": "prec0-1ms",
                      "distance": 8113,
                      "class": 135
                    },
                    {
                      "type": "transit",
                      "speed": 98,
                      "confidence": "prec5ms",
                      "distance": 6530,
                      "class": 225
                    }
                  ]
                },
                {
                  "eventState": "stop-Then-Proceed",
                  "timing": {
                    "startTime": 17546,
                    "minEndTime": 34472,
                    "maxEndTime": 8684,
                    "likelyTime": 34281,
                    "confidence": 0,
                    "nextTime": 4042
                  },
                  "speeds": [
                    {
                      "type": "greenwave",
                      "speed": 122,
                      "confidence": "prec5ms",
                      "distance": 405,
                      "class": 149
                    }
                  ]
                },
                {
                  "eventState": "stop-And-Remain",
                  "timing": {
                    "startTime": 17771,
                    "minEndTime": 7752,
                    "maxEndTime": 34279,
                    "likelyTime": 11471,
                    "confidence": 1,
                    "nextTime": 29034
                  },
                  "speeds": [
                    {
                      "type": "ecoDrive",
                      "speed": 129,
                      "confidence": "prec100ms",
                      "distance": 2598,
                      "class": 180
                    },
                    {
                      "type": "greenwave",
                      "speed": 311,
                      "confidence": "prec5ms",
                      "distance": 9189,
                      "class": 136
                    },
                    {
                      "type": "ecoDrive",
                      "speed": 38,
                      "confidence": "prec0-1ms",
                      "distance": 1833,
                      "class": 72
                    },
                    {
                      "type": "greenwave",
                      "speed": 139,
                      "confidence": "unavailable",
                      "distance": 1150,
                      "class": 73
                    },
                    {
                      "type": "none",
                      "speed": 97,
                      "confidence": "unavailable",
                      "distance": 2250,
                      "class": 222
                    }
                  ]
                }
              ],
              "maneuverAssistList": [
                {
                  "connectionID": 50,
                  "queueLength": 2545,
                  "availableStorageLength": 915,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                }
              ]
            }
          ],
          "maneuverAssistList": [
            {
              "connectionID": 209,
              "queueLength": 6043,
              "availableStorageLength": 7308,
              "waitOnStop": true,
              "pedBicycleDetect": true
            },
            {
              "connectionID": 30,
              "queueLength": 6426,
              "availableStorageLength": 5206,
              "waitOnStop": true,
              "pedBicycleDetect": true
            },
            {
              "connectionID": 101,
              "queueLength": 5339,
              "availableStorageLength": 1523,
              "waitOnStop": true,
              "pedBicycleDetect": true
            },
            {
              "connectionID": 96,
              "queueLength": 8514,
              "availableStorageLength": 5733,
              "waitOnStop": true,
              "pedBicycleDetect": true
            }
          ],
          "roadAuthorityID": {
            "relRdAuthID": "46533.21937.54223.23577"
          }
        },
        {
          "name": "IA5St",
          "id": {
            "region": 34051,
            "id": 18364
          },
          "revision": 10,
          "status": "4000",
          "moy": 71379,
          "timeStamp": 3474,
          "enabledLanes": [
            7,
            7,
            170,
            87,
            215
          ],
          "states": [
            {
              "movementName": "IA5",
              "signalGroup": 216,
              "state-time-speed": [
                {
                  "eventState": "unavailable",
                  "timing": {
                    "startTime": 5277,
                    "minEndTime": 13952,
                    "maxEndTime": 4792,
                    "likelyTime": 6821,
                    "confidence": 9,
                    "nextTime": 29856
                  },
                  "speeds": [
                    {
                      "type": "greenwave",
                      "speed": 485,
                      "confidence": "prec0-05ms",
                      "distance": 7027,
                      "class": 91
                    },
                    {
                      "type": "transit",
                      "speed": 263,
                      "confidence": "prec0-01ms",
                      "distance": 3027,
                      "class": 240
                    },
                    {
                      "type": "transit",
                      "speed": 51,
                      "confidence": "prec5ms",
                      "distance": 5168,
                      "class": 78
                    }
                  ]
                },
                {
                  "eventState": "permissive-Movement-Allowed",
                  "timing": {
                    "startTime": 25490,
                    "minEndTime": 13998,
                    "maxEndTime": 141,
                    "likelyTime": 16969,
                    "confidence": 13,
                    "nextTime": 15878
                  },
                  "speeds": [
                    {
                      "type": "transit",
                      "speed": 372,
                      "confidence": "prec5ms",
                      "distance": 9567,
                      "class": 2
                    },
                    {
                      "type": "greenwave",
                      "speed": 27,
                      "confidence": "prec0-05ms",
                      "distance": 622,
                      "class": 151
                    },
                    {
                      "type": "transit",
                      "speed": 344,
                      "confidence": "prec0-1ms",
                      "distance": 234,
                      "class": 79
                    },
                    {
                      "type": "greenwave",
                      "speed": 451,
                      "confidence": "prec5ms",
                      "distance": 7582,
                      "class": 184
                    },
                    {
                      "type": "transit",
                      "speed": 320,
                      "confidence": "prec0-01ms",
                      "distance": 9071,
                      "class": 111
                    }
                  ]
                },
                {
                  "eventState": "stop-Then-Proceed",
                  "timing": {
                    "startTime": 33234,
                    "minEndTime": 17937,
                    "maxEndTime": 23102,
                    "likelyTime": 17209,
                    "confidence": 14,
                    "nextTime": 20517
                  },
                  "speeds": [
                    {
                      "type": "greenwave",
                      "speed": 149,
                      "confidence": "prec5ms",
                      "distance": 3802,
                      "class": 132
                    }
                  ]
                },
                {
                  "eventState": "pre-Movement",
                  "timing": {
                    "startTime": 22802,
                    "minEndTime": 27247,
                    "maxEndTime": 12102,
                    "likelyTime": 26871,
                    "confidence": 2,
                    "nextTime": 27454
                  },
                  "speeds": [
                    {
                      "type": "transit",
                      "speed": 337,
                      "confidence": "prec0-05ms",
                      "distance": 5051,
                      "class": 14
                    },
                    {
                      "type": "transit",
                      "speed": 110,
                      "confidence": "prec1ms",
                      "distance": 1535,
                      "class": 158
                    },
                    {
                      "type": "transit",
                      "speed": 200,
                      "confidence": "prec1ms",
                      "distance": 9551,
                      "class": 238
                    }
                  ]
                },
                {
                  "eventState": "stop-And-Remain",
                  "timing": {
                    "startTime": 32414,
                    "minEndTime": 17340,
                    "maxEndTime": 11177,
                    "likelyTime": 26863,
                    "confidence": 0,
                    "nextTime": 14661
                  },
                  "speeds": [
                    {
                      "type": "greenwave",
                      "speed": 173,
                      "confidence": "unavailable",
                      "distance": 6662,
                      "class": 204
                    },
                    {
                      "type": "greenwave",
                      "speed": 154,
                      "confidence": "prec100ms",
                      "distance": 4385,
                      "class": 225
                    },
                    {
                      "type": "greenwave",
                      "speed": 277,
                      "confidence": "prec0-05ms",
                      "distance": 8324,
                      "class": 176
                    },
                    {
                      "type": "ecoDrive",
                      "speed": 170,
                      "confidence": "prec10ms",
                      "distance": 5429,
                      "class": 45
                    }
                  ]
                }
              ],
              "maneuverAssistList": [
                {
                  "connectionID": 156,
                  "queueLength": 3267,
                  "availableStorageLength": 6831,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 240,
                  "queueLength": 8431,
                  "availableStorageLength": 5769,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                }
              ]
            },
            {
              "movementName": "IA5S",
              "signalGroup": 148,
              "state-time-speed": [
                {
                  "eventState": "unavailable",
                  "timing": {
                    "startTime": 36028,
                    "minEndTime": 16620,
                    "maxEndTime": 3908,
                    "likelyTime": 24779,
                    "confidence": 2,
                    "nextTime": 5828
                  },
                  "speeds": [
                    {
                      "type": "ecoDrive",
                      "speed": 346,
                      "confidence": "prec5ms",
                      "distance": 1860,
                      "class": 110
                    },
                    {
                      "type": "greenwave",
                      "speed": 292,
                      "confidence": "prec0-01ms",
                      "distance": 6017,
                      "class": 90
                    },
                    {
                      "type": "none",
                      "speed": 364,
                      "confidence": "unavailable",
                      "distance": 9852,
                      "class": 37
                    },
                    {
                      "type": "transit",
                      "speed": 491,
                      "confidence": "prec0-1ms",
                      "distance": 2930,
                      "class": 156
                    },
                    {
                      "type": "transit",
                      "speed": 382,
                      "confidence": "prec0-1ms",
                      "distance": 7060,
                      "class": 211
                    }
                  ]
                },
                {
                  "eventState": "stop-And-Remain",
                  "timing": {
                    "startTime": 29078,
                    "minEndTime": 25246,
                    "maxEndTime": 14201,
                    "likelyTime": 32726,
                    "confidence": 1,
                    "nextTime": 24026
                  },
                  "speeds": [
                    {
                      "type": "ecoDrive",
                      "speed": 354,
                      "confidence": "unavailable",
                      "distance": 1808,
                      "class": 248
                    }
                  ]
                },
                {
                  "eventState": "protected-Movement-Allowed",
                  "timing": {
                    "startTime": 26151,
                    "minEndTime": 23376,
                    "maxEndTime": 13165,
                    "likelyTime": 27993,
                    "confidence": 14,
                    "nextTime": 3803
                  },
                  "speeds": [
                    {
                      "type": "transit",
                      "speed": 100,
                      "confidence": "prec100ms",
                      "distance": 920,
                      "class": 79
                    },
                    {
                      "type": "greenwave",
                      "speed": 118,
                      "confidence": "prec0-05ms",
                      "distance": 8363,
                      "class": 181
                    },
                    {
                      "type": "greenwave",
                      "speed": 434,
                      "confidence": "prec0-05ms",
                      "distance": 4334,
                      "class": 26
                    },
                    {
                      "type": "none",
                      "speed": 282,
                      "confidence": "prec10ms",
                      "distance": 7166,
                      "class": 58
                    }
                  ]
                }
              ],
              "maneuverAssistList": [
                {
                  "connectionID": 32,
                  "queueLength": 4269,
                  "availableStorageLength": 6191,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 149,
                  "queueLength": 5573,
                  "availableStorageLength": 4699,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 67,
                  "queueLength": 7712,
                  "availableStorageLength": 6626,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                }
              ]
            },
            {
              "movementName": "IA5",
              "signalGroup": 24,
              "state-time-speed": [
                {
                  "eventState": "caution-Conflicting-Traffic",
                  "timing": {
                    "startTime": 15008,
                    "minEndTime": 33889,
                    "maxEndTime": 8367,
                    "likelyTime": 14633,
                    "confidence": 1,
                    "nextTime": 12291
                  },
                  "speeds": [
                    {
                      "type": "ecoDrive",
                      "speed": 377,
                      "confidence": "prec0-05ms",
                      "distance": 9834,
                      "class": 193
                    },
                    {
                      "type": "transit",
                      "speed": 233,
                      "confidence": "prec0-1ms",
                      "distance": 7725,
                      "class": 45
                    },
                    {
                      "type": "none",
                      "speed": 1,
                      "confidence": "prec10ms",
                      "distance": 1658,
                      "class": 55
                    },
                    {
                      "type": "none",
                      "speed": 253,
                      "confidence": "prec0-01ms",
                      "distance": 2192,
                      "class": 221
                    }
                  ]
                },
                {
                  "eventState": "caution-Conflicting-Traffic",
                  "timing": {
                    "startTime": 18837,
                    "minEndTime": 24643,
                    "maxEndTime": 10342,
                    "likelyTime": 22014,
                    "confidence": 2,
                    "nextTime": 445
                  },
                  "speeds": [
                    {
                      "type": "transit",
                      "speed": 15,
                      "confidence": "prec0-1ms",
                      "distance": 7658,
                      "class": 173
                    },
                    {
                      "type": "none",
                      "speed": 88,
                      "confidence": "prec0-01ms",
                      "distance": 2981,
                      "class": 170
                    },
                    {
                      "type": "none",
                      "speed": 453,
                      "confidence": "unavailable",
                      "distance": 9004,
                      "class": 13
                    }
                  ]
                },
                {
                  "eventState": "pre-Movement",
                  "timing": {
                    "startTime": 31908,
                    "minEndTime": 5138,
                    "maxEndTime": 35296,
                    "likelyTime": 3741,
                    "confidence": 4,
                    "nextTime": 115
                  },
                  "speeds": [
                    {
                      "type": "transit",
                      "speed": 320,
                      "confidence": "unavailable",
                      "distance": 1096,
                      "class": 53
                    }
                  ]
                },
                {
                  "eventState": "stop-Then-Proceed",
                  "timing": {
                    "startTime": 3818,
                    "minEndTime": 27374,
                    "maxEndTime": 16726,
                    "likelyTime": 30133,
                    "confidence": 1,
                    "nextTime": 3156
                  },
                  "speeds": [
                    {
                      "type": "greenwave",
                      "speed": 485,
                      "confidence": "prec10ms",
                      "distance": 2911,
                      "class": 242
                    },
                    {
                      "type": "ecoDrive",
                      "speed": 311,
                      "confidence": "prec0-1ms",
                      "distance": 1958,
                      "class": 150
                    },
                    {
                      "type": "greenwave",
                      "speed": 286,
                      "confidence": "prec10ms",
                      "distance": 5553,
                      "class": 123
                    },
                    {
                      "type": "none",
                      "speed": 296,
                      "confidence": "unavailable",
                      "distance": 3284,
                      "class": 130
                    }
                  ]
                }
              ],
              "maneuverAssistList": [
                {
                  "connectionID": 103,
                  "queueLength": 2848,
                  "availableStorageLength": 734,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 108,
                  "queueLength": 2987,
                  "availableStorageLength": 8172,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 92,
                  "queueLength": 6983,
                  "availableStorageLength": 695,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 104,
                  "queueLength": 8319,
                  "availableStorageLength": 4924,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 190,
                  "queueLength": 8114,
                  "availableStorageLength": 3535,
                  "waitOnStop": true,
                  "pedBicycleDetect": true
                }
              ]
            }
          ],
          "maneuverAssistList": [
            {
              "connectionID": 186,
              "queueLength": 3124,
              "availableStorageLength": 3060,
              "waitOnStop": true,
              "pedBicycleDetect": true
            },
            {
              "connectionID": 165,
              "queueLength": 2489,
              "availableStorageLength": 3993,
              "waitOnStop": true,
              "pedBicycleDetect": true
            },
            {
              "connectionID": 206,
              "queueLength": 9306,
              "availableStorageLength": 5304,
              "waitOnStop": true,
              "pedBicycleDetect": true
            }
          ],
          "roadAuthorityID": {
            "relRdAuthID": "15360.61804.29190.16756.6915"
          }
        }
      ]
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
 
// MAP field validation

TEST(MapFieldValidationTest, ValidMapPasses) {
    std::string json = R"({
        "messageId": 18,
        "value": {
            "MapData": {
                "msgIssueRevision": 1,
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "refPoint": {"lat": 389519791, "long": -771483512},
                    "laneWidth": 366,
                    "laneSet": [{
                        "laneID": 1,
                        "laneAttributes": {
                            "directionalUse": "C0",
                            "sharedWith": "0000",
                            "laneType": {"vehicle": "00"}
                        },
                        "nodeList": {
                            "nodes": [
                                {"delta": {"node-XY1": {"x": 100, "y": 200}}},
                                {"delta": {"node-XY1": {"x": 150, "y": 250}}}
                            ]
                        }
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_TRUE(result.valid) << (result.errors.empty() ? "" : result.errors[0]);
}

TEST(MapFieldValidationTest, MissingMessageIdFails) {
    std::string json = R"({
        "value": {
            "MapData": {
                "msgIssueRevision": 1
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
}

TEST(MapFieldValidationTest, MissingMsgIssueRevisionFails) {
    std::string json = R"({
        "messageId": 18,
        "value": {
            "MapData": {
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "refPoint": {"lat": 389519791, "long": -771483512},
                    "laneSet": [{
                        "laneID": 1,
                        "laneAttributes": {
                            "directionalUse": "C0",
                            "sharedWith": "0000",
                            "laneType": {"vehicle": "00"}
                        },
                        "nodeList": {"nodes": [
                            {"delta": {"node-XY1": {"x": 1, "y": 2}}},
                            {"delta": {"node-XY1": {"x": 3, "y": 4}}}
                        ]}
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
}

TEST(MapFieldValidationTest, MissingIntersectionIdFails) {
    std::string json = R"({
        "messageId": 18,
        "value": {
            "MapData": {
                "msgIssueRevision": 1,
                "intersections": [{
                    "revision": 0,
                    "refPoint": {"lat": 389519791, "long": -771483512},
                    "laneSet": [{
                        "laneID": 1,
                        "laneAttributes": {
                            "directionalUse": "C0",
                            "sharedWith": "0000",
                            "laneType": {"vehicle": "00"}
                        },
                        "nodeList": {"nodes": [
                            {"delta": {"node-XY1": {"x": 1, "y": 2}}},
                            {"delta": {"node-XY1": {"x": 3, "y": 4}}}
                        ]}
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
}

TEST(MapFieldValidationTest, MissingRefPointFails) {
    std::string json = R"({
        "messageId": 18,
        "value": {
            "MapData": {
                "msgIssueRevision": 1,
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "laneSet": [{
                        "laneID": 1,
                        "laneAttributes": {
                            "directionalUse": "C0",
                            "sharedWith": "0000",
                            "laneType": {"vehicle": "00"}
                        },
                        "nodeList": {"nodes": [
                            {"delta": {"node-XY1": {"x": 1, "y": 2}}},
                            {"delta": {"node-XY1": {"x": 3, "y": 4}}}
                        ]}
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
}

TEST(MapFieldValidationTest, MissingRefPointLatFails) {
    std::string json = R"({
        "messageId": 18,
        "value": {
            "MapData": {
                "msgIssueRevision": 1,
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "refPoint": {"long": -771483512},
                    "laneSet": [{
                        "laneID": 1,
                        "laneAttributes": {
                            "directionalUse": "C0",
                            "sharedWith": "0000",
                            "laneType": {"vehicle": "00"}
                        },
                        "nodeList": {"nodes": [
                            {"delta": {"node-XY1": {"x": 1, "y": 2}}},
                            {"delta": {"node-XY1": {"x": 3, "y": 4}}}
                        ]}
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
}

TEST(MapFieldValidationTest, MissingLaneSetFails) {
    std::string json = R"({
        "messageId": 18,
        "value": {
            "MapData": {
                "msgIssueRevision": 1,
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "refPoint": {"lat": 389519791, "long": -771483512}
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
}

TEST(MapFieldValidationTest, MissingLaneAttributesFails) {
    std::string json = R"({
        "messageId": 18,
        "value": {
            "MapData": {
                "msgIssueRevision": 1,
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "refPoint": {"lat": 389519791, "long": -771483512},
                    "laneSet": [{
                        "laneID": 1,
                        "nodeList": {"nodes": [
                            {"delta": {"node-XY1": {"x": 1, "y": 2}}},
                            {"delta": {"node-XY1": {"x": 3, "y": 4}}}
                        ]}
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
}

TEST(MapFieldValidationTest, MissingNodeListFails) {
    std::string json = R"({
        "messageId": 18,
        "value": {
            "MapData": {
                "msgIssueRevision": 1,
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "refPoint": {"lat": 389519791, "long": -771483512},
                    "laneSet": [{
                        "laneID": 1,
                        "laneAttributes": {
                            "directionalUse": "C0",
                            "sharedWith": "0000",
                            "laneType": {"vehicle": "00"}
                        }
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
}

TEST(MapFieldValidationTest, BitStringFieldsPreservedAsStrings) {
    std::string json = R"({
        "messageId": 18,
        "value": {
            "MapData": {
                "msgIssueRevision": 1,
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "refPoint": {"lat": 389519791, "long": -771483512},
                    "laneSet": [{
                        "laneID": 1,
                        "laneAttributes": {
                            "directionalUse": "C0",
                            "sharedWith": "0000",
                            "laneType": {"vehicle": "00"}
                        },
                        "maneuvers": "0400",
                        "nodeList": {"nodes": [
                            {"delta": {"node-XY1": {"x": 100, "y": 200}}},
                            {"delta": {"node-XY1": {"x": 150, "y": 250}}}
                        ]}
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_TRUE(result.valid) << (result.errors.empty() ? "" : result.errors[0]);
}

TEST(MapFieldValidationTest, MultipleLanesPasses) {
    std::string json = R"({
        "messageId": 18,
        "value": {
            "MapData": {
                "msgIssueRevision": 1,
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "refPoint": {"lat": 389519791, "long": -771483512},
                    "laneSet": [
                        {
                            "laneID": 1,
                            "ingressApproach": 1,
                            "laneAttributes": {
                                "directionalUse": "C0",
                                "sharedWith": "0000",
                                "laneType": {"vehicle": "00"}
                            },
                            "nodeList": {"nodes": [
                                {"delta": {"node-XY1": {"x": 10, "y": 20}}},
                                {"delta": {"node-XY1": {"x": 30, "y": 40}}}
                            ]}
                        },
                        {
                            "laneID": 2,
                            "egressApproach": 1,
                            "laneAttributes": {
                                "directionalUse": "40",
                                "sharedWith": "0000",
                                "laneType": {"vehicle": "00"}
                            },
                            "nodeList": {"nodes": [
                                {"delta": {"node-XY1": {"x": -10, "y": -20}}},
                                {"delta": {"node-XY1": {"x": -30, "y": -40}}}
                            ]}
                        }
                    ]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_TRUE(result.valid) << (result.errors.empty() ? "" : result.errors[0]);
}
 
} // namespace