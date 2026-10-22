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
#include <string>
#include <vector>
#include <rapidjson/document.h>

#include "ODEForwardConversion.h"

namespace ODEForwardPlugin
{
    namespace
    {
        rapidjson::Document parse(const char *json)
        {
            rapidjson::Document doc;
            doc.Parse(json);
            EXPECT_FALSE(doc.HasParseError()) << "test fixture JSON failed to parse: " << json;
            return doc;
        }

        rapidjson::Document reparse(const std::string &json)
        {
            rapidjson::Document doc;
            doc.Parse(json.c_str());
            EXPECT_FALSE(doc.HasParseError()) << "convertToNum produced unparseable JSON: " << json;
            return doc;
        }
    }

    // toNumber

    TEST(ToNumberTest, ConvertsNumericStrings)
    {
        // Positive and negative (roadRegulatorID of -1 is the real-world default).
        rapidjson::Document doc = parse(R"json({"intersectionID": "105", "roadRegulatorID": "-1"})json");

        toNumber(doc, {"intersectionID", "roadRegulatorID"});

        EXPECT_TRUE(doc["intersectionID"].IsInt64());
        EXPECT_EQ(doc["intersectionID"].GetInt64(), 105);
        EXPECT_TRUE(doc["roadRegulatorID"].IsInt64());
        EXPECT_EQ(doc["roadRegulatorID"].GetInt64(), -1);
    }

    TEST(ToNumberTest, LeavesUnparseableOrIneligibleValuesAsStrings)
    {
        // Non-numeric text, trailing garbage, and embedded non-digits all fail
        rapidjson::Document doc = parse(R"json({
            "eventType": "SpatMinimumData",
            "trailing": "42 ",
            "embedded": "12x34"
        })json");

        toNumber(doc, {"eventType", "trailing", "embedded"});

        EXPECT_TRUE(doc["eventType"].IsString());
        EXPECT_TRUE(doc["trailing"].IsString());
        EXPECT_TRUE(doc["embedded"].IsString());
    }

    TEST(ToNumberTest, SkipsAbsentAndNonStringFields)
    {
        rapidjson::Document doc = parse(R"json({"already": 105})json");

        toNumber(doc, {"missing", "already"});

        EXPECT_FALSE(doc.HasMember("missing"));
        EXPECT_TRUE(doc["already"].IsInt64());
        EXPECT_EQ(doc["already"].GetInt64(), 105);
    }

    TEST(ToNumberTest, WarnsOnlyOnParseException)
    {
        rapidjson::Document doc = parse(R"json({
            "bad": "abc",
            "huge": "99999999999999999999",
            "trailing": "42 "
        })json");

        std::vector<std::string> warnings;
        toNumber(doc, {"bad", "huge", "trailing"},
            [&warnings](const std::string &m) { warnings.push_back(m); });


        // Two throwing fields warned; the trailing-space field did not
        ASSERT_EQ(warnings.size(), 2u);
        // Messages name the offending field.
        EXPECT_NE(warnings[0].find("'bad'"), std::string::npos);
        EXPECT_NE(warnings[1].find("'huge'"), std::string::npos);

        // All three left as strings regardless of whether they warned
        EXPECT_TRUE(doc["bad"].IsString());
        EXPECT_TRUE(doc["huge"].IsString());
        EXPECT_TRUE(doc["trailing"].IsString());
    }

    TEST(ToNumberTest, DoesNotInvokeWarnOnCleanConversion)
    {
        rapidjson::Document doc = parse(R"json({"intersectionID": "105"})json");

        std::vector<std::string> warnings;
        toNumber(doc, {"intersectionID"},
                 [&warnings](const std::string &m) { warnings.push_back(m); });


        EXPECT_TRUE(warnings.empty());
        EXPECT_TRUE(doc["intersectionID"].IsInt64());
    }

    // convertToNum

    TEST(ConvertToNumTest, FullEventRoundTrips)
    {
        const std::string input = R"json({
            "eventGeneratedAt": "1785779824579",
            "eventType": "SpatMinimumData",
            "intersectionID": "105",
            "roadRegulatorID": "-1",
            "source": "192.168.60.42",
            "numberOfMessages": "2",
            "messageCountA": "10",
            "messageCountB": "11",
            "missingDataElements": ["$.value.SPAT.intersections[0].roadAuthorityID is missing (#/properties/value)"],
            "timePeriod": {
                "beginTimestamp": "1785779824574",
                "endTimestamp": "1785779824579"
            }
        })json";

        rapidjson::Document out = reparse(convertToNum(input));

        // Numeric fields typed as numbers
        EXPECT_TRUE(out["eventGeneratedAt"].IsInt64());
        EXPECT_EQ(out["eventGeneratedAt"].GetInt64(), 1785779824579LL);
        EXPECT_TRUE(out["intersectionID"].IsInt64());
        EXPECT_TRUE(out["roadRegulatorID"].IsInt64());
        EXPECT_EQ(out["roadRegulatorID"].GetInt64(), -1);
        EXPECT_TRUE(out["numberOfMessages"].IsInt64());
        EXPECT_TRUE(out["messageCountA"].IsInt64());
        EXPECT_TRUE(out["messageCountB"].IsInt64());

        // Nested timePeriod converts
        EXPECT_TRUE(out["timePeriod"]["beginTimestamp"].IsInt64());
        EXPECT_TRUE(out["timePeriod"]["endTimestamp"].IsInt64());

        // String fields and the array are left alone
        EXPECT_TRUE(out["eventType"].IsString());
        EXPECT_TRUE(out["source"].IsString());
        EXPECT_TRUE(out["missingDataElements"].IsArray());
        EXPECT_TRUE(out["missingDataElements"][0].IsString());
    }

    TEST(ConvertToNumTest, ReturnsInputUnchangedOnBadInput)
    {
        // Unparseable JSON and non-object JSON are both forwarded
        // verbatim rather than dropped or throwing
        const std::string malformed = "{ this is not valid json";
        EXPECT_EQ(convertToNum(malformed), malformed);

        const std::string notObject = R"json([1, 2, 3])json";
        EXPECT_EQ(convertToNum(notObject), notObject);
    }

    TEST(ConvertToNumTest, LeavesNonNumericValueInNumericFieldAsString)
    {
        // A field in the numeric list whose value isn't a clean integer stays a string
        rapidjson::Document out = reparse(convertToNum(R"json({"intersectionID": "not-a-number"})json"));
        EXPECT_TRUE(out["intersectionID"].IsString());
    }

    TEST(ConvertToNumTest, ForwardsWarnSinkToFields)
    {
        // A throwing value in a numeric field routes a warning
        std::vector<std::string> warnings;
        convertToNum(R"json({"intersectionID": "abc"})json",
                     [&warnings](const std::string &m) { warnings.push_back(m); });

        EXPECT_EQ(warnings.size(), 1u);
        EXPECT_NE(warnings[0].find("'intersectionID'"), std::string::npos);
    }

}