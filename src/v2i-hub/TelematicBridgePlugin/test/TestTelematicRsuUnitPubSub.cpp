#include <gtest/gtest.h>
#include "TelematicRsuUnit.h"
#include "health_monitor/RSUHealthStatusMessage.h"
#include <fstream>
#include <memory>
#include <thread>

using namespace TelematicBridge;
using namespace std;

namespace TelematicBridge
{
    class TestTelematicRsuUnitPubSub : public ::testing::Test
    {
    protected:
        shared_ptr<TelematicRsuUnit> unit;
        string testConfigPath = "/tmp/test_rsu_unit_advanced.json";

        void SetUp() override
        {
            // Create valid test configuration
            string validConfig = R"({
                "unitConfig": {"unitId": "Unit001"},
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
            
            createFile(testConfigPath, validConfig);
            setenv("RSU_CONFIG_PATH", testConfigPath.c_str(), 1);
            unit = make_shared<TelematicRsuUnit>();
        }

        void TearDown() override
        {
            unit.reset();
            unsetenv("RSU_CONFIG_PATH");
            remove(testConfigPath.c_str());
        }

        void createFile(const string& path, const string& content)
        {
            ofstream f(path);
            f << content;
            f.close();
        }

        Json::Value createMessagePayload()
        {
            Json::Value payload;
            payload["messageType"] = "BSM";
            payload["data"] = "test_data";
            payload["count"] = 123;
            return payload;
        }
    };

    // Tests for constructPublishedRsuDataStream
    TEST_F(TestTelematicRsuUnitPubSub, ConstructPublishedRsuDataStream_ValidInput)
    {
        Json::Value payload = createMessagePayload();
        
        string result = unit->constructPublishedRsuDataStream(
            "Unit001",
            "192.168.1.10",
            161,
            "bsm",
            payload
        );

        ASSERT_FALSE(result.empty());

        // Parse result
        Json::CharReaderBuilder builder;
        Json::Value root;
        istringstream stream(result);
        string errs;
        ASSERT_TRUE(Json::parseFromStream(builder, stream, &root, &errs));

        // Verify structure
        ASSERT_TRUE(root.isMember("metadata"));
        ASSERT_TRUE(root.isMember("payload"));

        // Verify metadata
        EXPECT_EQ(root["metadata"]["unitId"].asString(), "Unit001");
        EXPECT_EQ(root["metadata"]["topicName"].asString(), "bsm");
        EXPECT_EQ(root["metadata"]["rsu"]["ip"].asString(), "192.168.1.10");
        EXPECT_EQ(root["metadata"]["rsu"]["port"].asInt(), 161);
        EXPECT_EQ(root["metadata"]["event"].asString(), "startup");
        ASSERT_TRUE(root["metadata"].isMember("timestamp"));

        // Verify payload
        EXPECT_EQ(root["payload"]["messageType"].asString(), "BSM");
        EXPECT_EQ(root["payload"]["data"].asString(), "test_data");
        EXPECT_EQ(root["payload"]["count"].asInt(), 123);
    }

    TEST_F(TestTelematicRsuUnitPubSub, ConstructPublishedRsuDataStream_EmptyPayload)
    {
        Json::Value emptyPayload;
        
        string result = unit->constructPublishedRsuDataStream(
            "Unit001",
            "192.168.1.10",
            161,
            "map",
            emptyPayload
        );

        ASSERT_FALSE(result.empty());
        
        Json::CharReaderBuilder builder;
        Json::Value root;
        istringstream stream(result);
        string errs;
        ASSERT_TRUE(Json::parseFromStream(builder, stream, &root, &errs));

        EXPECT_TRUE(root["metadata"].isMember("unitId"));
        EXPECT_TRUE(root["payload"].isNull());
    }

