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

        void createTestConfigFile(const string &path, const string &content)
        {
            ofstream file(path);
            file << content;
            file.close();
        }

        // Helper to clean up test files
        void removeTestFile(const string &path)
        {
            remove(path.c_str());
        }

        string getValidRSUConfigFileContent()
        {
            return R"({
                "Unit": {
                    "UnitID": "Unit002",
                    "MaxConnections": 10,
                    "BridgePluginHeartbeatInterval": 10,
                    "HealthMonitorPluginHeartbeatInterval": 10,
                    "RSUStatusMonitorInterval": 10
                },
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
                ],
                "timestamp": 12345678
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
        ASSERT_TRUE(root.isMember("rsuconfigs"));
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
        ASSERT_TRUE(root["unit"].isMember("unitid"));
        ASSERT_TRUE(root["unit"].isMember("maxconnections"));
        ASSERT_TRUE(root["unit"].isMember("bridgepluginheartbeatinterval"));
        ASSERT_TRUE(root["unit"].isMember("healthmonitorpluginheartbeatinterval"));
        ASSERT_TRUE(root["unit"].isMember("rsustatusmonitorinterval"));
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
        vector<string> requiredTopKeys = {"unit", "rsuconfigs", "timestamp"};
        for (const auto& key : requiredTopKeys)
        {
            ASSERT_TRUE(root.isMember(key));
        }

        // Check all required unit keys
        vector<string> requiredUnitKeys = {
            "unitid",
            "maxconnections",
            "bridgepluginheartbeatinterval",
            "healthmonitorpluginheartbeatinterval",
            "rsustatusmonitorinterval"
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

    TEST_F(TestTelematicRsuUnit, TestUpdateRSUStatus)
{
    unit = make_shared<TelematicRsuUnit>();

    // Create JSON object
    Json::Value updateMessage;

    // Add unit object
    Json::Value updateUnit;
    updateUnit["unitid"] = "Unit002";
    updateUnit["maxConnections"] = 10;
    updateUnit["bridgepluginheartbeatinterval"] = 100;
    updateUnit["healthmonitorpluginheartbeatinterval"] = 100;
    updateUnit["rsustatusmonitorinterval"] = 100;
    updateMessage["unit"] = updateUnit;

    // Add RSU config with all required fields
    Json::Value rsuConfig;
    rsuConfig["action"] = "add";
    rsuConfig["event"] = "update";

    Json::Value rsu;
    rsu["IP"] = "192.168.1.20";
    rsu["Port"] = 161;
    rsuConfig["rsu"] = rsu;

    Json::Value snmp;
    snmp["User"] = "testuser";
    snmp["PrivacyProtocol"] = "AES";
    snmp["AuthProtocol"] = "SHA";
    snmp["AuthPassPhrase"] = "testpass123";
    snmp["PrivacyPassPhrase"] = "testpriv123";
    snmp["RSUMIBVersion"] = "4.1";
    snmp["SecurityLevel"] = "authPriv";
    rsuConfig["snmp"] = snmp;

    updateMessage["rsuConfigs"].append(rsuConfig);
    updateMessage["timestamp"] = 1234567890;

    bool result = unit->updateRSUStatus(updateMessage);
    ASSERT_TRUE(result);

    // Second call with same RSU should fail (duplicate) since its already registered
    result = unit->updateRSUStatus(updateMessage);
    ASSERT_FALSE(result);
}

TEST_F(TestTelematicRsuUnit, TestConstructRSUConfigResponseDataStringSuccess)
{
    string testPath = "/tmp/test_rsu_config_load.json";
    setenv("RSU_CONFIG_PATH", testPath.c_str(), 1);
    createTestConfigFile(testPath, getValidRSUConfigFileContent());
    unit = make_shared<TelematicRsuUnit>();

    // Call with success = true
    string jsonStr = unit->constructRSUConfigResponseDataString(true);

    // Parse JSON
    Json::Value root;
    Json::CharReaderBuilder builder;
    istringstream jsonStream(jsonStr);
    string errs;
    ASSERT_TRUE(Json::parseFromStream(builder, jsonStream, &root, &errs));

    // Verify required keys exist
    ASSERT_TRUE(root.isMember("unit"));
    ASSERT_TRUE(root.isMember("rsuconfigs"));
    ASSERT_TRUE(root.isMember("status"));
    ASSERT_TRUE(root.isMember("timestamp"));

    // Verify status is "success"
    ASSERT_EQ(root["status"].asString(), "success");
    // Verify unit has unitId
    ASSERT_TRUE(root["unit"].isMember("unitid"));
    // Verify rsuConfig is an array
    ASSERT_TRUE(root["rsuconfigs"].isArray());

    unsetenv("RSU_CONFIG_PATH");
    removeTestFile(testPath);
}

}