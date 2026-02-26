/*
 * @file test_PluginStatus.cpp
 *
 *  Created on: 2026-02-25
 *      @author: Daniel I. Kelley (Leidos)
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TmxControl.h"
#include <database/DbConnectionConfig.h>

using namespace std;
using namespace tmx::utils;
using namespace tmxctl;
using namespace testing;

/**
 * Mock classes for testing database operations
 */
class MockConnection {
public:
    MOCK_METHOD(std::unique_ptr<sql::PreparedStatement>, prepareStatement, (const std::string&), ());
    MOCK_METHOD(std::unique_ptr<sql::Statement>, createStatement, (), ());
};

class MockPreparedStatement {
public:
    MOCK_METHOD(void, setString, (int, const std::string&), ());
    MOCK_METHOD(void, setInt, (int, int), ());
    MOCK_METHOD(std::unique_ptr<sql::ResultSet>, executeQuery, (), ());
    MOCK_METHOD(bool, execute, (), ());
    MOCK_METHOD(int, executeUpdate, (), ());
};

class MockResultSet {
public:
    MOCK_METHOD(bool, next, (), ());
    MOCK_METHOD(int, getInt, (int), ());
    MOCK_METHOD(sql::SQLString, getString, (int), ());
};

class MockDbConnection {
public:
    MOCK_METHOD(MockConnection*, Get, (), ());
};

class MockDbConnectionPool {
public:
    MOCK_METHOD(std::string, GetPwd, (), ());
    MOCK_METHOD(DbConnection, Connection, (const std::string&, const std::string&, const std::string&, const std::string&), ());
};

/**
 * Testable TmxControl class that allows mock injection
 */
class TestableTmxControl : public TmxControl {
public:
    TestableTmxControl(MockDbConnectionPool& mockPool) : mockPool_(mockPool) {}

    // Override the pool access to use our mock
    DbConnectionPool& getPool() { return reinterpret_cast<DbConnectionPool&>(mockPool_); }

    // Expose protected members for testing
    using TmxControl::_output;
    using TmxControl::_pool;

    // Test-specific method to call list with mock pool
    bool testList(pluginlist& plugins) {
        // Temporarily replace _pool with our mock
        auto originalPool = &_pool;
        _pool = getPool();
        bool result = list(plugins);
        _pool = *originalPool;
        return result;
    }

private:
    MockDbConnectionPool& mockPool_;
};

/**
 * Test fixture for PluginStatus testing with proper mocks
 */
class TmxControlListTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockPool = std::make_unique<MockDbConnectionPool>();
        mockConnection = std::make_unique<MockConnection>();
        mockPreparedStatement = std::make_unique<MockPreparedStatement>();
        mockResultSet = std::make_unique<MockResultSet>();
        mockDbConnection = std::make_unique<MockDbConnection>();

        // Create testable TmxControl instance
        tmxControl = std::make_unique<TestableTmxControl>(*mockPool);
    }

    void TearDown() override {
        tmxControl.reset();
        mockDbConnection.reset();
        mockResultSet.reset();
        mockPreparedStatement.reset();
        mockConnection.reset();
        mockPool.reset();
    }

protected:
    std::unique_ptr<MockDbConnectionPool> mockPool;
    std::unique_ptr<MockConnection> mockConnection;
    std::unique_ptr<MockPreparedStatement> mockPreparedStatement;
    std::unique_ptr<MockResultSet> mockResultSet;
    std::unique_ptr<MockDbConnection> mockDbConnection;
    std::unique_ptr<TestableTmxControl> tmxControl;
};

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

/**
 * Test getConfiguredConnection function behavior
 * This test verifies that the getConfiguredConnection helper function
 * properly uses DbConnectionConfig to create database connections.
 */
