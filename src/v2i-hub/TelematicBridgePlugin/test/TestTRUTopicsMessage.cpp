#include <gtest/gtest.h>
#include "data_selection/TRUTopicsMessage.h"

using namespace TelematicBridge;

class TestTRUTopicsMessage : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST(TestTRUTopicsMessage, Constructor)
{
    TRUTopicsMessage msg;
    
    EXPECT_TRUE(msg.getRsuTopics().empty());
    EXPECT_EQ("", msg.getUnitId());
}

TEST(TestTRUTopicsMessage, SettersAndGetters)
{
    TRUTopicsMessage msg;
    
    msg.setUnitId("Unit001");
    msg.setTimestamp(1769192434866);
    
    EXPECT_EQ("Unit001", msg.getUnitId());
    EXPECT_EQ(1769192434866, msg.getTimestamp());
}

TEST(TestTRUTopicsMessage, AddRsuTopics)
{
    TRUTopicsMessage msg;
    msg.setUnitId("Unit001");
    
    RSUTopicsMessage rsuMsg;
    rsuMsg.setRsuEndpoint({"192.168.1.1", 161});
    
    TopicMessage topic1;
    topic1.setName("Application_BSM_MessageReceiver");
    topic1.setSelected(true);
    
    TopicMessage topic2;
    topic2.setName("Application_MAP_MessageReceiver");
    topic2.setSelected(false);
    
    rsuMsg.addTopic(topic1);
    rsuMsg.addTopic(topic2);
    
    msg.addRsuTopic(rsuMsg);
    
    auto rsuTopics = msg.getRsuTopics();
    ASSERT_EQ(1, rsuTopics.size());
    
    auto topics = rsuTopics[0].getTopics();
    ASSERT_EQ(2, topics.size());
    EXPECT_EQ("Application_BSM_MessageReceiver", topics[0].getName());
    EXPECT_TRUE(topics[0].isSelected());
    EXPECT_EQ("Application_MAP_MessageReceiver", topics[1].getName());
    EXPECT_FALSE(topics[1].isSelected());
}

TEST(TestTRUTopicsMessage, ToString)
{
    TRUTopicsMessage msg;
    msg.setUnitId("Unit001");
    msg.setTimestamp(1769192434866);
    
    RSUTopicsMessage rsuMsg;
    rsuMsg.setRsuEndpoint({"192.168.1.1", 161});
    
    TopicMessage topic;
    topic.setName("Application_RSUStatus_RSUHealthMonitor");
    topic.setSelected(true);
    rsuMsg.addTopic(topic);
    
    msg.addRsuTopic(rsuMsg);
    
    std::string jsonStr = msg.toString();
    
    // Parse JSON
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream s(jsonStr);
    ASSERT_TRUE(Json::parseFromStream(reader, s, &root, &errs));
    
    EXPECT_EQ("Unit001", root["unitId"].asString());
    EXPECT_EQ("1769192434866", root["timestamp"].asString());
    
    ASSERT_TRUE(root["rsuTopics"].isArray());
    ASSERT_EQ(1, root["rsuTopics"].size());
    
    auto rsuTopic = root["rsuTopics"][0];
    EXPECT_EQ("192.168.1.1", rsuTopic["rsuEndpoint"]["ip"].asString());
    EXPECT_EQ(161, rsuTopic["rsuEndpoint"]["port"].asInt());
    
    ASSERT_TRUE(rsuTopic["topics"].isArray());
    ASSERT_EQ(1, rsuTopic["topics"].size());
    EXPECT_EQ("Application_RSUStatus_RSUHealthMonitor", rsuTopic["topics"][0]["name"].asString());
    EXPECT_TRUE(rsuTopic["topics"][0]["selected"].asBool());
}

TEST(TestTRUTopicsMessage, FromJson)
{
    const char* jsonStr = R"({
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
    
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream s(jsonStr);
    ASSERT_TRUE(Json::parseFromStream(reader, s, &root, &errs));
    
    auto msg = TRUTopicsMessage::fromJson(root);
    
    EXPECT_EQ("Unit001", msg.getUnitId());
    EXPECT_EQ(1769192434866, msg.getTimestamp());
    
    auto rsuTopics = msg.getRsuTopics();
    ASSERT_EQ(1, rsuTopics.size());
    
    EXPECT_EQ("192.168.1.1", rsuTopics[0].getRsuEndpoint().ip);
    EXPECT_EQ(161, rsuTopics[0].getRsuEndpoint().port);
    
    auto topics = rsuTopics[0].getTopics();
    ASSERT_EQ(1, topics.size());
    EXPECT_EQ("Application_BSM_MessageReceiver", topics[0].getName());
    EXPECT_TRUE(topics[0].isSelected());


    jsonStr = R"({
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
        "timestamp": 1769192434866,
        "unitId": "Unit001"
    })";

    std::istringstream ss(jsonStr);
    ASSERT_TRUE(Json::parseFromStream(reader, ss, &root, &errs));    
    msg = TRUTopicsMessage::fromJson(root);
    EXPECT_EQ(1769192434866, msg.getTimestamp());
}

TEST(TestTRUTopicsMessage, SetCurrentTimestamp)
{
    TRUTopicsMessage msg;
    
    auto timestampBefore = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    msg.setCurrentTimestamp();
    
    auto timestampAfter = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // Verify timestamp is set to current time (within a small range)
    EXPECT_GE(msg.getTimestamp(), timestampBefore);
    EXPECT_LE(msg.getTimestamp(), timestampAfter);
}

TEST(TestTRUTopicsMessage, MultipleRSUs)
{
    TRUTopicsMessage msg;
    msg.setUnitId("Unit001");
    
    // Add first RSU
    RSUTopicsMessage rsu1;
    rsu1.setRsuEndpoint({"192.168.1.1", 161});
    TopicMessage topic1;
    topic1.setName("Application_BSM_MessageReceiver");
    topic1.setSelected(true);
    rsu1.addTopic(topic1);
    msg.addRsuTopic(rsu1);
    
    // Add second RSU
    RSUTopicsMessage rsu2;
    rsu2.setRsuEndpoint({"192.168.1.2", 1610});
    TopicMessage topic2;
    topic2.setName("Application_MAP_MessageReceiver");
    topic2.setSelected(true);
    rsu2.addTopic(topic2);
    msg.addRsuTopic(rsu2);
    
    auto rsuTopics = msg.getRsuTopics();
    ASSERT_EQ(2, rsuTopics.size());
    
    EXPECT_EQ("192.168.1.1", rsuTopics[0].getRsuEndpoint().ip);
    EXPECT_EQ("192.168.1.2", rsuTopics[1].getRsuEndpoint().ip);
}

