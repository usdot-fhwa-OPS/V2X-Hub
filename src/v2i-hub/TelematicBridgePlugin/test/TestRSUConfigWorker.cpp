#include <gtest/gtest.h>
#include "RSUConfigWorker.h"
#include <fstream>
#include <memory>

using namespace TelematicBridge;
using namespace std;

namespace TelematicBridge
{
    class TestRSUConfigWorker : public ::testing::Test
    {
    protected:
        shared_ptr<truConfigWorker> worker;

        void SetUp() override
        {
            worker = make_shared<truConfigWorker>();
        }

        void TearDown() override
        {
            worker.reset();
            removeTestFile("/tmp/test_tru_config.json");
        }

        // Helper to create test config file
        void createTestConfigFile(const string& path, const string& content)
        {
            ofstream file(path);
            file << content;
            file.close();
        }

        // Helper to remove test files
        void removeTestFile(const string& path)
        {
            remove(path.c_str());
        }

        // Helper to create valid RSU config JSON
        Json::Value createValidRsuConfigJson()
        {
            Json::Value config;
            config["action"] = "add";
            config["event"] = "startup";

            Json::Value rsu;
            rsu["ip"] = "192.168.1.10";
            rsu["port"] = 161;
            config["rsu"] = rsu;

            Json::Value snmp;
            snmp["user"] = "admin";
            snmp["privacyprotocol"] = "AES";
            snmp["authprotocol"] = "SHA";
            snmp["authpassphrase"] = "pass123";
            snmp["privacypassphrase"] = "priv123";
            snmp["rsumibversion"] = "4.1";
            snmp["securitylevel"] = "authPriv";
            config["snmp"] = snmp;

            return config;
        }