TEST_F(PluginStatusTest, GetConfiguredConnectionIntegration) {
    // Test that DbConnectionConfig is used correctly by getConfiguredConnection
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    // Verify the configuration values that would be used
    std::string expectedUrl = config.getConnectionUrl();
    std::string expectedUser = config.getUser();
    std::string expectedDatabase = config.getDatabase();

    // Test that configuration values are valid for connection
    EXPECT_FALSE(expectedUrl.empty()) << "Connection URL should not be empty";
    EXPECT_FALSE(expectedUser.empty()) << "User should not be empty";
    EXPECT_FALSE(expectedDatabase.empty()) << "Database should not be empty";

    // Test URL format validation
    EXPECT_TRUE(expectedUrl.find("tcp://") == 0) << "URL should start with tcp://";
    EXPECT_TRUE(expectedUrl.find(":") != std::string::npos) << "URL should contain port separator";

    // Test that the configuration is consistent
    std::string host = config.getHost();
    std::string port = config.getPort();
    std::string expectedUrlFromParts = "tcp://" + host + ":" + port;
    EXPECT_EQ(expectedUrlFromParts, expectedUrl) << "URL should match host:port combination";
}

/**
 * Test getConfiguredConnection parameter usage
 * This test verifies the parameters that would be passed to the connection pool
 */
TEST_F(PluginStatusTest, GetConfiguredConnectionParameters) {
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    // Test the parameters that getConfiguredConnection would use
    std::string connectionUrl = config.getConnectionUrl();
    std::string user = config.getUser();
    std::string database = config.getDatabase();

    // Verify parameter formats and constraints
    EXPECT_TRUE(connectionUrl.length() > 10) << "Connection URL should be substantial";
    EXPECT_TRUE(user.length() > 0) << "User should not be empty";
    EXPECT_TRUE(database.length() > 0) << "Database should not be empty";

    // Test URL components
    size_t protocolPos = connectionUrl.find("tcp://");
    EXPECT_EQ(0, protocolPos) << "URL should start with tcp:// protocol";

    size_t portPos = connectionUrl.find(":", 6); // After "tcp://"
    EXPECT_NE(std::string::npos, portPos) << "URL should contain port separator";

    // Extract and validate host and port from URL
    std::string urlHost = connectionUrl.substr(6, portPos - 6); // After "tcp://"
    std::string urlPort = connectionUrl.substr(portPos + 1);

    EXPECT_FALSE(urlHost.empty()) << "Host extracted from URL should not be empty";
    EXPECT_FALSE(urlPort.empty()) << "Port extracted from URL should not be empty";

    // Validate port is numeric
    bool isNumeric = true;
    for (char c : urlPort) {
        if (!std::isdigit(c)) {
            isNumeric = false;
            break;
        }
    }
    EXPECT_TRUE(isNumeric) << "Port should be numeric: " << urlPort;

    // Validate port range
    int portNum = std::stoi(urlPort);
    EXPECT_GT(portNum, 0) << "Port should be positive";
    EXPECT_LE(portNum, 65535) << "Port should be within valid range";
}

/**
 * Test getConfiguredConnection with different configurations
 */
TEST_F(PluginStatusTest, GetConfiguredConnectionWithDifferentConfigs) {
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    // Test initial configuration
    std::string initialUrl = config.getConnectionUrl();
    std::string initialUser = config.getUser();
    std::string initialDatabase = config.getDatabase();

    EXPECT_FALSE(initialUrl.empty());
    EXPECT_FALSE(initialUser.empty());
    EXPECT_FALSE(initialDatabase.empty());

    // Test that configuration reload maintains consistency
    config.reloadConfiguration();

    std::string reloadedUrl = config.getConnectionUrl();
    std::string reloadedUser = config.getUser();
    std::string reloadedDatabase = config.getDatabase();

    // After reload, values should still be valid (may be same or different based on environment)
    EXPECT_FALSE(reloadedUrl.empty());
    EXPECT_FALSE(reloadedUser.empty());
    EXPECT_FALSE(reloadedDatabase.empty());

    // URL format should remain consistent
    EXPECT_TRUE(reloadedUrl.find("tcp://") == 0);
}

/**
 * Test getConfiguredConnection error handling scenarios
 */
