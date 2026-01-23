#include <gtest/gtest.h>
#include "health_monitor/HealthStatusMessageMapper.h"
#include <tmx/messages/routeable_message.hpp>
#include <jsoncpp/json/json.h>

using namespace TelematicBridge;

class TestHealthStatusMessageMapper : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TestHealthStatusMessageMapper, ToRsuHealthStatusMessage_ValidInput)
{
    // Create a mock routeable message with RSU status data
    std::string rsuStatusPayload = R"({
        "rsuIpAddress": "192.168.1.1",
        "rsuSnmpPort": "161",
        "event": "startup",
        "rsuMode": "4",
        "rsuID": "RSU-001"
    })";
    
    tmx::routeable_message msg;
    msg.set_payload(rsuStatusPayload);
    
    // Convert to RSUHealthStatusMessage
    auto rsuHealthStatus = HealthStatusMessageMapper::toRsuHealthStatusMessage(msg);
    
    // Verify
    EXPECT_EQ("192.168.1.1", rsuHealthStatus.getIp());
    EXPECT_EQ(161, rsuHealthStatus.getPort());
    EXPECT_EQ("4", rsuHealthStatus.getStatus());
    EXPECT_EQ("startup", rsuHealthStatus.getEvent());
}

TEST_F(TestHealthStatusMessageMapper, ToRsuHealthStatusMessage_MissingRsuIp)
{
    std::string rsuStatusPayload = R"({
        "rsuSnmpPort": "161",
        "event": "startup",
        "rsuMode": "4"
    })";
    
    tmx::routeable_message msg;
    msg.set_payload(rsuStatusPayload);
    
    // Should throw because rsuIpAddress is missing
    EXPECT_THROW(HealthStatusMessageMapper::toRsuHealthStatusMessage(msg), std::runtime_error);
}

TEST_F(TestHealthStatusMessageMapper, ToRsuHealthStatusMessage_MissingPort)
{
    std::string rsuStatusPayload = R"({
        "rsuIpAddress": "192.168.1.1",
        "event": "startup",
        "rsuMode": "4"
    })";
    
    tmx::routeable_message msg;
    msg.set_payload(rsuStatusPayload);
    
    // Should throw because rsuSnmpPort is missing
    EXPECT_THROW(HealthStatusMessageMapper::toRsuHealthStatusMessage(msg), std::runtime_error);
}

TEST_F(TestHealthStatusMessageMapper, ToRsuHealthStatusMessage_DefaultValues)
{
    // Test with minimal required fields
    std::string rsuStatusPayload = R"({
        "rsuIpAddress": "192.168.1.1",
        "rsuSnmpPort": "161"
    })";
    
    tmx::routeable_message msg;
    msg.set_payload(rsuStatusPayload);
    
    auto rsuHealthStatus = HealthStatusMessageMapper::toRsuHealthStatusMessage(msg);
    
    EXPECT_EQ("192.168.1.1", rsuHealthStatus.getIp());
    EXPECT_EQ(161, rsuHealthStatus.getPort());
    EXPECT_EQ("0", rsuHealthStatus.getStatus()); // Default when rsuMode missing
    EXPECT_EQ("", rsuHealthStatus.getEvent());    // Default when event missing
}

TEST_F(TestHealthStatusMessageMapper, ToRsuHealthStatusMessage_InvalidJson)
{
    std::string invalidPayload = "{ invalid json }";
    
    tmx::routeable_message msg;
    msg.set_payload(invalidPayload);
    
    EXPECT_THROW(HealthStatusMessageMapper::toRsuHealthStatusMessage(msg), std::runtime_error);
}

TEST_F(TestHealthStatusMessageMapper, ToUnitHealthStatusMessage)
{
    std::string unitId = "Unit001";
    std::string status = "running";
    int64_t timestamp = 1769192434866;
    
    auto unitStatus = HealthStatusMessageMapper::toUnitHealthStatusMessage(unitId, status, timestamp);
    
    EXPECT_EQ("Unit001", unitStatus.getUnitId());
    EXPECT_EQ("running", unitStatus.getBridgePluginStatus());
    EXPECT_EQ(1769192434866, unitStatus.getLastUpdatedTimestamp());
}

TEST_F(TestHealthStatusMessageMapper, ToUnitHealthStatusMessage_EmptyUnitId)
{
    std::string unitId = "";
    std::string status = "running";
    int64_t timestamp = 1769192434866;
    
    auto unitStatus = HealthStatusMessageMapper::toUnitHealthStatusMessage(unitId, status, timestamp);
    
    // Should default to "unknown" when unitId is empty
    EXPECT_EQ("unknown", unitStatus.getUnitId());
}

