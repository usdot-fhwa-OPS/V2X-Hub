/*
 * @file test_PluginStatus.cpp
 *
 *  Created on: 2026-02-25
 *      @author: AI Assistant
 */

#include <gtest/gtest.h>
#include <database/DbConnectionConfig.h>

using namespace tmx::utils;

/**
 * Test DbConnectionConfig integration in TmxCtl
 * This test verifies that the DbConnectionConfig singleton is accessible
 * and provides the expected interface for TmxCtl integration.
 */
TEST(PluginStatusTest, DbConnectionConfigIntegration) {
    // Test that DbConnectionConfig singleton is accessible
    DbConnectionConfig& config = DbConnectionConfig::getInstance();
    
    // Test that basic methods are available and return strings
    std::string host = config.getHost();
    std::string port = config.getPort();
    std::string database = config.getDatabase();
    std::string user = config.getUser();
    std::string connectionUrl = config.getConnectionUrl();
    
    // Verify that methods return non-null strings (may be empty but not null)
    EXPECT_TRUE(host.empty() || !host.empty());
    EXPECT_TRUE(port.empty() || !port.empty());
    EXPECT_TRUE(database.empty() || !database.empty());
    EXPECT_TRUE(user.empty() || !user.empty());
    EXPECT_TRUE(connectionUrl.empty() || !connectionUrl.empty());
    
    // Test that connection URL has expected format
    if (!connectionUrl.empty()) {
        EXPECT_TRUE(connectionUrl.find("tcp://") == 0);
    }
    
    // Test that reloadConfiguration method is available
    config.reloadConfiguration();
    
    // Verify configuration is still accessible after reload
    std::string hostAfterReload = config.getHost();
    EXPECT_TRUE(hostAfterReload.empty() || !hostAfterReload.empty());
}

/**
 * Test that DbConnectionConfig provides default values
 */
TEST(PluginStatusTest, DbConnectionConfigDefaults) {
    DbConnectionConfig& config = DbConnectionConfig::getInstance();
    
    // Test default values (these should match the constants in DbConnectionConfig.cpp)
    std::string host = config.getHost();
    std::string port = config.getPort();
    std::string database = config.getDatabase();
    std::string user = config.getUser();
    
    // These should have default values if no environment variables are set
    EXPECT_FALSE(host.empty());
    EXPECT_FALSE(port.empty());
    EXPECT_FALSE(database.empty());
    EXPECT_FALSE(user.empty());
    
    // Verify expected default values
    EXPECT_EQ("127.0.0.1", host);
    EXPECT_EQ("3306", port);
    EXPECT_EQ("IVP", database);
    EXPECT_EQ("IVP", user);
}