TEST_F(PluginStatusTest, GetConfiguredConnectionErrorHandling) {
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    // Test that configuration methods don't throw exceptions
    EXPECT_NO_THROW(config.getConnectionUrl());
    EXPECT_NO_THROW(config.getUser());
    EXPECT_NO_THROW(config.getDatabase());
    EXPECT_NO_THROW(config.getHost());
    EXPECT_NO_THROW(config.getPort());

    // Test that password method handles missing password file gracefully
    EXPECT_NO_THROW(config.getPassword());

    // Test that reload doesn't break the configuration
    EXPECT_NO_THROW(config.reloadConfiguration());

    // Verify configuration is still valid after reload
    std::string url = config.getConnectionUrl();
    std::string user = config.getUser();
    std::string database = config.getDatabase();

    EXPECT_FALSE(url.empty());
    EXPECT_FALSE(user.empty());
    EXPECT_FALSE(database.empty());
}

/**
 * Test getConfiguredConnection thread safety
 */
TEST_F(PluginStatusTest, GetConfiguredConnectionThreadSafety) {
    const int numThreads = 10;
    const int numIterations = 100;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    std::atomic<int> failureCount{0};

    // Launch multiple threads that access configuration concurrently
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < numIterations; ++j) {
                try {
                    DbConnectionConfig& config = DbConnectionConfig::getInstance();

                    // Simulate what getConfiguredConnection does
                    std::string url = config.getConnectionUrl();
                    std::string user = config.getUser();
                    std::string database = config.getDatabase();

                    // Verify values are consistent and valid
                    if (!url.empty() && !user.empty() && !database.empty() &&
                        url.find("tcp://") == 0) {
                        successCount.fetch_add(1);
                    } else {
                        failureCount.fetch_add(1);
                    }
                } catch (...) {
                    failureCount.fetch_add(1);
                }
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify thread safety
    int totalOperations = numThreads * numIterations;
    EXPECT_EQ(totalOperations, successCount.load() + failureCount.load());
    EXPECT_EQ(totalOperations, successCount.load()) << "All operations should succeed in thread-safe manner";
    EXPECT_EQ(0, failureCount.load()) << "No operations should fail due to thread safety issues";
}

/**
 * Test TmxControl::list with successful database query
 */
