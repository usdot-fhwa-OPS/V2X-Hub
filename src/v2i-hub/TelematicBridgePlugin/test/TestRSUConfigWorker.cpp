#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TelematicRsuUnit.h"
#include "TelematicBridgeException.h"
#include <fstream>
#include <thread>

using namespace std;
using namespace tmx::utils;
using ::testing::_;
using ::testing::Return;

namespace TelematicBridge
{
    class TestRSUConfigWorker : public ::testing::Test
    {
    public:
        TestRSUConfigWorker(){
            shared_ptr<TelematicRsuUnit> _telematicRsuUnitPtr = make_shared<TelematicRsuUnit>();
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

        Json::Value createValidRSUConfigJson(const string& ip = "192.168.1.10", const string& port = "161")
        {
            Json::Value config;
            config["action"] = "add";
            config["event"] = "startup";
            config["rsu"]["IP"] = ip;
            config["rsu"]["Port"] = std::stoi(port);
            config["snmp"]["User"] = "admin";
            config["snmp"]["PrivacyProtocol"] = "AES";
            config["snmp"]["AuthProtocol"] = "SHA";
            config["snmp"]["AuthPassPhrase"] = "pass123";
            config["snmp"]["PrivacyPassPhrase"] = "priv123";
            config["snmp"]["RSUMIBVersion"] = "4.1";
            config["snmp"]["SecurityLevel"] = "authPriv";
            return config;
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


    TEST_F(TestRSUConfigWorker, TestValidateRequiredKeysSuccess)
    {
        Json::Value json;
        json["key1"] = "value1";
        json["key2"] = "value2";

        vector<string> requiredKeys = {"key1", "key2"};

        ASSERT_NO_THROW(validateRequiredKeys(json, requiredKeys));

        requiredKeys = {"key3"};
        ASSERT_THROW(validateRequiredKeys(json, requiredKeys), std::runtime_error);
    }

    TEST_F(TestRSUConfigWorker, TestProcessRSUConfigSuccess)
    {
        Json::Value rsuConfigJson = createValidRSUConfigJson();
        vector<rsuConfig> rsuList;
        int16_t maxConnections = 10;

        bool result = processRSUConfig(rsuConfigJson, maxConnections, rsuList);

        ASSERT_TRUE(result);
        ASSERT_EQ(rsuList.size(), 1);
        ASSERT_EQ(rsuList[0].rsu.ip, "192.168.1.10");
        ASSERT_EQ(rsuList[0].rsu.port, 161);
        ASSERT_EQ(rsuList[0].action, "add");
        ASSERT_EQ(rsuList[0].event, "startup");
        ASSERT_EQ(rsuList[0].snmp.userKey, "admin");

        rsuList = {};
        rsuConfigJson = "";
        result = processRSUConfig(rsuConfigJson, maxConnections, rsuList);
        ASSERT_FALSE(result);
        ASSERT_TRUE(rsuList.empty());

        Json::Value rsuConfigJson2;
        rsuConfigJson2["rsu"]["IP"] = "192.168.1.10";
        rsuConfigJson2["rsu"]["Port"] = 161;
        result = processRSUConfig(rsuConfigJson2, maxConnections, rsuList);
        ASSERT_FALSE(result);
        ASSERT_TRUE(rsuList.empty());
    }

    TEST_F(TestRSUConfigWorker, TestRsuConfigToJsonValueComplete)
    {
        rsuConfig config;
        config.action = "add";
        config.event = "startup";
        config.rsu.ip = "192.168.1.10";
        config.rsu.port = 161;
        config.snmp.userKey = "admin";
        config.snmp.privProtocol = "AES";
        config.snmp.authProtocolKey = "SHA";
        config.snmp.authPassPhraseKey = "pass123";
        config.snmp.privPassPhrase = "priv123";
        config.snmp.rsuMIBVersionKey = "4.1";
        config.snmp.securityLevelKey = "authPriv";

        Json::Value result = rsuConfigToJsonValue(config);

        ASSERT_EQ(result["action"].asString(), "add");
        ASSERT_EQ(result["event"].asString(), "startup");
        ASSERT_EQ(result["rsu"]["IP"].asString(), "192.168.1.10");
        ASSERT_EQ(result["rsu"]["Port"].asString(), "161");
        ASSERT_EQ(result["snmp"]["User"].asString(), "admin");
        ASSERT_EQ(result["snmp"]["PrivacyProtocol"].asString(), "AES");
        ASSERT_EQ(result["snmp"]["AuthProtocol"].asString(), "SHA");
    }

    TEST_F(TestRSUConfigWorker, TestJsonValueToRsuConfigSuccess)
    {
        Json::Value json;
        json["action"] = "add";
        json["event"] = "startup";
        json["rsu"]["IP"] = "192.168.1.10";
        json["rsu"]["Port"] = 161;
        json["snmp"]["User"] = "admin";
        json["snmp"]["PrivacyProtocol"] = "AES";
        json["snmp"]["AuthProtocol"] = "SHA";
        json["snmp"]["AuthPassPhrase"] = "pass123";
        json["snmp"]["PrivacyPassPhrase"] = "priv123";
        json["snmp"]["RSUMIBVersion"] = "4.1";
        json["snmp"]["SecurityLevel"] = "authPriv";

        rsuConfig config;
        bool result = jsonValueToRsuConfig(json, config);

        ASSERT_TRUE(result);
        ASSERT_EQ(config.rsu.ip, "192.168.1.10");
        ASSERT_EQ(config.rsu.port, 161);
        ASSERT_EQ(config.action, "add");
        ASSERT_EQ(config.event, "startup");
        ASSERT_EQ(config.snmp.userKey, "admin");

        Json::Value json2;
        json2["action"] = "add";
        json2["event"] = "startup";
        json2["snmp"]["User"] = "admin";
        rsuConfig config2;
        result = jsonValueToRsuConfig(json2, config2);
        ASSERT_FALSE(result);
    }

    TEST_F(TestRSUConfigWorker, TestRsuConfigMessageToVectorSuccess)
    {
        Json::Value jsonArray(Json::arrayValue);

        Json::Value config1;
        config1["action"] = "add";
        config1["event"] = "startup";
        config1["rsu"]["IP"] = "192.168.1.10";
        config1["rsu"]["Port"] = 161;
        config1["snmp"]["User"] = "admin1";
        config1["snmp"]["PrivacyProtocol"] = "AES";
        config1["snmp"]["AuthProtocol"] = "SHA";
        config1["snmp"]["AuthPassPhrase"] = "pass1";
        config1["snmp"]["PrivacyPassPhrase"] = "priv1";
        config1["snmp"]["RSUMIBVersion"] = "4.1";
        config1["snmp"]["SecurityLevel"] = "authPriv";
        jsonArray.append(config1);

        Json::Value config2(config1);
        config2["rsu"]["IP"] = "192.168.1.11";
        jsonArray.append(config2);

        vector<rsuConfig> rsuList;
        bool result = rsuConfigMessageToVector(jsonArray, rsuList);

        ASSERT_TRUE(result);
        ASSERT_EQ(rsuList.size(), 2);
        ASSERT_EQ(rsuList[0].rsu.ip, "192.168.1.10");
        ASSERT_EQ(rsuList[1].rsu.ip, "192.168.1.11");
    }

    TEST_F(TestRSUConfigWorker, TestRsuConfigListToJsonArray)
    {
        vector<rsuConfig> rsuList;
        Json::Value result = rsuConfigListToJsonArray(rsuList);

        ASSERT_TRUE(result.isArray());
        ASSERT_EQ(result.size(), 0);

        rsuConfig config;
        config.action = "add";
        config.event = "startup";
        config.rsu.ip = "192.168.1.10";
        config.rsu.port = 161;
        config.snmp.userKey = "admin";
        config.snmp.privProtocol = "AES";
        rsuList.push_back(config);

        result = rsuConfigListToJsonArray(rsuList);

        ASSERT_TRUE(result.isArray());
        ASSERT_EQ(result.size(), 1);
        ASSERT_EQ(result[0]["rsu"]["IP"].asString(), "192.168.1.10");
        ASSERT_EQ(result[0]["action"].asString(), "add");
    }

    TEST_F(TestRSUConfigWorker, TestLoadRSUConfigListFromFileInvalidJSON)
    {
        string testPath = "/tmp/test_rsu_config_invalid.json";
        createTestConfigFile(testPath, "{invalid json}");

        vector<rsuConfig> rsuList;
        bool result = loadRSUConfigListFromFile(testPath, rsuList);

        ASSERT_FALSE(result);
        ASSERT_TRUE(rsuList.empty());

        removeTestFile(testPath);
    }

    TEST_F(TestRSUConfigWorker, TestLoadRSUConfigListFromFileSuccess)
    {
        string testPath = "/tmp/test_rsu_config_load.json";
        createTestConfigFile(testPath, getValidRSUConfigFileContent());

        vector<rsuConfig> rsuList;
        bool result = loadRSUConfigListFromFile(testPath, rsuList);

        ASSERT_TRUE(result);
        ASSERT_EQ(rsuList.size(), 1);
        ASSERT_EQ(rsuList[0].rsu.ip, "192.168.1.10");

        removeTestFile(testPath);
    }


}
