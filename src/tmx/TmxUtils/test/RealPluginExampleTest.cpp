/*
 * RealPluginExampleTest.cpp
 * 
 * This file demonstrates testing a real V2X Hub plugin (using a simplified example).
 * It shows how to:
 * 1. Test a plugin that inherits from PluginClient
 * 2. Use MockPluginClient for dependency injection
 * 3. Verify message processing and state management
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <test/MockPluginClient.h>

using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::Eq;

namespace {

/**
 * Simplified example of a real plugin that processes data
 * In practice, this would be your actual plugin code
 */
class DataProcessorPlugin : public tmx::utils::PluginClient {
public:
    explicit DataProcessorPlugin(const std::string &name = "DataProcessor")
        : PluginClient(name), 
          enabled(false), 
          data_count(0),
          last_error(ivpError_createError(IvpLogLevel_debug, IvpError_none,1)) {}

    virtual ~DataProcessorPlugin() = default;

    // Virtual method implementations
    int Main() override {
        return 0;
    }

    bool ProcessOptions(const boost::program_options::variables_map &) override {
        return true;
    }

    void OnConfigChanged(const char *key, const char *value) override {
        if (!key || !value) return;
        
        std::string key_str(key);
        std::string val_str(value);
        
        if (key_str == "Enabled") {
            enabled = (val_str == "true" || val_str == "1");
        } else if (key_str == "Mode") {
            mode = val_str;
        }
        
        config[key_str] = val_str;
    }

    void OnError(IvpError err) override {
        last_error = err;
        error_count++;
    }

    void OnMessageReceived(IvpMessage *msg) override {
        if (msg && enabled) {
            data_count++;
            last_message_type = msg->type ? std::string(msg->type) : "";
        }
    }

    void OnStateChange(IvpPluginState new_state) override {
        current_state = new_state;
    }

    // Application-specific methods to test
    bool IsEnabled() const { return enabled; }
    
    int GetProcessedDataCount() const { return data_count; }
    
    void ProcessData(const std::vector<uint8_t> &data) {
        if (!enabled || data.empty()) return;
        
        // Simulate processing
        data_count++;
        
        // In a real plugin, this might broadcast processed messages
        // BroadcastMessage(processedMsg);
    }
    
    std::string GetMode() const { return mode; }
    
    std::string GetConfigValue(const std::string &key) const {
        auto it = config.find(key);
        return it != config.end() ? it->second : "";
    }
    
    IvpPluginState GetCurrentState() const { return current_state; }
    
    IvpError GetLastError() const { return last_error; }
    
    int GetErrorCount() const { return error_count; }

private:
    bool enabled;
    int data_count;
    int error_count = 0;
    IvpPluginState current_state = IvpPluginState_connected;
    IvpError last_error;
    std::string mode;
    std::string last_message_type;
    std::map<std::string, std::string> config;
};

/**
 * Test fixture for the real plugin example
 */
class RealPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        plugin = std::make_unique<DataProcessorPlugin>("TestDataProcessor");
    }

    void TearDown() override {
        plugin.reset();
    }

    std::unique_ptr<DataProcessorPlugin> plugin;
};

// ==================== Configuration Tests ====================

/**
 * Test that the plugin correctly enables based on configuration
 */
TEST_F(RealPluginTest, EnablesBasedOnConfiguration) {
    // Arrange
    ASSERT_FALSE(plugin->IsEnabled());

    // Act
    plugin->OnConfigChanged("Enabled", "true");

    // Assert
    EXPECT_TRUE(plugin->IsEnabled());
}

/**
 * Test that the plugin disables correctly
 */
TEST_F(RealPluginTest, DisablesBasedOnConfiguration) {
    // Arrange
    plugin->OnConfigChanged("Enabled", "true");
    EXPECT_TRUE(plugin->IsEnabled());

    // Act
    plugin->OnConfigChanged("Enabled", "false");

    // Assert
    EXPECT_FALSE(plugin->IsEnabled());
}

/**
 * Test that the plugin accepts numeric boolean values
 */
TEST_F(RealPluginTest, AcceptsNumericBooleanValues) {
    // Arrange

    // Act - "1" should enable
    plugin->OnConfigChanged("Enabled", "1");
    EXPECT_TRUE(plugin->IsEnabled());

    // Act - "0" should disable
    plugin->OnConfigChanged("Enabled", "0");
    EXPECT_FALSE(plugin->IsEnabled());
}