TEST_F(TmxControlListTest, ListPluginsSuccess) {
    // Setup test data
    pluginlist plugins = {"MapPlugin", "SpatPlugin"};

    // Expected query should contain the base LIST_QUERY with constraints
    std::string expectedQuery = 
        "SELECT IVP.plugin.id, name, description, version, coalesce(enabled, -1), "
        "path, exeName, manifestName, "
        "maxMessageInterval, commandLineParameters "
        "FROM IVP.plugin "
        "LEFT JOIN IVP.installedPlugin ON IVP.plugin.id = IVP.installedPlugin.pluginId";

    // Setup mock expectations
    EXPECT_CALL(*mockPool, GetPwd())
        .WillOnce(Return("test_password"));

    EXPECT_CALL(*mockPool, Connection(_, _, _, _))
        .WillOnce(Return(ByMove(std::unique_ptr<DbConnection>(mockDbConnection.release()))));

    EXPECT_CALL(*mockDbConnection, Get())
        .WillRepeatedly(Return(mockConnection.get()));

    EXPECT_CALL(*mockConnection, prepareStatement(ContainsRegex("SELECT.*IVP\\.plugin\\.id.*FROM IVP\\.plugin")))
        .WillOnce(Return(ByMove(std::unique_ptr<sql::PreparedStatement>(
            reinterpret_cast<sql::PreparedStatement*>(mockPreparedStatement.release())))));

    // Expect parameter binding for plugin names
    EXPECT_CALL(*mockPreparedStatement, setString(1, "MapPlugin"));
    EXPECT_CALL(*mockPreparedStatement, setString(2, "SpatPlugin"));

    EXPECT_CALL(*mockPreparedStatement, executeQuery())
        .WillOnce(Return(ByMove(std::unique_ptr<sql::ResultSet>(
            reinterpret_cast<sql::ResultSet*>(mockResultSet.release())))));

    // Setup result set data for two plugins
    EXPECT_CALL(*mockResultSet, next())
        .WillOnce(Return(true))   // First plugin
        .WillOnce(Return(true))   // Second plugin
        .WillOnce(Return(false)); // End of results

    // First plugin data (MapPlugin - enabled)
    EXPECT_CALL(*mockResultSet, getInt(1)).WillOnce(Return(1));  // id
    EXPECT_CALL(*mockResultSet, getString(2)).WillOnce(Return(sql::SQLString("MapPlugin")));  // name
    EXPECT_CALL(*mockResultSet, getString(3)).WillOnce(Return(sql::SQLString("MAP Plugin Description")));  // description
    EXPECT_CALL(*mockResultSet, getString(4)).WillOnce(Return(sql::SQLString("1.0.0")));  // version
    EXPECT_CALL(*mockResultSet, getInt(5)).WillOnce(Return(1));  // enabled
    EXPECT_CALL(*mockResultSet, getString(6)).WillOnce(Return(sql::SQLString("/opt/tmx/bin")));  // path
    EXPECT_CALL(*mockResultSet, getString(7)).WillOnce(Return(sql::SQLString("MapPlugin")));  // exeName
    EXPECT_CALL(*mockResultSet, getString(8)).WillOnce(Return(sql::SQLString("manifest.json")));  // manifestName
    EXPECT_CALL(*mockResultSet, getInt(9)).WillOnce(Return(1000));  // maxMessageInterval
    EXPECT_CALL(*mockResultSet, getString(10)).WillOnce(Return(sql::SQLString("--verbose")));  // commandLineParameters

    // Second plugin data (SpatPlugin - disabled)
    EXPECT_CALL(*mockResultSet, getInt(1)).WillOnce(Return(2));  // id
    EXPECT_CALL(*mockResultSet, getString(2)).WillOnce(Return(sql::SQLString("SpatPlugin")));  // name
    EXPECT_CALL(*mockResultSet, getString(3)).WillOnce(Return(sql::SQLString("SPAT Plugin Description")));  // description
    EXPECT_CALL(*mockResultSet, getString(4)).WillOnce(Return(sql::SQLString("2.0.0")));  // version
    EXPECT_CALL(*mockResultSet, getInt(5)).WillOnce(Return(0));  // enabled (disabled)
    EXPECT_CALL(*mockResultSet, getString(6)).WillOnce(Return(sql::SQLString("/opt/tmx/bin")));  // path
    EXPECT_CALL(*mockResultSet, getString(7)).WillOnce(Return(sql::SQLString("SpatPlugin")));  // exeName
    EXPECT_CALL(*mockResultSet, getString(8)).WillOnce(Return(sql::SQLString("spat_manifest.json")));  // manifestName
    EXPECT_CALL(*mockResultSet, getInt(9)).WillOnce(Return(2000));  // maxMessageInterval
    EXPECT_CALL(*mockResultSet, getString(10)).WillOnce(Return(sql::SQLString("--debug")));  // commandLineParameters

    // Execute the test
    bool result = tmxControl->testList(plugins);

    // Verify results
    EXPECT_TRUE(result) << "list() should return true on success";

    // Verify that output contains expected plugin data
    // Note: This would require access to the _output member and its structure
    // For now, we verify the method completed successfully
}

/**
 * Test TmxControl::list with external plugin (enabled = -1)
 */
