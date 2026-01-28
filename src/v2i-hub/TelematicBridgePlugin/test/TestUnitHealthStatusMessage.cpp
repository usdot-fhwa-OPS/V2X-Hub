#include <gtest/gtest.h>
#include "health_monitor/UnitHealthStatusMessage.h"

using namespace TelematicBridge;

class TestUnitHealthStatusMessage : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST(TestUnitHealthStatusMessage, DefaultConstructor)
{
    UnitHealthStatusMessage msg;
    
    EXPECT_EQ("", msg.getUnitId());
    EXPECT_EQ("", msg.getBridgePluginStatus());
    EXPECT_EQ(0, msg.getLastUpdatedTimestamp());
}

TEST(TestUnitHealthStatusMessage, SettersAndGetters)
{
    UnitHealthStatusMessage msg;
    
    msg.setUnitId("Unit001");
    msg.setBridgePluginStatus("running");
    msg.setLastUpdatedTimestamp(1769192434866);
    
    EXPECT_EQ("Unit001", msg.getUnitId());
    EXPECT_EQ("running", msg.getBridgePluginStatus());
    EXPECT_EQ(1769192434866, msg.getLastUpdatedTimestamp());
}

TEST(TestUnitHealthStatusMessage, ToString)
{
    UnitHealthStatusMessage msg;
    msg.setUnitId("Unit001");
    msg.setBridgePluginStatus("running");
    msg.setLastUpdatedTimestamp(1769192434866);
    
    std::string jsonStr = msg.toString();
    
    // Parse JSON
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream s(jsonStr);
    ASSERT_TRUE(Json::parseFromStream(reader, s, &root, &errs));
    
    EXPECT_EQ("Unit001", root["unitId"].asString());
    EXPECT_EQ("running", root["bridgePluginStatus"].asString());
    EXPECT_EQ(1769192434866, root["lastUpdatedTimestamp"].asInt64());
}

TEST(TestUnitHealthStatusMessage, FromJson)
{
    const char* jsonStr = R"({
        "unitId": "Unit001",
        "bridgePluginStatus": "stopped",
        "lastUpdatedTimestamp": 1769192434866
    })";
    
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream s(jsonStr);
    ASSERT_TRUE(Json::parseFromStream(reader, s, &root, &errs));
    
    auto msg = UnitHealthStatusMessage::fromJson(root);
    
    EXPECT_EQ("Unit001", msg.getUnitId());
    EXPECT_EQ("stopped", msg.getBridgePluginStatus());
    EXPECT_EQ(1769192434866, msg.getLastUpdatedTimestamp());
}

TEST(TestUnitHealthStatusMessage, ToJsonAndBack)
{
    UnitHealthStatusMessage original;
    original.setUnitId("Unit002");
    original.setBridgePluginStatus("initializing");
    original.setLastUpdatedTimestamp(1769192500000);
    
    // Convert to JSON string
    std::string jsonStr = original.toString();
    
    // Parse back
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream s(jsonStr);
    ASSERT_TRUE(Json::parseFromStream(reader, s, &root, &errs));
    
    auto recovered = UnitHealthStatusMessage::fromJson(root);
    
    EXPECT_EQ(original.getUnitId(), recovered.getUnitId());
    EXPECT_EQ(original.getBridgePluginStatus(), recovered.getBridgePluginStatus());
    EXPECT_EQ(original.getLastUpdatedTimestamp(), recovered.getLastUpdatedTimestamp());
}