/**
 * Test that multiple configuration values are stored
 */
TEST_F(RealPluginTest, StoresMultipleConfigValues) {
    // Arrange
    // Act
    plugin->OnConfigChanged("Enabled", "true");
    plugin->OnConfigChanged("Mode", "Advanced");
    plugin->OnConfigChanged("Threshold", "50");

    // Assert
    EXPECT_EQ("true", plugin->GetConfigValue("Enabled"));
    EXPECT_EQ("Advanced", plugin->GetConfigValue("Mode"));
    EXPECT_EQ("50", plugin->GetConfigValue("Threshold"));
}

// ==================== Data Processing Tests ====================

/**
 * Test that the plugin processes data when enabled
 */
TEST_F(RealPluginTest, ProcessesDataWhenEnabled) {
    // Arrange
    plugin->OnConfigChanged("Enabled", "true");
    std::vector<uint8_t> test_data = {0x01, 0x02, 0x03};

    // Act
    plugin->ProcessData(test_data);

    // Assert
    EXPECT_EQ(1, plugin->GetProcessedDataCount());
}

/**
 * Test that the plugin ignores data when disabled
 */
TEST_F(RealPluginTest, IgnoresDataWhenDisabled) {
    // Arrange
    plugin->OnConfigChanged("Enabled", "false");
    std::vector<uint8_t> test_data = {0x01, 0x02, 0x03};

    // Act
    plugin->ProcessData(test_data);

    // Assert
    EXPECT_EQ(0, plugin->GetProcessedDataCount());
}

/**
 * Test that the plugin ignores empty data
 */
TEST_F(RealPluginTest, IgnoresEmptyData) {
    // Arrange
    plugin->OnConfigChanged("Enabled", "true");
    std::vector<uint8_t> empty_data;

    // Act
    plugin->ProcessData(empty_data);

    // Assert
    EXPECT_EQ(0, plugin->GetProcessedDataCount());
}

/**
 * Test processing multiple data packets
 */
TEST_F(RealPluginTest, ProcessesMultipleDataPackets) {
    // Arrange
    plugin->OnConfigChanged("Enabled", "true");
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};

    // Act
    plugin->ProcessData(data);
    plugin->ProcessData(data);
    plugin->ProcessData(data);

    // Assert
    EXPECT_EQ(3, plugin->GetProcessedDataCount());
}

// ==================== Message Handling Tests ====================

/**
 * Test that the plugin processes messages when enabled
 */
TEST_F(RealPluginTest, ProcessesMessagesWhenEnabled) {
    // Arrange
    plugin->OnConfigChanged("Enabled", "true");
    IvpMessage msg{};
    msg.type = const_cast<char*>("TestType");

    // Act
    plugin->OnMessageReceived(&msg);

    // Assert
    EXPECT_EQ(1, plugin->GetProcessedDataCount());
}

/**
 * Test that the plugin ignores messages when disabled
 */
TEST_F(RealPluginTest, IgnoresMessagesWhenDisabled) {
    // Arrange
    IvpMessage msg{};

    // Act
    plugin->OnMessageReceived(&msg);

    // Assert
    EXPECT_EQ(0, plugin->GetProcessedDataCount());
}

/**
 * Test that the plugin ignores null messages
 */
TEST_F(RealPluginTest, IgnoresNullMessages) {
    // Arrange
    plugin->OnConfigChanged("Enabled", "true");

    // Act
    plugin->OnMessageReceived(nullptr);

    // Assert
    EXPECT_EQ(0, plugin->GetProcessedDataCount());
}

// ==================== State Management Tests ====================

/**
 * Test state transitions
 */
TEST_F(RealPluginTest, TransitionsStateCorrectly) {
    // Arrange
    EXPECT_EQ(IvpPluginState_connected, plugin->GetCurrentState());

    // Act
    plugin->OnStateChange(IvpPluginState_disconnected);
    EXPECT_EQ(IvpPluginState_disconnected, plugin->GetCurrentState());

    plugin->OnStateChange(IvpPluginState_error);
    EXPECT_EQ(IvpPluginState_error, plugin->GetCurrentState());

    // Assert
    plugin->OnStateChange(IvpPluginState_registered);
    EXPECT_EQ(IvpPluginState_registered, plugin->GetCurrentState());
}

// ==================== Error Handling Tests ====================