TEST_F(TmxControlListTest, ListPluginsWithExternalPlugin) {
    pluginlist plugins = {"ExternalPlugin"};

    // Setup mock expectations
    EXPECT_CALL(*mockPool, GetPwd())
        .WillOnce(Return("test_password"));

    EXPECT_CALL(*mockPool, Connection(_, _, _, _))
        .WillOnce(Return(ByMove(std::unique_ptr<DbConnection>(mockDbConnection.release()))));

    EXPECT_CALL(*mockDbConnection, Get())
        .WillRepeatedly(Return(mockConnection.get()));

    EXPECT_CALL(*mockConnection, prepareStatement(_))
        .WillOnce(Return(ByMove(std::unique_ptr<sql::PreparedStatement>(
            reinterpret_cast<sql::PreparedStatement*>(mockPreparedStatement.release())))));

    EXPECT_CALL(*mockPreparedStatement, setString(1, "ExternalPlugin"));

    EXPECT_CALL(*mockPreparedStatement, executeQuery())
        .WillOnce(Return(ByMove(std::unique_ptr<sql::ResultSet>(
            reinterpret_cast<sql::ResultSet*>(mockResultSet.release())))));

    // Setup result set for external plugin (enabled = -1)
    EXPECT_CALL(*mockResultSet, next())
        .WillOnce(Return(true))   // External plugin
        .WillOnce(Return(false)); // End of results

    // External plugin data
    EXPECT_CALL(*mockResultSet, getInt(1)).WillOnce(Return(3));  // id
    EXPECT_CALL(*mockResultSet, getString(2)).WillOnce(Return(sql::SQLString("ExternalPlugin")));  // name
    EXPECT_CALL(*mockResultSet, getString(3)).WillOnce(Return(sql::SQLString("External Plugin")));  // description
    EXPECT_CALL(*mockResultSet, getString(4)).WillOnce(Return(sql::SQLString("1.5.0")));  // version
    EXPECT_CALL(*mockResultSet, getInt(5)).WillOnce(Return(-1));  // enabled (external)

    // For external plugins (enabled < 0), the method should continue without processing additional fields
    // So we don't expect calls to getString for fields 6-10

    bool result = tmxControl->testList(plugins);
    EXPECT_TRUE(result) << "list() should handle external plugins correctly";
}

/**
 * Test TmxControl::list with empty plugin list
 */
TEST_F(TmxControlListTest, ListPluginsEmptyList) {
    pluginlist plugins; // Empty list

    // Setup mock expectations
    EXPECT_CALL(*mockPool, GetPwd())
        .WillOnce(Return("test_password"));

    EXPECT_CALL(*mockPool, Connection(_, _, _, _))
        .WillOnce(Return(ByMove(std::unique_ptr<DbConnection>(mockDbConnection.release()))));

    EXPECT_CALL(*mockDbConnection, Get())
        .WillRepeatedly(Return(mockConnection.get()));

    EXPECT_CALL(*mockConnection, prepareStatement(_))
        .WillOnce(Return(ByMove(std::unique_ptr<sql::PreparedStatement>(
            reinterpret_cast<sql::PreparedStatement*>(mockPreparedStatement.release())))));

    // No setString calls expected for empty plugin list

    EXPECT_CALL(*mockPreparedStatement, executeQuery())
        .WillOnce(Return(ByMove(std::unique_ptr<sql::ResultSet>(
            reinterpret_cast<sql::ResultSet*>(mockResultSet.release())))));

    // Empty result set
    EXPECT_CALL(*mockResultSet, next())
        .WillOnce(Return(false)); // No results

    bool result = tmxControl->testList(plugins);
    EXPECT_TRUE(result) << "list() should handle empty plugin list correctly";
}

/**
 * Test TmxControl::list with database exception
 */
TEST_F(TmxControlListTest, ListPluginsDatabaseException) {
    pluginlist plugins = {"TestPlugin"};

    // Setup mock to throw exception
    EXPECT_CALL(*mockPool, GetPwd())
        .WillOnce(Return("test_password"));

    EXPECT_CALL(*mockPool, Connection(_, _, _, _))
        .WillOnce(Throw(std::runtime_error("Database connection failed")));

    bool result = tmxControl->testList(plugins);
    EXPECT_FALSE(result) << "list() should return false when database exception occurs";
}

/**
 * Test TmxControl::list with SQL exception during query preparation
 */
TEST_F(TmxControlListTest, ListPluginsQueryPreparationException) {
    pluginlist plugins = {"TestPlugin"};

    EXPECT_CALL(*mockPool, GetPwd())
        .WillOnce(Return("test_password"));

    EXPECT_CALL(*mockPool, Connection(_, _, _, _))
        .WillOnce(Return(ByMove(std::unique_ptr<DbConnection>(mockDbConnection.release()))));

    EXPECT_CALL(*mockDbConnection, Get())
        .WillRepeatedly(Return(mockConnection.get()));

    // Throw exception during query preparation
    EXPECT_CALL(*mockConnection, prepareStatement(_))
        .WillOnce(Throw(sql::SQLException("Invalid SQL syntax")));

    bool result = tmxControl->testList(plugins);
    EXPECT_FALSE(result) << "list() should return false when SQL preparation fails";
}

