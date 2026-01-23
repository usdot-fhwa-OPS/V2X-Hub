#include <gtest/gtest.h>
#include "data_selection/DataSelectionTracker.h"
#include <jsoncpp/json/json.h>

using namespace TelematicBridge;

class TestDataSelectionTracker : public ::testing::Test
{
protected:
    std::unique_ptr<DataSelectionTracker> tracker;

    void SetUp() override
    {
        tracker = std::make_unique<DataSelectionTracker>();
    }

    void TearDown() override
    {
        tracker.reset();
    }
};

TEST_F(TestDataSelectionTracker, UpdateRsuAvailableTopics)
{
    // Add available topics for an RSU
    tracker->updateRsuAvailableTopics("192.168.1.1", 161, "Application_BSM_MessageReceiver", "Unit001");
    tracker->updateRsuAvailableTopics("192.168.1.1", 161, "Application_MAP_MessageReceiver", "Unit001");
    
    // Get available topics as JSON string
    std::string availableTopicsJson = tracker->latestAvailableTopicsMessageToJsonString();
    
    // Parse and verify
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream s(availableTopicsJson);
    ASSERT_TRUE(Json::parseFromStream(reader, s, &root, &errs));
    
    ASSERT_TRUE(root.isMember("unitId"));
    EXPECT_EQ("Unit001", root["unitId"].asString());
    
    ASSERT_TRUE(root.isMember("rsuTopics"));
    ASSERT_TRUE(root["rsuTopics"].isArray());
    ASSERT_GT(root["rsuTopics"].size(), 0);
    
    // Verify RSU endpoint
    auto rsuTopic = root["rsuTopics"][0];
    ASSERT_TRUE(rsuTopic.isMember("rsuEndpoint"));
    EXPECT_EQ("192.168.1.1", rsuTopic["rsuEndpoint"]["ip"].asString());
    EXPECT_EQ(161, rsuTopic["rsuEndpoint"]["port"].asInt());
}

TEST_F(TestDataSelectionTracker, InRsuSelectedTopics)
{
    // Initially, no topics are selected
    EXPECT_FALSE(tracker->inRsuSelectedTopics("192.168.1.1", "Application_BSM_MessageReceiver"));
    
    // Update selected topics
    const char* selectedTopicsJson = R"({
        "rsuTopics": [{
            "rsuEndpoint": {
                "ip": "192.168.1.1",
                "port": 161
            },
            "topics": [{
                "name": "Application_BSM_MessageReceiver",
                "selected": true
            }]
        }],
        "timestamp": "1769192434866",
        "unitId": "Unit001"
    })";
    
    tracker->updateLatestSelectedTopics(selectedTopicsJson);
    
    // Now the topic should be selected
    EXPECT_TRUE(tracker->inRsuSelectedTopics("192.168.1.1", "Application_BSM_MessageReceiver"));
    EXPECT_FALSE(tracker->inRsuSelectedTopics("192.168.1.1", "Application_MAP_MessageReceiver"));
}

TEST_F(TestDataSelectionTracker, UpdateLatestSelectedTopics_MultipleRSUs)
{
    const char* selectedTopicsJson = R"({
        "rsuTopics": [
            {
                "rsuEndpoint": {
                    "ip": "192.168.1.1",
                    "port": 161
                },
                "topics": [{
                    "name": "Application_BSM_MessageReceiver",
                    "selected": true
                }]
            },
            {
                "rsuEndpoint": {
                    "ip": "192.168.1.2",
                    "port": 161
                },
                "topics": [{
                    "name": "Application_MAP_MessageReceiver",
                    "selected": true
                }]
            }
        ],
        "timestamp": "1769192434866",
        "unitId": "Unit001"
    })";
    
    tracker->updateLatestSelectedTopics(selectedTopicsJson);
    
    // Verify topics for different RSUs
    EXPECT_TRUE(tracker->inRsuSelectedTopics("192.168.1.1", "Application_BSM_MessageReceiver"));
    EXPECT_FALSE(tracker->inRsuSelectedTopics("192.168.1.1", "Application_MAP_MessageReceiver"));
    
    EXPECT_TRUE(tracker->inRsuSelectedTopics("192.168.1.2", "Application_MAP_MessageReceiver"));
    EXPECT_FALSE(tracker->inRsuSelectedTopics("192.168.1.2", "Application_BSM_MessageReceiver"));
}

TEST_F(TestDataSelectionTracker, UpdateLatestSelectedTopics_ClearsPreviousSelections)
{
    // Select initial topics
    const char* initialJson = R"({
        "rsuTopics": [{
            "rsuEndpoint": {
                "ip": "192.168.1.1",
                "port": 161
            },
            "topics": [{
                "name": "Application_BSM_MessageReceiver",
                "selected": true
            }]
        }],
        "timestamp": "1769192434866",
        "unitId": "Unit001"
    })";
    
    tracker->updateLatestSelectedTopics(initialJson);
    EXPECT_TRUE(tracker->inRsuSelectedTopics("192.168.1.1", "Application_BSM_MessageReceiver"));
    
    // Update with new selection (should clear old ones)
    const char* updatedJson = R"({
        "rsuTopics": [{
            "rsuEndpoint": {
                "ip": "192.168.1.1",
                "port": 161
            },
            "topics": [{
                "name": "Application_MAP_MessageReceiver",
                "selected": true
            }]
        }],
        "timestamp": "1769192434867",
        "unitId": "Unit001"
    })";
    
    tracker->updateLatestSelectedTopics(updatedJson);
    
    // Old topic should no longer be selected
    EXPECT_FALSE(tracker->inRsuSelectedTopics("192.168.1.1", "Application_BSM_MessageReceiver"));
    EXPECT_TRUE(tracker->inRsuSelectedTopics("192.168.1.1", "Application_MAP_MessageReceiver"));
}

TEST_F(TestDataSelectionTracker, UpdateLatestSelectedTopics_InvalidJson)
{
    const char* invalidJson = "{ invalid json }";
    
    EXPECT_THROW(tracker->updateLatestSelectedTopics(invalidJson), std::runtime_error);
}

TEST_F(TestDataSelectionTracker, LatestSelectedTopicsMessageToJsonString)
{
    const char* selectedTopicsJson = R"({
        "rsuTopics": [{
            "rsuEndpoint": {
                "ip": "192.168.1.1",
                "port": 161
            },
            "topics": [{
                "name": "Application_RSUStatus_RSUHealthMonitor",
                "selected": true
            }]
        }],
        "timestamp": "1769192434866",
        "unitId": "Unit001"
    })";
    
    tracker->updateLatestSelectedTopics(selectedTopicsJson);
    
    std::string result = tracker->latestSelectedTopicsMessageToJsonString();
    
    // Parse result
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream s(result);
    ASSERT_TRUE(Json::parseFromStream(reader, s, &root, &errs));
    
    EXPECT_TRUE(root.isMember("unitId"));
    EXPECT_EQ("Unit001", root["unitId"].asString());
    EXPECT_TRUE(root.isMember("timestamp"));
    EXPECT_TRUE(root.isMember("rsuTopics"));
}
