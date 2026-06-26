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

#include <IntersectionValidationPlugin.h>
#include <MessageIntervalValidator.h>
#include <FieldValidation.h>
#include <RevisionCounterValidator.h>
#include <ODEForwarding.h>

using namespace tmx::messages;
using namespace IntersectionValidation;

namespace
{

  const std::string SPAT_SCHEMA_PATH = "../../../v2i-hub/IntersectionValidationPlugin/resources/spat.schema.json";
  const std::string MAP_SCHEMA_PATH = "../../../v2i-hub/IntersectionValidationPlugin/resources/map.schema.json";

  // Frequency Validation Tests

  // When lastTimestampMs is 0, we should treat it as the first message and return an interval of 0
  TEST(FrequencyValidationTest, InitialMessageIntervalIsZero)
  {
    auto result = calculateMessageInterval(0, 1000, SPAT_INTERVAL_MAX_THRESHOLD_MS);
    EXPECT_EQ(0u, result);
  }

  TEST(FrequencyValidationTest, SpatIntervalWithinThreshold)
  {
    auto result = calculateMessageInterval(1000, 1100, SPAT_INTERVAL_MAX_THRESHOLD_MS);
    EXPECT_EQ(100u, result);
  }

  TEST(FrequencyValidationTest, SpatIntervalExceedsThreshold)
  {
    EXPECT_THROW(
        calculateMessageInterval(1000, 1301, SPAT_INTERVAL_MAX_THRESHOLD_MS),
        tmx::TmxException);
  }

  TEST(FrequencyValidationTest, SpatIntervalCurrentTimestampEarlierThanLastTimestamp)
  {
    EXPECT_THROW(
        calculateMessageInterval(1001, 1000, SPAT_INTERVAL_MAX_THRESHOLD_MS),
        tmx::TmxException);
  }

  TEST(FrequencyValidationTest, MapIntervalWithinThreshold)
  {
    auto result = calculateMessageInterval(1000, 1050, MAP_INTERVAL_MAX_THRESHOLD_MS);
    EXPECT_EQ(50u, result);
  }

  TEST(FrequencyValidationTest, MapIntervalExceedsThreshold)
  {
    EXPECT_THROW(
        calculateMessageInterval(1000, 1101, MAP_INTERVAL_MAX_THRESHOLD_MS),
        tmx::TmxException);
  }

  TEST(FrequencyValidationTest, MapIntervalCurrentTimestampEarlierThanLastTimestamp)
  {
    EXPECT_THROW(
        calculateMessageInterval(1001, 1000, MAP_INTERVAL_MAX_THRESHOLD_MS),
        tmx::TmxException);
  }

  // SPaT Field Validation Tests

  TEST(FileLoadingTest, LoadExistingFile)
  {
    std::string path = "/tmp/test_schema.json";
    std::ofstream out(path);
    out << R"({"type": "object"})";
    out.close();

    std::string contents = loadFileContents(path);
    EXPECT_EQ(R"({"type": "object"})", contents);
    std::remove(path.c_str());
  }

  TEST(FileLoadingTest, LoadNonExistentFileThrows)
  {
    EXPECT_THROW(loadFileContents("/tmp/does_not_exist.json"), std::runtime_error);
  }

  TEST(SchemaValidationTest, InvalidJsonFails)
  {
    std::string schema = R"({"type": "object", "required": ["name"]})";
    auto result = validateJsonAgainstSchema("not json", schema);
    EXPECT_FALSE(result.valid);
    EXPECT_EQ("Failed to parse input JSON", result.errors[0]);
  }

  TEST(SchemaValidationTest, InvalidSchemaFails)
  {
    auto result = validateJsonAgainstSchema("{}", "not a schema");
    EXPECT_FALSE(result.valid);
    EXPECT_EQ("Failed to parse JSON schema", result.errors[0]);
  }

  TEST(SchemaValidationTest, NonExistentSchemaFileFails)
  {
    auto result = validateJsonAgainstSchemaFile(R"({})", "/tmp/missing_schema.json");
    EXPECT_FALSE(result.valid);
    EXPECT_EQ("Failed to open file: /tmp/missing_schema.json", result.errors[0]);
  }