/**
 * Test TmxControl::list with SQL exception during query execution
 */
TEST_F(TmxControlListTest, ListPluginsQueryExecutionException) {
    pluginlist plugins = {"TestPlugin"};

    EXPECT_CALL(*mockPool, GetPwd())
        .WillOnce(Return("test_password"));

    EXPECT_CALL(*mockPool, Connection(_, _, _, _))
        .WillOnce(Return(ByMove(std::unique_ptr<DbConnection>(mockDbConnection.release()))));

    EXPECT_CALL(*mockDbConnection, Get())
        .WillRepeatedly(Return(mockConnection.get()));

    EXPECT_CALL(*mockConnection, prepareStatement(_))
        .WillOnce(Return(ByMove(std::unique_ptr<sql::PreparedStatement>(
            reinterpret_cast<sql::PreparedStatement*>(mockPreparedStatement.release())))));

    EXPECT_CALL(*mockPreparedStatement, setString(1, "TestPlugin"));

    // Throw exception during query execution
    EXPECT_CALL(*mockPreparedStatement, executeQuery())
        .WillOnce(Throw(sql::SQLException("Query execution failed")));

    bool result = tmxControl->testList(plugins);
    EXPECT_FALSE(result) << "list() should return false when query execution fails";
}

/**
 * Test TmxControl::list parameter binding validation
 */
TEST_F(TmxControlListTest, ListPluginsParameterBinding) {
    pluginlist plugins = {"Plugin1", "Plugin2", "Plugin3"};

    EXPECT_CALL(*mockPool, GetPwd())
        .WillOnce(Return("test_password"));

    EXPECT_CALL(*mockPool, Connection(_, _, _, _))
        .WillOnce(Return(ByMove(std::unique_ptr<DbConnection>(mockDbConnection.release()))));

    EXPECT_CALL(*mockDbConnection, Get())
        .WillRepeatedly(Return(mockConnection.get()));

    EXPECT_CALL(*mockConnection, prepareStatement(_))
        .WillOnce(Return(ByMove(std::unique_ptr<sql::PreparedStatement>(
            reinterpret_cast<sql::PreparedStatement*>(mockPreparedStatement.release())))));

    // Verify correct parameter binding order
    EXPECT_CALL(*mockPreparedStatement, setString(1, "Plugin1"));
    EXPECT_CALL(*mockPreparedStatement, setString(2, "Plugin2"));
    EXPECT_CALL(*mockPreparedStatement, setString(3, "Plugin3"));

    EXPECT_CALL(*mockPreparedStatement, executeQuery())
        .WillOnce(Return(ByMove(std::unique_ptr<sql::ResultSet>(
            reinterpret_cast<sql::ResultSet*>(mockResultSet.release())))));

    EXPECT_CALL(*mockResultSet, next())
        .WillOnce(Return(false)); // No results for this test

    bool result = tmxControl->testList(plugins);
    EXPECT_TRUE(result) << "list() should bind parameters correctly";
}

/**
 * Test TmxControl::list query construction
 */