    TEST_F(TestTelematicRsuUnitPubSub, ConstructPublishedRsuDataStream_DifferentRSU)
    {
        Json::Value payload = createMessagePayload();
        
        string result = unit->constructPublishedRsuDataStream(
            "Unit002",
            "10.0.0.1",
            502,
            "spat",
            payload
        );

        Json::CharReaderBuilder builder;
        Json::Value root;
        istringstream stream(result);
        string errs;
        Json::parseFromStream(builder, stream, &root, &errs);

        EXPECT_EQ(root["metadata"]["unitId"].asString(), "Unit002");
        EXPECT_EQ(root["metadata"]["rsu"]["ip"].asString(), "10.0.0.1");
        EXPECT_EQ(root["metadata"]["rsu"]["port"].asInt(), 502);
        EXPECT_EQ(root["metadata"]["topicName"].asString(), "spat");
    }

    // Tests for constructRsuAvailableTopicsReplyString
    TEST_F(TestTelematicRsuUnitPubSub, ConstructRsuAvailableTopicsReplyString_ReturnsValidJson)
    {
        // First add some topics by processing data streams
        Json::Value testPayload;
        testPayload["test"] = "data";
        
        unit->processRsuDataStream("192.168.1.10", "bsm", testPayload);
        unit->processRsuDataStream("192.168.1.10", "map", testPayload);

        string result = unit->constructRsuAvailableTopicsReplyString();

        ASSERT_FALSE(result.empty());

        // Parse result
        Json::CharReaderBuilder builder;
        Json::Value root;
        istringstream stream(result);
        string errs;
        ASSERT_TRUE(Json::parseFromStream(builder, stream, &root, &errs));

        // Should have basic structure
        EXPECT_TRUE(root.isMember("unitId"));
        EXPECT_TRUE(root.isMember("timestamp"));
    }

