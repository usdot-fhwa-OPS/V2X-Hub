#include <gtest/gtest.h>
#include "health_monitor/RSUHealthStatusMessage.h"

using namespace TelematicBridge;

class TestRSUHealthStatusMessage : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST(TestRSUHealthStatusMessage, Constructor)
{
    RSUHealthStatusMessage msg("192.168.1.1", 161, "1", "startup");
    
    EXPECT_EQ("192.168.1.1", msg.getIp());
    EXPECT_EQ(161, msg.getPort());
    EXPECT_EQ("other", msg.getStatus());
}

TEST(TestRSUHealthStatusMessage, ToString)
{
    RSUHealthStatusMessage msg("192.168.1.1", 161, "2", "startup");
    
    std::string jsonStr = msg.toString();
    
    // Parse JSON
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream s(jsonStr);
    ASSERT_TRUE(Json::parseFromStream(reader, s, &root, &errs));
    
    EXPECT_EQ("startup", root["event"].asString());
    EXPECT_EQ("standby", root["status"].asString());
    EXPECT_TRUE(root.isMember("rsu"));
    EXPECT_EQ("192.168.1.1", root["rsu"]["ip"].asString());
    EXPECT_EQ(161, root["rsu"]["port"].asInt());
}

TEST(TestRSUHealthStatusMessage, EqualityOperator)
{
    RSUHealthStatusMessage msg1("192.168.1.1", 161, "2", "startup");
    RSUHealthStatusMessage msg2("192.168.1.1", 161, "2", "startup");
    RSUHealthStatusMessage msg3("192.168.1.2", 161, "2", "startup");
    
    EXPECT_TRUE(msg1 == msg2);
    EXPECT_FALSE(msg1 == msg3);
}

TEST(TestRSUHealthStatusMessage, GetRsuId)
{
    RSUHealthStatusMessage msg("192.168.1.1", 161, "2", "startup");
    
    EXPECT_EQ("192.168.1.1:161", msg.getRsuId());
}

TEST(TestRSUHealthStatusMessage, DifferentPorts)
{
    RSUHealthStatusMessage msg1("192.168.1.1", 161, "2", "startup");
    RSUHealthStatusMessage msg2("192.168.1.1", 1610, "3", "startup");
    
    EXPECT_NE(msg1.getRsuId(), msg2.getRsuId());
    EXPECT_FALSE(msg1 == msg2);
}
