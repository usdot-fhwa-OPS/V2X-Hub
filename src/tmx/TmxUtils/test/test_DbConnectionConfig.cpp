/*
 * @file test_DbConnectionConfig.cpp
 *
 *  Created on: 2026-02-18
 *      @author: Daniel I. Kelley (Leidos)
 */

#include <gtest/gtest.h>
#include <thread>
#include <fstream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include "database/DbConnectionConfig.h"

using namespace tmx::utils;

/**
 * RAII helper class to manage environment variables during testing
 */
class EnvironmentVariableGuard {
public:
    EnvironmentVariableGuard(const std::string& varName) : varName_(varName) {
        // Save original value
        const char* originalValue = std::getenv(varName_.c_str());
        if (originalValue != nullptr) {
            originalValue_ = std::string(originalValue);
            hasOriginalValue_ = true;
        } else {
            hasOriginalValue_ = false;
        }
    }
    
    ~EnvironmentVariableGuard() {
        // Restore original value
        if (hasOriginalValue_) {
            setenv(varName_.c_str(), originalValue_.c_str(), 1);
        } else {
            unsetenv(varName_.c_str());
        }
    }
    
    void set(const std::string& value) {
        setenv(varName_.c_str(), value.c_str(), 1);
    }
    
    void unset() {
        unsetenv(varName_.c_str());
    }

private:
    std::string varName_;
    std::string originalValue_;
    bool hasOriginalValue_;
};

/**
 * RAII helper class to manage temporary files during testing
 */
class TempFileGuard {
public:
    TempFileGuard(const std::string& content = "") {
        // Create unique temporary file name
        char tempTemplate[] = "/tmp/dbconfig_test_XXXXXX";
        int fd = mkstemp(tempTemplate);
        if (fd == -1) {
            throw std::runtime_error("Failed to create temporary file");
        }
        
        filePath_ = std::string(tempTemplate);
        
        // Write content if provided
        if (!content.empty()) {
            write(fd, content.c_str(), content.length());
        }
        
        close(fd);
    }
    
    ~TempFileGuard() {
        // Clean up temporary file
        if (!filePath_.empty()) {
            unlink(filePath_.c_str());
        }
    }
    
    const std::string& getPath() const {
        return filePath_;
    }
    
    void writeContent(const std::string& content) {
        std::ofstream file(filePath_);
        file << content;
        file.close();
    }

private:
    std::string filePath_;
};

/**
 * Test fixture for DbConnectionConfig tests
 */
class DbConnectionConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create environment variable guards for all MySQL-related variables
        hostGuard_ = std::make_unique<EnvironmentVariableGuard>("MYSQL_HOST");
        portGuard_ = std::make_unique<EnvironmentVariableGuard>("MYSQL_PORT");
        databaseGuard_ = std::make_unique<EnvironmentVariableGuard>("MYSQL_DATABASE");
        userGuard_ = std::make_unique<EnvironmentVariableGuard>("MYSQL_USER");
        passwordGuard_ = std::make_unique<EnvironmentVariableGuard>("MYSQL_PASSWORD");
        
        // Clear all environment variables for clean test state
        hostGuard_->unset();
        portGuard_->unset();
        databaseGuard_->unset();
        userGuard_->unset();
        passwordGuard_->unset();
    }
    
    void TearDown() override {
        // Environment variables will be restored by guard destructors
    }

protected:
    std::unique_ptr<EnvironmentVariableGuard> hostGuard_;
    std::unique_ptr<EnvironmentVariableGuard> portGuard_;
    std::unique_ptr<EnvironmentVariableGuard> databaseGuard_;
    std::unique_ptr<EnvironmentVariableGuard> userGuard_;
    std::unique_ptr<EnvironmentVariableGuard> passwordGuard_;
};

/**
 * Test that getInstance() returns the same singleton instance
 */