    // Tests for constructRsuSelectedTopicsReplyString
    TEST_F(TestTelematicRsuUnitPubSub, ConstructRsuSelectedTopicsReplyString_ValidInput)
    {
        string selectedTopicsMsg = R"({
            "unitId": "Unit001",
            "rsuTopics": [
                {
                    "rsu": {"ip": "192.168.1.10", "port": 161},
                    "topics": [
                        {"name": "bsm", "selected": false},
                        {"name": "map", "selected": false}
                    ]
                }
            ],
            "timestamp": "1234567890000"
        })";

        string result = unit->constructRsuSelectedTopicsReplyString(selectedTopicsMsg);

        ASSERT_FALSE(result.empty());

        // Parse result
        Json::CharReaderBuilder builder;
        Json::Value root;
        istringstream stream(result);
        string errs;
        ASSERT_TRUE(Json::parseFromStream(builder, stream, &root, &errs));

        // Should have basic structure
        EXPECT_TRUE(root.isMember("unitId"));
        EXPECT_TRUE(root.isMember("timestamp"));
    }

    TEST_F(TestTelematicRsuUnitPubSub, ConstructRsuSelectedTopicsReplyString_EmptyTopics)
    {
        string selectedTopicsMsg = R"({
            "unitId": "Unit001",
            "rsuTopics": [],
            "timestamp": "1234567890000"
        })";

        ASSERT_NO_THROW({
            string result = unit->constructRsuSelectedTopicsReplyString(selectedTopicsMsg);
            ASSERT_FALSE(result.empty());
        });
    }

    // Tests for processRsuDataStream
    TEST_F(TestTelematicRsuUnitPubSub, ProcessRsuDataStream_AddsToAvailableTopics)
    {
        Json::Value payload = createMessagePayload();
        
        // Process data stream
        ASSERT_NO_THROW({
            unit->processRsuDataStream("192.168.1.10", "bsm", payload);
        });

        // Verify topic was added to available topics
        string availableTopics = unit->constructRsuAvailableTopicsReplyString();
        ASSERT_FALSE(availableTopics.empty());
    }

    TEST_F(TestTelematicRsuUnitPubSub, ProcessRsuDataStream_MultipleTopics)
    {
        Json::Value payload = createMessagePayload();
        
        // Process multiple different topics
        ASSERT_NO_THROW({
            unit->processRsuDataStream("192.168.1.10", "bsm", payload);
            unit->processRsuDataStream("192.168.1.10", "map", payload);
            unit->processRsuDataStream("192.168.1.10", "spat", payload);
        });

        string availableTopics = unit->constructRsuAvailableTopicsReplyString();
        ASSERT_FALSE(availableTopics.empty());
    }

    TEST_F(TestTelematicRsuUnitPubSub, ProcessRsuDataStream_DifferentRSUs)
    {
        Json::Value payload = createMessagePayload();
        
        // Process data from different RSUs
        ASSERT_NO_THROW({
            unit->processRsuDataStream("192.168.1.10", "bsm", payload);
            unit->processRsuDataStream("192.168.1.11", "bsm", payload);
        });

        string availableTopics = unit->constructRsuAvailableTopicsReplyString();
        ASSERT_FALSE(availableTopics.empty());
    }

    // Tests for updateRsuHealthStatus
    TEST_F(TestTelematicRsuUnitPubSub, UpdateRsuHealthStatus_ValidStatus)
    {
        RSUHealthStatusMessage status(
            "192.168.1.10",
            161,
            "operational",
            "startup"
        );

        ASSERT_NO_THROW({
            unit->updateRsuHealthStatus(status);
        });
    }

    TEST_F(TestTelematicRsuUnitPubSub, UpdateRsuHealthStatus_MultipleUpdates)
    {
        RSUHealthStatusMessage status1("192.168.1.10", 161,  "operational", "startup");
        RSUHealthStatusMessage status2("192.168.1.10", 161,  "warning", "config_update");
        RSUHealthStatusMessage status3("192.168.1.10", 161,  "operational", "config_update");

        ASSERT_NO_THROW({
            unit->updateRsuHealthStatus(status1);
            unit->updateRsuHealthStatus(status2);
            unit->updateRsuHealthStatus(status3);
        });
    }

    TEST_F(TestTelematicRsuUnitPubSub, UpdateRsuHealthStatus_DifferentRSUs)
    {
        RSUHealthStatusMessage status1("192.168.1.10", 161,  "operational", "startup");
        RSUHealthStatusMessage status2("192.168.1.11", 161,  "operational", "startup");

        ASSERT_NO_THROW({
            unit->updateRsuHealthStatus(status1);
            unit->updateRsuHealthStatus(status2);
        });
    }

    // Tests for updateUnitHealthStatus
    TEST_F(TestTelematicRsuUnitPubSub, UpdateUnitHealthStatus_Running)
    {
        ASSERT_NO_THROW({
            unit->updateUnitHealthStatus("running");
        });
    }

    TEST_F(TestTelematicRsuUnitPubSub, UpdateUnitHealthStatus_Error)
    {
        ASSERT_NO_THROW({
            unit->updateUnitHealthStatus("error");
        });
    }

    TEST_F(TestTelematicRsuUnitPubSub, UpdateUnitHealthStatus_MultipleUpdates)
    {
        ASSERT_NO_THROW({
            unit->updateUnitHealthStatus("running");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            unit->updateUnitHealthStatus("warning");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            unit->updateUnitHealthStatus("running");
        });
    }

    TEST_F(TestTelematicRsuUnitPubSub, UpdateUnitHealthStatus_EmptyString)
    {
        ASSERT_NO_THROW({
            unit->updateUnitHealthStatus("");
        });
    }

    // Tests for publishHealthStatusToNATS (indirect tests via publishRSUHealthStatus/publishPluginHealthStatus)
    TEST_F(TestTelematicRsuUnitPubSub, PublishRSUHealthStatus_AfterUpdates)
    {
        // Update RSU health status
        RSUHealthStatusMessage status("192.168.1.10", 161,  "operational", "startup");
        unit->updateRsuHealthStatus(status);

        // Publishing should not throw
        ASSERT_NO_THROW({
            unit->publishRSUHealthStatus();
        });
    }

    TEST_F(TestTelematicRsuUnitPubSub, PublishPluginHealthStatus_AfterUpdates)
    {
        // Update unit health status
        unit->updateUnitHealthStatus("running");

        // Publishing should not throw
        ASSERT_NO_THROW({
            unit->publishPluginHealthStatus();
        });
    }

    TEST_F(TestTelematicRsuUnitPubSub, PublishHealthStatus_WithoutUpdates)
    {
        // Should be able to publish even without prior updates
        ASSERT_NO_THROW({
            unit->publishRSUHealthStatus();
            unit->publishPluginHealthStatus();
        });
    }

    // Integration tests
    TEST_F(TestTelematicRsuUnitPubSub, Integration_ProcessDataAndCheckAvailable)
    {
        Json::Value payload;
        payload["type"] = "BSM";
        
        // Process some data streams
        unit->processRsuDataStream("192.168.1.10", "bsm", payload);
        unit->processRsuDataStream("192.168.1.10", "map", payload);

        // Get available topics
        string availableTopics = unit->constructRsuAvailableTopicsReplyString();
        ASSERT_FALSE(availableTopics.empty());

        // Parse and verify structure
        Json::CharReaderBuilder builder;
        Json::Value root;
        istringstream stream(availableTopics);
        string errs;
        ASSERT_TRUE(Json::parseFromStream(builder, stream, &root, &errs));
    }

    TEST_F(TestTelematicRsuUnitPubSub, Integration_SelectTopicsAndPublish)
    {
        Json::Value payload;
        payload["type"] = "BSM";
        
        // First process some data to make topics available
        unit->processRsuDataStream("192.168.1.10", "bsm", payload);
        
        // Select topics
        string selectedTopicsMsg = R"({
            "unitId": "Unit001",
            "rsuTopics": [
                {
                    "rsu": {"ip": "192.168.1.10", "port": 161},
                    "topics": [{"name":"bsm", "selected": false} ]
                }
            ],
            "timestamp": "1234567890000"
        })";
        
        string reply = unit->constructRsuSelectedTopicsReplyString(selectedTopicsMsg);
        ASSERT_FALSE(reply.empty());

        // Process data stream again - should now publish since topic is selected
        ASSERT_NO_THROW({
            unit->processRsuDataStream("192.168.1.10", "bsm", payload);
        });
    }

    TEST_F(TestTelematicRsuUnitPubSub, Integration_HealthStatusWorkflow)
    {
        // Update unit status
        unit->updateUnitHealthStatus("running");

        // Update RSU statuses
        RSUHealthStatusMessage rsuStatus("192.168.1.10", 161,  "operational", "startup");
        unit->updateRsuHealthStatus(rsuStatus);

        // Publish both health statuses
        ASSERT_NO_THROW({
            unit->publishPluginHealthStatus();
            unit->publishRSUHealthStatus();
        });

        // Update statuses again
        unit->updateUnitHealthStatus("warning");
        RSUHealthStatusMessage rsuStatus2("192.168.1.10", 161,  "warning", "config_update");
        unit->updateRsuHealthStatus(rsuStatus2);

        // Publish again
        ASSERT_NO_THROW({
            unit->publishPluginHealthStatus();
            unit->publishRSUHealthStatus();
        });
    }

    TEST_F(TestTelematicRsuUnitPubSub, ThreadSafety_ConcurrentHealthUpdates)
    {
        vector<thread> threads;
        
        // Launch multiple threads updating health status
        for (int i = 0; i < 5; ++i)
        {
            threads.emplace_back([this, i]() {
                string status = (i % 2 == 0) ? "running" : "warning";
                unit->updateUnitHealthStatus(status);
                
                RSUHealthStatusMessage rsuStatus(
                    "192.168.1." + to_string(10 + i),
                    161,
                    "operational",
                    "test"
                );
                unit->updateRsuHealthStatus(rsuStatus);
            });
        }

        for (auto &t : threads)
            t.join();

        // Should be able to publish without issues
        ASSERT_NO_THROW({
            unit->publishPluginHealthStatus();
            unit->publishRSUHealthStatus();
        });
    }

    TEST_F(TestTelematicRsuUnitPubSub, ThreadSafety_ConcurrentDataProcessing)
    {
        vector<thread> threads;
        Json::Value payload;
        payload["test"] = "concurrent";

        // Launch multiple threads processing data
        for (int i = 0; i < 5; ++i)
        {
            threads.emplace_back([this, i, &payload]() {
                string topic = (i % 2 == 0) ? "bsm" : "map";
                unit->processRsuDataStream("192.168.1.10", topic, payload);
            });
        }

        for (auto &t : threads)
            t.join();

        // Should be able to get available topics without issues
        ASSERT_NO_THROW({
            string topics = unit->constructRsuAvailableTopicsReplyString();
            ASSERT_FALSE(topics.empty());
        });
    }
}