  TEST(ConvertNumericStringsTest, ConvertsStringToIntWhenSchemaExpectsInteger)
  {
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

  TEST(ConvertNumericStringsTest, LeavesStringWhenSchemaExpectsString)
  {
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

  TEST(ConvertNumericStringsTest, LeavesNonNumericStringUnchanged)
  {
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

  TEST(ConvertNumericStringsTest, HandlesSchemaTypeArray)
  {
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

  TEST(ConvertNumericStringsTest, SkipsFieldNotInSchema)
  {
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

  TEST(ConvertNumericStringsTest, RecursesIntoNestedObjects)
  {
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

  TEST(ConvertNumericStringsTest, RecursesIntoArrayItems)
  {
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
                  "waitOnStop": "true",
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 34,
                  "queueLength": 2359,
                  "availableStorageLength": 6137,
                  "waitOnStop": "true",
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
                  "waitOnStop": "true",
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
                  "waitOnStop": "true",
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
              "waitOnStop": "true",
              "pedBicycleDetect": true
            },
            {
              "connectionID": 30,
              "queueLength": 6426,
              "availableStorageLength": 5206,
              "waitOnStop": "true",
              "pedBicycleDetect": true
            },
            {
              "connectionID": 101,
              "queueLength": 5339,
              "availableStorageLength": 1523,
              "waitOnStop": "true",
              "pedBicycleDetect": true
            },
            {
              "connectionID": 96,
              "queueLength": 8514,
              "availableStorageLength": 5733,
              "waitOnStop": "true",
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
                  "waitOnStop": "true",
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 240,
                  "queueLength": 8431,
                  "availableStorageLength": 5769,
                  "waitOnStop": "true",
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
                  "waitOnStop": "true",
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 149,
                  "queueLength": 5573,
                  "availableStorageLength": 4699,
                  "waitOnStop": "true",
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 67,
                  "queueLength": 7712,
                  "availableStorageLength": 6626,
                  "waitOnStop": "true",
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
                  "waitOnStop": "true",
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 108,
                  "queueLength": 2987,
                  "availableStorageLength": 8172,
                  "waitOnStop": "true",
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 92,
                  "queueLength": 6983,
                  "availableStorageLength": 695,
                  "waitOnStop": "true",
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 104,
                  "queueLength": 8319,
                  "availableStorageLength": 4924,
                  "waitOnStop": "true",
                  "pedBicycleDetect": true
                },
                {
                  "connectionID": 190,
                  "queueLength": 8114,
                  "availableStorageLength": 3535,
                  "waitOnStop": "true",
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
              "waitOnStop": "true",
              "pedBicycleDetect": true
            },
            {
              "connectionID": 165,
              "queueLength": 2489,
              "availableStorageLength": 3993,
              "waitOnStop": "true",
              "pedBicycleDetect": true
            },
            {
              "connectionID": 206,
              "queueLength": 9306,
              "availableStorageLength": 5304,
              "waitOnStop": "true",
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

  TEST(SpatFieldValidationTest, MissingMessageIdFails)
  {
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

  TEST(SpatFieldValidationTest, MissingIntersectionsFails)
  {
    std::string json = R"({
        "messageId": 19,
        "value": {
            "SPAT": {}
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, SPAT_SCHEMA_PATH);
    EXPECT_FALSE(result.valid);
  }

  TEST(SpatFieldValidationTest, MissingIntersectionIdFails)
  {
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

  TEST(SpatFieldValidationTest, MissingRevisionFails)
  {
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

  TEST(SpatFieldValidationTest, MissingStatusFails)
  {
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

  TEST(ConvertNumericStringsTest, ConvertsBooleanStringTrue)
  {
    std::string schemaStr = R"({"type": "object", "properties": {"active": {"type": "boolean"}}})";
    std::string jsonStr = R"({"active": "true"})";

    rapidjson::Document schemaDoc;
    schemaDoc.Parse(schemaStr.c_str());

    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);

    EXPECT_TRUE(doc["active"].IsBool());
    EXPECT_TRUE(doc["active"].GetBool());
  }

  TEST(ConvertNumericStringsTest, ConvertsBooleanStringFalse)
  {
    std::string schemaStr = R"({"type": "object", "properties": {"active": {"type": "boolean"}}})";
    std::string jsonStr = R"({"active": "false"})";

    rapidjson::Document schemaDoc;
    schemaDoc.Parse(schemaStr.c_str());

    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);

    EXPECT_TRUE(doc["active"].IsBool());
    EXPECT_FALSE(doc["active"].GetBool());
  }

  TEST(ConvertNumericStringsTest, LeavesBooleanWhenSchemaExpectsString)
  {
    std::string schemaStr = R"({"type": "object", "properties": {"flag": {"type": "string"}}})";
    std::string jsonStr = R"({"flag": "true"})";

    rapidjson::Document schemaDoc;
    schemaDoc.Parse(schemaStr.c_str());

    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);

    EXPECT_TRUE(doc["flag"].IsString());
    EXPECT_STREQ("true", doc["flag"].GetString());
  }

  TEST(ConvertNumericStringsTest, LeavesInvalidBooleanStringUnchanged)
  {
    std::string schemaStr = R"({"type": "object", "properties": {"active": {"type": "boolean"}}})";
    std::string jsonStr = R"({"active": "yes"})";

    rapidjson::Document schemaDoc;
    schemaDoc.Parse(schemaStr.c_str());

    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);

    EXPECT_TRUE(doc["active"].IsString());
    EXPECT_STREQ("yes", doc["active"].GetString());
  }

  TEST(ConvertNumericStringsTest, ConvertsBooleanInArray)
  {
    std::string schemaStr = R"({
        "type": "object",
        "properties": {
            "items": {
                "type": "array",
                "items": {
                    "type": "object",
                    "properties": {
                        "enabled": {"type": "boolean"}
                    }
                }
            }
        }
    })";
    std::string jsonStr = R"({"items": [{"enabled": "true"}, {"enabled": "false"}]})";

    rapidjson::Document schemaDoc;
    schemaDoc.Parse(schemaStr.c_str());

    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);

    EXPECT_TRUE(doc["items"][0]["enabled"].IsBool());
    EXPECT_TRUE(doc["items"][0]["enabled"].GetBool());
    EXPECT_TRUE(doc["items"][1]["enabled"].IsBool());
    EXPECT_FALSE(doc["items"][1]["enabled"].GetBool());
  }

  // MAP field validation

  TEST(MapFieldValidationTest, ValidMapPasses)
  {
    std::string json = R"({
  "messageId": 18,
  "value": {
    "MapData":{
      "timeStamp": 415833,
      "msgIssueRevision": 76,
      "layerType": "curveData",
      "layerID": 88,
      "intersections": [
        {
          "name": "JRWQERRMGBSHOPBFAQOUALCEYSUCYJUSNHFMUTNWMXIDOIUDIRHRSDROQKQGM",
          "id": {
            "region": 8519,
            "id": 6782
          },
          "revision": 88,
          "refPoint": {
            "lat": 702102039,
            "long": 648915460,
            "elevation": 6268
          },
          "laneWidth": 30296,
          "speedLimits": [
            {
              "type": "truckMaxSpeed",
              "speed": 7638
            },
            {
              "type": "vehiclesWithTrailersMinSpeed",
              "speed": 8087
            },
            {
              "type": "maxSpeedInSchoolZone",
              "speed": 5331
            }
          ],
          "laneSet": [
            {
              "laneID": 237,
              "name": "UPMGGWXJMVTVNIBUL",
              "ingressApproach": 8,
              "egressApproach": 6,
              "laneAttributes": {
                "directionalUse": "C0",
                "sharedWith": "C9C0",
                "laneType": {
                  "median": "3953"
                }
              },
              "maneuvers": "91A0",
              "nodeList": {
                "nodes": [
                  {
                    "delta": {
                      "node-XY3": {
                        "x": -1689,
                        "y": -402
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "hydrantPresent"
                      ],
                      "disabled": [
                        "costToPark"
                      ],
                      "enabled": [
                        "turnOutPointOnLeft",
                        "taperToLeft"
                      ],
                      "data": [
                        {
                          "laneAngle": 35
                        },
                        {
                          "laneCrownPointLeft": 98
                        },
                        {
                          "pathEndPointAngle": -149
                        },
                        {
                          "speedLimits": [
                            {
                              "type": "truckMaxSpeed",
                              "speed": 4325
                            }
                          ]
                        }
                      ],
                      "dWidth": 273,
                      "dElevation": 165
                    }
                  },
                  {
                    "delta": {
                      "node-XY3": {
                        "x": -1347,
                        "y": 776
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "curbPresentAtStepOff",
                        "reserved",
                        "hydrantPresent",
                        "reserved"
                      ],
                      "disabled": [
                        "freeParking",
                        "sharedBikeLane",
                        "doNotBlock",
                        "transitStopOnLeft"
                      ],
                      "enabled": [
                        "mergingLaneLeft",
                        "taperToRight",
                        "rumbleStripPresent",
                        "curbOnLeft"
                      ],
                      "data": [
                        {
                          "speedLimits": [
                            {
                              "type": "maxSpeedInSchoolZone",
                              "speed": 4776
                            },
                            {
                              "type": "maxSpeedInSchoolZone",
                              "speed": 7892
                            },
                            {
                              "type": "vehiclesWithTrailersMaxSpeed",
                              "speed": 1073
                            },
                            {
                              "type": "maxSpeedInConstructionZone",
                              "speed": 7471
                            }
                          ]
                        },
                        {
                          "laneAngle": 106
                        },
                        {
                          "laneCrownPointLeft": 42
                        },
                        {
                          "laneCrownPointLeft": -108
                        }
                      ],
                      "dWidth": -91,
                      "dElevation": 182
                    }
                  },
                  {
                    "delta": {
                      "node-XY4": {
                        "x": 3786,
                        "y": -2826
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "roundedCapStyleA"
                      ],
                      "disabled": [
                        "transitStopInLane"
                      ],
                      "enabled": [
                        "taperToLeft",
                        "doNotBlock",
                        "adjacentParkingOnLeft",
                        "mergingLaneLeft"
                      ],
                      "data": [
                        {
                          "laneCrownPointRight": -96
                        }
                      ],
                      "dWidth": 65,
                      "dElevation": -15
                    }
                  }
                ]
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 225,
                    "maneuver": "A540"
                  },
                  "remoteIntersection": {
                    "region": 1160,
                    "id": 49417
                  },
                  "signalGroup": 36,
                  "userClass": 154,
                  "connectionID": 241
                }
              ],
              "overlays": [
                156,
                223,
                245,
                130
              ]
            },
            {
              "laneID": 231,
              "name": "LAMMXAFQVTJEGIGQIRB",
              "ingressApproach": 14,
              "egressApproach": 5,
              "laneAttributes": {
                "directionalUse": "C0",
                "sharedWith": "77C0",
                "laneType": {
                  "median": "F79D"
                }
              },
              "maneuvers": "4C10",
              "nodeList": {
                "nodes": [
                  {
                    "delta": {
                      "node-XY1": {
                        "x": -352,
                        "y": 246
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "mergePoint",
                        "downstreamStopLine",
                        "downstreamStartNode"
                      ],
                      "disabled": [
                        "doNotBlock"
                      ],
                      "enabled": [
                        "loadingzoneOnRight"
                      ],
                      "data": [
                        {
                          "laneCrownPointCenter": 54
                        },
                        {
                          "laneAngle": 71
                        }
                      ],
                      "dWidth": -505,
                      "dElevation": -232
                    }
                  },
                  {
                    "delta": {
                      "node-LatLon": {
                        "lon": 24320810,
                        "lat": -377322091
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "downstreamStopLine",
                        "mergePoint",
                        "curbPresentAtStepOff"
                      ],
                      "disabled": [
                        "rfSignalRequestPresent",
                        "adjacentBikeLaneOnLeft",
                        "loadingzoneOnRight",
                        "curbOnLeft"
                      ],
                      "enabled": [
                        "bikeBoxInFront",
                        "parallelParking"
                      ],
                      "data": [
                        {
                          "laneCrownPointCenter": -52
                        },
                        {
                          "laneCrownPointCenter": 67
                        },
                        {
                          "laneAngle": 171
                        }
                      ],
                      "dWidth": 6,
                      "dElevation": 467
                    }
                  },
                  {
                    "delta": {
                      "node-XY1": {
                        "x": 5,
                        "y": -405
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "curbPresentAtStepOff",
                        "downstreamStopLine",
                        "stopLine"
                      ],
                      "disabled": [
                        "lowCurbsPresent",
                        "transitStopOnRight"
                      ],
                      "enabled": [
                        "mergingLaneLeft",
                        "lowCurbsPresent",
                        "safeIsland",
                        "transitStopOnRight"
                      ],
                      "data": [
                        {
                          "laneCrownPointCenter": 94
                        },
                        {
                          "laneCrownPointCenter": 92
                        }
                      ],
                      "dWidth": -319,
                      "dElevation": 132
                    }
                  }
                ]
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 76,
                    "maneuver": "5C10"
                  },
                  "remoteIntersection": {
                    "region": 24129,
                    "id": 27370
                  },
                  "signalGroup": 137,
                  "userClass": 240,
                  "connectionID": 39
                },
                {
                  "connectingLane": {
                    "lane": 246,
                    "maneuver": "07E0"
                  },
                  "remoteIntersection": {
                    "region": 48391,
                    "id": 61519
                  },
                  "signalGroup": 46,
                  "userClass": 74,
                  "connectionID": 235
                }
              ],
              "overlays": [
                211,
                2,
                240,
                81
              ]
            },
            {
              "laneID": 50,
              "name": "BAAJXWARQRUPJJVWUKUGHUQKBMVIIVRPSSDELUJBMWHMDGK",
              "ingressApproach": 8,
              "egressApproach": 4,
              "laneAttributes": {
                "directionalUse": "80",
                "sharedWith": "01C0",
                "laneType": {
                  "trackedVehicle": "40F0"
                }
              },
              "maneuvers": "8C80",
              "nodeList": {
                "nodes": [
                  {
                    "delta": {
                      "node-XY3": {
                        "x": 475,
                        "y": 1271
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "stopLine",
                        "curbPresentAtStepOff",
                        "downstreamStartNode",
                        "safeIsland"
                      ],
                      "disabled": [
                        "freeParking"
                      ],
                      "enabled": [
                        "transitStopOnRight"
                      ],
                      "data": [
                        {
                          "laneCrownPointCenter": -11
                        },
                        {
                          "laneCrownPointCenter": 75
                        },
                        {
                          "laneCrownPointRight": 3
                        },
                        {
                          "speedLimits": [
                            {
                              "type": "truckNightMaxSpeed",
                              "speed": 2408
                            },
                            {
                              "type": "vehicleNightMaxSpeed",
                              "speed": 6729
                            }
                          ]
                        }
                      ],
                      "dWidth": 414,
                      "dElevation": -209
                    }
                  },
                  {
                    "delta": {
                      "node-LatLon": {
                        "lon": -980228109,
                        "lat": -32828165
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "downstreamStopLine"
                      ],
                      "disabled": [
                        "partialCurbIntrusion"
                      ],
                      "enabled": [
                        "midBlockCurbPresent",
                        "transitStopOnRight"
                      ],
                      "data": [
                        {
                          "laneAngle": -11
                        },
                        {
                          "laneCrownPointLeft": 21
                        },
                        {
                          "pathEndPointAngle": -40
                        },
                        {
                          "laneCrownPointCenter": 74
                        }
                      ],
                      "dWidth": -237,
                      "dElevation": 131
                    }
                  },
                  {
                    "delta": {
                      "node-XY4": {
                        "x": 425,
                        "y": 1204
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "roundedCapStyleB"
                      ],
                      "disabled": [
                        "loadingzoneOnRight",
                        "curbOnRight",
                        "mergingLaneLeft",
                        "adjacentParkingOnRight"
                      ],
                      "enabled": [
                        "partialCurbIntrusion",
                        "adjacentParkingOnRight",
                        "adjacentBikeLaneOnLeft"
                      ],
                      "data": [
                        {
                          "laneAngle": -125
                        },
                        {
                          "speedLimits": [
                            {
                              "type": "vehicleMaxSpeed",
                              "speed": 5546
                            },
                            {
                              "type": "vehiclesWithTrailersMinSpeed",
                              "speed": 3471
                            },
                            {
                              "type": "vehicleNightMaxSpeed",
                              "speed": 3587
                            }
                          ]
                        },
                        {
                          "pathEndPointAngle": 37
                        },
                        {
                          "pathEndPointAngle": 146
                        }
                      ],
                      "dWidth": 274,
                      "dElevation": 192
                    }
                  },
                  {
                    "delta": {
                      "node-XY5": {
                        "x": -6881,
                        "y": 860
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "hydrantPresent",
                        "stopLine",
                        "reserved",
                        "downstreamStopLine"
                      ],
                      "disabled": [
                        "curbOnLeft",
                        "midBlockCurbPresent",
                        "loadingzoneOnLeft",
                        "reserved"
                      ],
                      "enabled": [
                        "freeParking",
                        "mergingLaneRight",
                        "adjacentBikeLaneOnLeft",
                        "curbOnLeft"
                      ],
                      "data": [
                        {
                          "laneCrownPointRight": -69
                        },
                        {
                          "laneCrownPointCenter": 115
                        }
                      ],
                      "dWidth": -491,
                      "dElevation": 353
                    }
                  }
                ]
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 152,
                    "maneuver": "DFC0"
                  },
                  "remoteIntersection": {
                    "region": 59187,
                    "id": 20956
                  },
                  "signalGroup": 172,
                  "userClass": 225,
                  "connectionID": 68
                },
                {
                  "connectingLane": {
                    "lane": 32,
                    "maneuver": "8930"
                  },
                  "remoteIntersection": {
                    "region": 29665,
                    "id": 55977
                  },
                  "signalGroup": 40,
                  "userClass": 199,
                  "connectionID": 51
                },
                {
                  "connectingLane": {
                    "lane": 170,
                    "maneuver": "E940"
                  },
                  "remoteIntersection": {
                    "region": 12514,
                    "id": 42065
                  },
                  "signalGroup": 93,
                  "userClass": 240,
                  "connectionID": 76
                },
                {
                  "connectingLane": {
                    "lane": 78,
                    "maneuver": "8510"
                  },
                  "remoteIntersection": {
                    "region": 53064,
                    "id": 46548
                  },
                  "signalGroup": 130,
                  "userClass": 6,
                  "connectionID": 98
                }
              ],
              "overlays": [
                206,
                130,
                17
              ]
            },
            {
              "laneID": 180,
              "name": "NJCNIJKMIELXFWNVXIYMELFH",
              "ingressApproach": 9,
              "egressApproach": 13,
              "laneAttributes": {
                "directionalUse": "C0",
                "sharedWith": "8C40",
                "laneType": {
                  "parking": "BF0D"
                }
              },
              "maneuvers": "8D50",
              "nodeList": {
                "nodes": [
                  {
                    "delta": {
                      "node-XY1": {
                        "x": -167,
                        "y": 105
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "closedToTraffic",
                        "reserved"
                      ],
                      "disabled": [
                        "parallelParking",
                        "freeParking"
                      ],
                      "enabled": [
                        "rfSignalRequestPresent",
                        "turnOutPointOnLeft"
                      ],
                      "data": [
                        {
                          "pathEndPointAngle": 112
                        }
                      ],
                      "dWidth": -269,
                      "dElevation": 340
                    }
                  },
                  {
                    "delta": {
                      "node-XY5": {
                        "x": 854,
                        "y": -5682
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "divergePoint",
                        "divergePoint",
                        "divergePoint",
                        "roundedCapStyleB"
                      ],
                      "disabled": [
                        "whiteLine",
                        "taperToLeft",
                        "lowCurbsPresent",
                        "adjacentBikeLaneOnRight"
                      ],
                      "enabled": [
                        "headInParking",
                        "sharedBikeLane",
                        "doNotBlock"
                      ],
                      "data": [
                        {
                          "laneCrownPointRight": 43
                        },
                        {
                          "laneCrownPointRight": 36
                        },
                        {
                          "speedLimits": [
                            {
                              "type": "maxSpeedInConstructionZone",
                              "speed": 4487
                            },
                            {
                              "type": "vehicleMaxSpeed",
                              "speed": 2676
                            },
                            {
                              "type": "vehicleNightMaxSpeed",
                              "speed": 3945
                            },
                            {
                              "type": "truckNightMaxSpeed",
                              "speed": 1846
                            }
                          ]
                        },
                        {
                          "speedLimits": [
                            {
                              "type": "maxSpeedInSchoolZoneWhenChildrenArePresent",
                              "speed": 7160
                            },
                            {
                              "type": "truckMaxSpeed",
                              "speed": 1451
                            },
                            {
                              "type": "unknown",
                              "speed": 1146
                            },
                            {
                              "type": "truckNightMaxSpeed",
                              "speed": 5022
                            }
                          ]
                        }
                      ],
                      "dWidth": 397,
                      "dElevation": 264
                    }
                  }
                ]
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 36,
                    "maneuver": "FBD0"
                  },
                  "remoteIntersection": {
                    "region": 38786,
                    "id": 8582
                  },
                  "signalGroup": 207,
                  "userClass": 225,
                  "connectionID": 0
                },
                {
                  "connectingLane": {
                    "lane": 215,
                    "maneuver": "D980"
                  },
                  "remoteIntersection": {
                    "region": 36512,
                    "id": 62874
                  },
                  "signalGroup": 86,
                  "userClass": 212,
                  "connectionID": 41
                },
                {
                  "connectingLane": {
                    "lane": 84,
                    "maneuver": "02E0"
                  },
                  "remoteIntersection": {
                    "region": 36642,
                    "id": 37414
                  },
                  "signalGroup": 159,
                  "userClass": 63,
                  "connectionID": 89
                }
              ],
              "overlays": [
                119,
                170,
                227,
                241
              ]
            }
          ],
          "roadAuthorityID": {
            "fullRdAuthID": "2.31.264"
          }
        },
        {
          "name": "VJNAIENE",
          "id": {
            "region": 31368,
            "id": 50481
          },
          "revision": 5,
          "refPoint": {
            "lat": 561517654,
            "long": 1119656094,
            "elevation": 8871
          },
          "laneWidth": 28042,
          "speedLimits": [
            {
              "type": "vehicleNightMaxSpeed",
              "speed": 4933
            },
            {
              "type": "vehiclesWithTrailersMinSpeed",
              "speed": 1507
            },
            {
              "type": "vehiclesWithTrailersMaxSpeed",
              "speed": 1624
            },
            {
              "type": "vehicleMinSpeed",
              "speed": 1832
            }
          ],
          "laneSet": [
            {
              "laneID": 99,
              "name": "COJSVBDKMURRWNPMLNEGQTJICGLLQWBAKKWGOHIXSQMGFAPLGQPQHQWS",
              "ingressApproach": 14,
              "egressApproach": 13,
              "laneAttributes": {
                "directionalUse": "40",
                "sharedWith": "4D00",
                "laneType": {
                  "median": "9F46"
                }
              },
              "maneuvers": "2370",
              "nodeList": {
                "nodes": [
                  {
                    "delta": {
                      "node-XY6": {
                        "x": -25809,
                        "y": -25348
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "safeIsland",
                        "curbPresentAtStepOff",
                        "downstreamStopLine"
                      ],
                      "disabled": [
                        "costToPark",
                        "mergingLaneRight"
                      ],
                      "enabled": [
                        "timeRestrictionsOnParking",
                        "rumbleStripPresent"
                      ],
                      "data": [
                        {
                          "laneCrownPointRight": 21
                        },
                        {
                          "laneAngle": 28
                        }
                      ],
                      "dWidth": -466,
                      "dElevation": -152
                    }
                  },
                  {
                    "delta": {
                      "node-XY5": {
                        "x": -7157,
                        "y": 148
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "safeIsland",
                        "stopLine",
                        "closedToTraffic",
                        "downstreamStartNode"
                      ],
                      "disabled": [
                        "taperToLeft"
                      ],
                      "enabled": [
                        "freeParking",
                        "turnOutPointOnLeft",
                        "partialCurbIntrusion"
                      ],
                      "data": [
                        {
                          "laneAngle": -131
                        },
                        {
                          "laneAngle": -96
                        }
                      ],
                      "dWidth": -55,
                      "dElevation": 137
                    }
                  },
                  {
                    "delta": {
                      "node-XY2": {
                        "x": 911,
                        "y": 406
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "curbPresentAtStepOff"
                      ],
                      "disabled": [
                        "lowCurbsPresent",
                        "turnOutPointOnRight",
                        "sharedWithTrackedVehicle",
                        "transitStopOnLeft"
                      ],
                      "enabled": [
                        "mergingLaneLeft",
                        "rumbleStripPresent",
                        "turnOutPointOnRight",
                        "transitStopOnLeft"
                      ],
                      "data": [
                        {
                          "laneCrownPointCenter": 70
                        }
                      ],
                      "dWidth": 416,
                      "dElevation": 349
                    }
                  }
                ]
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 221,
                    "maneuver": "FAA0"
                  },
                  "remoteIntersection": {
                    "region": 36121,
                    "id": 50025
                  },
                  "signalGroup": 184,
                  "userClass": 56,
                  "connectionID": 88
                },
                {
                  "connectingLane": {
                    "lane": 226,
                    "maneuver": "6FD0"
                  },
                  "remoteIntersection": {
                    "region": 47221,
                    "id": 47675
                  },
                  "signalGroup": 190,
                  "userClass": 19,
                  "connectionID": 106
                }
              ],
              "overlays": [
                156,
                114,
                186
              ]
            }
          ],
          "roadAuthorityID": {
            "relRdAuthID": "20688"
          }
        }
      ],
      "roadSegments": [
        {
          "name": "CPYHVNV",
          "id": {
            "region": 33263,
            "id": 38364
          },
          "revision": 43,
          "refPoint": {
            "lat": 71734265,
            "long": -1289512729,
            "elevation": 42339
          },
          "laneWidth": 11126,
          "speedLimits": [
            {
              "type": "maxSpeedInSchoolZone",
              "speed": 6398
            },
            {
              "type": "vehiclesWithTrailersNightMaxSpeed",
              "speed": 1497
            },
            {
              "type": "unknown",
              "speed": 3397
            },
            {
              "type": "vehiclesWithTrailersMaxSpeed",
              "speed": 5686
            }
          ],
          "roadLaneSet": [
            {
              "laneID": 81,
              "name": "YNHGVCBKPWSVQKPJHAEN",
              "ingressApproach": 6,
              "egressApproach": 14,
              "laneAttributes": {
                "directionalUse": "80",
                "sharedWith": "CF40",
                "laneType": {
                  "parking": "403B"
                }
              },
              "maneuvers": "9AB0",
              "nodeList": {
                "nodes": [
                  {
                    "delta": {
                      "node-LatLon": {
                        "lon": -273478191,
                        "lat": -300445483
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "roundedCapStyleB",
                        "curbPresentAtStepOff",
                        "curbPresentAtStepOff"
                      ],
                      "disabled": [
                        "mergingLaneRight"
                      ],
                      "enabled": [
                        "unEvenPavementPresent",
                        "midBlockCurbPresent",
                        "headInParking"
                      ],
                      "data": [
                        {
                          "laneCrownPointLeft": -116
                        },
                        {
                          "laneCrownPointRight": -24
                        }
                      ],
                      "dWidth": -459,
                      "dElevation": -26
                    }
                  },
                  {
                    "delta": {
                      "node-XY1": {
                        "x": 323,
                        "y": -188
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "downstreamStartNode",
                        "divergePoint",
                        "roundedCapStyleA"
                      ],
                      "disabled": [
                        "doNotBlock",
                        "doNotBlock"
                      ],
                      "enabled": [
                        "parallelParking"
                      ],
                      "data": [
                        {
                          "pathEndPointAngle": 76
                        },
                        {
                          "pathEndPointAngle": 13
                        },
                        {
                          "laneCrownPointRight": 52
                        }
                      ],
                      "dWidth": -33,
                      "dElevation": 189
                    }
                  },
                  {
                    "delta": {
                      "node-XY2": {
                        "x": -465,
                        "y": 615
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "mergePoint",
                        "curbPresentAtStepOff",
                        "curbPresentAtStepOff",
                        "divergePoint"
                      ],
                      "disabled": [
                        "taperToCenterLine",
                        "partialCurbIntrusion"
                      ],
                      "enabled": [
                        "loadingzoneOnLeft",
                        "partialCurbIntrusion",
                        "curbOnLeft"
                      ],
                      "data": [
                        {
                          "speedLimits": [
                            {
                              "type": "maxSpeedInSchoolZoneWhenChildrenArePresent",
                              "speed": 1665
                            },
                            {
                              "type": "maxSpeedInConstructionZone",
                              "speed": 1204
                            }
                          ]
                        },
                        {
                          "laneAngle": 55
                        },
                        {
                          "laneCrownPointRight": -50
                        }
                      ],
                      "dWidth": 308,
                      "dElevation": -116
                    }
                  },
                  {
                    "delta": {
                      "node-LatLon": {
                        "lon": 1736879831,
                        "lat": 234983173
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "roundedCapStyleA"
                      ],
                      "disabled": [
                        "transitStopOnRight",
                        "rfSignalRequestPresent"
                      ],
                      "enabled": [
                        "mergingLaneLeft",
                        "safeIsland"
                      ],
                      "data": [
                        {
                          "laneCrownPointCenter": -36
                        },
                        {
                          "laneAngle": -80
                        }
                      ],
                      "dWidth": 84,
                      "dElevation": -235
                    }
                  }
                ]
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 6,
                    "maneuver": "59B0"
                  },
                  "remoteIntersection": {
                    "region": 33041,
                    "id": 86
                  },
                  "signalGroup": 127,
                  "userClass": 170,
                  "connectionID": 211
                },
                {
                  "connectingLane": {
                    "lane": 72,
                    "maneuver": "D9D0"
                  },
                  "remoteIntersection": {
                    "region": 2232,
                    "id": 57119
                  },
                  "signalGroup": 39,
                  "userClass": 100,
                  "connectionID": 76
                }
              ],
              "overlays": [
                120,
                123
              ]
            },
            {
              "laneID": 201,
              "name": "AKRVNRNPAOVKYQVXTVMLYWMIHNYEEJEEXGTXXGBIHTKABCLMFUSGCFJVQ",
              "ingressApproach": 0,
              "egressApproach": 7,
              "laneAttributes": {
                "directionalUse": "40",
                "sharedWith": "5780",
                "laneType": {
                  "trackedVehicle": "F2D5"
                }
              },
              "maneuvers": "4120",
              "nodeList": {
                "nodes": [
                  {
                    "delta": {
                      "node-LatLon": {
                        "lon": -98156291,
                        "lat": 183505986
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "roundedCapStyleA",
                        "divergePoint",
                        "reserved"
                      ],
                      "disabled": [
                        "lowCurbsPresent",
                        "costToPark"
                      ],
                      "enabled": [
                        "adjacentParkingOnLeft",
                        "partialCurbIntrusion",
                        "taperToRight",
                        "timeRestrictionsOnParking"
                      ],
                      "data": [
                        {
                          "laneCrownPointCenter": 55
                        }
                      ],
                      "dWidth": 30,
                      "dElevation": 66
                    }
                  },
                  {
                    "delta": {
                      "node-XY6": {
                        "x": 29643,
                        "y": 32602
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "downstreamStopLine"
                      ],
                      "disabled": [
                        "safeIsland"
                      ],
                      "enabled": [
                        "mergingLaneLeft",
                        "audibleSignalingPresent",
                        "unEvenPavementPresent"
                      ],
                      "data": [
                        {
                          "laneCrownPointCenter": 84
                        },
                        {
                          "pathEndPointAngle": 96
                        }
                      ],
                      "dWidth": 221,
                      "dElevation": -291
                    }
                  }
                ]
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 199,
                    "maneuver": "92D0"
                  },
                  "remoteIntersection": {
                    "region": 37976,
                    "id": 21733
                  },
                  "signalGroup": 159,
                  "userClass": 194,
                  "connectionID": 61
                }
              ],
              "overlays": [
                145,
                233,
                74
              ]
            },
            {
              "laneID": 235,
              "name": "NKWEQTSGFGLTIIBKENRLOGOKAMJCFNVUFDGHWGAHWMRCJYLVFEWJSCTSYXBKA",
              "ingressApproach": 2,
              "egressApproach": 1,
              "laneAttributes": {
                "directionalUse": "80",
                "sharedWith": "A840",
                "laneType": {
                  "vehicle": {
                    "value": "35",
                    "length": 8
                  }
                }
              },
              "maneuvers": "CD00",
              "nodeList": {
                "computed": {
                  "referenceLaneId": 61,
                  "offsetXaxis": {
                    "small": 1899
                  },
                  "offsetYaxis": {
                    "large": -28817
                  },
                  "rotateXY": 12948,
                  "scaleXaxis": -838,
                  "scaleYaxis": -736
                }
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 212,
                    "maneuver": "D300"
                  },
                  "remoteIntersection": {
                    "region": 52860,
                    "id": 47223
                  },
                  "signalGroup": 130,
                  "userClass": 231,
                  "connectionID": 10
                },
                {
                  "connectingLane": {
                    "lane": 74,
                    "maneuver": "CFC0"
                  },
                  "remoteIntersection": {
                    "region": 11311,
                    "id": 41024
                  },
                  "signalGroup": 141,
                  "userClass": 195,
                  "connectionID": 124
                },
                {
                  "connectingLane": {
                    "lane": 130,
                    "maneuver": "8140"
                  },
                  "remoteIntersection": {
                    "region": 3495,
                    "id": 60672
                  },
                  "signalGroup": 70,
                  "userClass": 187,
                  "connectionID": 118
                }
              ],
              "overlays": [
                220,
                178,
                254
              ]
            },
            {
              "laneID": 11,
              "name": "HHMIBDFUQJSOKPCKKNPPRUDCBHSRGVGLJBQIBR",
              "ingressApproach": 2,
              "egressApproach": 12,
              "laneAttributes": {
                "directionalUse": "00",
                "sharedWith": "E340",
                "laneType": {
                  "trackedVehicle": "737A"
                }
              },
              "maneuvers": "1C80",
              "nodeList": {
                "nodes": [
                  {
                    "delta": {
                      "node-XY3": {
                        "x": 979,
                        "y": -765
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "curbPresentAtStepOff",
                        "downstreamStartNode"
                      ],
                      "disabled": [
                        "adjacentBikeLaneOnRight",
                        "loadingzoneOnRight",
                        "sharedBikeLane",
                        "taperToCenterLine"
                      ],
                      "enabled": [
                        "safeIsland",
                        "transitStopInLane",
                        "turnOutPointOnRight"
                      ],
                      "data": [
                        {
                          "laneCrownPointCenter": -95
                        }
                      ],
                      "dWidth": -97,
                      "dElevation": 169
                    }
                  },
                  {
                    "delta": {
                      "node-XY5": {
                        "x": 847,
                        "y": 5704
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "curbPresentAtStepOff",
                        "downstreamStopLine"
                      ],
                      "disabled": [
                        "loadingzoneOnLeft",
                        "reserved"
                      ],
                      "enabled": [
                        "transitStopOnLeft",
                        "lowCurbsPresent"
                      ],
                      "data": [
                        {
                          "laneCrownPointCenter": 88
                        },
                        {
                          "laneCrownPointRight": 43
                        },
                        {
                          "speedLimits": [
                            {
                              "type": "vehicleNightMaxSpeed",
                              "speed": 1736
                            },
                            {
                              "type": "vehiclesWithTrailersMinSpeed",
                              "speed": 7753
                            }
                          ]
                        }
                      ],
                      "dWidth": -48,
                      "dElevation": -337
                    }
                  },
                  {
                    "delta": {
                      "node-XY4": {
                        "x": 3778,
                        "y": -2277
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "hydrantPresent",
                        "roundedCapStyleA",
                        "closedToTraffic",
                        "stopLine"
                      ],
                      "disabled": [
                        "transitStopInLane",
                        "transitStopInLane",
                        "midBlockCurbPresent",
                        "transitStopInLane"
                      ],
                      "enabled": [
                        "parallelParking",
                        "mergingLaneLeft"
                      ],
                      "data": [
                        {
                          "speedLimits": [
                            {
                              "type": "vehicleMaxSpeed",
                              "speed": 5411
                            },
                            {
                              "type": "maxSpeedInConstructionZone",
                              "speed": 2135
                            },
                            {
                              "type": "unknown",
                              "speed": 6167
                            }
                          ]
                        },
                        {
                          "laneAngle": 75
                        },
                        {
                          "speedLimits": [
                            {
                              "type": "unknown",
                              "speed": 4692
                            }
                          ]
                        },
                        {
                          "laneAngle": 111
                        }
                      ],
                      "dWidth": 26,
                      "dElevation": -152
                    }
                  }
                ]
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 131,
                    "maneuver": "B6E0"
                  },
                  "remoteIntersection": {
                    "region": 12122,
                    "id": 57752
                  },
                  "signalGroup": 237,
                  "userClass": 114,
                  "connectionID": 88
                }
              ],
              "overlays": [
                194,
                41,
                44,
                211
              ]
            }
          ]
        },
        {
          "name": "SDTSNFJQSJTVPFRIJDQKVFORBJKUSSCXVXYFDKVAYJKJVVY",
          "id": {
            "region": 20508,
            "id": 7089
          },
          "revision": 33,
          "refPoint": {
            "lat": -806034592,
            "long": -373014989,
            "elevation": 30987
          },
          "laneWidth": 28106,
          "speedLimits": [
            {
              "type": "maxSpeedInSchoolZoneWhenChildrenArePresent",
              "speed": 857
            },
            {
              "type": "maxSpeedInSchoolZoneWhenChildrenArePresent",
              "speed": 1730
            }
          ],
          "roadLaneSet": [
            {
              "laneID": 216,
              "name": "RGUYIPLBDRKKWUKJELQYFLQYCVHUPPXTMSJDVUPNUYSGWT",
              "ingressApproach": 9,
              "egressApproach": 1,
              "laneAttributes": {
                "directionalUse": "C0",
                "sharedWith": "4D00",
                "laneType": {
                  "sidewalk": "B9DE"
                }
              },
              "maneuvers": "4AE0",
              "nodeList": {
                "computed": {
                  "referenceLaneId": 227,
                  "offsetXaxis": {
                    "large": -6653
                  },
                  "offsetYaxis": {
                    "small": 1007
                  },
                  "rotateXY": 3460,
                  "scaleXaxis": 1357,
                  "scaleYaxis": 1306
                }
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 165,
                    "maneuver": "00E0"
                  },
                  "remoteIntersection": {
                    "region": 51369,
                    "id": 45017
                  },
                  "signalGroup": 226,
                  "userClass": 206,
                  "connectionID": 165
                },
                {
                  "connectingLane": {
                    "lane": 70,
                    "maneuver": "7270"
                  },
                  "remoteIntersection": {
                    "region": 65062,
                    "id": 46188
                  },
                  "signalGroup": 65,
                  "userClass": 99,
                  "connectionID": 120
                },
                {
                  "connectingLane": {
                    "lane": 42,
                    "maneuver": "4830"
                  },
                  "remoteIntersection": {
                    "region": 15425,
                    "id": 25075
                  },
                  "signalGroup": 14,
                  "userClass": 227,
                  "connectionID": 107
                },
                {
                  "connectingLane": {
                    "lane": 153,
                    "maneuver": "4AF0"
                  },
                  "remoteIntersection": {
                    "region": 58631,
                    "id": 55240
                  },
                  "signalGroup": 148,
                  "userClass": 193,
                  "connectionID": 103
                }
              ],
              "overlays": [
                40
              ]
            },
            {
              "laneID": 90,
              "name": "KGRSPLBSXQMOPQUOYJDJKGYVPRGIMHIQKO",
              "ingressApproach": 10,
              "egressApproach": 2,
              "laneAttributes": {
                "directionalUse": "00",
                "sharedWith": "E9C0",
                "laneType": {
                  "bikeLane": "CA8C"
                }
              },
              "maneuvers": "CD00",
              "nodeList": {
                "nodes": [
                  {
                    "delta": {
                      "node-LatLon": {
                        "lon": -522244173,
                        "lat": -434481673
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "curbPresentAtStepOff",
                        "downstreamStopLine",
                        "stopLine",
                        "safeIsland"
                      ],
                      "disabled": [
                        "reserved",
                        "lowCurbsPresent",
                        "turnOutPointOnRight",
                        "adjacentParkingOnRight"
                      ],
                      "enabled": [
                        "transitStopOnLeft",
                        "turnOutPointOnRight",
                        "taperToLeft"
                      ],
                      "data": [
                        {
                          "laneAngle": -19
                        },
                        {
                          "laneCrownPointRight": -96
                        }
                      ],
                      "dWidth": 242,
                      "dElevation": -18
                    }
                  },
                  {
                    "delta": {
                      "node-XY5": {
                        "x": -3048,
                        "y": 2642
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "safeIsland"
                      ],
                      "disabled": [
                        "adaptiveTimingPresent"
                      ],
                      "enabled": [
                        "taperToCenterLine",
                        "partialCurbIntrusion",
                        "loadingzoneOnRight"
                      ],
                      "data": [
                        {
                          "laneCrownPointLeft": 27
                        },
                        {
                          "laneCrownPointCenter": 96
                        },
                        {
                          "laneCrownPointRight": 90
                        }
                      ],
                      "dWidth": 89,
                      "dElevation": -134
                    }
                  },
                  {
                    "delta": {
                      "node-XY2": {
                        "x": 229,
                        "y": 435
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "stopLine",
                        "roundedCapStyleA",
                        "roundedCapStyleA",
                        "safeIsland"
                      ],
                      "disabled": [
                        "partialCurbIntrusion",
                        "adjacentParkingOnRight",
                        "lowCurbsPresent",
                        "whiteLine"
                      ],
                      "enabled": [
                        "adjacentParkingOnRight",
                        "bikeBoxInFront"
                      ],
                      "data": [
                        {
                          "laneCrownPointLeft": 119
                        },
                        {
                          "laneCrownPointCenter": 43
                        },
                        {
                          "laneCrownPointLeft": 116
                        }
                      ],
                      "dWidth": 164,
                      "dElevation": 18
                    }
                  }
                ]
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 176,
                    "maneuver": "FC30"
                  },
                  "remoteIntersection": {
                    "region": 52717,
                    "id": 15063
                  },
                  "signalGroup": 181,
                  "userClass": 140,
                  "connectionID": 184
                },
                {
                  "connectingLane": {
                    "lane": 227,
                    "maneuver": "C400"
                  },
                  "remoteIntersection": {
                    "region": 20808,
                    "id": 49821
                  },
                  "signalGroup": 205,
                  "userClass": 218,
                  "connectionID": 164
                },
                {
                  "connectingLane": {
                    "lane": 42,
                    "maneuver": "2F00"
                  },
                  "remoteIntersection": {
                    "region": 35331,
                    "id": 48387
                  },
                  "signalGroup": 141,
                  "userClass": 140,
                  "connectionID": 103
                },
                {
                  "connectingLane": {
                    "lane": 93,
                    "maneuver": "F160"
                  },
                  "remoteIntersection": {
                    "region": 31954,
                    "id": 42479
                  },
                  "signalGroup": 249,
                  "userClass": 238,
                  "connectionID": 225
                }
              ],
              "overlays": [
                56,
                37
              ]
            },
            {
              "laneID": 77,
              "name": "QPUCM",
              "ingressApproach": 9,
              "egressApproach": 14,
              "laneAttributes": {
                "directionalUse": "80",
                "sharedWith": "7740",
                "laneType": {
                  "striping": "FA07"
                }
              },
              "maneuvers": "6F70",
              "nodeList": {
                "computed": {
                  "referenceLaneId": 121,
                  "offsetXaxis": {
                    "large": -18104
                  },
                  "offsetYaxis": {
                    "small": 826
                  },
                  "rotateXY": 27247,
                  "scaleXaxis": 132,
                  "scaleYaxis": 553
                }
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 187,
                    "maneuver": "9770"
                  },
                  "remoteIntersection": {
                    "region": 52561,
                    "id": 15198
                  },
                  "signalGroup": 130,
                  "userClass": 20,
                  "connectionID": 99
                },
                {
                  "connectingLane": {
                    "lane": 120,
                    "maneuver": "EAA0"
                  },
                  "remoteIntersection": {
                    "region": 26979,
                    "id": 64450
                  },
                  "signalGroup": 146,
                  "userClass": 182,
                  "connectionID": 53
                }
              ],
              "overlays": [
                224,
                37,
                80,
                94
              ]
            }
          ]
        },
        {
          "name": "LPOTAFVUQNVUSKRLNKBARBCBLPQSGRPBVLKCBELRFKTFBKAFYSXOOVXX",
          "id": {
            "region": 2452,
            "id": 3155
          },
          "revision": 124,
          "refPoint": {
            "lat": 471868940,
            "long": 1694017460,
            "elevation": 11290
          },
          "laneWidth": 18654,
          "speedLimits": [
            {
              "type": "maxSpeedInConstructionZone",
              "speed": 2215
            },
            {
              "type": "vehicleMaxSpeed",
              "speed": 4638
            }
          ],
          "roadLaneSet": [
            {
              "laneID": 138,
              "name": "UOQFWUDBEGAUPTFGGUXPKPFMCLTELQDGUFGMWA",
              "ingressApproach": 10,
              "egressApproach": 5,
              "laneAttributes": {
                "directionalUse": "40",
                "sharedWith": "5F00",
                "laneType": {
                  "vehicle": {
                    "value": "40",
                    "length": 8
                  }
                }
              },
              "maneuvers": "48B0",
              "nodeList": {
                "nodes": [
                  {
                    "delta": {
                      "node-XY3": {
                        "x": -785,
                        "y": -1144
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "roundedCapStyleA",
                        "roundedCapStyleA",
                        "stopLine"
                      ],
                      "disabled": [
                        "timeRestrictionsOnParking"
                      ],
                      "enabled": [
                        "adjacentBikeLaneOnRight",
                        "adjacentParkingOnLeft",
                        "sharedWithTrackedVehicle"
                      ],
                      "data": [
                        {
                          "laneCrownPointLeft": -101
                        },
                        {
                          "pathEndPointAngle": -109
                        },
                        {
                          "laneCrownPointLeft": -2
                        },
                        {
                          "pathEndPointAngle": 148
                        }
                      ],
                      "dWidth": -478,
                      "dElevation": 90
                    }
                  },
                  {
                    "delta": {
                      "node-XY1": {
                        "x": -281,
                        "y": 276
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "reserved"
                      ],
                      "disabled": [
                        "lowCurbsPresent",
                        "lowCurbsPresent",
                        "adjacentBikeLaneOnLeft"
                      ],
                      "enabled": [
                        "reserved",
                        "rfSignalRequestPresent",
                        "safeIsland"
                      ],
                      "data": [
                        {
                          "laneCrownPointLeft": 109
                        }
                      ],
                      "dWidth": -246,
                      "dElevation": 144
                    }
                  },
                  {
                    "delta": {
                      "node-XY6": {
                        "x": -32732,
                        "y": 14726
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "safeIsland"
                      ],
                      "disabled": [
                        "loadingzoneOnLeft",
                        "headInParking",
                        "unEvenPavementPresent",
                        "transitStopInLane"
                      ],
                      "enabled": [
                        "midBlockCurbPresent",
                        "transitStopOnLeft"
                      ],
                      "data": [
                        {
                          "pathEndPointAngle": -90
                        }
                      ],
                      "dWidth": 392,
                      "dElevation": 277
                    }
                  }
                ]
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 224,
                    "maneuver": "98F0"
                  },
                  "remoteIntersection": {
                    "region": 4583,
                    "id": 6490
                  },
                  "signalGroup": 25,
                  "userClass": 140,
                  "connectionID": 39
                },
                {
                  "connectingLane": {
                    "lane": 135,
                    "maneuver": "A500"
                  },
                  "remoteIntersection": {
                    "region": 25373,
                    "id": 15256
                  },
                  "signalGroup": 49,
                  "userClass": 35,
                  "connectionID": 21
                },
                {
                  "connectingLane": {
                    "lane": 167,
                    "maneuver": "52E0"
                  },
                  "remoteIntersection": {
                    "region": 29521,
                    "id": 12547
                  },
                  "signalGroup": 93,
                  "userClass": 171,
                  "connectionID": 116
                },
                {
                  "connectingLane": {
                    "lane": 68,
                    "maneuver": "4E40"
                  },
                  "remoteIntersection": {
                    "region": 20788,
                    "id": 44045
                  },
                  "signalGroup": 66,
                  "userClass": 212,
                  "connectionID": 163
                }
              ],
              "overlays": [
                225,
                232
              ]
            },
            {
              "laneID": 159,
              "name": "PVQVIDWCSSSNIBNMMEKTPMUAISTQMQIIPQNWQUJAS",
              "ingressApproach": 10,
              "egressApproach": 8,
              "laneAttributes": {
                "directionalUse": "C0",
                "sharedWith": "8D00",
                "laneType": {
                  "median": "5401"
                }
              },
              "maneuvers": "7D40",
              "nodeList": {
                "nodes": [
                  {
                    "delta": {
                      "node-XY3": {
                        "x": 297,
                        "y": 1119
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "stopLine",
                        "downstreamStopLine",
                        "mergePoint",
                        "downstreamStopLine"
                      ],
                      "disabled": [
                        "adaptiveTimingPresent",
                        "turnOutPointOnRight",
                        "bikeBoxInFront"
                      ],
                      "enabled": [
                        "taperToCenterLine",
                        "taperToRight",
                        "unEvenPavementPresent"
                      ],
                      "data": [
                        {
                          "speedLimits": [
                            {
                              "type": "maxSpeedInSchoolZone",
                              "speed": 6129
                            },
                            {
                              "type": "maxSpeedInConstructionZone",
                              "speed": 4670
                            }
                          ]
                        }
                      ],
                      "dWidth": -480,
                      "dElevation": 396
                    }
                  },
                  {
                    "delta": {
                      "node-LatLon": {
                        "lon": 1299083759,
                        "lat": 123359613
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "roundedCapStyleA",
                        "divergePoint"
                      ],
                      "disabled": [
                        "adjacentParkingOnLeft"
                      ],
                      "enabled": [
                        "partialCurbIntrusion",
                        "taperToLeft",
                        "reserved"
                      ],
                      "data": [
                        {
                          "laneCrownPointRight": -33
                        },
                        {
                          "laneCrownPointRight": -97
                        },
                        {
                          "pathEndPointAngle": -34
                        }
                      ],
                      "dWidth": -392,
                      "dElevation": 406
                    }
                  },
                  {
                    "delta": {
                      "node-XY3": {
                        "x": -915,
                        "y": 896
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "stopLine"
                      ],
                      "disabled": [
                        "rfSignalRequestPresent",
                        "taperToLeft",
                        "sharedWithTrackedVehicle",
                        "loadingzoneOnRight"
                      ],
                      "enabled": [
                        "safeIsland",
                        "partialCurbIntrusion",
                        "mergingLaneLeft",
                        "adjacentParkingOnLeft"
                      ],
                      "data": [
                        {
                          "laneCrownPointRight": -77
                        }
                      ],
                      "dWidth": 424,
                      "dElevation": -238
                    }
                  }
                ]
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 87,
                    "maneuver": "0080"
                  },
                  "remoteIntersection": {
                    "region": 43272,
                    "id": 27486
                  },
                  "signalGroup": 168,
                  "userClass": 216,
                  "connectionID": 125
                }
              ],
              "overlays": [
                38,
                113,
                190
              ]
            },
            {
              "laneID": 26,
              "name": "XOLFDXONITUFDQTXDRFM",
              "ingressApproach": 6,
              "egressApproach": 4,
              "laneAttributes": {
                "directionalUse": "C0",
                "sharedWith": "D700",
                "laneType": {
                  "sidewalk": "06DF"
                }
              },
              "maneuvers": "5690",
              "nodeList": {
                "nodes": [
                  {
                    "delta": {
                      "node-XY4": {
                        "x": 535,
                        "y": -40
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "hydrantPresent",
                        "downstreamStopLine",
                        "safeIsland",
                        "curbPresentAtStepOff"
                      ],
                      "disabled": [
                        "lowCurbsPresent"
                      ],
                      "enabled": [
                        "rfSignalRequestPresent",
                        "adaptiveTimingPresent",
                        "taperToRight"
                      ],
                      "data": [
                        {
                          "laneCrownPointRight": 6
                        },
                        {
                          "speedLimits": [
                            {
                              "type": "vehiclesWithTrailersMinSpeed",
                              "speed": 4697
                            },
                            {
                              "type": "vehicleNightMaxSpeed",
                              "speed": 6167
                            }
                          ]
                        }
                      ],
                      "dWidth": 149,
                      "dElevation": -137
                    }
                  },
                  {
                    "delta": {
                      "node-XY5": {
                        "x": 3559,
                        "y": -6656
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "closedToTraffic",
                        "closedToTraffic",
                        "divergePoint"
                      ],
                      "disabled": [
                        "curbOnLeft",
                        "safeIsland",
                        "doNotBlock"
                      ],
                      "enabled": [
                        "doNotBlock"
                      ],
                      "data": [
                        {
                          "speedLimits": [
                            {
                              "type": "vehiclesWithTrailersMaxSpeed",
                              "speed": 5955
                            },
                            {
                              "type": "truckMinSpeed",
                              "speed": 4925
                            }
                          ]
                        }
                      ],
                      "dWidth": 29,
                      "dElevation": -197
                    }
                  }
                ]
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 37,
                    "maneuver": "A110"
                  },
                  "remoteIntersection": {
                    "region": 58175,
                    "id": 38879
                  },
                  "signalGroup": 88,
                  "userClass": 31,
                  "connectionID": 60
                },
                {
                  "connectingLane": {
                    "lane": 201,
                    "maneuver": "7680"
                  },
                  "remoteIntersection": {
                    "region": 13762,
                    "id": 26774
                  },
                  "signalGroup": 28,
                  "userClass": 108,
                  "connectionID": 249
                }
              ],
              "overlays": [
                86,
                21
              ]
            },
            {
              "laneID": 244,
              "name": "AFCHAPMDAYIISDWWOWTKCFP",
              "ingressApproach": 2,
              "egressApproach": 8,
              "laneAttributes": {
                "directionalUse": "C0",
                "sharedWith": "A900",
                "laneType": {
                  "median": "A9C1"
                }
              },
              "maneuvers": "B700",
              "nodeList": {
                "computed": {
                  "referenceLaneId": 170,
                  "offsetXaxis": {
                    "small": 533
                  },
                  "offsetYaxis": {
                    "small": -255
                  },
                  "rotateXY": 23698,
                  "scaleXaxis": -875,
                  "scaleYaxis": 730
                }
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 76,
                    "maneuver": "5D90"
                  },
                  "remoteIntersection": {
                    "region": 2225,
                    "id": 20353
                  },
                  "signalGroup": 90,
                  "userClass": 41,
                  "connectionID": 38
                },
                {
                  "connectingLane": {
                    "lane": 253,
                    "maneuver": "5B70"
                  },
                  "remoteIntersection": {
                    "region": 575,
                    "id": 47122
                  },
                  "signalGroup": 76,
                  "userClass": 175,
                  "connectionID": 29
                },
                {
                  "connectingLane": {
                    "lane": 131,
                    "maneuver": "A8A0"
                  },
                  "remoteIntersection": {
                    "region": 15311,
                    "id": 50909
                  },
                  "signalGroup": 67,
                  "userClass": 113,
                  "connectionID": 42
                },
                {
                  "connectingLane": {
                    "lane": 24,
                    "maneuver": "D5A0"
                  },
                  "remoteIntersection": {
                    "region": 18437,
                    "id": 32193
                  },
                  "signalGroup": 97,
                  "userClass": 194,
                  "connectionID": 134
                }
              ],
              "overlays": [
                6,
                195,
                219
              ]
            }
          ]
        },
        {
          "name": "CIRSCYU",
          "id": {
            "region": 50924,
            "id": 30160
          },
          "revision": 42,
          "refPoint": {
            "lat": -135402320,
            "long": 715075671,
            "elevation": 22560
          },
          "laneWidth": 29125,
          "speedLimits": [
            {
              "type": "truckMaxSpeed",
              "speed": 6305
            },
            {
              "type": "vehicleNightMaxSpeed",
              "speed": 6498
            }
          ],
          "roadLaneSet": [
            {
              "laneID": 103,
              "name": "BRGVLQKUNWDWRJRJ",
              "ingressApproach": 11,
              "egressApproach": 4,
              "laneAttributes": {
                "directionalUse": "00",
                "sharedWith": "33C0",
                "laneType": {
                  "crosswalk": "ED8B"
                }
              },
              "maneuvers": "5BB0",
              "nodeList": {
                "nodes": [
                  {
                    "delta": {
                      "node-XY6": {
                        "x": 10075,
                        "y": -2604
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "stopLine"
                      ],
                      "disabled": [
                        "turnOutPointOnLeft",
                        "bikeBoxInFront",
                        "midBlockCurbPresent",
                        "curbOnRight"
                      ],
                      "enabled": [
                        "costToPark"
                      ],
                      "data": [
                        {
                          "laneAngle": -44
                        },
                        {
                          "speedLimits": [
                            {
                              "type": "vehicleMinSpeed",
                              "speed": 1275
                            },
                            {
                              "type": "vehiclesWithTrailersMaxSpeed",
                              "speed": 7594
                            },
                            {
                              "type": "truckMinSpeed",
                              "speed": 6705
                            },
                            {
                              "type": "vehicleMaxSpeed",
                              "speed": 144
                            }
                          ]
                        },
                        {
                          "laneAngle": -128
                        },
                        {
                          "laneCrownPointRight": -50
                        }
                      ],
                      "dWidth": 411,
                      "dElevation": 414
                    }
                  },
                  {
                    "delta": {
                      "node-XY6": {
                        "x": -16326,
                        "y": -19209
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "curbPresentAtStepOff",
                        "divergePoint",
                        "downstreamStartNode"
                      ],
                      "disabled": [
                        "adjacentBikeLaneOnRight",
                        "adjacentParkingOnRight"
                      ],
                      "enabled": [
                        "taperToLeft",
                        "mergingLaneRight",
                        "taperToRight"
                      ],
                      "data": [
                        {
                          "pathEndPointAngle": -88
                        }
                      ],
                      "dWidth": 208,
                      "dElevation": 325
                    }
                  },
                  {
                    "delta": {
                      "node-XY1": {
                        "x": -101,
                        "y": -199
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "reserved"
                      ],
                      "disabled": [
                        "sharedBikeLane",
                        "mergingLaneLeft",
                        "rumbleStripPresent",
                        "lowCurbsPresent"
                      ],
                      "enabled": [
                        "costToPark",
                        "mergingLaneLeft",
                        "rfSignalRequestPresent",
                        "curbOnRight"
                      ],
                      "data": [
                        {
                          "pathEndPointAngle": 106
                        },
                        {
                          "laneAngle": 36
                        }
                      ],
                      "dWidth": 158,
                      "dElevation": 404
                    }
                  },
                  {
                    "delta": {
                      "node-XY3": {
                        "x": 176,
                        "y": 1769
                      }
                    },
                    "attributes": {
                      "localNode": [
                        "divergePoint"
                      ],
                      "disabled": [
                        "adjacentParkingOnRight",
                        "curbOnLeft"
                      ],
                      "enabled": [
                        "loadingzoneOnRight",
                        "mergingLaneRight",
                        "reserved",
                        "sharedBikeLane"
                      ],
                      "data": [
                        {
                          "pathEndPointAngle": -11
                        },
                        {
                          "laneAngle": -125
                        },
                        {
                          "laneCrownPointCenter": 92
                        },
                        {
                          "speedLimits": [
                            {
                              "type": "vehicleNightMaxSpeed",
                              "speed": 8034
                            }
                          ]
                        }
                      ],
                      "dWidth": 304,
                      "dElevation": -411
                    }
                  }
                ]
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 163,
                    "maneuver": "D0B0"
                  },
                  "remoteIntersection": {
                    "region": 15456,
                    "id": 16885
                  },
                  "signalGroup": 61,
                  "userClass": 220,
                  "connectionID": 17
                },
                {
                  "connectingLane": {
                    "lane": 50,
                    "maneuver": "F450"
                  },
                  "remoteIntersection": {
                    "region": 36901,
                    "id": 54367
                  },
                  "signalGroup": 109,
                  "userClass": 73,
                  "connectionID": 189
                },
                {
                  "connectingLane": {
                    "lane": 216,
                    "maneuver": "5540"
                  },
                  "remoteIntersection": {
                    "region": 59313,
                    "id": 12281
                  },
                  "signalGroup": 213,
                  "userClass": 227,
                  "connectionID": 183
                }
              ],
              "overlays": [
                115,
                230,
                216,
                160
              ]
            },
            {
              "laneID": 98,
              "name": "BNC",
              "ingressApproach": 2,
              "egressApproach": 1,
              "laneAttributes": {
                "directionalUse": "C0",
                "sharedWith": "9A40",
                "laneType": {
                  "sidewalk": "C684"
                }
              },
              "maneuvers": "4600",
              "nodeList": {
                "computed": {
                  "referenceLaneId": 181,
                  "offsetXaxis": {
                    "small": -1563
                  },
                  "offsetYaxis": {
                    "small": -559
                  },
                  "rotateXY": 16756,
                  "scaleXaxis": -1465,
                  "scaleYaxis": -2041
                }
              },
              "connectsTo": [
                {
                  "connectingLane": {
                    "lane": 140,
                    "maneuver": "D360"
                  },
                  "remoteIntersection": {
                    "region": 35146,
                    "id": 31551
                  },
                  "signalGroup": 30,
                  "userClass": 220,
                  "connectionID": 145
                }
              ],
              "overlays": [
                225,
                171,
                23
              ]
            }
          ]
        }
      ],
      "dataParameters": {
        
      },
      "restrictionList": [
        {
          "id": 167,
          "users": [
            {
              "basicType": "equippedTaxis"
            },
            {
              "basicType": "equippedTaxis"
            }
          ]
        },
        {
          "id": 130,
          "users": [
            {
              "basicType": "equippedTransit"
            },
            {
              "basicType": "wheelchairUsers"
            }
          ]
        }
      ]
    }
  }
})";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_TRUE(result.valid) << (result.errors.empty() ? "" : result.errors[0]);
  }

  TEST(MapFieldValidationTest, MissingMessageIdFails)
  {
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

  TEST(MapFieldValidationTest, MissingMsgIssueRevisionFails)
  {
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

  TEST(MapFieldValidationTest, MissingIntersectionIdFails)
  {
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

  TEST(MapFieldValidationTest, MissingRefPointFails)
  {
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

  TEST(MapFieldValidationTest, MissingRefPointLatFails)
  {
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

  TEST(MapFieldValidationTest, MissingLaneSetFails)
  {
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

  TEST(MapFieldValidationTest, MissingLaneAttributesFails)
  {
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

  TEST(MapFieldValidationTest, MissingNodeListFails)
  {
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

  TEST(MapFieldValidationTest, BitStringFieldsPreservedAsStrings)
  {
    std::string json = R"({
        "messageId": 18,
        "value": {
            "MapData": {
                "msgIssueRevision": 1,
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "refPoint": {"lat": 389519791, "long": -771483512, "elevation": 100},
                    "laneWidth": 366,
                    "speedLimits": [{"type": "vehicleMaxSpeed", "speed": 500}],
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
                        ]},
                        "connectsTo": [{
                            "connectingLane": {"lane": 2, "maneuver": "0400"},
                            "signalGroup": 1
                        }]
                    }]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_TRUE(result.valid) << (result.errors.empty() ? "" : result.errors[0]);
  }

  TEST(MapFieldValidationTest, MultipleLanesPasses)
  {
    std::string json = R"({
        "messageId": 18,
        "value": {
            "MapData": {
                "msgIssueRevision": 1,
                "intersections": [{
                    "id": {"id": 12111},
                    "revision": 0,
                    "refPoint": {"lat": 389519791, "long": -771483512, "elevation": 100},
                    "laneWidth": 366,
                    "speedLimits": [{"type": "vehicleMaxSpeed", "speed": 500}],
                    "laneSet": [
                        {
                            "laneID": 1,
                            "ingressApproach": 1,
                            "laneAttributes": {
                                "directionalUse": "C0",
                                "sharedWith": "0000",
                                "laneType": {"vehicle": "00"}
                            },
                            "maneuvers": "0400",
                            "nodeList": {"nodes": [
                                {"delta": {"node-XY1": {"x": 10, "y": 20}}},
                                {"delta": {"node-XY1": {"x": 30, "y": 40}}}
                            ]},
                            "connectsTo": [{
                                "connectingLane": {"lane": 2, "maneuver": "0400"},
                                "signalGroup": 1
                            }]
                        },
                        {
                            "laneID": 2,
                            "egressApproach": 1,
                            "laneAttributes": {
                                "directionalUse": "40",
                                "sharedWith": "0000",
                                "laneType": {"vehicle": "00"}
                            },
                            "maneuvers": "0800",
                            "nodeList": {"nodes": [
                                {"delta": {"node-XY1": {"x": -10, "y": -20}}},
                                {"delta": {"node-XY1": {"x": -30, "y": -40}}}
                            ]},
                            "connectsTo": [{
                                "connectingLane": {"lane": 1, "maneuver": "0800"},
                                "signalGroup": 2
                            }]
                        }
                    ]
                }]
            }
        }
    })";
    auto result = validateJsonAgainstSchemaFile(json, MAP_SCHEMA_PATH);
    EXPECT_TRUE(result.valid) << (result.errors.empty() ? "" : result.errors[0]);
  }

  rapidjson::Document parseJson(const std::string &json)
  {
    rapidjson::Document doc;
    doc.Parse(json.c_str());
    return doc;
  }
  // ---- SPaT Revision Counter Tests ----

  // SPaT content changed → revision must increment
  TEST(SpatRevisionCounterTest, ContentChangedRevisionIncremented)
  {
    RevisionCounterValidator validator;

    // First message: revision 0, signalGroup 2
    auto msg1 = parseJson(R"({
        "messageId": 19,
        "value": {"SPAT": {"intersections": [{
            "id": {"id": 12111},
            "revision": 0,
            "status": "0000",
            "states": [{"signalGroup": 2, "state-time-speed": [
                {"eventState": "stop-And-Remain", "timing": {"minEndTime": 22120}}
            ]}]
        }]}}
    })");

    // Second message: revision 1, signalGroup changed to 4 (content changed)
    auto msg2 = parseJson(R"({
        "messageId": 19,
        "value": {"SPAT": {"intersections": [{
            "id": {"id": 12111},
            "revision": 1,
            "status": "0000",
            "states": [{"signalGroup": 4, "state-time-speed": [
                {"eventState": "protected-Movement-Allowed", "timing": {"minEndTime": 22180}}
            ]}]
        }]}}
    })");

    auto result1 = validator.validateSpatRevision(msg1);
    EXPECT_TRUE(result1.valid); // First message — no previous to compare

    auto result2 = validator.validateSpatRevision(msg2);
    EXPECT_TRUE(result2.valid) << (result2.violations.empty() ? "" : result2.violations[0]);
  }

  // SPaT content changed but revision NOT incremented, violation
  TEST(SpatRevisionCounterTest, ContentChangedRevisionNotIncremented)
  {
    RevisionCounterValidator validator;

    auto msg1 = parseJson(R"({
        "messageId": 19,
        "value": {"SPAT": {"intersections": [{
            "id": {"id": 12111},
            "revision": 5,
            "status": "0000",
            "states": [{"signalGroup": 2, "state-time-speed": [
                {"eventState": "stop-And-Remain", "timing": {"minEndTime": 22120}}
            ]}]
        }]}}
    })");

    // Content changed (different eventState) but revision stays at 5
    auto msg2 = parseJson(R"({
        "messageId": 19,
        "value": {"SPAT": {"intersections": [{
            "id": {"id": 12111},
            "revision": 5,
            "status": "0000",
            "states": [{"signalGroup": 2, "state-time-speed": [
                {"eventState": "protected-Movement-Allowed", "timing": {"minEndTime": 22180}}
            ]}]
        }]}}
    })");

    validator.validateSpatRevision(msg1);
    auto result = validator.validateSpatRevision(msg2);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(1u, result.violations.size());
    EXPECT_NE(std::string::npos, result.violations[0].find("content changed but revision counter did not increment"));
  }

  // SPaT content unchanged, revision must stay the same
  TEST(SpatRevisionCounterTest, ContentUnchangedRevisionSame)
  {
    RevisionCounterValidator validator;

    auto msg = parseJson(R"({
        "messageId": 19,
        "value": {"SPAT": {"intersections": [{
            "id": {"id": 12111},
            "revision": 3,
            "status": "0000",
            "states": [{"signalGroup": 2, "state-time-speed": [
                {"eventState": "stop-And-Remain", "timing": {"minEndTime": 22120}}
            ]}]
        }]}}
    })");

    validator.validateSpatRevision(msg);
    auto result = validator.validateSpatRevision(msg); // Same message again

    EXPECT_TRUE(result.valid) << (result.violations.empty() ? "" : result.violations[0]);
  }

  // SPaT content unchanged but revision incremented, violation
  TEST(SpatRevisionCounterTest, ContentUnchangedRevisionIncremented)
  {
    RevisionCounterValidator validator;

    auto msg1 = parseJson(R"({
        "messageId": 19,
        "value": {"SPAT": {"intersections": [{
            "id": {"id": 12111},
            "revision": 3,
            "status": "0000",
            "states": [{"signalGroup": 2, "state-time-speed": [
                {"eventState": "stop-And-Remain", "timing": {"minEndTime": 22120}}
            ]}]
        }]}}
    })");

    // Same content but revision incremented from 3 to 4
    auto msg2 = parseJson(R"({
        "messageId": 19,
        "value": {"SPAT": {"intersections": [{
            "id": {"id": 12111},
            "revision": 4,
            "status": "0000",
            "states": [{"signalGroup": 2, "state-time-speed": [
                {"eventState": "stop-And-Remain", "timing": {"minEndTime": 22120}}
            ]}]
        }]}}  
    })");

    validator.validateSpatRevision(msg1);
    auto result = validator.validateSpatRevision(msg2);

    EXPECT_FALSE(result.valid);
    EXPECT_EQ(1u, result.violations.size());
    EXPECT_NE(std::string::npos, result.violations[0].find("content unchanged but revision counter incremented"));
  }

  // SPaT only timestamp changed, revision must stay the same (no violation)
  TEST(SpatRevisionCounterTest, OnlyTimestampChangedRevisionSame)
  {
    RevisionCounterValidator validator;

    auto msg1 = parseJson(R"({
        "messageId": 19,
        "value": {"SPAT": {"timeStamp": 35176, "intersections": [{
            "id": {"id": 12111},
            "revision": 3,
            "status": "0000",
            "timeStamp": 100,
            "states": [{"signalGroup": 2, "state-time-speed": [
                {"eventState": "stop-And-Remain", "timing": {"minEndTime": 22120}}
            ]}]
        }]}}
    })");

    // Only timeStamp changed — content is the same
    auto msg2 = parseJson(R"({
        "messageId": 19,
        "value": {"SPAT": {"timeStamp": 35176, "intersections": [{
            "id": {"id": 12111},
            "revision": 3,
            "status": "0000",
            "timeStamp": 200,
            "states": [{"signalGroup": 2, "state-time-speed": [
                {"eventState": "stop-And-Remain", "timing": {"minEndTime": 22120}}
            ]}]
        }]}}
    })");

    validator.validateSpatRevision(msg1);
    auto result = validator.validateSpatRevision(msg2);

    EXPECT_TRUE(result.valid) << (result.violations.empty() ? "" : result.violations[0]);
  }

  // ---- MAP Revision Counter Tests ----

  // MAP content changed, msgIssueRevision must increment
  TEST(MapRevisionCounterTest, ContentChangedMsgRevisionIncremented)
  {
    RevisionCounterValidator validator;

    auto msg1 = parseJson(R"({
        "messageId": 18,
        "value": {"MapData": {
            "msgIssueRevision": 1,
            "intersections": [{
                "id": {"id": 9001},
                "revision": 0,
                "refPoint": {"lat": 389519791, "long": -771483512},
                "laneSet": [{"laneID": 1, "laneAttributes": {
                    "directionalUse": "C0", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 100, "y": 200}}},
                    {"delta": {"node-XY1": {"x": 150, "y": 250}}}
                ]}}]
            }]
        }}
    })");

    // laneID changed from 1 to 2, revisions incremented
    auto msg2 = parseJson(R"({
        "messageId": 18,
        "value": {"MapData": {
            "msgIssueRevision": 2,
            "intersections": [{
                "id": {"id": 9001},
                "revision": 1,
                "refPoint": {"lat": 389519791, "long": -771483512},
                "laneSet": [{"laneID": 2, "laneAttributes": {
                    "directionalUse": "C0", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 100, "y": 200}}},
                    {"delta": {"node-XY1": {"x": 150, "y": 250}}}
                ]}}]
            }]
        }}
    })");

    validator.validateMapRevision(msg1);
    auto result = validator.validateMapRevision(msg2);
    EXPECT_TRUE(result.valid) << (result.violations.empty() ? "" : result.violations[0]);
  }

  // MAP content changed but msgIssueRevision NOT incremented
  TEST(MapRevisionCounterTest, ContentChangedMsgRevisionNotIncremented)
  {
    RevisionCounterValidator validator;

    auto msg1 = parseJson(R"({
        "messageId": 18,
        "value": {"MapData": {
            "msgIssueRevision": 5,
            "intersections": [{
                "id": {"id": 9001},
                "revision": 0,
                "refPoint": {"lat": 389519791, "long": -771483512},
                "laneSet": [{"laneID": 1, "laneAttributes": {
                    "directionalUse": "C0", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 100, "y": 200}}},
                    {"delta": {"node-XY1": {"x": 150, "y": 250}}}
                ]}}]
            }]
        }}
    })");

    // Content changed but msgIssueRevision stays at 5
    auto msg2 = parseJson(R"({
        "messageId": 18,
        "value": {"MapData": {
            "msgIssueRevision": 5,
            "intersections": [{
                "id": {"id": 9001},
                "revision": 0,
                "refPoint": {"lat": 389519791, "long": -771483512},
                "laneSet": [{"laneID": 99, "laneAttributes": {
                    "directionalUse": "C0", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 100, "y": 200}}},
                    {"delta": {"node-XY1": {"x": 150, "y": 250}}}
                ]}}]
            }]
        }}
    })");

    validator.validateMapRevision(msg1);
    auto result = validator.validateMapRevision(msg2);

    EXPECT_FALSE(result.valid);
    EXPECT_TRUE(result.violations.size() >= 1);

    bool foundMsgRevisionViolation = false;
    for (const auto &v : result.violations)
    {
      if (v.find("msgIssueRevision did not increment") != std::string::npos)
      {
        foundMsgRevisionViolation = true;
      }
    }
    EXPECT_TRUE(foundMsgRevisionViolation);
  }

  // MAP content unchanged, msgIssueRevision must stay the same
  TEST(MapRevisionCounterTest, ContentUnchangedMsgRevisionSame)
  {
    RevisionCounterValidator validator;

    auto msg = parseJson(R"({
        "messageId": 18,
        "value": {"MapData": {
            "msgIssueRevision": 3,
            "intersections": [{
                "id": {"id": 9001},
                "revision": 0,
                "refPoint": {"lat": 389519791, "long": -771483512},
                "laneSet": [{"laneID": 1, "laneAttributes": {
                    "directionalUse": "C0", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 100, "y": 200}}},
                    {"delta": {"node-XY1": {"x": 150, "y": 250}}}
                ]}}]
            }]
        }}
    })");

    validator.validateMapRevision(msg);
    auto result = validator.validateMapRevision(msg); // Same message
    EXPECT_TRUE(result.valid) << (result.violations.empty() ? "" : result.violations[0]);
  }

  // MAP content unchanged but msgIssueRevision incremented, violation
  TEST(MapRevisionCounterTest, ContentUnchangedMsgRevisionIncremented)
  {
    RevisionCounterValidator validator;

    auto msg1 = parseJson(R"({
        "messageId": 18,
        "value": {"MapData": {
            "msgIssueRevision": 3,
            "intersections": [{
                "id": {"id": 9001},
                "revision": 0,
                "refPoint": {"lat": 389519791, "long": -771483512},
                "laneSet": [{"laneID": 1, "laneAttributes": {
                    "directionalUse": "C0", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 100, "y": 200}}},
                    {"delta": {"node-XY1": {"x": 150, "y": 250}}}
                ]}}]
            }]
        }}
    })");

    // Same content but msgIssueRevision changed from 3 to 4
    auto msg2 = parseJson(R"({
        "messageId": 18,
        "value": {"MapData": {
            "msgIssueRevision": 4,
            "intersections": [{
                "id": {"id": 9001},
                "revision": 0,
                "refPoint": {"lat": 389519791, "long": -771483512},
                "laneSet": [{"laneID": 1, "laneAttributes": {
                    "directionalUse": "C0", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 100, "y": 200}}},
                    {"delta": {"node-XY1": {"x": 150, "y": 250}}}
                ]}}]
            }]
        }}
    })");

    validator.validateMapRevision(msg1);
    auto result = validator.validateMapRevision(msg2);

    EXPECT_FALSE(result.valid);
    bool foundViolation = false;
    for (const auto &v : result.violations)
    {
      if (v.find("msgIssueRevision incremented") != std::string::npos)
      {
        foundViolation = true;
      }
    }
    EXPECT_TRUE(foundViolation);
  }

  // MAP intersection content changed, intersection revision must increment
  TEST(MapRevisionCounterTest, IntersectionChangedRevisionIncremented)
  {
    RevisionCounterValidator validator;

    auto msg1 = parseJson(R"({
        "messageId": 18,
        "value": {"MapData": {
            "msgIssueRevision": 1,
            "intersections": [
                {"id": {"id": 9001}, "revision": 0,
                 "refPoint": {"lat": 100, "long": 200},
                 "laneSet": [{"laneID": 1, "laneAttributes": {
                    "directionalUse": "C0", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                 }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 10, "y": 20}}},
                    {"delta": {"node-XY1": {"x": 30, "y": 40}}}
                 ]}}]},
                {"id": {"id": 9002}, "revision": 0,
                 "refPoint": {"lat": 300, "long": 400},
                 "laneSet": [{"laneID": 5, "laneAttributes": {
                    "directionalUse": "40", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                 }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 50, "y": 60}}},
                    {"delta": {"node-XY1": {"x": 70, "y": 80}}}
                 ]}}]}
            ]
        }}
    })");

    // Intersection 9001 changed, revision incremented.
    // Intersection 9002 unchanged, revision stays at 0.
    // msgIssueRevision incremented because 9001 changed.
    auto msg2 = parseJson(R"({
        "messageId": 18,
        "value": {"MapData": {
            "msgIssueRevision": 2,
            "intersections": [
                {"id": {"id": 9001}, "revision": 1,
                 "refPoint": {"lat": 100, "long": 200},
                 "laneSet": [{"laneID": 2, "laneAttributes": {
                    "directionalUse": "C0", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                 }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 10, "y": 20}}},
                    {"delta": {"node-XY1": {"x": 30, "y": 40}}}
                 ]}}]},
                {"id": {"id": 9002}, "revision": 0,
                 "refPoint": {"lat": 300, "long": 400},
                 "laneSet": [{"laneID": 5, "laneAttributes": {
                    "directionalUse": "40", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                 }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 50, "y": 60}}},
                    {"delta": {"node-XY1": {"x": 70, "y": 80}}}
                 ]}}]}
            ]
        }}
    })");

    validator.validateMapRevision(msg1);
    auto result = validator.validateMapRevision(msg2);
    EXPECT_TRUE(result.valid) << (result.violations.empty() ? "" : result.violations[0]);
  }

  // MAP intersection changed but intersection revision NOT incremented, violation
  TEST(MapRevisionCounterTest, IntersectionChangedRevisionNotIncremented)
  {
    RevisionCounterValidator validator;

    auto msg1 = parseJson(R"({
        "messageId": 18,
        "value": {"MapData": {
            "msgIssueRevision": 1,
            "intersections": [{
                "id": {"id": 9001},
                "revision": 0,
                "refPoint": {"lat": 100, "long": 200},
                "laneSet": [{"laneID": 1, "laneAttributes": {
                    "directionalUse": "C0", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 10, "y": 20}}},
                    {"delta": {"node-XY1": {"x": 30, "y": 40}}}
                ]}}]
            }]
        }}
    })");

    // Content changed but intersection revision stays at 0
    auto msg2 = parseJson(R"({
        "messageId": 18,
        "value": {"MapData": {
            "msgIssueRevision": 2,
            "intersections": [{
                "id": {"id": 9001},
                "revision": 0,
                "refPoint": {"lat": 999, "long": 888},
                "laneSet": [{"laneID": 1, "laneAttributes": {
                    "directionalUse": "C0", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 10, "y": 20}}},
                    {"delta": {"node-XY1": {"x": 30, "y": 40}}}
                ]}}]
            }]
        }}
    })");

    validator.validateMapRevision(msg1);
    auto result = validator.validateMapRevision(msg2);

    EXPECT_FALSE(result.valid);
    bool foundIntersectionViolation = false;
    for (const auto &v : result.violations)
    {
      if (v.find("intersection 9001") != std::string::npos &&
          v.find("content changed but revision counter did not increment") != std::string::npos)
      {
        foundIntersectionViolation = true;
      }
    }
    EXPECT_TRUE(foundIntersectionViolation);
  }

  // First message — no previous state, should always pass
  TEST(SpatRevisionCounterTest, FirstMessageAlwaysPasses)
  {
    RevisionCounterValidator validator;

    auto msg = parseJson(R"({
        "messageId": 19,
        "value": {"SPAT": {"intersections": [{
            "id": {"id": 12111},
            "revision": 0,
            "status": "0000",
            "states": [{"signalGroup": 2, "state-time-speed": [
                {"eventState": "stop-And-Remain", "timing": {"minEndTime": 22120}}
            ]}]
        }]}}
    })");

    auto result = validator.validateSpatRevision(msg);
    EXPECT_TRUE(result.valid);
    EXPECT_TRUE(result.violations.empty());
  }

  TEST(MapRevisionCounterTest, FirstMessageAlwaysPasses)
  {
    RevisionCounterValidator validator;

    auto msg = parseJson(R"({
        "messageId": 18,
        "value": {"MapData": {
            "msgIssueRevision": 1,
            "intersections": [{
                "id": {"id": 9001},
                "revision": 0,
                "refPoint": {"lat": 100, "long": 200},
                "laneSet": [{"laneID": 1, "laneAttributes": {
                    "directionalUse": "C0", "sharedWith": "0000",
                    "laneType": {"vehicle": "00"}
                }, "nodeList": {"nodes": [
                    {"delta": {"node-XY1": {"x": 10, "y": 20}}},
                    {"delta": {"node-XY1": {"x": 30, "y": 40}}}
                ]}}]
            }]
        }}
    })");

    auto result = validator.validateMapRevision(msg);
    EXPECT_TRUE(result.valid);
    EXPECT_TRUE(result.violations.empty());
  }

  // Missing intersections — should pass without violations
  TEST(SpatRevisionCounterTest, MissingIntersectionsNoViolation)
  {
    RevisionCounterValidator validator;
    auto msg = parseJson(R"({"messageId": 19, "value": {"SPAT": {}}})");
    auto result = validator.validateSpatRevision(msg);
    EXPECT_TRUE(result.valid);
  }

  TEST(MapRevisionCounterTest, MissingIntersectionsNoViolation)
  {
    RevisionCounterValidator validator;
    auto msg = parseJson(R"({"messageId": 18, "value": {"MapData": {"msgIssueRevision": 1}}})");
    auto result = validator.validateMapRevision(msg);
    EXPECT_TRUE(result.valid);
  }

  TEST(SpatRevisionCounterTest, JSONParseFailure)
  {
    RevisionCounterValidator validator;
    auto invalidJson = parseJson(R"({"messageId": 19, "value": {"SPAT": {)"); // Malformed JSON
    auto result = validator.validateSpatRevision(invalidJson);
    EXPECT_FALSE(result.valid);
    EXPECT_TRUE(result.violations.size() >= 1);
    EXPECT_NE(std::string::npos, result.violations[0].find("Failed to parse SPaT JSON"));
  }

  TEST(MapRevisionCounterTest, JSONParseFailure)
  {
    RevisionCounterValidator validator;
    auto invalidJson = parseJson(R"({"messageId": 19, "value": {"MAP": {)"); // Malformed JSON
    auto result = validator.validateMapRevision(invalidJson);
    EXPECT_FALSE(result.valid);
    EXPECT_TRUE(result.violations.size() >= 1);
    EXPECT_NE(std::string::npos, result.violations[0].find("Failed to parse MAP JSON"));
  }

  // Frequency Throttling Test

  IntersectionChangeInfo makeChange(int id, bool contentChanged)
  {
    IntersectionChangeInfo c;
    c.id = id;
    c.contentChanged = contentChanged;
    return c;
  }

  TEST(ShouldForwardTest, FirstMessageForwards)
  {
    RevisionCounterResult r;
    r.comparisonPerformed = false;
    EXPECT_TRUE(planForwarding(r));
  }

  TEST(ShouldForwardTest, NoContentChangeDoesNotForward)
  {
    RevisionCounterResult r;
    r.comparisonPerformed = true;
    r.intersectionChanges.push_back(makeChange(1, false));
    EXPECT_FALSE(planForwarding(r));
  }

  TEST(ShouldForwardTest, ContentChangedForwards)
  {
    RevisionCounterResult r;
    r.comparisonPerformed = true;
    r.intersectionChanges.push_back(makeChange(1, true));
    EXPECT_TRUE(planForwarding(r));
  }

  TEST(ShouldForwardTest, AnyIntersectionChangedForwards)
  {
    RevisionCounterResult r;
    r.comparisonPerformed = true;
    r.intersectionChanges.push_back(makeChange(59963, false));
    r.intersectionChanges.push_back(makeChange(18364, true));
    EXPECT_TRUE(planForwarding(r));
  }

  TEST(ShouldForwardTest, AllIntersectionsUnchangedDoesNotForward)
  {
    RevisionCounterResult r;
    r.comparisonPerformed = true;
    r.intersectionChanges.push_back(makeChange(59963, false));
    r.intersectionChanges.push_back(makeChange(18364, false));
    EXPECT_FALSE(planForwarding(r));
  }

  TEST(RevisionResultTest, SpatContentChangeFlaggedInIntersectionChanges)
  {
    RevisionCounterValidator validator;
    auto d1 = parseJson(R"({"value":{"SPAT":{"intersections":[
        {"id":{"id":59963},"revision":34,"status":"0000"}]}}})");
    validator.validateSpatRevision(d1);

    auto d2 = parseJson(R"({"value":{"SPAT":{"intersections":[
        {"id":{"id":59963},"revision":34,"status":"0040"}]}}})");
    auto r = validator.validateSpatRevision(d2);

    EXPECT_EQ(r.intersectionChanges.size(), 1u);
    EXPECT_EQ(r.intersectionChanges[0].id, 59963);
    EXPECT_TRUE(r.intersectionChanges[0].contentChanged);
  }

  TEST(RevisionResultTest, SpatNoContentChangeFlaggedFalse)
  {
    RevisionCounterValidator validator;
    auto d1 = parseJson(R"({"value":{"SPAT":{"intersections":[
        {"id":{"id":1},"revision":34,"status":"0000"}]}}})");
    validator.validateSpatRevision(d1);

    auto d2 = parseJson(R"({"value":{"SPAT":{"intersections":[
        {"id":{"id":1},"revision":34,"status":"0000"}]}}})"); // identical
    auto r = validator.validateSpatRevision(d2);

    EXPECT_EQ(r.intersectionChanges.size(), 1u);
    EXPECT_FALSE(r.intersectionChanges[0].contentChanged);
  }

  TEST(RevisionResultTest, MapContentChangeFlaggedInIntersectionChanges)
  {
    RevisionCounterValidator validator;
    auto d1 = parseJson(R"({"value":{"MapData":{"msgIssueRevision":5,"intersections":[
        {"id":{"id":1},"revision":0,"laneWidth":360}]}}})");
    validator.validateMapRevision(d1);

    auto d2 = parseJson(R"({"value":{"MapData":{"msgIssueRevision":5,"intersections":[
        {"id":{"id":1},"revision":0,"laneWidth":999}]}}})"); // content moved
    auto r = validator.validateMapRevision(d2);

    EXPECT_EQ(r.intersectionChanges.size(), 1u);
    EXPECT_TRUE(r.intersectionChanges[0].contentChanged);
  }

  TEST(RevisionResultTest, SpatMultiIntersectionOnlyFirstChanged)
  {
    RevisionCounterValidator validator;
    auto d1 = parseJson(R"({"value":{"SPAT":{"intersections":[
        {"id":{"id":59963},"revision":0,"status":"0000"},
        {"id":{"id":18364},"revision":0,"status":"0000"}]}}})");
    validator.validateSpatRevision(d1);

    auto d2 = parseJson(R"({"value":{"SPAT":{"intersections":[
        {"id":{"id":59963},"revision":0,"status":"0040"},
        {"id":{"id":18364},"revision":0,"status":"0000"}]}}})"); // only #1 moved
    auto r = validator.validateSpatRevision(d2);

    ASSERT_EQ(r.intersectionChanges.size(), 2u);
    EXPECT_EQ(r.intersectionChanges[0].id, 59963);
    EXPECT_TRUE(r.intersectionChanges[0].contentChanged);
    EXPECT_EQ(r.intersectionChanges[1].id, 18364);
    EXPECT_FALSE(r.intersectionChanges[1].contentChanged);
  }

  TEST(RevisionResultTest, SpatMultiIntersectionBothChanged)
  {
    RevisionCounterValidator validator;
    auto d1 = parseJson(R"({"value":{"SPAT":{"intersections":[
        {"id":{"id":59963},"revision":0,"status":"0000"},
        {"id":{"id":18364},"revision":0,"status":"0000"}]}}})");
    validator.validateSpatRevision(d1);

    auto d2 = parseJson(R"({"value":{"SPAT":{"intersections":[
        {"id":{"id":59963},"revision":0,"status":"0040"},
        {"id":{"id":18364},"revision":0,"status":"0080"}]}}})"); // both moved
    auto r = validator.validateSpatRevision(d2);

    ASSERT_EQ(r.intersectionChanges.size(), 2u);
    EXPECT_TRUE(r.intersectionChanges[0].contentChanged);
    EXPECT_TRUE(r.intersectionChanges[1].contentChanged);
  }

  TEST(RevisionResultTest, SpatMultiIntersectionNoneChanged)
  {
    RevisionCounterValidator validator;
    auto d1 = parseJson(R"({"value":{"SPAT":{"intersections":[
        {"id":{"id":59963},"revision":0,"status":"0000"},
        {"id":{"id":18364},"revision":0,"status":"0000"}]}}})");
    validator.validateSpatRevision(d1);

    auto d2 = parseJson(R"({"value":{"SPAT":{"intersections":[
        {"id":{"id":59963},"revision":0,"status":"0000"},
        {"id":{"id":18364},"revision":0,"status":"0000"}]}}})"); // identical
    auto r = validator.validateSpatRevision(d2);

    ASSERT_EQ(r.intersectionChanges.size(), 2u);
    EXPECT_FALSE(r.intersectionChanges[0].contentChanged);
    EXPECT_FALSE(r.intersectionChanges[1].contentChanged);
  }

  TEST(RevisionResultTest, MapMultiIntersectionOnlyFirstChanged)
  {
    RevisionCounterValidator validator;
    auto d1 = parseJson(R"({"value":{"MapData":{"msgIssueRevision":5,"intersections":[
        {"id":{"id":1},"revision":0,"laneWidth":360},
        {"id":{"id":2},"revision":0,"laneWidth":720}]}}})");
    validator.validateMapRevision(d1);

    auto d2 = parseJson(R"({"value":{"MapData":{"msgIssueRevision":6,"intersections":[
        {"id":{"id":1},"revision":0,"laneWidth":999},
        {"id":{"id":2},"revision":0,"laneWidth":720}]}}})"); // only #1 moved
    auto r = validator.validateMapRevision(d2);

    ASSERT_EQ(r.intersectionChanges.size(), 2u);
    EXPECT_EQ(r.intersectionChanges[0].id, 1);
    EXPECT_TRUE(r.intersectionChanges[0].contentChanged);
    EXPECT_EQ(r.intersectionChanges[1].id, 2);
    EXPECT_FALSE(r.intersectionChanges[1].contentChanged);
  }

  TEST(RevisionResultTest, MapMultiIntersectionBothChanged)
  {
    RevisionCounterValidator validator;
    auto d1 = parseJson(R"({"value":{"MapData":{"msgIssueRevision":5,"intersections":[
        {"id":{"id":1},"revision":0,"laneWidth":360},
        {"id":{"id":2},"revision":0,"laneWidth":720}]}}})");
    validator.validateMapRevision(d1);

    auto d2 = parseJson(R"({"value":{"MapData":{"msgIssueRevision":6,"intersections":[
        {"id":{"id":1},"revision":0,"laneWidth":999},
        {"id":{"id":2},"revision":0,"laneWidth":888}]}}})"); // both moved
    auto r = validator.validateMapRevision(d2);

    ASSERT_EQ(r.intersectionChanges.size(), 2u);
    EXPECT_TRUE(r.intersectionChanges[0].contentChanged);
    EXPECT_TRUE(r.intersectionChanges[1].contentChanged);
  }

  TEST(RevisionResultTest, MapMultiIntersectionNoneChanged)
  {
    RevisionCounterValidator validator;
    auto d1 = parseJson(R"({"value":{"MapData":{"msgIssueRevision":5,"intersections":[
        {"id":{"id":1},"revision":0,"laneWidth":360},
        {"id":{"id":2},"revision":0,"laneWidth":720}]}}})");
    validator.validateMapRevision(d1);

    auto d2 = parseJson(R"({"value":{"MapData":{"msgIssueRevision":5,"intersections":[
        {"id":{"id":1},"revision":0,"laneWidth":360},
        {"id":{"id":2},"revision":0,"laneWidth":720}]}}})"); // identical
    auto r = validator.validateMapRevision(d2);

    ASSERT_EQ(r.intersectionChanges.size(), 2u);
    EXPECT_FALSE(r.intersectionChanges[0].contentChanged);
    EXPECT_FALSE(r.intersectionChanges[1].contentChanged);
  }

  TEST(EvaluateProgressionTest, SpatContentSameRevisionBumpedViolation)
  {
    IntersectionChangeInfo change;
    change.id = 1;
    change.contentChanged = false;
    change.revisionChanged = true;
    change.currentRevision = 6;

    MsgProgressionCtx ctx;
    RevisionCounterValidator::evaluateProgression(change, /*prevRevision=*/5, ctx);

    EXPECT_TRUE(change.progressionViolation);
    EXPECT_EQ(5, change.progressionCountA);
    EXPECT_EQ(6, change.progressionCountB);
  }

  // SPaT: content changed with proper +1 increment
  TEST(EvaluateProgressionTest, SpatContentChangedProperIncrementNoViolation)
  {
    IntersectionChangeInfo change;
    change.contentChanged = true;
    change.revisionChanged = true;
    change.currentRevision = 6;

    MsgProgressionCtx ctx;
    RevisionCounterValidator::evaluateProgression(change, 5, ctx);

    EXPECT_FALSE(change.progressionViolation);
  }

  // SPaT: content + revision both unchanged
  TEST(EvaluateProgressionTest, SpatNoChangeNoViolation)
  {
    IntersectionChangeInfo change;
    change.contentChanged = false;
    change.revisionChanged = false;
    change.currentRevision = 5;

    MsgProgressionCtx ctx;
    RevisionCounterValidator::evaluateProgression(change, 5, ctx);

    EXPECT_FALSE(change.progressionViolation);
  }

  // SPaT: content changed but revision did NOT properly increment
  TEST(EvaluateProgressionTest, SpatContentChangedBadIncrementViolation)
  {
    IntersectionChangeInfo change;
    change.contentChanged = true;
    change.revisionChanged = true;
    change.currentRevision = 8; // prev 5, +1 would be 6, not 8

    MsgProgressionCtx ctx;
    RevisionCounterValidator::evaluateProgression(change, 5, ctx);

    EXPECT_TRUE(change.progressionViolation);
    EXPECT_EQ(5, change.progressionCountA);
    EXPECT_EQ(8, change.progressionCountB);
  }

  // SPaT: mod-128 wrap (127 → 0) is a proper increment
  TEST(EvaluateProgressionTest, SpatModWrapNoViolation)
  {
    IntersectionChangeInfo change;
    change.contentChanged = true;
    change.revisionChanged = true;
    change.currentRevision = 0;

    MsgProgressionCtx ctx;
    RevisionCounterValidator::evaluateProgression(change, 127, ctx);

    EXPECT_FALSE(change.progressionViolation);
  }

  // MAP: content changed + intersection revision stuck 
  TEST(EvaluateProgressionTest, MapContentChangedRevisionStuckReportsRevisionPair)
  {
    IntersectionChangeInfo change;
    change.contentChanged = true;
    change.revisionChanged = false;
    change.currentRevision = 5; // stuck at 5

    MsgProgressionCtx ctx;
    ctx.mapStyle = true;
    ctx.hasMsgPrev = true;
    ctx.prevMsgRevision = 5;
    ctx.currentMsgRevision = 6; // msgRev fine; revision is the problem
    RevisionCounterValidator::evaluateProgression(change, 5, ctx);

    EXPECT_TRUE(change.progressionViolation);
    EXPECT_EQ(5, change.progressionCountA); // revision pair takes priority
    EXPECT_EQ(5, change.progressionCountB);
  }

  // MAP: content changed, intersection rev proper +1, but msgIssueRevision stuck
  TEST(EvaluateProgressionTest, MapContentChangedMsgRevStuckReportsMsgRevPair)
  {
    IntersectionChangeInfo change;
    change.contentChanged = true;
    change.revisionChanged = true;
    change.currentRevision = 6; // prev 5, proper +1

    MsgProgressionCtx ctx;
    ctx.mapStyle = true;
    ctx.hasMsgPrev = true;
    ctx.prevMsgRevision = 5;
    ctx.currentMsgRevision = 5; // stuck → this is the reported pair
    RevisionCounterValidator::evaluateProgression(change, 5, ctx);

    EXPECT_TRUE(change.progressionViolation);
    EXPECT_EQ(5, change.progressionCountA);
    EXPECT_EQ(5, change.progressionCountB);
  }

  // MAP: content changed, both intersection rev and msgRev increment properly
  TEST(EvaluateProgressionTest, MapAllProperNoViolation)
  {
    IntersectionChangeInfo change;
    change.contentChanged = true;
    change.revisionChanged = true;
    change.currentRevision = 6;

    MsgProgressionCtx ctx;
    ctx.mapStyle = true;
    ctx.hasMsgPrev = true;
    ctx.prevMsgRevision = 5;
    ctx.currentMsgRevision = 6;
    RevisionCounterValidator::evaluateProgression(change, 5, ctx);

    EXPECT_FALSE(change.progressionViolation);
  }

  // MAP: no previous msgIssueRevision seen yet
  TEST(EvaluateProgressionTest, MapNoMsgPrevRevisionProperNoViolation)
  {
    IntersectionChangeInfo change;
    change.contentChanged = true;
    change.revisionChanged = true;
    change.currentRevision = 6;

    MsgProgressionCtx ctx;
    ctx.mapStyle = true;
    ctx.hasMsgPrev = false; // first MAP — no msgRev comparison
    ctx.currentMsgRevision = 99;
    RevisionCounterValidator::evaluateProgression(change, 5, ctx);

    EXPECT_FALSE(change.progressionViolation);
  }

} // namespace