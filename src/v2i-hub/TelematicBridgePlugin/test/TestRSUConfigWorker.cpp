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
            config[ACTION_KEY] = "add";
            config[EVENT_KEY] = "startup";
            config[RSU_KEY][IP_KEY] = ip;
            config[RSU_KEY][PORT_KEY] = std::stoi(port);
            config[SNMP_KEY][USER_KEY] = "admin";
            config[SNMP_KEY][PRIVACY_PROTOCOL_KEY] = "AES";
            config[SNMP_KEY][AUTH_PROTOCOL_KEY] = "SHA";
            config[SNMP_KEY][AUTH_PASS_PHRASE_KEY] = "pass123";
            config[SNMP_KEY][PRIVACY_PASS_PHRASE_KEY] = "priv123";
            config[SNMP_KEY][RSU_MIB_VERSION_KEY] = "4.1";
            config[SNMP_KEY][SECURITY_LEVEL_KEY] = "authPriv";
            return config;
        }

        string getValidRSUConfigFileContent()
        {
            return R"({
                "rsuConfigs": [
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
        rsuConfigJson2[RSU_KEY][IP_KEY] = "192.168.1.10";
        rsuConfigJson2[RSU_KEY][PORT_KEY] = 161;
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

        ASSERT_EQ(result[ACTION_KEY].asString(), "add");
        ASSERT_EQ(result[EVENT_KEY].asString(), "startup");
        ASSERT_EQ(result[RSU_KEY][IP_KEY].asString(), "192.168.1.10");
        ASSERT_EQ(result[RSU_KEY][PORT_KEY].asString(), "161");
        ASSERT_EQ(result[SNMP_KEY][USER_KEY].asString(), "admin");
        ASSERT_EQ(result[SNMP_KEY][PRIVACY_PROTOCOL_KEY].asString(), "AES");
        ASSERT_EQ(result[SNMP_KEY][AUTH_PROTOCOL_KEY].asString(), "SHA");
    }

    TEST_F(TestRSUConfigWorker, TestJsonValueToRsuConfigSuccess)
    {
        Json::Value json;
        json[ACTION_KEY] = "add";
        json[EVENT_KEY] = "startup";
        json[RSU_KEY][IP_KEY] = "192.168.1.10";
        json[RSU_KEY][PORT_KEY] = 161;
        json[SNMP_KEY][USER_KEY] = "admin";
        json[SNMP_KEY][PRIVACY_PROTOCOL_KEY] = "AES";
        json[SNMP_KEY][AUTH_PROTOCOL_KEY] = "SHA";
        json[SNMP_KEY][AUTH_PASS_PHRASE_KEY] = "pass123";
        json[SNMP_KEY][PRIVACY_PASS_PHRASE_KEY] = "priv123";
        json[SNMP_KEY][RSU_MIB_VERSION_KEY] = "4.1";
        json[SNMP_KEY][SECURITY_LEVEL_KEY] = "authPriv";

        rsuConfig config;
        bool result = jsonValueToRsuConfig(json, config);

        ASSERT_TRUE(result);
        ASSERT_EQ(config.rsu.ip, "192.168.1.10");
        ASSERT_EQ(config.rsu.port, 161);
        ASSERT_EQ(config.action, "add");
        ASSERT_EQ(config.event, "startup");
        ASSERT_EQ(config.snmp.userKey, "admin");

        Json::Value json2;
        json2[ACTION_KEY] = "add";
        json2[EVENT_KEY] = "startup";
        json2[SNMP_KEY][USER_KEY] = "admin";
        rsuConfig config2;
        result = jsonValueToRsuConfig(json2, config2);
        ASSERT_FALSE(result);
    }

    TEST_F(TestRSUConfigWorker, TestRsuConfigMessageToVectorSuccess)
    {
        Json::Value jsonArray(Json::arrayValue);

        Json::Value config1;
        config1[ACTION_KEY] = "add";
        config1[EVENT_KEY] = "startup";
        config1[RSU_KEY][IP_KEY] = "192.168.1.10";
        config1[RSU_KEY][PORT_KEY] = 161;
        config1[SNMP_KEY][USER_KEY] = "admin1";
        config1[SNMP_KEY][PRIVACY_PROTOCOL_KEY] = "AES";
        config1[SNMP_KEY][AUTH_PROTOCOL_KEY] = "SHA";
        config1[SNMP_KEY][AUTH_PASS_PHRASE_KEY] = "pass1";
        config1[SNMP_KEY][PRIVACY_PASS_PHRASE_KEY] = "priv1";
        config1[SNMP_KEY][RSU_MIB_VERSION_KEY] = "4.1";
        config1[SNMP_KEY][SECURITY_LEVEL_KEY] = "authPriv";
        jsonArray.append(config1);

        Json::Value config2(config1);
        config2[RSU_KEY][IP_KEY] = "192.168.1.11";
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
        ASSERT_EQ(result[0][RSU_KEY][IP_KEY].asString(), "192.168.1.10");
        ASSERT_EQ(result[0][ACTION_KEY].asString(), "add");
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