TEST_F(DbConnectionConfigTest, SingletonInstance) {
    DbConnectionConfig& instance1 = DbConnectionConfig::getInstance();
    DbConnectionConfig& instance2 = DbConnectionConfig::getInstance();
    
    // Should be the same instance
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * Test default values when no environment variables are set
 */
TEST_F(DbConnectionConfigTest, DefaultValues) {
    // Reload to ensure clean state
    DbConnectionConfig::getInstance().reloadConfiguration();
    
    DbConnectionConfig& config = DbConnectionConfig::getInstance();
    
    EXPECT_EQ("127.0.0.1", config.getHost());
    EXPECT_EQ("3306", config.getPort());
    EXPECT_EQ("IVP", config.getDatabase());
    EXPECT_EQ("IVP", config.getUser());
    EXPECT_EQ("tcp://127.0.0.1:3306", config.getConnectionUrl());
}

/**
 * Test custom values when environment variables are set
 */
TEST_F(DbConnectionConfigTest, CustomEnvironmentValues) {
    // Set custom environment variables
    hostGuard_->set("custom-host.example.com");
    portGuard_->set("5432");
    databaseGuard_->set("CustomDB");
    userGuard_->set("CustomUser");
    
    // Reload configuration to pick up new values
    DbConnectionConfig::getInstance().reloadConfiguration();
    
    DbConnectionConfig& config = DbConnectionConfig::getInstance();
    
    EXPECT_EQ("custom-host.example.com", config.getHost());
    EXPECT_EQ("5432", config.getPort());
    EXPECT_EQ("CustomDB", config.getDatabase());
    EXPECT_EQ("CustomUser", config.getUser());
    EXPECT_EQ("tcp://custom-host.example.com:5432", config.getConnectionUrl());
}

/**
 * Test connection URL format
 */
TEST_F(DbConnectionConfigTest, ConnectionUrlFormat) {
    hostGuard_->set("192.168.1.100");
    portGuard_->set("3307");
    
    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();
    
    std::string expectedUrl = "tcp://192.168.1.100:3307";
    EXPECT_EQ(expectedUrl, config.getConnectionUrl());
}

/**
 * Test password handling when MYSQL_PASSWORD environment variable is not set
 */
TEST_F(DbConnectionConfigTest, PasswordFileNotSet) {
    // Ensure MYSQL_PASSWORD is not set
    passwordGuard_->unset();
    
    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();
    
    // Should return empty string when password file is not configured
    EXPECT_EQ("", config.getPassword());
}

/**
 * Test password handling when password file does not exist
 */
TEST_F(DbConnectionConfigTest, PasswordFileDoesNotExist) {
    // Set MYSQL_PASSWORD to a non-existent file
    passwordGuard_->set("/tmp/non_existent_password_file_12345");
    
    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();
    
    // Should return empty string when file doesn't exist
    EXPECT_EQ("", config.getPassword());
}

/**
 * Test password handling when password file is empty
 */
TEST_F(DbConnectionConfigTest, PasswordFileEmpty) {
    // Create empty temporary file
    TempFileGuard tempFile("");
    passwordGuard_->set(tempFile.getPath());
    
    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();
    
    // Should return empty string for empty file
    EXPECT_EQ("", config.getPassword());
}

/**
 * Test successful password file reading
 */
TEST_F(DbConnectionConfigTest, PasswordFileValid) {
    // Create temporary file with password
    std::string expectedPassword = "test_password_123";
    TempFileGuard tempFile(expectedPassword);
    passwordGuard_->set(tempFile.getPath());
    
    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();
    
    EXPECT_EQ(expectedPassword, config.getPassword());
}

/**
 * Test password file with multiple lines (should only read first line)
 */
TEST_F(DbConnectionConfigTest, PasswordFileMultipleLines) {
    // Create temporary file with multiple lines
    std::string fileContent = "first_line_password\nsecond_line\nthird_line";
    TempFileGuard tempFile(fileContent);
    passwordGuard_->set(tempFile.getPath());
    
    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();
    
    // Should only return the first line
    EXPECT_EQ("first_line_password", config.getPassword());
}

/**
 * Test that reloadConfiguration picks up changed environment variables
 */
TEST_F(DbConnectionConfigTest, ReloadConfiguration) {
    // Set initial values
    hostGuard_->set("initial-host");
    portGuard_->set("1111");
    
    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();
    
    // Verify initial values
    EXPECT_EQ("initial-host", config.getHost());
    EXPECT_EQ("1111", config.getPort());
    
    // Change environment variables
    hostGuard_->set("updated-host");
    portGuard_->set("2222");
    
    // Reload configuration
    config.reloadConfiguration();
    
    // Verify updated values
    EXPECT_EQ("updated-host", config.getHost());
    EXPECT_EQ("2222", config.getPort());
    EXPECT_EQ("tcp://updated-host:2222", config.getConnectionUrl());
}

/**
 * Test thread safety of concurrent access to getters
 */
TEST_F(DbConnectionConfigTest, ThreadSafetyConcurrentAccess) {
    // Set up configuration
    hostGuard_->set("thread-test-host");
    portGuard_->set("9999");
    databaseGuard_->set("ThreadTestDB");
    userGuard_->set("ThreadTestUser");
    
    DbConnectionConfig::getInstance().reloadConfiguration();
    
    const int numThreads = 10;
    const int numIterations = 100;
    std::vector<std::thread> threads;
    std::vector<bool> results(numThreads, true);
    
    // Launch multiple threads that concurrently access configuration
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, i]() {
            DbConnectionConfig& config = DbConnectionConfig::getInstance();
            
            for (int j = 0; j < numIterations; ++j) {
                // All threads should get consistent values
                if (config.getHost() != "thread-test-host" ||
                    config.getPort() != "9999" ||
                    config.getDatabase() != "ThreadTestDB" ||
                    config.getUser() != "ThreadTestUser" ||
                    config.getConnectionUrl() != "tcp://thread-test-host:9999") {
                    results[i] = false;
                    break;
                }
                
                // Small delay to increase chance of race conditions
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all threads got consistent results
    for (int i = 0; i < numThreads; ++i) {
        EXPECT_TRUE(results[i]) << "Thread " << i << " got inconsistent results";
    }
}

/**
 * Test thread safety of concurrent reload operations
 */
TEST_F(DbConnectionConfigTest, ThreadSafetyConcurrentReload) {
    const int numThreads = 5;
    std::vector<std::thread> threads;
    
    // Set initial configuration
    hostGuard_->set("reload-test-host");
    portGuard_->set("8888");
    
    // Launch multiple threads that concurrently reload configuration
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&]() {
            DbConnectionConfig& config = DbConnectionConfig::getInstance();
            
            for (int j = 0; j < 50; ++j) {
                config.reloadConfiguration();
                
                // Verify we can still get values (they should be consistent)
                std::string host = config.getHost();
                std::string port = config.getPort();
                std::string url = config.getConnectionUrl();
                
                // Values should be valid (not empty) and consistent
                EXPECT_FALSE(host.empty());
                EXPECT_FALSE(port.empty());
                EXPECT_FALSE(url.empty());
                EXPECT_EQ("tcp://" + host + ":" + port, url);
                
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
}

/**
 * Test handling of empty environment variables (should use defaults)
 */
TEST_F(DbConnectionConfigTest, EmptyEnvironmentVariables) {
    // Set environment variables to empty strings
    hostGuard_->set("");
    portGuard_->set("");
    databaseGuard_->set("");
    userGuard_->set("");
    
    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();
    
    // Should fall back to default values
    EXPECT_EQ("127.0.0.1", config.getHost());
    EXPECT_EQ("3306", config.getPort());
    EXPECT_EQ("IVP", config.getDatabase());
    EXPECT_EQ("IVP", config.getUser());
}

/**
 * Test singleton behavior across multiple threads
 */
TEST_F(DbConnectionConfigTest, SingletonThreadSafety) {
    const int numThreads = 10;
    std::vector<std::thread> threads;
    std::vector<DbConnectionConfig*> instances(numThreads);
    
    // Launch multiple threads that get the singleton instance
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, i]() {
            instances[i] = &DbConnectionConfig::getInstance();
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all threads got the same instance
    for (int i = 1; i < numThreads; ++i) {
        EXPECT_EQ(instances[0], instances[i]) << "Thread " << i << " got different singleton instance";
    }
}

/**
 * Test environment variables with special characters
 */
TEST_F(DbConnectionConfigTest, SpecialCharactersInEnvironmentVariables) {
    // Test with special characters that might appear in real configurations
    hostGuard_->set("db-server.example.com");
    portGuard_->set("3306");
    databaseGuard_->set("test_db-2024");
    userGuard_->set("user_name-123");

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    EXPECT_EQ("db-server.example.com", config.getHost());
    EXPECT_EQ("3306", config.getPort());
    EXPECT_EQ("test_db-2024", config.getDatabase());
    EXPECT_EQ("user_name-123", config.getUser());
    EXPECT_EQ("tcp://db-server.example.com:3306", config.getConnectionUrl());
}

/**
 * Test environment variables with Unicode characters
 */
TEST_F(DbConnectionConfigTest, UnicodeCharactersInEnvironmentVariables) {
    // Test with Unicode characters
    hostGuard_->set("测试服务器.example.com");
    databaseGuard_->set("数据库");
    userGuard_->set("用户名");

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    EXPECT_EQ("测试服务器.example.com", config.getHost());
    EXPECT_EQ("数据库", config.getDatabase());
    EXPECT_EQ("用户名", config.getUser());
}

/**
 * Test very long environment variable values
 */
TEST_F(DbConnectionConfigTest, VeryLongEnvironmentVariables) {
    // Create very long strings to test boundary conditions
    std::string longHost(1000, 'a');
    longHost += ".example.com";
    std::string longDatabase(500, 'b');
    std::string longUser(300, 'c');

    hostGuard_->set(longHost);
    databaseGuard_->set(longDatabase);
    userGuard_->set(longUser);

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    EXPECT_EQ(longHost, config.getHost());
    EXPECT_EQ(longDatabase, config.getDatabase());
    EXPECT_EQ(longUser, config.getUser());

    // Verify connection URL is properly formatted even with long host
    std::string expectedUrl = "tcp://" + longHost + ":3306";
    EXPECT_EQ(expectedUrl, config.getConnectionUrl());
}

/**
 * Test environment variables with whitespace
 */
TEST_F(DbConnectionConfigTest, WhitespaceInEnvironmentVariables) {
    // Test with leading/trailing whitespace (should be preserved)
    hostGuard_->set("  spaced-host.com  ");
    portGuard_->set("  3307  ");
    databaseGuard_->set("  SpacedDB  ");
    userGuard_->set("  SpacedUser  ");

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    // Environment variables should preserve whitespace as-is
    EXPECT_EQ("  spaced-host.com  ", config.getHost());
    EXPECT_EQ("  3307  ", config.getPort());
    EXPECT_EQ("  SpacedDB  ", config.getDatabase());
    EXPECT_EQ("  SpacedUser  ", config.getUser());
}

/**
 * Test password file with whitespace content
 */
TEST_F(DbConnectionConfigTest, PasswordFileWithWhitespace) {
    // Test password with leading/trailing whitespace
    std::string passwordWithSpaces = "  password_with_spaces  ";
    TempFileGuard tempFile(passwordWithSpaces);
    passwordGuard_->set(tempFile.getPath());

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    // Should preserve whitespace in password
    EXPECT_EQ(passwordWithSpaces, config.getPassword());
}

/**
 * Test password file with only whitespace
 */
TEST_F(DbConnectionConfigTest, PasswordFileOnlyWhitespace) {
    // Test file containing only whitespace
    TempFileGuard tempFile("   \t\n   ");
    passwordGuard_->set(tempFile.getPath());

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    // Should return the whitespace as-is (first line)
    EXPECT_EQ("   \t", config.getPassword());
}

/**
 * Test password file with special characters
 */
TEST_F(DbConnectionConfigTest, PasswordFileSpecialCharacters) {
    // Test password with special characters
    std::string specialPassword = "p@ssw0rd!#$%^&*()_+-={}[]|\\:;\"'<>?,./~`";
    TempFileGuard tempFile(specialPassword);
    passwordGuard_->set(tempFile.getPath());

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    EXPECT_EQ(specialPassword, config.getPassword());
}

/**
 * Test password file with Unicode characters
 */
TEST_F(DbConnectionConfigTest, PasswordFileUnicodeCharacters) {
    // Test password with Unicode characters
    std::string unicodePassword = "密码123пароль";
    TempFileGuard tempFile(unicodePassword);
    passwordGuard_->set(tempFile.getPath());

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    EXPECT_EQ(unicodePassword, config.getPassword());
}

/**
 * Test very long password
 */
TEST_F(DbConnectionConfigTest, VeryLongPassword) {
    // Test with very long password
    std::string longPassword(10000, 'x');
    TempFileGuard tempFile(longPassword);
    passwordGuard_->set(tempFile.getPath());

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    EXPECT_EQ(longPassword, config.getPassword());
}

/**
 * Test password file with no read permissions
 */
TEST_F(DbConnectionConfigTest, PasswordFileNoReadPermissions) {
    // Create temporary file with content
    TempFileGuard tempFile("secret_password");

    // Remove read permissions
    chmod(tempFile.getPath().c_str(), 0000);

    passwordGuard_->set(tempFile.getPath());

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    // Should return empty string when file is not readable
    EXPECT_EQ("", config.getPassword());

    // Restore permissions for cleanup
    chmod(tempFile.getPath().c_str(), 0644);
}

/**
 * Test concurrent password file reading
 */
TEST_F(DbConnectionConfigTest, ConcurrentPasswordFileReading) {
    // Create password file
    TempFileGuard tempFile("concurrent_password");
    passwordGuard_->set(tempFile.getPath());

    DbConnectionConfig::getInstance().reloadConfiguration();

    const int numThreads = 10;
    const int numIterations = 50;
    std::vector<std::thread> threads;
    std::vector<std::vector<std::string>> results(numThreads);

    // Launch multiple threads that concurrently read password
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, i]() {
            DbConnectionConfig& config = DbConnectionConfig::getInstance();
            results[i].reserve(numIterations);

            for (int j = 0; j < numIterations; ++j) {
                results[i].push_back(config.getPassword());
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify all threads got consistent results
    for (int i = 0; i < numThreads; ++i) {
        for (int j = 0; j < numIterations; ++j) {
            EXPECT_EQ("concurrent_password", results[i][j])
                << "Thread " << i << " iteration " << j << " got inconsistent password";
        }
    }
}

/**
 * Test password file modification during reading
 */
TEST_F(DbConnectionConfigTest, PasswordFileModificationDuringReading) {
    // Create initial password file
    TempFileGuard tempFile("initial_password");
    passwordGuard_->set(tempFile.getPath());

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    // Verify initial password
    EXPECT_EQ("initial_password", config.getPassword());

    // Modify the file content
    tempFile.writeContent("modified_password");

    // Reading should still work (may get old or new content depending on timing)
    std::string password = config.getPassword();
    EXPECT_TRUE(password == "initial_password" || password == "modified_password");

    // After reload, should get new content
    config.reloadConfiguration();
    EXPECT_EQ("modified_password", config.getPassword());
}

/**
 * Test connection URL with IPv6 addresses
 */
TEST_F(DbConnectionConfigTest, ConnectionUrlIPv6) {
    // Test with IPv6 address
    hostGuard_->set("::1");
    portGuard_->set("3306");

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    EXPECT_EQ("tcp://::1:3306", config.getConnectionUrl());
}

/**
 * Test connection URL with port edge cases
 */
TEST_F(DbConnectionConfigTest, ConnectionUrlPortEdgeCases) {
    // Test with minimum port number
    hostGuard_->set("localhost");
    portGuard_->set("1");

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    EXPECT_EQ("tcp://localhost:1", config.getConnectionUrl());

    // Test with maximum port number
    portGuard_->set("65535");
    config.reloadConfiguration();

    EXPECT_EQ("tcp://localhost:65535", config.getConnectionUrl());
}

/**
 * Test multiple rapid reloads
 */
TEST_F(DbConnectionConfigTest, MultipleRapidReloads) {
    hostGuard_->set("rapid-reload-host");
    portGuard_->set("7777");

    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    // Perform many rapid reloads
    for (int i = 0; i < 100; ++i) {
        config.reloadConfiguration();

        // Verify configuration is still consistent
        EXPECT_EQ("rapid-reload-host", config.getHost());
        EXPECT_EQ("7777", config.getPort());
        EXPECT_EQ("tcp://rapid-reload-host:7777", config.getConnectionUrl());
    }
}

/**
 * Test configuration consistency during concurrent operations
 */
TEST_F(DbConnectionConfigTest, ConfigurationConsistencyDuringConcurrentOperations) {
    hostGuard_->set("consistency-test-host");
    portGuard_->set("8888");
    databaseGuard_->set("ConsistencyDB");
    userGuard_->set("ConsistencyUser");

    DbConnectionConfig::getInstance().reloadConfiguration();

    const int numThreads = 8;
    const int numIterations = 200;
    std::vector<std::thread> threads;
    std::atomic<bool> allConsistent{true};

    // Launch threads that perform mixed operations
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, i]() {
            DbConnectionConfig& config = DbConnectionConfig::getInstance();

            for (int j = 0; j < numIterations; ++j) {
                // Get all values in one "transaction"
                std::string host = config.getHost();
                std::string port = config.getPort();
                std::string database = config.getDatabase();
                std::string user = config.getUser();
                std::string url = config.getConnectionUrl();

                // Verify internal consistency
                std::string expectedUrl = "tcp://" + host + ":" + port;
                if (url != expectedUrl) {
                    allConsistent = false;
                    break;
                }

                // Verify values are from the expected set
                if (host != "consistency-test-host" ||
                    port != "8888" ||
                    database != "ConsistencyDB" ||
                    user != "ConsistencyUser") {
                    allConsistent = false;
                    break;
                }

                // Occasionally trigger reload from some threads
                if (i % 2 == 0 && j % 50 == 0) {
                    config.reloadConfiguration();
                }

                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_TRUE(allConsistent.load()) << "Configuration was inconsistent during concurrent operations";
}

/**
 * Test default constants accessibility
 */
TEST_F(DbConnectionConfigTest, DefaultConstants) {
    // Test that default constants are accessible and have expected values
    EXPECT_EQ("127.0.0.1", DbConnectionConfig::DEFAULT_HOST);
    EXPECT_EQ("3306", DbConnectionConfig::DEFAULT_PORT);
    EXPECT_EQ("IVP", DbConnectionConfig::DEFAULT_DATABASE);
    EXPECT_EQ("IVP", DbConnectionConfig::DEFAULT_USER);
}

/**
 * Test environment variable precedence over defaults
 */
TEST_F(DbConnectionConfigTest, EnvironmentVariablePrecedence) {
    // Set some variables but not others
    hostGuard_->set("custom-host");
    // Leave port unset to test default
    databaseGuard_->set("CustomDB");
    // Leave user unset to test default

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    // Should use custom values where set, defaults where not set
    EXPECT_EQ("custom-host", config.getHost());
    EXPECT_EQ("3306", config.getPort()); // Default
    EXPECT_EQ("CustomDB", config.getDatabase());
    EXPECT_EQ("IVP", config.getUser()); // Default
}

/**
 * Test password file with binary content
 */
TEST_F(DbConnectionConfigTest, PasswordFileBinaryContent) {
    // Create file with binary content (including null bytes)
    std::string binaryPassword = "password\0with\0nulls";
    TempFileGuard tempFile;

    // Write binary content directly
    std::ofstream file(tempFile.getPath(), std::ios::binary);
    file.write(binaryPassword.c_str(), binaryPassword.length());
    file.close();

    passwordGuard_->set(tempFile.getPath());

    DbConnectionConfig::getInstance().reloadConfiguration();
    DbConnectionConfig& config = DbConnectionConfig::getInstance();

    // Should read until first null byte or newline
    std::string result = config.getPassword();
    EXPECT_EQ("password", result);
}

/**
 * Test repeated getInstance calls performance
 */
TEST_F(DbConnectionConfigTest, RepeatedGetInstancePerformance) {
    // This test ensures getInstance() is efficient for repeated calls
    const int numCalls = 10000;
    auto start = std::chrono::high_resolution_clock::now();

    DbConnectionConfig* lastInstance = nullptr;
    for (int i = 0; i < numCalls; ++i) {
        DbConnectionConfig& instance = DbConnectionConfig::getInstance();
        lastInstance = &instance;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Should complete quickly (less than 100ms for 10k calls)
    EXPECT_LT(duration.count(), 100000);
    EXPECT_NE(nullptr, lastInstance);
}