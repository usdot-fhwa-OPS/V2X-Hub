#include <gtest/gtest.h>
#include "health_monitor/TRUHealthStatusTracker.h"
#include "health_monitor/RSUHealthStatusMessage.h"
#include "health_monitor/UnitHealthStatusMessage.h"

using namespace TelematicBridge;

class TestTRUHealthStatusTracker : public ::testing::Test
{
protected:
    std::unique_ptr<TRUHealthStatusTracker> tracker;

    void SetUp() override
    {
        tracker = std::make_unique<TRUHealthStatusTracker>();
    }

    void TearDown() override
    {
        tracker.reset();
    }
};

TEST_F(TestTRUHealthStatusTracker, UpdateRsuStatus)
{
    RSUHealthStatusMessage rsuStatus("192.168.1.1", 161, "2", "startup");
    
    tracker->updateRsuStatus(rsuStatus);
    
    // Get snapshot and verify
    auto snapshot = tracker->getSnapshot();
    
    ASSERT_EQ(1, snapshot.getRsuHealthStatus().size());
}

TEST_F(TestTRUHealthStatusTracker, UpdateRsuStatus_MultipleRSUs)
{
    RSUHealthStatusMessage rsu1("192.168.1.1", 161, "2", "startup");
    RSUHealthStatusMessage rsu2("192.168.1.2", 161, "2", "startup");
    RSUHealthStatusMessage rsu3("192.168.1.3", 1610, "3", "update");
    
    tracker->updateRsuStatus(rsu1);
    tracker->updateRsuStatus(rsu2);
    tracker->updateRsuStatus(rsu3);
    
    auto snapshot = tracker->getSnapshot();
    
    EXPECT_EQ(3, snapshot.getRsuHealthStatus().size());
}

TEST_F(TestTRUHealthStatusTracker, UpdateRsuStatus_UpdateExisting)
{
    RSUHealthStatusMessage rsuInitial("192.168.1.1", 161, "2", "startup");
    tracker->updateRsuStatus(rsuInitial);
    
    // Update the same RSU with different status
    RSUHealthStatusMessage rsuUpdated("192.168.1.1", 161, "3", "maintenance");
    tracker->updateRsuStatus(rsuUpdated);
    
    auto snapshot = tracker->getSnapshot();
    
    // Should still have only one RSU (updated, not added)
    ASSERT_EQ(1, snapshot.getRsuHealthStatus().size());
    EXPECT_EQ("operate", snapshot.getRsuHealthStatus()[0].getStatus());
    EXPECT_EQ("maintenance", snapshot.getRsuHealthStatus()[0].toJson()["event"].asString());
}


TEST_F(TestTRUHealthStatusTracker, UpdateUnitStatus)
{
    UnitHealthStatusMessage unitStatus;
    unitStatus.setUnitId("Unit001");
    unitStatus.setBridgePluginStatus("running");
    unitStatus.setLastUpdatedTimestamp(1769192434866);
    
    tracker->updateUnitStatus(unitStatus);
    
    auto snapshot = tracker->getSnapshot();
    
    EXPECT_EQ("Unit001", snapshot.getUnitHealthStatus().getUnitId());
    EXPECT_EQ("running", snapshot.getUnitHealthStatus().getBridgePluginStatus());
    EXPECT_EQ(1769192434866, snapshot.getUnitHealthStatus().getLastUpdatedTimestamp());
}

TEST_F(TestTRUHealthStatusTracker, GetSnapshot_ThreadSafety)
{
    // Add some data
    RSUHealthStatusMessage rsu("192.168.1.1", 161, "2", "startup");
    tracker->updateRsuStatus(rsu);
    
    UnitHealthStatusMessage unit;
    unit.setUnitId("Unit001");
    unit.setBridgePluginStatus("running");
    tracker->updateUnitStatus(unit);
    
    // Get multiple snapshots (simulating concurrent access)
    auto snapshot1 = tracker->getSnapshot();
    
    // Both snapshots should be valid and contain the same data
    EXPECT_EQ(1, snapshot1.getRsuHealthStatus().size());
    
    EXPECT_EQ("Unit001", snapshot1.getUnitHealthStatus().getUnitId());
}

TEST_F(TestTRUHealthStatusTracker, ToString)
{
    RSUHealthStatusMessage rsu("192.168.1.1", 161, "2", "startup");
    tracker->updateRsuStatus(rsu);
    
    UnitHealthStatusMessage unit;
    unit.setUnitId("Unit001");
    unit.setBridgePluginStatus("running");
    unit.setLastUpdatedTimestamp(1769192434866);
    tracker->updateUnitStatus(unit);
    
    auto snapshot = tracker->getSnapshot();
    std::string jsonStr = snapshot.toString();
    
    // Parse and verify JSON structure
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::string errs;
    std::istringstream s(jsonStr);
    ASSERT_TRUE(Json::parseFromStream(reader, s, &root, &errs));
    
    EXPECT_TRUE(root.isMember("unitConfig"));
    EXPECT_TRUE(root.isMember("rsuConfigs"));
    EXPECT_TRUE(root.isMember("timestamp"));
    
    EXPECT_EQ("Unit001", root["unitConfig"]["unitId"].asString());
    EXPECT_EQ("running", root["unitConfig"]["bridgePluginStatus"].asString());
    
    ASSERT_TRUE(root["rsuConfigs"].isArray());
    ASSERT_EQ(1, root["rsuConfigs"].size());
    EXPECT_EQ("192.168.1.1", root["rsuConfigs"][0]["rsu"]["ip"].asString());
}
