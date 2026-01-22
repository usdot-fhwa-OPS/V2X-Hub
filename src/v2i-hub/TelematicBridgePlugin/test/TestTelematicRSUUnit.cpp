#include <gtest/gtest.h>
#include "TelematicRsuUnit.h"
#include <fstream>
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

        void TearDown() override
        {
            unit.reset();
            remove("/tmp/test_config.json");
        }

        void createFile(const string& path, const string& content)
        {
            ofstream f(path);
            f << content;
            f.close();
        }

        string getValidConfig()
        {
            return R"({
                "unitConfig": [{"unitId": "Unit001"}],
                "rsuConfigs": [{
                    "action": "add",
                    "event": "startup",
                    "rsu": {"ip": "192.168.1.10", "port": 161},
                    "snmp": {
                        "user": "admin",
                        "privacyProtocol": "AES",
                        "authProtocol": "SHA",
                        "authPassPhrase": "pass",
                        "privacyPassPhrase": "priv",
                        "rsuMibVersion": "4.1",
                        "securityLevel": "authPriv"
                    }
                }],
                "timestamp": 1234567890
            })";
        }

        Json::Value getUpdateMsg()
        {
            Json::Value msg;
            msg["unitConfig"][0]["unitId"] = "Unit001";
            msg["rsuConfigs"][0]["action"] = "add";
            msg["rsuConfigs"][0]["event"] = "update";
            msg["rsuConfigs"][0]["rsu"]["ip"] = "192.168.1.20";
            msg["rsuConfigs"][0]["rsu"]["port"] = 161;
            msg["rsuConfigs"][0]["snmp"]["user"] = "admin";
            msg["rsuConfigs"][0]["snmp"]["privacyProtocol"] = "AES";
            msg["rsuConfigs"][0]["snmp"]["authProtocol"] = "SHA";
            msg["rsuConfigs"][0]["snmp"]["authPassPhrase"] = "pass";
            msg["rsuConfigs"][0]["snmp"]["privacyPassPhrase"] = "priv";
            msg["rsuConfigs"][0]["snmp"]["rsuMibVersion"] = "4.1";
            msg["rsuConfigs"][0]["snmp"]["securityLevel"] = "authPriv";
            msg["timestamp"] = 1234567890;
            return msg;
        }
    };

    TEST_F(TestTelematicRsuUnit, ConstructorNoConfig)
    {
        unit = make_shared<TelematicRsuUnit>();
        ASSERT_NE(unit, nullptr);
    }

    TEST_F(TestTelematicRsuUnit, ConstructorWithValidConfig)
    {
        createFile("/tmp/test_config.json", getValidConfig());
        setenv("RSU_CONFIG_PATH", "/tmp/test_config.json", 1);
        unit = make_shared<TelematicRsuUnit>();
        ASSERT_NE(unit, nullptr);
        unsetenv("RSU_CONFIG_PATH");
    }

    TEST_F(TestTelematicRsuUnit, ConstructorWithInvalidConfig)
    {
        createFile("/tmp/test_config.json", "{bad}");
        setenv("RSU_CONFIG_PATH", "/tmp/test_config.json", 1);
        unit = make_shared<TelematicRsuUnit>();
        ASSERT_NE(unit, nullptr);
        unsetenv("RSU_CONFIG_PATH");
    }

    TEST_F(TestTelematicRsuUnit, UpdateRSUStatusSuccess)
    {
        createFile("/tmp/test_config.json", getValidConfig());
        setenv("RSU_CONFIG_PATH", "/tmp/test_config.json", 1);
        unit = make_shared<TelematicRsuUnit>();
        ASSERT_TRUE(unit->updateRSUStatus(getUpdateMsg()));
        unsetenv("RSU_CONFIG_PATH");

        ASSERT_EQ(unit->getRsuConfigTopic(), "unit.Unit001.register.rsu.config");
    }

    TEST_F(TestTelematicRsuUnit, UpdateRSUStatusFail)
    {
        unit = make_shared<TelematicRsuUnit>();
        Json::Value bad;
        bad["timestamp"] = 1;
        ASSERT_FALSE(unit->updateRSUStatus(bad));
    }

    TEST_F(TestTelematicRsuUnit, ConstructRegistrationString)
    {
        unit = make_shared<TelematicRsuUnit>();
        string json = unit->constructRSURegistrationDataString();
        ASSERT_FALSE(json.empty());
        Json::Value root;
        Json::CharReaderBuilder builder;
        istringstream stream(json);
        string errs;
        ASSERT_TRUE(Json::parseFromStream(builder, stream, &root, &errs));
    }

    TEST_F(TestTelematicRsuUnit, ConstructResponseSuccess)
    {
        unit = make_shared<TelematicRsuUnit>();
        string json = unit->constructRSUConfigResponseDataString(true);
        Json::Value root;
        Json::CharReaderBuilder builder;
        istringstream stream(json);
        string errs;
        Json::parseFromStream(builder, stream, &root, &errs);
        ASSERT_EQ(root["status"].asString(), "success");
    }

    TEST_F(TestTelematicRsuUnit, ConstructResponseFail)
    {
        unit = make_shared<TelematicRsuUnit>();
        string json = unit->constructRSUConfigResponseDataString(false);
        Json::Value root;
        Json::CharReaderBuilder builder;
        istringstream stream(json);
        string errs;
        Json::parseFromStream(builder, stream, &root, &errs);
        ASSERT_EQ(root["status"].asString(), "failed");
    }

    TEST_F(TestTelematicRsuUnit, ThreadSafety)
    {
        unit = make_shared<TelematicRsuUnit>();
        vector<thread> threads;
        vector<string> results(5);
        for (int i = 0; i < 5; ++i)
            threads.emplace_back([this, i, &results]() {
                results[i] = unit->constructRSURegistrationDataString();
            });
        for (auto &t : threads)
            t.join();
        for (const auto &r : results)
            ASSERT_FALSE(r.empty());
    }

    TEST_F(TestTelematicRsuUnit, Destructor)
    {
        unit = make_shared<TelematicRsuUnit>();
        unit.reset();
    }

    TEST_F(TestTelematicRsuUnit, ProcessConfigUpdateSuccess)
    {
        createFile("/tmp/test_config.json", getValidConfig());
        setenv("RSU_CONFIG_PATH", "/tmp/test_config.json", 1);
        unit = make_shared<TelematicRsuUnit>();

        auto [success, response] = unit->processConfigUpdateAndGenerateResponse(getUpdateMsg());

        ASSERT_TRUE(success);
        ASSERT_FALSE(response.empty());

        Json::Value root;
        Json::CharReaderBuilder builder;
        istringstream stream(response);
        string errs;
        Json::parseFromStream(builder, stream, &root, &errs);
        ASSERT_EQ(root["status"].asString(), "success");

        unsetenv("RSU_CONFIG_PATH");
    }

    TEST_F(TestTelematicRsuUnit, ProcessConfigUpdateFailure)
    {
        unit = make_shared<TelematicRsuUnit>();

        Json::Value badMsg;
        badMsg["timestamp"] = 1;

        auto [success, response] = unit->processConfigUpdateAndGenerateResponse(badMsg);

        ASSERT_FALSE(success);
        ASSERT_FALSE(response.empty());

        Json::Value root;
        Json::CharReaderBuilder builder;
        istringstream stream(response);
        string errs;
        Json::parseFromStream(builder, stream, &root, &errs);
        ASSERT_EQ(root["status"].asString(), "failed");
    }
}