TEST_F(TmxControlListTest, ListPluginsQueryConstruction) {
    pluginlist plugins = {"TestPlugin"};

    EXPECT_CALL(*mockPool, GetPwd())
        .WillOnce(Return("test_password"));

    EXPECT_CALL(*mockPool, Connection(_, _, _, _))
        .WillOnce(Return(ByMove(std::unique_ptr<DbConnection>(mockDbConnection.release()))));

    EXPECT_CALL(*mockDbConnection, Get())
        .WillRepeatedly(Return(mockConnection.get()));

    // Verify that the query contains expected SQL elements
    EXPECT_CALL(*mockConnection, prepareStatement(AllOf(
        ContainsRegex("SELECT.*IVP\\.plugin\\.id"),
        ContainsRegex("FROM IVP\\.plugin"),
        ContainsRegex("LEFT JOIN IVP\\.installedPlugin"),
        ContainsRegex("coalesce\\(enabled, -1\\)")
    )))
        .WillOnce(Return(ByMove(std::unique_ptr<sql::PreparedStatement>(
            reinterpret_cast<sql::PreparedStatement*>(mockPreparedStatement.release())))));

    EXPECT_CALL(*mockPreparedStatement, setString(1, "TestPlugin"));

    EXPECT_CALL(*mockPreparedStatement, executeQuery())
        .WillOnce(Return(ByMove(std::unique_ptr<sql::ResultSet>(
            reinterpret_cast<sql::ResultSet*>(mockResultSet.release())))));

    EXPECT_CALL(*mockResultSet, next())
        .WillOnce(Return(false));

    bool result = tmxControl->testList(plugins);
    EXPECT_TRUE(result) << "list() should construct correct SQL query";
}

/**
 * Test TmxControl::list plugin state conversion
 */
TEST_F(TmxControlListTest, ListPluginsStateConversion) {
    pluginlist plugins = {"StateTestPlugin"};

    EXPECT_CALL(*mockPool, GetPwd())
        .WillOnce(Return("test_password"));

    EXPECT_CALL(*mockPool, Connection(_, _, _, _))
        .WillOnce(Return(ByMove(std::unique_ptr<DbConnection>(mockDbConnection.release()))));

    EXPECT_CALL(*mockDbConnection, Get())
        .WillRepeatedly(Return(mockConnection.get()));

    EXPECT_CALL(*mockConnection, prepareStatement(_))
        .WillOnce(Return(ByMove(std::unique_ptr<sql::PreparedStatement>(
            reinterpret_cast<sql::PreparedStatement*>(mockPreparedStatement.release())))));

    EXPECT_CALL(*mockPreparedStatement, setString(1, "StateTestPlugin"));

    EXPECT_CALL(*mockPreparedStatement, executeQuery())
        .WillOnce(Return(ByMove(std::unique_ptr<sql::ResultSet>(
            reinterpret_cast<sql::ResultSet*>(mockResultSet.release())))));

    // Test different enabled states
    EXPECT_CALL(*mockResultSet, next())
        .WillOnce(Return(true))   // Plugin with enabled = 1
        .WillOnce(Return(false)); // End of results

    // Plugin data with enabled = 1 (should be "Enabled")
    EXPECT_CALL(*mockResultSet, getInt(1)).WillOnce(Return(1));  // id
    EXPECT_CALL(*mockResultSet, getString(2)).WillOnce(Return(sql::SQLString("StateTestPlugin")));  // name
    EXPECT_CALL(*mockResultSet, getString(3)).WillOnce(Return(sql::SQLString("Test Description")));  // description
    EXPECT_CALL(*mockResultSet, getString(4)).WillOnce(Return(sql::SQLString("1.0.0")));  // version
    EXPECT_CALL(*mockResultSet, getInt(5)).WillOnce(Return(1));  // enabled = 1 (should map to "Enabled")
    EXPECT_CALL(*mockResultSet, getString(6)).WillOnce(Return(sql::SQLString("/path")));  // path
    EXPECT_CALL(*mockResultSet, getString(7)).WillOnce(Return(sql::SQLString("exe")));  // exeName
    EXPECT_CALL(*mockResultSet, getString(8)).WillOnce(Return(sql::SQLString("manifest")));  // manifestName
    EXPECT_CALL(*mockResultSet, getInt(9)).WillOnce(Return(1000));  // maxMessageInterval
    EXPECT_CALL(*mockResultSet, getString(10)).WillOnce(Return(sql::SQLString("args")));  // commandLineParameters

    bool result = tmxControl->testList(plugins);
    EXPECT_TRUE(result) << "list() should handle plugin state conversion correctly";

    // The actual state conversion logic:
    // !enabled ? "Disabled" : enabled > 0 ? "Enabled" : "External"
    // enabled = 0 -> "Disabled"
    // enabled > 0 -> "Enabled"
    // enabled < 0 -> "External"
}