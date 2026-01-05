#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TelematicRsuUnit.h"
#include "RSUConfigWorker.h"
#include <fstream>
#include <thread>
#include <memory>

using namespace TelematicBridge;
using namespace std;

namespace TelematicBridge
{
    class TestTelematicRsuUnit : public ::testing::Test
    {
    protected:
        shared_ptr<TelematicRsuUnit> unit;

        void SetUp() override
        {
            unsetenv("RSU_CONFIG_PATH");
        }

        string getValidRSUConfigFileContent()
        {
            return R"({
                "RSUConfigs": [
                    {
                        "action": "add",
                        "event": "startup",
                        "rsu": {
                            "IP": "192.168.1.10",
                            "Port": 161
                        },
                        "snmp": {
                            "User": "admin",
                            "PrivacyProtocol": "AES",
                            "AuthProtocol": "SHA",
                            "AuthPassPhrase": "pass123",
                            "PrivacyPassPhrase": "priv123",
                            "RSUMIBVersion": "4.1",
                            "SecurityLevel": "authPriv"
                        }
                    }
                ]
            })";
        }
    };

    TEST_F(TestTelematicRsuUnit, TestConstructRSURegistrationDataStringBasic)
    {
        unit = make_shared<TelematicRsuUnit>();

        string jsonStr = unit->constructRSURegistrationDataString();

        ASSERT_FALSE(jsonStr.empty());

        // Parse and validate JSON structure
        Json::Value root;
        Json::CharReaderBuilder builder;
        istringstream jsonStream(jsonStr);
        string errs;
        ASSERT_TRUE(Json::parseFromStream(builder, jsonStream, &root, &errs));

        // Verify top-level structure
        ASSERT_TRUE(root.isMember("unit"));
        ASSERT_TRUE(root.isMember("rsuConfigs"));
        ASSERT_TRUE(root.isMember("timestamp"));
    }

    TEST_F(TestTelematicRsuUnit, TestConstructRSURegistrationDataStringUnitObject)
    {
        unit = make_shared<TelematicRsuUnit>();

        string jsonStr = unit->constructRSURegistrationDataString();

        Json::Value root;
        Json::CharReaderBuilder builder;
        istringstream jsonStream(jsonStr);
        string errs;
        Json::parseFromStream(builder, jsonStream, &root, &errs);

        // Verify unit object structure
        ASSERT_TRUE(root["unit"].isObject());
        ASSERT_TRUE(root["unit"].isMember("unitId"));
        ASSERT_TRUE(root["unit"].isMember("maxConnections"));
        ASSERT_TRUE(root["unit"].isMember("bridgePluginHeartbeatInterval"));
        ASSERT_TRUE(root["unit"].isMember("healthMonitorPluginHeartbeatInterval"));
        ASSERT_TRUE(root["unit"].isMember("rsuStatusMonitorInterval"));
    }

    TEST_F(TestTelematicRsuUnit, TestConstructRSURegistrationDataStringThreadSafety)
    {
        unit = make_shared<TelematicRsuUnit>();

        const int numThreads = 10;
        vector<thread> threads;
        vector<string> results(numThreads);

        for (int i = 0; i < numThreads; ++i)
        {
            threads.emplace_back([this, i, &results]() {
                results[i] = unit->constructRSURegistrationDataString();
            });
        }

        for (auto &t : threads)
        {
            t.join();
        }

        // All results should be valid JSON
        for (const auto &result : results)
        {
            ASSERT_FALSE(result.empty());
            Json::Value root;
            Json::CharReaderBuilder builder;
            istringstream jsonStream(result);
            string errs;
            ASSERT_TRUE(Json::parseFromStream(builder, jsonStream, &root, &errs));
        }
    }

    TEST_F(TestTelematicRsuUnit, TestJSONContainsAllRequiredKeys)
    {
        unit = make_shared<TelematicRsuUnit>();

        string jsonStr = unit->constructRSURegistrationDataString();

        Json::Value root;
        Json::CharReaderBuilder builder;
        istringstream jsonStream(jsonStr);
        string errs;
        Json::parseFromStream(builder, jsonStream, &root, &errs);

        // Check all required top-level keys
        vector<string> requiredTopKeys = {"unit", "rsuConfigs", "timestamp"};
        for (const auto& key : requiredTopKeys)
        {
            ASSERT_TRUE(root.isMember(key));
        }

        // Check all required unit keys
        vector<string> requiredUnitKeys = {
            "unitId",
            "maxConnections",
            "bridgePluginHeartbeatInterval",
            "healthMonitorPluginHeartbeatInterval",
            "rsuStatusMonitorInterval"
        };
        for (const auto& key : requiredUnitKeys)
        {
            ASSERT_TRUE(root["unit"].isMember(key));
        }
    }

    TEST_F(TestTelematicRsuUnit, TestConstructRSURegistrationDataStringEmptyUnit)
    {
        unit = make_shared<TelematicRsuUnit>();

        // Even with empty/default unit config, should produce valid JSON
        string jsonStr = unit->constructRSURegistrationDataString();

        ASSERT_FALSE(jsonStr.empty());

        Json::Value root;
        Json::CharReaderBuilder builder;
        istringstream jsonStream(jsonStr);
        string errs;
        ASSERT_TRUE(Json::parseFromStream(builder, jsonStream, &root, &errs));
    }
}