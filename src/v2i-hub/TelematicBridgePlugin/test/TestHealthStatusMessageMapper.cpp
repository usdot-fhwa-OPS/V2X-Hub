#include <gtest/gtest.h>
#include "health_monitor/HealthStatusMessageMapper.h"
#include <tmx/messages/routeable_message.hpp>
#include <tmx/IvpMessage.h>
#include <tmx/json/cJSON.h>
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
    // Create IvpMessage with JSON payload
    std::string rsuStatusPayload = R"({"rsuIpAddress":"192.168.1.1","rsuSnmpPort":"161","event":"startup","rsuMode":"3","rsuID":"RSU-001"})";
    
    // Parse JSON string to cJSON object
    cJSON *payloadJson = cJSON_Parse(rsuStatusPayload.c_str());
    
    // Create IvpMessage with parsed JSON payload
    IvpMessage *ivpMsg = ivpMsg_create(
        nullptr,
        nullptr,
        IVP_ENCODING_JSON,
        IvpMsgFlags_None,
        payloadJson
    );
    
    // Construct routeable_message from IvpMessage (same as production code)
    tmx::routeable_message msg(ivpMsg);
    
    // Convert to RSUHealthStatusMessage
    auto rsuHealthStatus = HealthStatusMessageMapper::toRsuHealthStatusMessage(msg);
    
    // Verify
    EXPECT_EQ("192.168.1.1", rsuHealthStatus.getIp());
    EXPECT_EQ(161, rsuHealthStatus.getPort());
    EXPECT_EQ("operate", rsuHealthStatus.getStatus()); // "3" converts to "operate"
    EXPECT_EQ("startup", rsuHealthStatus.getEvent());
    
    // Cleanup
    ivpMsg_destroy(ivpMsg);
}

TEST_F(TestHealthStatusMessageMapper, ToRsuHealthStatusMessage_MissingRsuIp)
{
    std::string rsuStatusPayload = R"({"rsuSnmpPort":"161","event":"startup","rsuMode":"4"})";
    
    cJSON *payloadJson = cJSON_Parse(rsuStatusPayload.c_str());
    
    IvpMessage *ivpMsg = ivpMsg_create(
        nullptr,
        nullptr,
        IVP_ENCODING_JSON,
        IvpMsgFlags_None,
        payloadJson
    );
    
    tmx::routeable_message msg(ivpMsg);
    
    // Should throw because rsuIpAddress is missing
    EXPECT_THROW(HealthStatusMessageMapper::toRsuHealthStatusMessage(msg), std::runtime_error);
    
    ivpMsg_destroy(ivpMsg);
}

TEST_F(TestHealthStatusMessageMapper, ToRsuHealthStatusMessage_MissingPort)
{
    std::string rsuStatusPayload = R"({"rsuIpAddress":"192.168.1.1","event":"startup","rsuMode":"4"})";
    
    cJSON *payloadJson = cJSON_Parse(rsuStatusPayload.c_str());
    
    IvpMessage *ivpMsg = ivpMsg_create(
        nullptr,
        nullptr,
        IVP_ENCODING_JSON,
        IvpMsgFlags_None,
        payloadJson
    );
    
    tmx::routeable_message msg(ivpMsg);
    
    // Should throw because rsuSnmpPort is missing
    EXPECT_THROW(HealthStatusMessageMapper::toRsuHealthStatusMessage(msg), std::runtime_error);
    
    ivpMsg_destroy(ivpMsg);
}

TEST_F(TestHealthStatusMessageMapper, ToRsuHealthStatusMessage_DefaultValues)
{
    std::string rsuStatusPayload = R"({"rsuIpAddress":"192.168.1.1","rsuSnmpPort":"161"})";
    
    cJSON *payloadJson = cJSON_Parse(rsuStatusPayload.c_str());
    
    IvpMessage *ivpMsg = ivpMsg_create(
        nullptr,
        nullptr,
        IVP_ENCODING_JSON,
        IvpMsgFlags_None,
        payloadJson
    );
    
    tmx::routeable_message msg(ivpMsg);
    
    auto rsuHealthStatus = HealthStatusMessageMapper::toRsuHealthStatusMessage(msg);
    
    EXPECT_EQ("192.168.1.1", rsuHealthStatus.getIp());
    EXPECT_EQ(161, rsuHealthStatus.getPort());
    EXPECT_EQ("unknown", rsuHealthStatus.getStatus()); // "0" (default) converts to "unknown"
    EXPECT_EQ("", rsuHealthStatus.getEvent());    // Default when event missing
    
    ivpMsg_destroy(ivpMsg);
}

TEST_F(TestHealthStatusMessageMapper, ToRsuHealthStatusMessage_InvalidJson)
{
    std::string invalidPayload = "{ invalid json }";
    
    cJSON *payloadJson = cJSON_Parse(invalidPayload.c_str());
    
    IvpMessage *ivpMsg = ivpMsg_create(
        nullptr,
        nullptr,
        IVP_ENCODING_JSON,
        IvpMsgFlags_None,
        payloadJson
    );
    
    tmx::routeable_message msg(ivpMsg);
    
    EXPECT_THROW(HealthStatusMessageMapper::toRsuHealthStatusMessage(msg), std::runtime_error);
    
    ivpMsg_destroy(ivpMsg);
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