/**
 * Test error tracking
 */
TEST_F(RealPluginTest, TracksErrors) {
    // Arrange
    EXPECT_EQ(0, plugin->GetErrorCount());

    // Act
    plugin->OnError(ivpError_createError(IvpLogLevel_debug, IvpError_none,1));
    plugin->OnError(ivpError_createError(IvpLogLevel_debug, IvpError_none,1));

    // Assert
    EXPECT_EQ(2, plugin->GetErrorCount());
    auto error = plugin->GetLastError();
    EXPECT_EQ(IvpLogLevel_debug, error.level);
    EXPECT_EQ(IvpError_none, error.error);
    EXPECT_EQ(1, error.sysErrNo);
}

// ==================== Integration Tests ====================

/**
 * Test realistic plugin lifecycle
 */
TEST_F(RealPluginTest, HandlesRealisticLifecycle) {
    // Phase 1: Initialization
    plugin->OnStateChange(IvpPluginState_connected);
    EXPECT_EQ(IvpPluginState_connected, plugin->GetCurrentState());

    // Phase 2: Configuration
    plugin->OnConfigChanged("Enabled", "true");
    plugin->OnConfigChanged("Mode", "Standard");
    EXPECT_TRUE(plugin->IsEnabled());
    EXPECT_EQ("Standard", plugin->GetMode());

    // Phase 3: Startup
    plugin->OnStateChange(IvpPluginState_registered);
    EXPECT_EQ(IvpPluginState_registered, plugin->GetCurrentState());

    // Phase 4: Process messages
    IvpMessage msg1{}, msg2{}, msg3{};
    plugin->OnMessageReceived(&msg1);
    plugin->OnMessageReceived(&msg2);
    plugin->OnMessageReceived(&msg3);
    EXPECT_EQ(3, plugin->GetProcessedDataCount());

    // Phase 5: Configuration update
    plugin->OnConfigChanged("Mode", "Advanced");
    EXPECT_EQ("Advanced", plugin->GetMode());

    // Phase 6: Process more data
    std::vector<uint8_t> data = {0xFF, 0xAA};
    plugin->ProcessData(data);
    EXPECT_EQ(4, plugin->GetProcessedDataCount());

    // Phase 7: Shutdown
    plugin->OnStateChange(IvpPluginState_disconnected);
    EXPECT_EQ(IvpPluginState_disconnected, plugin->GetCurrentState());
}

/**
 * Test configuration changes during runtime
 */
TEST_F(RealPluginTest, HandlesDynamicConfigurationChanges) {
    // Arrange
    plugin->OnStateChange(IvpPluginState_connected);
    plugin->OnConfigChanged("Enabled", "true");

    // Act & Assert - Process message while enabled
    IvpMessage msg{};
    plugin->OnMessageReceived(&msg);
    EXPECT_EQ(1, plugin->GetProcessedDataCount());

    // Act & Assert - Disable and verify message is ignored
    plugin->OnConfigChanged("Enabled", "false");
    plugin->OnMessageReceived(&msg);
    EXPECT_EQ(1, plugin->GetProcessedDataCount()); // Still 1

    // Act & Assert - Re-enable and verify processing resumes
    plugin->OnConfigChanged("Enabled", "true");
    plugin->OnMessageReceived(&msg);
    EXPECT_EQ(2, plugin->GetProcessedDataCount()); // Now 2
}

/**
 * Stress test: Process many messages in sequence
 */
TEST_F(RealPluginTest, HandlesHighMessageVolume) {
    // Arrange
    plugin->OnConfigChanged("Enabled", "true");
    const int MESSAGE_COUNT = 1000;

    // Act
    for (int i = 0; i < MESSAGE_COUNT; i++) {
        IvpMessage msg{};
        plugin->OnMessageReceived(&msg);
    }

    // Assert
    EXPECT_EQ(MESSAGE_COUNT, plugin->GetProcessedDataCount());
}

/**
 * Test configuration consistency after many updates
 */
TEST_F(RealPluginTest, MaintainsConfigConsistency) {
    // Arrange
    // Act - Apply many configuration changes
    for (int i = 0; i < 100; i++) {
        plugin->OnConfigChanged("Key", std::to_string(i).c_str());
    }

    // Assert - Last value should be retained
    EXPECT_EQ("99", plugin->GetConfigValue("Key"));
}

} // namespace