        // Helper to create valid complete config file content
        string getValidCompleteConfigFileContent()
        {
            return R"({
                "unitConfig": [
                    {
                        "unitID": "Unit001",
                        "name": "TestUnit",
                        "maxConnections": 10,
                        "pluginHeartbeatInterval": 30,
                        "healthMonitorPluginHeartbeatInterval": 60,
                        "rsuStatusMonitorINterval": 120
                    }
                ],
                "rsuConfigs": [
                    {
                        "action": "add",
                        "event": "startup",
                        "rsu": {
                            "ip": "192.168.1.10",
                            "port": 161
                        },
                        "snmp": {
                            "user": "admin",
                            "privacyprotocol": "AES",
                            "authprotocol": "SHA",
                            "authpassphrase": "pass123",
                            "privacypassphrase": "priv123",
                            "rsumibversion": "4.1",
                            "securitylevel": "authPriv"
                        }
                    }
                ],
                "timestamp": 1234567890
            })";
        }
    };

    // ==================== Action Conversion Tests ====================

    TEST_F(TestRSUConfigWorker, TestStringToActionAdd)
    {
        Json::Value config = createValidRsuConfigJson();
        config["action"] = "add";

        rsuConfig rsu;
        bool result = worker->jsonValueToRsuConfig(config, rsu);

        ASSERT_TRUE(result);
        ASSERT_EQ(rsu.actionType, action::add);
    }

    TEST_F(TestRSUConfigWorker, TestStringToActionDelete)
    {
        Json::Value config = createValidRsuConfigJson();
        config["action"] = "delete";

        rsuConfig rsu;
        bool result = worker->jsonValueToRsuConfig(config, rsu);

        ASSERT_TRUE(result);
        ASSERT_EQ(rsu.actionType, action::remove);
    }

    TEST_F(TestRSUConfigWorker, TestStringToActionUnknown)
    {
        Json::Value config = createValidRsuConfigJson();
        config["action"] = "invalid_action";

        rsuConfig rsu;
        bool result = worker->jsonValueToRsuConfig(config, rsu);

        ASSERT_TRUE(result);
        ASSERT_EQ(rsu.actionType, action::unknown);
    }

    TEST_F(TestRSUConfigWorker, TestActionDefaultsToAdd)
    {
        Json::Value config = createValidRsuConfigJson();
        config.removeMember("action");  // Remove action key

        rsuConfig rsu;
        bool result = worker->jsonValueToRsuConfig(config, rsu);

        ASSERT_TRUE(result);
        ASSERT_EQ(rsu.actionType, action::add);
    }

    // ==================== jsonValueToRsuConfig Tests ====================

    TEST_F(TestRSUConfigWorker, TestJsonValueToRsuConfigSuccess)
    {
        Json::Value config = createValidRsuConfigJson();
        rsuConfig rsu;

        bool result = worker->jsonValueToRsuConfig(config, rsu);

        ASSERT_TRUE(result);
        ASSERT_EQ(rsu.event, "startup");
        ASSERT_EQ(rsu.rsu.ip, "192.168.1.10");
        ASSERT_EQ(rsu.rsu.port, 161);
        ASSERT_EQ(rsu.snmp.userKey, "admin");
    }

    TEST_F(TestRSUConfigWorker, TestJsonValueToRsuConfigMissingEvent)
    {
        Json::Value config = createValidRsuConfigJson();
        config.removeMember("event");

        rsuConfig rsu;
        bool result = worker->jsonValueToRsuConfig(config, rsu);

        ASSERT_FALSE(result);
    }

    TEST_F(TestRSUConfigWorker, TestJsonValueToRsuConfigMissingRsuObject)
    {
        Json::Value config = createValidRsuConfigJson();
        config.removeMember("rsu");

        rsuConfig rsu;
        bool result = worker->jsonValueToRsuConfig(config, rsu);

        ASSERT_FALSE(result);
    }

    TEST_F(TestRSUConfigWorker, TestJsonValueToRsuConfigMissingSnmpObject)
    {
        Json::Value config = createValidRsuConfigJson();
        config.removeMember("snmp");

        rsuConfig rsu;
        bool result = worker->jsonValueToRsuConfig(config, rsu);

        ASSERT_FALSE(result);
    }

    TEST_F(TestRSUConfigWorker, TestJsonValueToRsuConfigDefaultPort)
    {
        Json::Value config = createValidRsuConfigJson();
        config["rsu"].removeMember("port");

        rsuConfig rsu;
        bool result = worker->jsonValueToRsuConfig(config, rsu);

        ASSERT_TRUE(result);
        ASSERT_EQ(rsu.rsu.port, 8080);  // Default port
    }

    TEST_F(TestRSUConfigWorker, TestJsonValueToRsuConfigInvalidRsuType)
    {
        Json::Value config = createValidRsuConfigJson();
        config["rsu"] = "not_an_object";  // Wrong type

        rsuConfig rsu;
        bool result = worker->jsonValueToRsuConfig(config, rsu);

        ASSERT_FALSE(result);
    }

    // ==================== loadRSUConfigListFromFile Tests ====================

    TEST_F(TestRSUConfigWorker, TestLoadRSUConfigListFromFileSuccess)
    {
        string testPath = "/tmp/test_tru_config.json";
        createTestConfigFile(testPath, getValidCompleteConfigFileContent());

        bool result = worker->loadRSUConfigListFromFile(testPath);

        ASSERT_TRUE(result);

        // Verify loaded config
        string unitId = worker->getUnitId();
        ASSERT_EQ(unitId, "Unit001");
    }

    TEST_F(TestRSUConfigWorker, TestLoadRSUConfigListFromFileNotFound)
    {
        bool result = worker->loadRSUConfigListFromFile("/nonexistent/path.json");

        ASSERT_FALSE(result);
    }

    TEST_F(TestRSUConfigWorker, TestLoadRSUConfigListFromFileInvalidJson)
    {
        string testPath = "/tmp/test_tru_config_invalid.json";
        createTestConfigFile(testPath, "{invalid json}");

        bool result = worker->loadRSUConfigListFromFile(testPath);

        ASSERT_FALSE(result);

        removeTestFile(testPath);
    }

    TEST_F(TestRSUConfigWorker, TestLoadRSUConfigListFromFileMissingKeys)
    {
        string testPath = "/tmp/test_tru_config_minimal.json";
        createTestConfigFile(testPath, R"({"someOtherKey": "value"})");

        // Should succeed but not load anything
        bool result = worker->loadRSUConfigListFromFile(testPath);

        removeTestFile(testPath);
    }

    // ==================== updateTRUStatus Tests ====================

    TEST_F(TestRSUConfigWorker, TestUpdateTRUStatusSuccess)
    {
        // Load initial config to set unit ID
        string testPath = "/tmp/test_tru_config.json";
        createTestConfigFile(testPath, getValidCompleteConfigFileContent());
        worker->loadRSUConfigListFromFile(testPath);
        removeTestFile(testPath);

        // Create complete valid message
        Json::Value updateMessage;
        Json::Value unitConfig;
        unitConfig["unitID"] = "Unit001";
        updateMessage["unitConfig"].append(unitConfig);

        Json::Value rsuConfig = createValidRsuConfigJson();
        updateMessage["rsuConfigs"].append(rsuConfig);
        updateMessage["timestamp"] = 1234567890;

        bool result = worker->updateTRUStatus(updateMessage);

        ASSERT_TRUE(result);
    }

    TEST_F(TestRSUConfigWorker, TestUpdateTRUStatusMissingUnitConfig)
    {
        Json::Value updateMessage;
        updateMessage["rsuConfigs"].append(createValidRsuConfigJson());
        updateMessage["timestamp"] = 1234567890;

        bool result = worker->updateTRUStatus(updateMessage);

        ASSERT_FALSE(result);
    }

    TEST_F(TestRSUConfigWorker, TestUpdateTRUStatusMissingRsuConfigs)
    {
        Json::Value updateMessage;
        Json::Value unitConfig;
        unitConfig["unitID"] = "Unit001";
        updateMessage["unitConfig"].append(unitConfig);
        updateMessage["timestamp"] = 1234567890;

        bool result = worker->updateTRUStatus(updateMessage);

        ASSERT_FALSE(result);
    }

    TEST_F(TestRSUConfigWorker, TestUpdateTRUStatusMissingTimestamp)
    {
        Json::Value updateMessage;
        Json::Value unitConfig;
        unitConfig["unitID"] = "Unit001";
        updateMessage["unitConfig"].append(unitConfig);
        updateMessage["rsuConfigs"].append(createValidRsuConfigJson());

        bool result = worker->updateTRUStatus(updateMessage);

        ASSERT_FALSE(result);
    }

    TEST_F(TestRSUConfigWorker, TestUpdateTRUStatusMismatchedUnitId)
    {
        string testPath = "/tmp/test_tru_config_mismatch.json";
        createTestConfigFile(testPath, getValidCompleteConfigFileContent());
        worker->loadRSUConfigListFromFile(testPath);
        removeTestFile(testPath);

        Json::Value updateMessage;
        Json::Value unitConfig;
        unitConfig["unitID"] = "WrongUnit";  // Doesn't match Unit001
        updateMessage["unitConfig"].append(unitConfig);
        updateMessage["rsuConfigs"].append(createValidRsuConfigJson());
        updateMessage["timestamp"] = 1234567890;

        bool result = worker->updateTRUStatus(updateMessage);

        ASSERT_FALSE(result);
    }

    // ==================== rsuConfigToJsonValue Tests ====================

    TEST_F(TestRSUConfigWorker, TestRsuConfigToJsonValue)
    {
        // Create an rsuConfig struct
        rsuConfig config;
        config.actionType = action::add;
        config.event = "test_event";
        config.rsu.ip = "192.168.1.100";
        config.rsu.port = 8080;
        config.snmp.userKey = "testuser";
        config.snmp.privProtocol = "AES256";
        config.snmp.authProtocolKey = "SHA256";
        config.snmp.authPassPhraseKey = "authpass";
        config.snmp.privPassPhrase = "privpass";
        config.snmp.rsuMIBVersionKey = "5.0";
        config.snmp.securityLevelKey = "authPriv";

        Json::Value configJson = createValidRsuConfigJson();
        configJson["rsu"]["ip"] = "192.168.1.100";
        configJson["event"] = "test_event";

        Json::Value message;
        message["rsuConfigs"].append(configJson);

        worker->updateTRUStatus(message);

        Json::Value output = worker->getTruConfigAsJsonArray();

        ASSERT_TRUE(output.isMember("rsuConfigs"));
        ASSERT_TRUE(output["rsuConfigs"].isArray());
        ASSERT_TRUE(output["rsuConfigs"].empty());
    }

    // ==================== getTruConfigAsJsonArray Tests ====================

    TEST_F(TestRSUConfigWorker, TestGetTruConfigAsJsonArrayStructure)
    {
        Json::Value result = worker->getTruConfigAsJsonArray();

        ASSERT_TRUE(result.isMember("unitConfig"));
        ASSERT_TRUE(result.isMember("rsuConfigs"));
        ASSERT_TRUE(result.isMember("timestamp"));
    }

    TEST_F(TestRSUConfigWorker, TestGetTruConfigAsJsonArrayWithLoadedData)
    {
        string testPath = "/tmp/test_tru_config_loaded.json";
        createTestConfigFile(testPath, getValidCompleteConfigFileContent());
        worker->loadRSUConfigListFromFile(testPath);
        removeTestFile(testPath);

        Json::Value result = worker->getTruConfigAsJsonArray();

        ASSERT_TRUE(result["rsuConfigs"].isArray());
        ASSERT_GT(result["rsuConfigs"].size(), 0);
        ASSERT_EQ(result["rsuConfigs"][0]["rsu"]["ip"].asString(), "192.168.1.10");
    }

    // ==================== getTRUConfigResponse Tests ====================

    TEST_F(TestRSUConfigWorker, TestGetTRUConfigResponseSuccess)
    {
        Json::Value result = worker->getTRUConfigResponse(true);

        ASSERT_TRUE(result.isMember("unitConfig"));
        ASSERT_TRUE(result.isMember("rsuConfigs"));
        ASSERT_TRUE(result.isMember("status"));
        ASSERT_TRUE(result.isMember("timestamp"));
        ASSERT_EQ(result["status"].asString(), "success");
    }

    TEST_F(TestRSUConfigWorker, TestGetTRUConfigResponseFailed)
    {
        Json::Value result = worker->getTRUConfigResponse(false);

        ASSERT_TRUE(result.isMember("status"));
        ASSERT_EQ(result["status"].asString(), "failed");
    }

    TEST_F(TestRSUConfigWorker, TestGetTRUConfigResponseStructure)
    {
        string testPath = "/tmp/test_tru_config_response.json";
        createTestConfigFile(testPath, getValidCompleteConfigFileContent());
        worker->loadRSUConfigListFromFile(testPath);
        removeTestFile(testPath);

        Json::Value result = worker->getTRUConfigResponse(true);

        ASSERT_TRUE(result["unitConfig"].isArray());
        ASSERT_GT(result["unitConfig"].size(), 0);
        ASSERT_TRUE(result["unitConfig"][0].isMember("unitID"));

        ASSERT_TRUE(result["rsuConfigs"].isArray());
        if (result["rsuConfigs"].size() > 0) {
            ASSERT_TRUE(result["rsuConfigs"][0].isMember("ip"));
            ASSERT_TRUE(result["rsuConfigs"][0].isMember("port"));
        }
    }

    // ==================== getUnitId Tests ====================

    TEST_F(TestRSUConfigWorker, TestGetUnitIdDefault)
    {
        string unitId = worker->getUnitId();
        // Default should be empty
        ASSERT_TRUE(unitId.empty());
    }

    TEST_F(TestRSUConfigWorker, TestGetUnitIdAfterLoad)
    {
        string testPath = "/tmp/test_tru_config_unitid.json";
        createTestConfigFile(testPath, getValidCompleteConfigFileContent());
        worker->loadRSUConfigListFromFile(testPath);
        removeTestFile(testPath);

        string unitId = worker->getUnitId();
        ASSERT_EQ(unitId, "Unit001");
    }

    TEST_F(TestRSUConfigWorker, TestCompleteWorkflow)
    {
        // 1. Load initial config
        string testPath = "/tmp/test_tru_workflow.json";
        createTestConfigFile(testPath, getValidCompleteConfigFileContent());

        bool loadResult = worker->loadRSUConfigListFromFile(testPath);
        ASSERT_TRUE(loadResult);
        removeTestFile(testPath);

        // 2. Verify unit ID was loaded
        ASSERT_EQ(worker->getUnitId(), "Unit001");

        // 3. Update with new RSU config
        Json::Value updateMsg;
        Json::Value newRsu = createValidRsuConfigJson();
        newRsu["rsu"]["ip"] = "192.168.1.50";
        updateMsg["rsuConfigs"].append(newRsu);

        bool updateResult = worker->updateTRUStatus(updateMsg);
        ASSERT_FALSE(updateResult);

        // 4. Get full config
        Json::Value fullConfig = worker->getTruConfigAsJsonArray();
        ASSERT_TRUE(fullConfig.isMember("rsuConfigs"));
        ASSERT_GT(fullConfig["rsuConfigs"].size(), 0);

        // 5. Get response
        Json::Value response = worker->getTRUConfigResponse(true);
        ASSERT_EQ(response["status"].asString(), "success");
    }

    TEST_F(TestRSUConfigWorker, TestMultipleRsuConfigs)
    {
        string testPath = "/tmp/test_tru_workflow.json";
        createTestConfigFile(testPath, getValidCompleteConfigFileContent());

        worker->loadRSUConfigListFromFile(testPath);

        Json::Value updateMsg;
        Json::Value unitConfig;
        unitConfig["unitID"] = "Unit001";
        updateMsg["unitConfig"].append(unitConfig);
        updateMsg["timestamp"] = 1234567890;

        // Add multiple RSU configs
        for (int i = 1; i <= 3; ++i) {
            Json::Value rsu = createValidRsuConfigJson();
            rsu["rsu"]["ip"] = "192.168.1." + to_string(10 + i);
            updateMsg["rsuConfigs"].append(rsu);
        }

        bool result = worker->updateTRUStatus(updateMsg);
        ASSERT_TRUE(result);

        Json::Value config = worker->getTruConfigAsJsonArray();
        ASSERT_EQ(config["rsuConfigs"].size(), 4);
    }

    TEST_F(TestRSUConfigWorker, TestRoundTripConversion)
    {
        // 1. Load initial config
        string testPath = "/tmp/test_tru_workflow.json";
        createTestConfigFile(testPath, getValidCompleteConfigFileContent());

        worker->loadRSUConfigListFromFile(testPath);

        Json::Value updateMsg;
        Json::Value unitConfig;
        unitConfig["unitID"] = "Unit001";
        updateMsg["unitConfig"].append(unitConfig);
        updateMsg["timestamp"] = 1234567890;

        // Create config
        Json::Value originalConfig = createValidRsuConfigJson();
        originalConfig["rsu"]["ip"] = "192.168.1.99";
        originalConfig["event"] = "roundtrip_test";

        // Load it
        updateMsg["rsuConfigs"].append(originalConfig);
        worker->updateTRUStatus(updateMsg);

        // Get it back
        Json::Value retrieved = worker->getTruConfigAsJsonArray();

        ASSERT_TRUE(retrieved["rsuConfigs"].isArray());
        ASSERT_GT(retrieved["rsuConfigs"].size(), 0);

        // Verify key fields match
        ASSERT_EQ(retrieved["rsuConfigs"][0]["rsu"]["ip"].asString(), "192.168.1.99");
        ASSERT_EQ(retrieved["rsuConfigs"][0]["event"].asString(), "roundtrip_test");
    }
}