#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <functional>
#include "PluginClient.h"

namespace tmx {
namespace utils {
namespace test {

/**
 * MockPluginClient provides a testable mock of PluginClient.
 * 
 * Usage in tests:
 * @code
 * class YourPluginTest : public ::testing::Test {
 * protected:
 *     MockPluginClient mock_plugin{"TestPlugin"};
 * };
 * @endcode
 */
class MockPluginClient : public PluginClient {
public:
    explicit MockPluginClient(const std::string &name = "MockPlugin");
    virtual ~MockPluginClient();

    // ==================== Virtual Method Mocks ====================
    
    /// Mock Main plugin loop
    MOCK_METHOD(int, Main, (), (override));
    
    /// Mock configuration option processing
    MOCK_METHOD(bool, ProcessOptions, 
                (const boost::program_options::variables_map &), (override));
    
    /// Mock configuration change handler
    MOCK_METHOD(void, OnConfigChanged, 
                (const char *key, const char *value), (override));
    
    /// Mock error handler
    MOCK_METHOD(void, OnError, (IvpError err), (override));
    
    /// Mock message received handler
    MOCK_METHOD(void, OnMessageReceived, (IvpMessage *msg), (override));
    
    /// Mock state change handler
    MOCK_METHOD(void, OnStateChange, (IvpPluginState state), (override));

    // ==================== Helper Methods for Testing ====================
    
    /// Get the plugin name
    std::string GetPluginName() { return this->GetName(); }
    
    /// Simulate configuration change (calls the virtual OnConfigChanged)
    void SimulateConfigChange(const char *key, const char *value) {
        OnConfigChanged(key, value);
    }
    
    /// Simulate state change (calls the virtual OnStateChange)
    void SimulateStateChange(IvpPluginState state) {
        OnStateChange(state);
    }
    
    /// Get last broadcast message (if message capture is enabled)
    tmx::routeable_message GetLastBroadcastMessage() const {
        if (broadcast_messages.empty()) {
            throw std::runtime_error("No messages have been broadcast");
        }
        return broadcast_messages.back();
    }
    
    /// Get all broadcast messages
    const std::vector<tmx::routeable_message>& GetAllBroadcastMessages() const {
        return broadcast_messages;
    }
    
    /// Clear the broadcast message history
    void ClearBroadcastHistory() {
        broadcast_messages.clear();
    }
    
    /// Enable automatic capturing of broadcast messages
    void EnableMessageCapture() {
        capture_enabled = true;
    }
    
    /// Disable automatic capturing of broadcast messages
    void DisableMessageCapture() {
        capture_enabled = false;
    }

protected:
    /// Override BroadcastMessage to optionally capture messages
    void BroadcastMessage(const tmx::routeable_message &routeableMsg) override {
        if (capture_enabled) {
            broadcast_messages.push_back(routeableMsg);
        }
    }

private:
    bool capture_enabled = false;
    std::vector<tmx::routeable_message> broadcast_messages;
};

/**
 * Base class for testing a plugin that inherits from PluginClient.
 * 
 * Usage:
 * @code
 * class MyPluginTest : public PluginClientTestBase<MyPlugin> {
 * protected:
 *     void SetUp() override {
 *         PluginClientTestBase::SetUp();
 *         // Additional setup
 *     }
 * };
 * @endcode
 */
template<typename PluginType>
class PluginClientTestBase : public ::testing::Test {
public:
    using PluginTestType = PluginType;

protected:
    virtual void SetUp() {
        // Create plugin instance - subclasses will override if needed
        plugin = std::make_unique<PluginType>("TestPlugin");
    }

    virtual void TearDown() {
        plugin.reset();
    }

    std::unique_ptr<PluginType> plugin;
};

/**
 * Matcher for IvpMessage by type.
 * Usage: EXPECT_CALL(..., OnMessageReceived(IvpMessageHasType("BSM")));
 */
MATCHER_P(IvpMessageHasType, expected_type, "") {
    return arg != nullptr && 
           arg->type != nullptr && 
           strcmp(arg->type, expected_type) == 0;
}

/**
 * Matcher for IvpMessage by subtype.
 * Usage: EXPECT_CALL(..., OnMessageReceived(IvpMessageHasSubType("TIM")));
 */
MATCHER_P(IvpMessageHasSubType, expected_subtype, "") {
    return arg != nullptr && 
           arg->subtype != nullptr && 
           strcmp(arg->subtype, expected_subtype) == 0;
}

/**
 * Matcher for checking both type and subtype.
 * Usage: EXPECT_CALL(..., OnMessageReceived(IvpMessageOfType("J2735", "BSM")));
 */
MATCHER_P2(IvpMessageOfType, expected_type, expected_subtype, "") {
    return arg != nullptr && 
           arg->type != nullptr && 
           arg->subtype != nullptr &&
           strcmp(arg->type, expected_type) == 0 &&
           strcmp(arg->subtype, expected_subtype) == 0;
}

} // namespace test
} // namespace utils
} // namespace tmx
