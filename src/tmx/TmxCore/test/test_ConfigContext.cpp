/*
 * test_ConfigContext.cpp
 *
 *  Created on: Apr 7, 2026
 *      Author: Expert Tester
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <vector>
#include <string>
#include <sstream>

// Include the class under test
#include "../src/database/ConfigContext.h"

using namespace std;
using namespace testing;

// Mock SQL Statement class
class MockStatement : public sql::Statement {
public:
    MOCK_METHOD(bool, execute, (const std::string& sql), (override));
    MOCK_METHOD(sql::ResultSet*, executeQuery, (const std::string& sql), (override));
    MOCK_METHOD(int, executeUpdate, (const std::string& sql), (override));
    MOCK_METHOD(void, close, (), (override));
    MOCK_METHOD(bool, getMoreResults, (), (override));
    MOCK_METHOD(void, cancel, (), (override));
    MOCK_METHOD(void, clearWarnings, (), (override));
    MOCK_METHOD(const sql::SQLWarning*, getWarnings, (), (override));
    MOCK_METHOD(size_t, getMaxFieldSize, (), (override));
    MOCK_METHOD(void, setMaxFieldSize, (size_t max), (override));
    MOCK_METHOD(size_t, getMaxRows, (), (override));
    MOCK_METHOD(void, setMaxRows, (size_t max), (override));
    MOCK_METHOD(bool, getMoreResults, (int current), (override));
    MOCK_METHOD(sql::ResultSet*, getResultSet, (), (override));
    MOCK_METHOD(sql::ResultSet::enum_type, getResultSetType, (), (override));
    MOCK_METHOD(uint64_t, getUpdateCount, (), (override));
    MOCK_METHOD(void, setCursorName, (const std::string& name), (override));
    MOCK_METHOD(void, setEscapeProcessing, (bool enable), (override));
    MOCK_METHOD(void, setFetchDirection, (int direction), (override));
    MOCK_METHOD(int, getFetchDirection, (), (override));
    MOCK_METHOD(void, setFetchSize, (size_t rows), (override));
    MOCK_METHOD(size_t, getFetchSize, (), (override));
    MOCK_METHOD(void, setMaxRows, (int max), (override));
    MOCK_METHOD(void, setQueryTimeout, (int seconds), (override));
    MOCK_METHOD(int, getQueryTimeout, (), (override));
    MOCK_METHOD(void, setResultSetType, (sql::ResultSet::enum_type type), (override));
};

// Testable ConfigContext class that allows mocking
class TestableConfigContext : public ConfigContext {
public:
    TestableConfigContext() : mockStatement(nullptr) {}
    
    void setMockStatement(MockStatement* stmt) {
        mockStatement = stmt;
    }
    
    // Override getStatement to return our mock
    sql::Statement* getStatement() override {
        if (mockStatement) {
            return mockStatement;
        }
        return ConfigContext::getStatement();
    }
    
    // Expose the method under test for testing
    void testInitializePluginConfigParameters(unsigned int pluginId, 
                                            std::vector<PluginConfigurationParameterEntry>& entries) {
        initializePluginConfigParameters(pluginId, entries);
    }
    
private:
    MockStatement* mockStatement;
};

// Test fixture class
class ConfigContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockStatement = std::make_unique<MockStatement>();
        configContext = std::make_unique<TestableConfigContext>();
        configContext->setMockStatement(mockStatement.get());
    }
    
    void TearDown() override {
        configContext.reset();
        mockStatement.reset();
    }
    
    // Helper method to create test entries
    std::vector<PluginConfigurationParameterEntry> createTestEntries(int count = 3) {
        std::vector<PluginConfigurationParameterEntry> entries;
        for (int i = 0; i < count; ++i) {
            PluginConfigurationParameterEntry entry;
            entry.key = "test_key_" + std::to_string(i);
            entry.value = "test_value_" + std::to_string(i);
            entry.defaultValue = "default_value_" + std::to_string(i);
            entry.description = "Test description " + std::to_string(i);
            entries.push_back(entry);
        }
        return entries;
    }
    
    // Helper method to verify SQL contains expected patterns
    void expectSqlContains(const std::string& sql, const std::vector<std::string>& patterns) {
        for (const auto& pattern : patterns) {
            EXPECT_THAT(sql, HasSubstr(pattern)) << "SQL should contain: " << pattern;
        }
    }
    
protected:
    std::unique_ptr<MockStatement> mockStatement;
    std::unique_ptr<TestableConfigContext> configContext;
};

// Test Case: Insert New Plugin Parameters
TEST_F(ConfigContextTest, InsertNewPluginParameters) {
    unsigned int pluginId = 123;
    auto entries = createTestEntries(2);
    
    // Expect INSERT statements for each entry
    EXPECT_CALL(*mockStatement, execute(_))
        .Times(3) // 2 INSERT + 1 DELETE
        .WillRepeatedly([this](const std::string& sql) {
            // Verify INSERT statement structure
            if (sql.find("INSERT INTO") != std::string::npos) {
                expectSqlContains(sql, {
                    "INSERT INTO `pluginConfigurationParameter`",
                    "`pluginId`, `key`, `value`, `defaultValue`, `description`",
                    "ON DUPLICATE KEY UPDATE",
                    "defaultValue = VALUES(defaultValue)",
                    "description = VALUES(description)"
                });
                
                // Should NOT contain value update for regular plugins
                EXPECT_THAT(sql, Not(HasSubstr("value = VALUES(value)")));
            }
            return true;
        });
    
    configContext->testInitializePluginConfigParameters(pluginId, entries);
}

// Test Case: System Parameters Special Handling (pluginId == 0)
TEST_F(ConfigContextTest, SystemParametersSpecialHandling) {
    unsigned int pluginId = 0; // System parameters
    auto entries = createTestEntries(1);
    
    EXPECT_CALL(*mockStatement, execute(_))
        .Times(1) // Only INSERT, no DELETE for system parameters
        .WillOnce([](const std::string& sql) {
            // Verify system parameters update value field
            EXPECT_THAT(sql, HasSubstr("INSERT INTO `pluginConfigurationParameter`"));
            EXPECT_THAT(sql, HasSubstr("ON DUPLICATE KEY UPDATE"));
            EXPECT_THAT(sql, HasSubstr("defaultValue = VALUES(defaultValue)"));
            EXPECT_THAT(sql, HasSubstr("description = VALUES(description)"));
            EXPECT_THAT(sql, HasSubstr("value = VALUES(value)")); // Special for system params
            return true;
        });
    
    configContext->testInitializePluginConfigParameters(pluginId, entries);
}

// Test Case: Delete Obsolete Parameters
TEST_F(ConfigContextTest, DeleteObsoleteParameters) {
    unsigned int pluginId = 456;
    auto entries = createTestEntries(2);
    
    EXPECT_CALL(*mockStatement, execute(_))
        .Times(3) // 2 INSERT + 1 DELETE
        .WillRepeatedly([](const std::string& sql) {
            if (sql.find("DELETE FROM") != std::string::npos) {
                // Verify DELETE statement structure
                EXPECT_THAT(sql, HasSubstr("DELETE FROM `pluginConfigurationParameter`"));
                EXPECT_THAT(sql, HasSubstr("WHERE `pluginId` = '456'"));
                EXPECT_THAT(sql, HasSubstr("AND `key` NOT IN"));
                EXPECT_THAT(sql, HasSubstr("'test_key_0'"));
                EXPECT_THAT(sql, HasSubstr("'test_key_1'"));
            }
            return true;
        });
    
    configContext->testInitializePluginConfigParameters(pluginId, entries);
}

// Test Case: Empty Entries Vector
TEST_F(ConfigContextTest, EmptyEntriesVector) {
    unsigned int pluginId = 789;
    std::vector<PluginConfigurationParameterEntry> entries; // Empty
    
    EXPECT_CALL(*mockStatement, execute(_))
        .Times(1) // Only DELETE statement
        .WillOnce([](const std::string& sql) {
            // Should delete all parameters for the plugin
            EXPECT_THAT(sql, HasSubstr("DELETE FROM `pluginConfigurationParameter`"));
            EXPECT_THAT(sql, HasSubstr("WHERE `pluginId` = '789'"));
            EXPECT_THAT(sql, Not(HasSubstr("AND `key` NOT IN")));
            return true;
        });
    
    configContext->testInitializePluginConfigParameters(pluginId, entries);
}

// Test Case: Exception Handling - SQL Execution Failure
TEST_F(ConfigContextTest, SqlExecutionFailure) {
    unsigned int pluginId = 1001;
    auto entries = createTestEntries(1);
    
    EXPECT_CALL(*mockStatement, execute(_))
        .WillOnce(Throw(sql::SQLException("Database connection lost")));
    
    // Should propagate the exception
    EXPECT_THROW(
        configContext->testInitializePluginConfigParameters(pluginId, entries),
        sql::SQLException
    );
}

// Test Case: System Parameters No Delete Operation
TEST_F(ConfigContextTest, SystemParametersNoDelete) {
    unsigned int pluginId = 0; // System parameters
    auto entries = createTestEntries(3);
    
    // System parameters should only do INSERT operations, no DELETE
    EXPECT_CALL(*mockStatement, execute(_))
        .Times(3) // Only 3 INSERT statements, no DELETE
        .WillRepeatedly([](const std::string& sql) {
            // All calls should be INSERT statements
            EXPECT_THAT(sql, HasSubstr("INSERT INTO `pluginConfigurationParameter`"));
            EXPECT_THAT(sql, Not(HasSubstr("DELETE FROM")));
            return true;
        });
    
    configContext->testInitializePluginConfigParameters(pluginId, entries);
}

// Test Case: Edge Case - Single Entry
TEST_F(ConfigContextTest, SingleEntry) {
    unsigned int pluginId = 3000;
    auto entries = createTestEntries(1);
    
    std::vector<std::string> capturedSql;
    
    EXPECT_CALL(*mockStatement, execute(_))
        .Times(2) // 1 INSERT + 1 DELETE
        .WillRepeatedly([&capturedSql](const std::string& sql) {
            capturedSql.push_back(sql);
            return true;
        });
    
    configContext->testInitializePluginConfigParameters(pluginId, entries);
    
    // Verify both INSERT and DELETE were called
    ASSERT_EQ(capturedSql.size(), 2);
    EXPECT_THAT(capturedSql[0], HasSubstr("INSERT INTO"));
    EXPECT_THAT(capturedSql[1], HasSubstr("DELETE FROM"));
}