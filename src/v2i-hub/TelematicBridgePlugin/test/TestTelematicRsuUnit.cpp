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
                "RSUStatusMonitorInterval": 10,
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
                "timestamp": 12345678,
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
        string testPath = "/tmp/test_rsu_config_invalid.json";
        setenv("RSU_CONFIG_PATH", testPath.c_str(),1);
        createTestConfigFile(testPath, "{invalid json}");
        unit = make_shared<TelematicRsuUnit>();

        Json::Value updateMessage;

        // Add RSU config with all required fields
        Json::Value rsuConfig;
        rsuConfig["unit"] = "unit";
        rsuConfig["action"] = "add";
        rsuConfig["event"] = "update";
        rsuConfig["rsu"]["IP"] = "192.168.1.20";
        rsuConfig["rsu"]["Port"] = 161;
        rsuConfig["snmp"]["User"] = "testuser";
        rsuConfig["snmp"]["PrivacyProtocol"] = "AES";
        rsuConfig["snmp"]["AuthProtocol"] = "SHA";
        rsuConfig["snmp"]["AuthPassPhrase"] = "testpass123";
        rsuConfig["snmp"]["PrivacyPassPhrase"] = "testpriv123";
        rsuConfig["snmp"]["RSUMIBVersion"] = "4.1";
        rsuConfig["snmp"]["SecurityLevel"] = "authPriv";

        // Use correct key names expected by updateRSUStatus
        updateMessage["rsuConfigs"].append(rsuConfig);  // Note: "rsuConfigs" not "rsuconfigs"
        updateMessage["timestamp"] = 1234567890;

        // Call updateRSUStatus
        bool result = unit->updateRSUStatus(updateMessage);

        // Verify it returns true
        ASSERT_TRUE(result);
    }
}